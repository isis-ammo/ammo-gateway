/*
 * GatewayReceiver.cpp
 *
 *  Created on: Aug 28, 2012
 *      Author: jwilliams
 */

#include "GatewayReceiver.h"
#include <sstream>
#include "json/json.h"
#include "protocol/AmmoMessages.pb.h"
#include "SerialConfigurationManager.h"
#include "GpsThread.h"
#include <limits>
#include <cmath>
#include <ace/UUID.h>

using namespace std;

#define htonll(x) (((int64_t)(ntohl((int32_t)((x << 32) >> 32))) << 32) | (uint32_t)ntohl(((int)(x >> 32))))

#ifdef WIN32
#define atoll(str) _atoi64(str)
#endif

GatewayReceiver::GatewayReceiver(GpsThread *gpsThread) : gpsThread(gpsThread), receivedMessageCount(0), pliIndex(0) {
  // TODO Auto-generated constructor stub
  SerialConfigurationManager *config = SerialConfigurationManager::getInstance();
  pliRelayPerCycle = config->getPliRelayPerCycle();
  pliRelayEnabled = config->getPliRelayEnabled();
  pliRelayNodeName = config->getPliRelayNodeName();
  rangeScale = config->getRangeScale();
  timeScale = config->getTimeScale();
}

GatewayReceiver::~GatewayReceiver() {
  // TODO Auto-generated destructor stub
}

void GatewayReceiver::onPushDataReceived(
    ammo::gateway::GatewayConnector* sender,
    ammo::gateway::PushData& pushData) {
  LOG_DEBUG("Push data received: uri: " << pushData.uri << " type: " << pushData.mimeType);
  LOG_DEBUG("  data: " << pushData.data.c_str());

  if(pushData.mimeType == "ammo/transapps.pli.locations") {
    /*
     * Terse PLI type definition:
     *   Name : string (uint16_t size + UTF-8 data)
     *   Lat  : int32
     *   Lon  : int32
     *   createdTime: uint32; time from json / 1000
     *   GroupPliBlob : blob
     */
    Json::Value root;
    Json::Reader reader;

    //We only parse the c_str component of the data because the data may contain
    //binary data at the end (separated by a null, which is why this works).
    bool parsingSuccessful = reader.parse(pushData.data.c_str(), root);

    if(parsingSuccessful) {
      string name = root["name"].asString();
      int32_t lat = atoi(root["lat"].asString().c_str());
      int32_t lon = atoi(root["lon"].asString().c_str());
      uint32_t time = atoll(root["created"].asString().c_str()) / 1000;
      LOG_DEBUG("XXX name=" << name << " lat=" << lat << " lon=" << lon << " time=" << time);

#ifdef PLI_PASSTHROUGH
      //Pass through PLI messages 1:1
      ostringstream tersePayload;
      appendString(tersePayload, name);
      appendInt32(tersePayload, lat);
      appendInt32(tersePayload, lon);
      appendUInt32(tersePayload, time);

      //TODO: parse and forward PLI relay, too
      std::string pliRelayBlob("\0", 1);
      appendBlob(tersePayload, pliRelayBlob);

      ammo::protocol::MessageWrapper msg;
      msg.set_type(ammo::protocol::MessageWrapper_MessageType_TERSE_MESSAGE);
      msg.set_message_priority(0);
      ammo::protocol::TerseMessage *terseMsg = msg.mutable_terse_message();
      terseMsg->set_mime_type(5);
      terseMsg->set_data(tersePayload.str());

      std::string serializedMessage = msg.SerializeAsString();
      //we need a pointer to this data, so create a new string and copy it
      std::string *messageToSend = new std::string(serializedMessage);
      addReceivedMessage(messageToSend);
#else
      //Use PLI relay to pass through PLI
      PliInfo location;
      location.lat = lat;
      location.lon = lon;
      location.createdTime = time;

      pliMapMutex.acquire();
      pliMap[name] = location;
      pliMapMutex.release();
#endif
    } else {
      LOG_ERROR("Couldn't parse PLI data from gateway");
    }
  } else if (pushData.mimeType == "ammo/transapps.chat.message_groupAll") {
    /* Terse Chat type definition:
     *   Originator : string
     *   Text : string
     *   Created Date : int64
     */
    Json::Value root;
    Json::Reader reader;

    //We only parse the c_str component of the data because the data may contain
    //binary data at the end (separated by a null, which is why this works).
    bool parsingSuccessful = reader.parse(pushData.data.c_str(), root);

    if(parsingSuccessful) {
      string originator = root["originator"].asString();
      string text = root["text"].asString();
      int64_t time = atoll(root["created_date"].asString().c_str());
      ACE_Utils::UUID *uuid = ACE_Utils::UUID_GENERATOR::instance ()->generate_UUID ();


      ostringstream tersePayload;
      appendString(tersePayload, string(uuid->to_string()->c_str()));
      appendString(tersePayload, originator);
      appendString(tersePayload, text);
      appendInt32(tersePayload, time / 1000);

      ammo::protocol::MessageWrapper msg;
      msg.set_type(ammo::protocol::MessageWrapper_MessageType_TERSE_MESSAGE);
      msg.set_message_priority(0);
      ammo::protocol::TerseMessage *terseMsg = msg.mutable_terse_message();
      terseMsg->set_mime_type(4);
      terseMsg->set_data(tersePayload.str());

      std::string serializedMessage = msg.SerializeAsString();
      //we need a pointer to this data, so create a new string and copy it
      std::string *messageToSend = new std::string(serializedMessage);
      addReceivedMessage(messageToSend);
    } else {
      LOG_ERROR("Couldn't parse Chat data from gateway");
    }
  }
}

