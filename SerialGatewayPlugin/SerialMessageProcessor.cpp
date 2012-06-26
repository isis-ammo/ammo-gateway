#include "SerialMessageProcessor.h"
#include "SerialServiceHandler.h"

#include "log.h"
#include <stdint.h>
#include <iostream>
#include <sstream>

#define ntohll(x) ( ( (int64_t)  (ntohl((int)((x << 32) >> 32))) << 32) | (uint32_t)ntohl(((int)(x >> 32)))  ) //By Runner
#define htonll(x) ntohll(x)      


using namespace ammo::gateway;

const char DEFAULT_PRIORITY = 50;

SerialMessageProcessor::SerialMessageProcessor(SerialServiceHandler *serviceHandler) :
closed(false),
closeMutex(),
newMessageMutex(),
newMessageAvailable(newMessageMutex),
commsHandler(serviceHandler),
gatewayConnector(NULL),
deviceId(""),
deviceIdAuthenticated(false)
{
  //need to initialize GatewayConnector in the main thread; the constructor always
  //happens in the main thread
  gatewayConnector = new GatewayConnector(this);
}

SerialMessageProcessor::~SerialMessageProcessor() {
  LOG_TRACE((long) commsHandler << " In ~SerialMessageProcessor()");
  if(gatewayConnector) {
    delete gatewayConnector;
  }
}

int SerialMessageProcessor::open(void *args) {
  closed = false;
  return 0;
}

int SerialMessageProcessor::close(unsigned long flags) {
  LOG_TRACE((long) commsHandler << " Closing MessageProcessor (in SerialMessageProcessor.close())");
  closeMutex.acquire();
  closed = true;
  closeMutex.release();
  
  signalNewMessageAvailable();
  return 0;
}

bool SerialMessageProcessor::isClosed() {
  volatile bool ret; //does this need to be volatile to keep the compiler from optimizing it out?

  closeMutex.acquire();
  ret = closed;
  closeMutex.release();
  
  return ret;
}

int SerialMessageProcessor::svc() {
  while(!isClosed()) {    
    ammo::protocol::MessageWrapper *msg = NULL;

    do {
      msg = commsHandler->getNextReceivedMessage();
      if(msg) {
        processMessage(*msg);
        delete msg;
      }
    } while (msg != NULL);
    if(!isClosed()) {
      newMessageMutex.acquire();
      newMessageAvailable.wait();
      newMessageMutex.release();
    }
  }
  return 0;
}

void SerialMessageProcessor::signalNewMessageAvailable() {
  newMessageMutex.acquire();
  newMessageAvailable.signal();
  newMessageMutex.release();
}

