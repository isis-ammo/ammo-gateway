#include "CrossGatewayServiceHandler.h"
#include "GatewayCore.h"
#include "GatewayConfigurationManager.h"
#include "protocol/GatewayPrivateMessages.pb.h"

#include <iostream>

#include "log.h"

#include "ace/OS_NS_errno.h"


/* CrossGatewayServiceHandler::CrossGatewayServiceHandler(ACE_Thread_Manager *tm = 0, ACE_Message_Queue<ACE_NULL_SYNCH> *mq = 0, ACE_Reactor *reactor = ACE_Reactor::instance())
: ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH>(tm, mq, reactor)
{
  
} */

int CrossGatewayServiceHandler::open(void *ptr) {
  if(super::open(ptr) == -1) {
    return -1;
    
  }
  state = READING_SIZE;
  dataSize = 0;
  checksum = 0;
  collectedData = NULL;
  position = 0;
  gatewayId = "";
  gatewayIdAuthenticated = false;
  
  //send an authentication message to the other gateway
  ammo::gateway::protocol::GatewayWrapper newMsg;
  newMsg.set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_ASSOCIATE_CROSS_GATEWAY);
  newMsg.mutable_associate_cross_gateway()->set_gateway_id(GatewayConfigurationManager::getInstance()->getCrossGatewayId());
  newMsg.mutable_associate_cross_gateway()->set_key(GatewayConfigurationManager::getInstance()->getCrossGatewayId());
  LOG_DEBUG("Sending associate message to connected gateway...");
  this->sendData(newMsg);
  
  return 0;
}

int CrossGatewayServiceHandler::handle_input(ACE_HANDLE fd) {
  //LOG_TRACE("In handle_input");
  int count = 0;
  
  if(state == READING_SIZE) {
    count = this->peer().recv_n(&dataSize, sizeof(dataSize));
    //LOG_TRACE("SIZE Read " << count << " bytes");
  } else if(state == READING_CHECKSUM) {
    count = this->peer().recv_n(&checksum, sizeof(checksum));
    //LOG_TRACE("SUM Read " << count << " bytes");
  } else if(state == READING_DATA) {
    count = this->peer().recv(collectedData + position, dataSize - position);
    //LOG_TRACE("DATA Read " << count << " bytes");
  } else {
    LOG_ERROR("Invalid state!");
  }
  
  
  
  if(count > 0) {
    if(state == READING_SIZE) {
      collectedData = new char[dataSize];
      position = 0;
      //LOG_TRACE("Got data size (" << dataSize << ")");
      state = READING_CHECKSUM;
    } else if(state == READING_CHECKSUM) {
      //LOG_TRACE("Got data checksum (" << checksum << ")");
      state = READING_DATA;
    } else if(state == READING_DATA) {
      //LOG_TRACE("Got some data...");
      position += count;
      if(position == dataSize) {
        //LOG_TRACE("Got all the data... processing");
        processData(collectedData, dataSize, checksum);
        //LOG_TRACE("Processsing complete.  Deleting buffer.");
        delete collectedData;
        collectedData = NULL;
        dataSize = 0;
        position = 0;
        state = READING_SIZE;
      }
    }
  } else if(count == 0) {
    LOG_INFO("Connection closed.");
    return -1;
  } else if(count == -1 && ACE_OS::last_error () != EWOULDBLOCK) {
    LOG_ERROR("Socket error occurred. (" << ACE_OS::last_error() << ")");
    return -1;
  }
  //LOG_INFO("Leaving handle_input()");
  return 0;
}

// This method comes stright from ACE sample code:  see ace_wrappers/examples/APG/Reactor/Client.cpp
int CrossGatewayServiceHandler::handle_output(ACE_HANDLE) {
  ACE_Message_Block *mb;
  ACE_Time_Value nowait(ACE_OS::gettimeofday ());
  
  while (-1 != this->getq(mb, &nowait)) {
    ssize_t send_cnt = this->peer().send(mb->rd_ptr(), mb->length());
    if (send_cnt == -1) {
      LOG_ERROR("Send error...");
    } else {
      mb->rd_ptr(static_cast<size_t>(send_cnt));
    }
    if (mb->length() > 0) {
      this->ungetq(mb);
      break;
    }
    mb->release();
  }
  
  if (this->msg_queue ()->is_empty ()) {
    this->reactor()->cancel_wakeup(this, ACE_Event_Handler::WRITE_MASK);
  } else {
    this->reactor()->schedule_wakeup(this, ACE_Event_Handler::WRITE_MASK);
  }
  
  return 0;
}