void GatewayReceiver::addPli(std::string &username, int32_t lat, int32_t lon, int32_t createdTime) {
  //this method is only used for relay of PLI from serial; the pliRelayEnabled flag only disables that
  if(pliRelayEnabled) {
    PliInfo location;
    location.lat = lat;
    location.lon = lon;
    location.createdTime = createdTime;

    pliMapMutex.acquire();
    pliMap[username] = location;
    pliMapMutex.release();
  }
}

void GatewayReceiver::appendString(ostringstream &stream, const std::string &str) {
  if(str.length() > numeric_limits<uint16_t>::max()) {
    LOG_WARN("String too long, putting zero length string instead");
    appendUInt16(stream, 0);
  } else {
    appendUInt16(stream,str.length());
    stream << str;
  }
}

void GatewayReceiver::appendInt64(ostringstream &stream, const int64_t val) {
  int64_t networkVal = htonll(val);
  stream.write(reinterpret_cast<char *>(&networkVal), sizeof(val));
}

void GatewayReceiver::appendInt32(ostringstream &stream, const int32_t val) {
  int32_t networkVal = htonl(val);
  stream.write(reinterpret_cast<char *>(&networkVal), sizeof(val));
}

void GatewayReceiver::appendUInt32(ostringstream &stream, const uint32_t val) {
  uint32_t networkVal = htonl(val);
  stream.write(reinterpret_cast<char *>(&networkVal), sizeof(val));
}

void GatewayReceiver::appendInt16(ostringstream &stream, const int16_t val) {
  int16_t networkVal = htons(val);
  stream.write(reinterpret_cast<char *>(&networkVal), sizeof(val));
}

void GatewayReceiver::appendInt8(ostringstream &stream, const int8_t val) {
  stream.write(reinterpret_cast<const char *>(&val), sizeof(val));
}

void GatewayReceiver::appendUInt16(ostringstream &stream, const uint16_t val) {
  uint16_t networkVal = htons(val);
  stream.write(reinterpret_cast<char *>(&networkVal), sizeof(val));
}

void GatewayReceiver::appendBlob(ostringstream &stream, const std::string &blob) {
  //blob format is currently exactly the same as a string, so treat it as such
  appendString(stream, blob);
}