void SerialMessageProcessor::processMessage(ammo::protocol::MessageWrapper &msg) {
  LOG_TRACE((long) commsHandler << " Message Received: " << msg.DebugString());
  
  if(msg.type() == ammo::protocol::MessageWrapper_MessageType_AUTHENTICATION_MESSAGE) {
    LOG_DEBUG((long) commsHandler << " Received Authentication Message...");
    if(gatewayConnector != NULL) {
      ammo::protocol::AuthenticationMessage authMessage = msg.authentication_message();
      gatewayConnector->associateDevice(authMessage.device_id(), authMessage.user_id(), authMessage.user_key());
      this->deviceId = authMessage.device_id();
    }
  } else if(msg.type() == ammo::protocol::MessageWrapper_MessageType_DATA_MESSAGE) {
    LOG_DEBUG((long) commsHandler << " Received Data Message...");
    if(gatewayConnector != NULL) {
      ammo::protocol::DataMessage dataMessage = msg.data_message();
      MessageScope scope;
      if(dataMessage.scope() == ammo::protocol::LOCAL) {
        scope = SCOPE_LOCAL;
      } else {
        scope = SCOPE_GLOBAL;
      }
      
      PushData pushData;
      pushData.uri = dataMessage.uri();
      pushData.mimeType = dataMessage.mime_type();
      pushData.data = dataMessage.data();
      pushData.scope = scope;
      pushData.encoding = dataMessage.encoding();
      gatewayConnector->pushData(pushData);
      ammo::protocol::MessageWrapper *ackMsg = new ammo::protocol::MessageWrapper();
      ammo::protocol::PushAcknowledgement *ack = ackMsg->mutable_push_acknowledgement();
      ack->set_uri(dataMessage.uri());
      ackMsg->set_type(ammo::protocol::MessageWrapper_MessageType_PUSH_ACKNOWLEDGEMENT);
      LOG_DEBUG(commsHandler << " Sending push acknowledgment to connected device...");
      ackMsg->set_message_priority(DEFAULT_PRIORITY);
      commsHandler->sendMessage(ackMsg, DEFAULT_PRIORITY);
      
    }
  } else if(msg.type() == ammo::protocol::MessageWrapper_MessageType_TERSE_MESSAGE) {
    LOG_DEBUG((long) commsHandler << " Received Terse Message...");
    if(gatewayConnector != NULL) {
      ammo::protocol::TerseMessage dataMessage = msg.terse_message();
      MessageScope scope = SCOPE_GLOBAL;
      
      PushData pushData;
std::string originUser;
      switch( dataMessage.mime_type() ) {
      case 1:			// SMS - not implemented
          return;
      case 2:			// PLI
	pushData.mimeType = "ammo/com.aterrasys.nevada.locations";
	pushData.data = parseTerseData(2, dataMessage.data().c_str(), originUser );
	pushData.uri = "serial-pli";
        pushData.originUsername = originUser;
        break;
      case 5:			// New Transapps PLI
	pushData.mimeType = "ammo/transapps.pli.locations";
	pushData.data = parseTerseData(5, dataMessage.data().c_str(), originUser );
	pushData.uri = "serial-pli";
        pushData.originUsername = originUser;
        break;
      case 3:			// Dash
        pushData.mimeType = "ammo/edu.vu.isis.ammo.dash.event";
        pushData.data = parseTerseData(3, dataMessage.data().c_str(), originUser );
        pushData.uri = "serial-dash-event";
        pushData.originUsername = originUser;
        break;
      case 4:			// Dash
        pushData.mimeType = "ammo/transapps.chat.message_groupAll";
        pushData.data = parseTerseData(4, dataMessage.data().c_str(), originUser );
        pushData.uri = "serial-chat";
        pushData.originUsername = originUser;
        break;
      }

      pushData.scope = scope;
      gatewayConnector->pushData(pushData);

    }
  } else if(msg.type() == ammo::protocol::MessageWrapper_MessageType_SUBSCRIBE_MESSAGE) {
    LOG_DEBUG((long) commsHandler << " Received Subscribe Message...");
    MessageScope scope;
    if(msg.subscribe_message().scope() == ammo::protocol::LOCAL) {
      scope = SCOPE_LOCAL;
    } else {
      scope = SCOPE_GLOBAL;
    }
    
    if(gatewayConnector != NULL) {
      ammo::protocol::SubscribeMessage subscribeMessage = msg.subscribe_message();
      
      gatewayConnector->registerDataInterest(subscribeMessage.mime_type(), this, scope);
    }
  } else if(msg.type() == ammo::protocol::MessageWrapper_MessageType_UNSUBSCRIBE_MESSAGE) {
    LOG_DEBUG((long) commsHandler << " Received Unubscribe Message...");
    MessageScope scope;
    if(msg.unsubscribe_message().scope() == ammo::protocol::LOCAL) {
      scope = SCOPE_LOCAL;
    } else {
      scope = SCOPE_GLOBAL;
    }
    
    if(gatewayConnector != NULL) {
      ammo::protocol::UnsubscribeMessage unsubscribeMessage = msg.unsubscribe_message();
      
      gatewayConnector->unregisterDataInterest(unsubscribeMessage.mime_type(), scope);
    }
  } else if(msg.type() == ammo::protocol::MessageWrapper_MessageType_PULL_REQUEST) {
    LOG_DEBUG((long) commsHandler << " Received Pull Request Message...");
    if(gatewayConnector != NULL && deviceIdAuthenticated) {
      ammo::protocol::PullRequest pullRequest = msg.pull_request();
      // register for pull response - 
      gatewayConnector->registerPullResponseInterest(pullRequest.mime_type(), this);
      // now send request
      PullRequest req;
      req.requestUid = pullRequest.request_uid();
      req.pluginId = this->deviceId;
      req.mimeType = pullRequest.mime_type();
      req.query = pullRequest.query();
      req.projection = pullRequest.projection();
      req.maxResults = pullRequest.max_results();
      req.startFromCount = pullRequest.start_from_count();
      req.liveQuery = pullRequest.live_query();
      gatewayConnector->pullRequest(req);
    } else {
      if(!deviceIdAuthenticated) {
        LOG_ERROR((long) commsHandler << " Attempted to send a pull request before authentication.");
      }
    }
  } else if(msg.type() == ammo::protocol::MessageWrapper_MessageType_HEARTBEAT) {
    LOG_DEBUG((long) commsHandler << " Received Heartbeat from device...");
    ammo::protocol::Heartbeat heartbeat = msg.heartbeat();
    
    ammo::protocol::MessageWrapper *heartbeatAck = new ammo::protocol::MessageWrapper();
    ammo::protocol::Heartbeat *ack = heartbeatAck->mutable_heartbeat();
    ack->set_sequence_number(heartbeat.sequence_number());
    heartbeatAck->set_type(ammo::protocol::MessageWrapper_MessageType_HEARTBEAT);
    heartbeatAck->set_message_priority(DEFAULT_PRIORITY);
    
    LOG_DEBUG((long) commsHandler << " Sending heartbeat acknowledgement to connected device...");
    commsHandler->sendMessage(heartbeatAck, DEFAULT_PRIORITY);
  }
}