void CrossGatewayServiceHandler::sendData(ammo::gateway::protocol::GatewayWrapper &msg) {
  /*Fixme: potential deadlock here
  unsigned int messageSize = msg.ByteSize();
  char *messageToSend = new char[messageSize];
  msg.SerializeToArray(messageToSend, messageSize);
  unsigned int messageChecksum = ACE::crc32(messageToSend, messageSize);
  
  ACE_Message_Block *messageSizeBlock = new ACE_Message_Block(sizeof(messageSize));
  messageSizeBlock->copy((char *) &messageSize, sizeof(messageSize));
  this->putq(messageSizeBlock);
  
  ACE_Message_Block *messageChecksumBlock = new ACE_Message_Block(sizeof(messageChecksum));
  messageChecksumBlock->copy((char *) &messageChecksum, sizeof(messageChecksum));
  this->putq(messageChecksumBlock);
  
  ACE_Message_Block *messageToSendBlock = new ACE_Message_Block(messageSize);
  messageToSendBlock->copy(messageToSend, messageSize);
  this->putq(messageToSendBlock);
  
  this->reactor()->schedule_wakeup(this, ACE_Event_Handler::WRITE_MASK);*/
  
  unsigned int messageSize = msg.ByteSize();
  char *messageToSend = new char[messageSize];
  
  if(msg.IsInitialized()) { //Don't send a message if it's missing required fields (SerializeToArray is supposed to check for this, but doesn't)
    msg.SerializeToArray(messageToSend, messageSize);
    unsigned int messageChecksum = ACE::crc32(messageToSend, messageSize);
    
    this->peer().send_n(&messageSize, sizeof(messageSize));
    this->peer().send_n(&messageChecksum, sizeof(messageChecksum));
    this->peer().send_n(messageToSend, messageSize);
  } else {
    LOG_ERROR("SEND ERROR:  Message is missing a required element.");
  }
}

int CrossGatewayServiceHandler::processData(char *data, unsigned int messageSize, unsigned int messageChecksum) {
  //Validate checksum
  unsigned int calculatedChecksum = ACE::crc32(data, messageSize);
  if(calculatedChecksum != messageChecksum) {
    LOG_ERROR("Invalid checksum--  we've been sent bad data (perhaps a message size mismatch?)");
    return -1;
  }
  
  //checksum is valid; parse the data
  ammo::gateway::protocol::GatewayWrapper msg;
  bool result = msg.ParseFromArray(data, messageSize);
  if(result == false) {
    LOG_ERROR("GatewayWrapper could not be deserialized.");
    LOG_ERROR("Client must have sent something that isn't a protocol buffer (or the wrong type).");
    return -1;
  }
  //LOG_TRACE("Message Received: " << msg.DebugString());
  
  if(msg.type() == ammo::gateway::protocol::GatewayWrapper_MessageType_ASSOCIATE_CROSS_GATEWAY) {
    LOG_DEBUG("Received Associate CrossGateway...");
    //TODO: split out into a different function and do more here
    ammo::gateway::protocol::GatewayWrapper newMsg;
    newMsg.set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_ASSOCIATE_RESULT);
    newMsg.mutable_associate_result()->set_result(ammo::gateway::protocol::AssociateResult_Status_SUCCESS);
    this->sendData(newMsg);
    gatewayId = msg.associate_cross_gateway().gateway_id();
    gatewayIdAuthenticated = true;
    GatewayCore::getInstance()->registerCrossGatewayConnection(gatewayId, this);
  } else if(msg.type() == ammo::gateway::protocol::GatewayWrapper_MessageType_ASSOCIATE_RESULT) {
    
  } else if(msg.type() == ammo::gateway::protocol::GatewayWrapper_MessageType_REGISTER_DATA_INTEREST) {
    LOG_DEBUG("Received Register Data Interest...");
    std::string mime_type = msg.register_data_interest().mime_type();
    //bool result = GatewayCore::getInstance()->registerDataInterest(mime_type, this);
    if(result == true) {
      registeredHandlers.push_back(mime_type);
    }
  } else if(msg.type() == ammo::gateway::protocol::GatewayWrapper_MessageType_UNREGISTER_DATA_INTEREST) {
    LOG_DEBUG("Received Unregister Data Interest...");
    std::string mime_type = msg.unregister_data_interest().mime_type();
    //bool result = GatewayCore::getInstance()->unregisterDataInterest(mime_type, this);
    if(result == true) {
      for(std::vector<std::string>::iterator it = registeredHandlers.begin(); it != registeredHandlers.end(); it++) {
        if((*it) == mime_type) {
          registeredHandlers.erase(it);
        }
      }
    }
  } else if(msg.type() == ammo::gateway::protocol::GatewayWrapper_MessageType_PUSH_DATA) {
    LOG_DEBUG("Received Push Data...");
    GatewayCore::getInstance()->pushData(msg.push_data().uri(), msg.push_data().mime_type(), msg.push_data().data(), msg.push_data().origin_user());
  }
  
  return 0;
}