std::string GatewayReceiver::getNextPliRelayPacket() {
  //get the next n locations to be sent
  pliMapMutex.acquire();
  PliMap tempPliMap;

  //get iterator to the first item that's supposed to be sent
  PliMap::iterator pliMapIt = pliMap.begin();
  if(pliMapIt == pliMap.end()) {
    //empty map; don't send anything
    pliMapMutex.release();
    return "";
  }

  for(int i = 0; i < pliIndex; i++) {
    pliMapIt++; //nothing ever deletes from the list--  so we don't need to worry about hitting the end here
  }


  double latDouble = 0;
  double lonDouble = 0;
  
  if(gpsThread->getPosition(latDouble, lonDouble) == false) {
    LOG_WARN("Can't forward PLI; no GPS lock");
    return "";
  }
  
  int32_t baseLat = (int) (latDouble * 1e6);
  int32_t baseLon = (int) (lonDouble * 1e6);
  time_t baseTime = time(NULL);

  //And prepare the PLI packet
  ostringstream tersePayload;
  appendString(tersePayload, pliRelayNodeName);
  appendInt32(tersePayload, baseLat);
  appendInt32(tersePayload, baseLon);
  appendUInt32(tersePayload, baseTime);

  ostringstream relayBlob;

  int8_t deltaCount = 0;
  
  PliMap::iterator baseObject = pliMapIt;

  while(deltaCount < pliRelayPerCycle) {
    int32_t dLat = (baseLat - pliMapIt->second.lat) / rangeScale;
    int32_t dLon = (baseLon - pliMapIt->second.lon) / rangeScale;
    int32_t dTime = (baseTime - pliMapIt->second.createdTime) / timeScale;
    if(numeric_limits<int16_t>::min() <= dLat && dLat <= numeric_limits<int16_t>::max() &&
        numeric_limits<int16_t>::min() <= dLon && dLon <= numeric_limits<int16_t>::max() &&
        numeric_limits<int8_t>::min() <= dTime && dTime <= numeric_limits<int8_t>::max()) {
      appendString(relayBlob, pliMapIt->first);
      appendInt16(relayBlob, dLat);
      appendInt16(relayBlob, dLon);
      appendInt8(relayBlob, dTime);
      appendInt8(relayBlob, 1); //TODO: add hop count
      deltaCount++;
    }
    
    pliMapIt++;
    pliIndex++;
    if(pliMapIt == pliMap.end()) {
      //wrap around to the beginning of the map if we hit the end
      pliMapIt = pliMap.begin();
      pliIndex = 0;
    }

    if(pliMapIt == baseObject) {
      //we've wrapped back around to where we started; don't repeat
      break;
    }
  }

  ostringstream finalRelayBlob;

  appendInt8(finalRelayBlob, deltaCount);
  finalRelayBlob << relayBlob.str();

  appendBlob(tersePayload, finalRelayBlob.str());

  pliMapMutex.release();

  ammo::protocol::MessageWrapper msg;
  msg.set_type(ammo::protocol::MessageWrapper_MessageType_TERSE_MESSAGE);
  msg.set_message_priority(0);
  ammo::protocol::TerseMessage *terseMsg = msg.mutable_terse_message();
  terseMsg->set_mime_type(5);
  terseMsg->set_data(tersePayload.str());

  std::string serializedMessage = msg.SerializeAsString();

  return serializedMessage;
}

std::string *GatewayReceiver::getNextReceivedMessage() {
  std::string *msg = NULL;
  receiveQueueMutex.acquire();
  if(!receiveQueue.empty()) {
    msg = receiveQueue.front();
    receiveQueue.pop();
  }
  receiveQueueMutex.release();

  return msg;
}

void GatewayReceiver::addReceivedMessage(std::string *msg) {
  receiveQueueMutex.acquire();
  receivedMessageCount++;
  receiveQueue.push(msg);
  receiveQueueMutex.release();
}

int GatewayReceiver::getNextMessageSize() {
  int size = 0;
  receiveQueueMutex.acquire();
  if(!receiveQueue.empty()) {
    size = receiveQueue.front()->length();
  }
  receiveQueueMutex.release();

  return size;
}

bool GatewayReceiver::isMessageAvailable() {
  bool result = false;
  receiveQueueMutex.acquire();
  result = !receiveQueue.empty();
  receiveQueueMutex.release();

  return result;
}