void SerialMessageProcessor::onConnect(GatewayConnector *sender) {
}

void SerialMessageProcessor::onDisconnect(GatewayConnector *sender) {
  
}

void SerialMessageProcessor::onPushDataReceived(GatewayConnector *sender, ammo::gateway::PushData &pushData) {
  LOG_DEBUG((long) commsHandler << " Sending subscribed data to device...");
  LOG_DEBUG((long) commsHandler << "    " << pushData);
  
  std::string dataString(pushData.data.begin(), pushData.data.end());
  ammo::protocol::MessageWrapper *msg = new ammo::protocol::MessageWrapper;
  ammo::protocol::DataMessage *dataMsg = msg->mutable_data_message();
  dataMsg->set_uri(pushData.uri);
  dataMsg->set_mime_type(pushData.mimeType);
  dataMsg->set_encoding(pushData.encoding);
  dataMsg->set_data(dataString);
  
  msg->set_type(ammo::protocol::MessageWrapper_MessageType_DATA_MESSAGE);
  msg->set_message_priority(DEFAULT_PRIORITY);
  
  LOG_DEBUG((long) commsHandler << " Sending Data Push message to connected device");
  commsHandler->sendMessage(msg, DEFAULT_PRIORITY);
}

void SerialMessageProcessor::onPullResponseReceived(GatewayConnector *sender, ammo::gateway::PullResponse &response) {
  LOG_DEBUG((long) commsHandler << " Sending pull response to device...");
  LOG_DEBUG((long) commsHandler << "    URI: " << response.uri << ", Type: " << response.mimeType);
  
  std::string dataString(response.data.begin(), response.data.end());
  ammo::protocol::MessageWrapper *msg = new ammo::protocol::MessageWrapper();
  ammo::protocol::PullResponse *pullMsg = msg->mutable_pull_response();

  pullMsg->set_request_uid(response.requestUid);
  pullMsg->set_plugin_id(response.pluginId);
  pullMsg->set_mime_type(response.mimeType);
  pullMsg->set_uri(response.uri);
  pullMsg->set_encoding(response.encoding);
  pullMsg->set_data(dataString);
  
  msg->set_type(ammo::protocol::MessageWrapper_MessageType_PULL_RESPONSE);
  msg->set_message_priority(DEFAULT_PRIORITY);
  
  LOG_DEBUG((long) commsHandler << " Sending Pull Response message to connected device");
  commsHandler->sendMessage(msg, DEFAULT_PRIORITY);
}