bool CrossGatewayServiceHandler::sendSubscribeMessage(std::string mime_type) {
  ammo::gateway::protocol::GatewayWrapper msg;
  ammo::gateway::protocol::RegisterDataInterest *subscribeMsg = msg.mutable_register_data_interest();
  subscribeMsg->set_mime_type(mime_type);
  
  msg.set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_REGISTER_DATA_INTEREST);
  
  LOG_DEBUG("Sending Subscribe message to connected gateway");
  this->sendData(msg);
  
  return true;
}

bool CrossGatewayServiceHandler::sendUnsubscribeMessage(std::string mime_type) {
  ammo::gateway::protocol::GatewayWrapper msg;
  ammo::gateway::protocol::UnregisterDataInterest *unsubscribeMsg = msg.mutable_unregister_data_interest();
  unsubscribeMsg->set_mime_type(mime_type);
  
  msg.set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_UNREGISTER_DATA_INTEREST);
  
  LOG_DEBUG("Sending Subscribe message to connected gateway");
  this->sendData(msg);
  
  return true;
}

bool CrossGatewayServiceHandler::sendPushedData(std::string uri, std::string mimeType, const std::string &data, std::string originUser) {
  ammo::gateway::protocol::GatewayWrapper msg;
  ammo::gateway::protocol::PushData *pushMsg = msg.mutable_push_data();
  pushMsg->set_uri(uri);
  pushMsg->set_mime_type(mimeType);
  pushMsg->set_data(data);
  pushMsg->set_origin_user(originUser);
  
  msg.set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_PUSH_DATA);
  
  LOG_DEBUG("Sending Data Push message to connected gateway");
  this->sendData(msg);
  
  return true;
}


CrossGatewayServiceHandler::~CrossGatewayServiceHandler() {
  LOG_DEBUG("CrossGatewayServiceHandler being destroyed!");
  LOG_DEBUG("Unregistering data handlers...");
  for(std::vector<std::string>::iterator it = registeredHandlers.begin(); it != registeredHandlers.end(); it++) {
    GatewayCore::getInstance()->unsubscribeCrossGateway(*it, gatewayId);
  }
  
  GatewayCore::getInstance()->unregisterCrossGatewayConnection(gatewayId);
  
  LOG_DEBUG("Unregistering pull request handlers...");
  for(std::vector<std::string>::iterator it = registeredPullRequestHandlers.begin(); it != registeredPullRequestHandlers.end(); it++) {
    //GatewayCore::getInstance()->unregisterPullInterest(*it, this);
  }
}

std::ostream& operator<< (std::ostream& out, const CrossGatewayServiceHandler& handler) {
    out << &handler;
    return out;
}

std::ostream& operator<< (std::ostream& out, const CrossGatewayServiceHandler* handler) {
    // Since operator<< is a friend of the CrossGatewayServiceHandler class, 
    // we can access handler's members directly.
    out << "(" << reinterpret_cast<void const *>(handler) << " " << handler->state << ", " << handler->gatewayId << ")";
    return out;
}