void SerialMessageProcessor::onAuthenticationResponse(GatewayConnector *sender, bool result) {
  LOG_DEBUG((long) commsHandler << " Delegate: onAuthenticationResponse");
  if(result == true) {
    deviceIdAuthenticated = true;
  }
  
  ammo::protocol::MessageWrapper *newMsg = new ammo::protocol::MessageWrapper();
  newMsg->set_type(ammo::protocol::MessageWrapper_MessageType_AUTHENTICATION_RESULT);
  newMsg->set_message_priority(DEFAULT_PRIORITY);
  newMsg->mutable_authentication_result()->set_result(result ? ammo::protocol::AuthenticationResult_Status_SUCCESS : ammo::protocol::AuthenticationResult_Status_SUCCESS);
  commsHandler->sendMessage(newMsg, DEFAULT_PRIORITY);
}

std::string SerialMessageProcessor::extractString(const char *terse, int& cursor)
{
  uint16_t nlen = ntohs ( *(uint16_t *)&(terse[cursor]) );
  if(nlen > 0) {
    cursor += 2;
    std::ostringstream oname;
    for (uint16_t i=0; i<nlen; i++) {
      oname << terse[cursor]; cursor += 1;
    }
    return oname.str();
  } else {
    //fall back to old-style string encoding (four-byte length, UTF-16 string) if
    //size is 0.  Hopefully, we don't receive anything legitimate with a length
    //of 0 (PLI and Chat don't seem to, but this is something to keep an eye out
    //for).
    LOG_WARN("Falling back to old string encoding");
    return extractOldStyleString(terse, cursor);
  }
}

std::string SerialMessageProcessor::extractOldStyleString(const char *terse, int& cursor)
{
  uint32_t nlen = ntohl ( *(uint32_t *)&(terse[cursor]) ); cursor += sizeof(uint32_t);
  std::ostringstream oname;
  for (uint32_t i=0; i<nlen; i++) {
    uint16_t uchar = ntohs( *(uint16_t *)&terse[cursor] ); cursor += 2;
    oname << static_cast<uint8_t>(uchar & 0xff);
  }
  return oname.str();
}

std::string SerialMessageProcessor::extractBlob(const char *terse, int& cursor)
{
  uint16_t length = ntohl ( *(uint16_t *)&(terse[cursor]) );
  cursor += sizeof(uint16_t);
  
  std::string blob(&terse[cursor], length);
  cursor += length;
  
  return blob;
}

int8_t SerialMessageProcessor::extractInt8(const char* terse, int& cursor)
{
  int8_t result = (int8_t) terse[cursor];
  cursor += sizeof(int8_t);
  return result;
}

int16_t SerialMessageProcessor::extractInt16(const char *terse, int& cursor)
{
  int16_t result = ntohs( *(int16_t *)&terse[cursor] );
  cursor += sizeof(int16_t);
  return result;
}

int32_t SerialMessageProcessor::extractInt32(const char *terse, int& cursor)
{
  int32_t result = ntohl( *(int32_t *)&terse[cursor] );
  cursor += sizeof(int32_t);
  return result;
}

int64_t SerialMessageProcessor::extractInt64(const char *terse, int& cursor)
{
  int64_t result = ntohll( *(int64_t *)&terse[cursor] );
  cursor += sizeof(int64_t);
  return result;
}


std::string SerialMessageProcessor::parseTerseData(int mt, const char *terse, std::string &originUser) {
  std::ostringstream jsonStr;
  int cursor = 0;
  switch(mt) {
  case 2:			// PLI
    /*
    LID -- Java Long (8)
    UserId - Java Long (8)
    UnitId - Java Long (8)
    Name - Text : Int (2), UTF8 Char (1 byte per)
    Lat - Java Long (8)
    Lon - Java Long (8)
    Created - Java Long (8)
    Modified - Java Long (8)
    */
    /* this will need to change - as MAP PLI definition is goig to change */
    {
      uint64_t lid  = (uint64_t)extractInt64(terse, cursor);
      uint64_t uid  = (uint64_t)extractInt64(terse, cursor);
      uint64_t unid = (uint64_t)extractInt64(terse, cursor);
      LOG_INFO((long) this << std::hex << "PLI: l(" << lid << ") u(" << uid << ") un(" << unid << ")");
      originUser = extractString(terse, cursor);
      int64_t lat      = extractInt64(terse, cursor);
      int64_t lon      = extractInt64(terse, cursor);
      int64_t created  = extractInt64(terse, cursor);
      int64_t modified = extractInt64(terse, cursor);

      // JSON
      // {\"lid\":\"0\",\"lon\":\"-74888318\",\"unitid\":\"1\",\"created\":\"1320329753964\",\"name\":\"ahammer\",\"userid\":\"731\",\"lat\":\"40187744\",\"modified\":\"0\"}
      jsonStr << "{\"lid\":\"" << lid << "\",\"userid\":\"" << uid << "\",\"unitid\":\"" << unid << "\",\"name\":\"" << originUser
        << "\",\"lat\":\"" << lat << "\",\"lon\":\"" << lon << "\",\"created\":\"" << created << "\",\"modified\":\"" << modified
        << "\"}";
    }
    
    break;
  case 5:			// Transapps (Sandeep mod...) PLI
    /*
    Name - Text : Int (2), UTF8 Char (1 byte per)
    Lat - Java Int (4)
    Lon - Java Int (4)
    Created - Java Int (4)
    */
    /* this will need to change - as MAP PLI definition is goig to change */
    {
      originUser = extractString(terse, cursor);
      int32_t lat      = extractInt32(terse, cursor);
      int32_t lon      = extractInt32(terse, cursor);
      uint32_t created  = extractInt32(terse, cursor);
      std::string groupPliBlob = extractBlob(terse, cursor);
      
      //update timestamp of last received PLI, so we know what the last 
      //received time is for delta PLI
      latestPliTimestamps[originUser] = created;

      // JSON
      // {\"lid\":\"0\",\"lon\":\"-74888318\",\"unitid\":\"1\",\"created\":\"1320329753964\",\"name\":\"ahammer\",\"userid\":\"731\",\"lat\":\"40187744\",\"modified\":\"0\"}
      jsonStr << "{\"name\":\"" << originUser
	      << "\",\"lat\":\"" << lat << "\",\"lon\":\"" << lon
	      << "\",\"altitude\":\"" << 0 << "\",\"accuracy\":\"" << 0
	      << "\",\"created\":\"" << 1000*(uint64_t)created << "\",\"modified\":\"" << 1000*(uint64_t)created
	      << "\"}";
    }
    
    break;

  case 3:			// Dash-Event
    break;
  case 4:			// Group-chat
    /*
      originator - Text : Int (2), UTF Char (1 byte per)
      text - Text : Int (2), UTF Char (1 byte per)
      created_date Java Long (8)
    */
    {
      std::string originator = extractString(terse, cursor);
      originUser = originator;
      std::string text = extractString(terse, cursor);
      int64_t created = extractInt64(terse, cursor);
      ACE_Utils::UUID *uuid = ACE_Utils::UUID_GENERATOR::instance ()->generate_UUID ();
      // JSON
      // "{\"created_date\":\"1339572928976\",\"text\":\"Wwwww\",\"modified_date\":\"1339572928984\",\"status\":\"21\",\"receipts\":\"0\",\"group_id\":\"All\",\"media_count\":\"0\",\"longitude\":\"0\",\"uuid\":\"9bf10c58-9154-4be8-8f63-e6a79a5ecbc1\",\"latitude\":\"0\",\"originator\":\"mark\"}"
      jsonStr << "{\"created_date\":\"" << created << "\",\"text\":\"" << text << "\",\"modified_date\":\"" << created << "\",\"status\":\"21\",\"receipts\":\"0\",\"group_id\":\"All\",\"media_count\":\"0\",\"longitude\":\"0\",\"uuid\":\"" << uuid->to_string()->c_str() << "\",\"latitude\":\"0\",\"originator\":\""<< originator << "\"}";
    }
					     
    
  }
  LOG_INFO((long) this << jsonStr.str() );
  
  return jsonStr.str();
}

void SerialMessageProcessor::parseGroupPliBlob(std::string groupPliBlob, int32_t baseLat, int32_t baseLon, uint32_t baseTime) {
  int cursor = 0;
  const char *groupPliBlobArray = groupPliBlob.data();
  
  int8_t count = extractInt8(groupPliBlobArray, cursor);
  for(int i = 0; i < count; ++i) {
    std::string originUsername = extractString(groupPliBlobArray, cursor);
    int32_t dLat = extractInt16(groupPliBlobArray, cursor);
    int32_t dLon = extractInt16(groupPliBlobArray, cursor);
    int16_t dCreatedTime = extractInt8(groupPliBlobArray, cursor);
    
    int32_t latitude = baseLat - dLat;
    int32_t longitude = baseLon - dLon;
    uint32_t createdTime = baseTime - dCreatedTime;
    
    TimestampMap::iterator it = latestTimestamps.find(originUsername);
    if(it != latestTimestamps.end() && createdTime < it->second) {
      //received delta PLI is older than the one we already have; discard it
      LOG_TRACE("Dropping PLI relay message from " << originUsername << " because it's too old");
    } else {
      //received delta PLI is newer than the one we have or we haven't gotten
      //one before, update map and send it
      latestTimestamps[originUsername] = createdTime;
      std::string pliString = generateTransappsPli(originUsername, latitude, longitude, createdTime);
      
      PushData pushData;
      pushData.mimeType = "ammo/transapps.pli.locations";
    	pushData.data = pliString;
    	pushData.uri = "serial-pli";
      pushData.originUsername = originUsername;
      pushData.scope = SCOPE_GLOBAL;
      
      LOG_TRACE("Sending group PLI relay message: " << pushData.data);
      gatewayConnector->pushData(pushData);
    }
  }
}

std::string SerialMessageProcessor::generateTransappsPli(std::string originUser, int32_t lat, int32_t lon, uint32_t created) {
  std::ostringstream jsonStr;
  jsonStr << "{\"name\":\"" << originUser
	        << "\",\"lat\":\"" << lat << "\",\"lon\":\"" << lon
	        << "\",\"altitude\":\"" << 0 << "\",\"accuracy\":\"" << 0
	        << "\",\"created\":\"" << 1000*(uint64_t)created << "\",\"modified\":\"" << 1000*(uint64_t)created
	        << "\"}";
  return jsonStr.str();
}


void testParseTerse() {
  SerialMessageProcessor test( (SerialServiceHandler *)0 );
  struct t1 {
    uint64_t l;
    uint64_t u;
    uint64_t un;
    uint32_t  nl;
    uint16_t nm[8];
    uint64_t lat;
    uint64_t lon;
    uint64_t cre;
    uint64_t mod;
  } td = {
    0,
    0x12,
    0x1234,
    0x8,
    { 't', 'a', '1', '5', '2', '-', '1', '4' },
    0xaabbccdd11223344ull,
    0x56781234,
    0x43218765,
    0x87654321
  };
  
  char terseBe[]={
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0x12,
    0,0,0,0,0,0,0x12,0x34,
    0,0,0,8,
    0,'t',0,'a',0,'1',0,'5',0,'2',0,'-',0,'1',0,'4',
    0xaa,0xbb,0xcc,0xdd,0x11,0x22,0x33,0x44,
    0,0,0,0,0x56,0x78,0x12,0x34,
    0,0,0,0,0x43,0x21,0x87,0x65,
    0,0,0,0,0x87,0x65,0x43,0x21
  };
  const char *tds = (const char *)&td;
  for (size_t i=0; i<sizeof(td); i++) {
    std::cout << std::hex << (static_cast<int>(tds[i]) & 0xff);
  }
  std::cout << std::endl;
  
  std::string originUser;
  test.parseTerseData(2, (const char *)&td, originUser);
  test.parseTerseData(2, (const char *)&terseBe[0], originUser);
}

