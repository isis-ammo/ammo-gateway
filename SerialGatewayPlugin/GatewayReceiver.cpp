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

using namespace std;

GatewayReceiver::GatewayReceiver() : receivedMessageCount(0) {
  // TODO Auto-generated constructor stub
  
}

GatewayReceiver::~GatewayReceiver() {
  // TODO Auto-generated destructor stub
}

void GatewayReceiver::onPushDataReceived(
    ammo::gateway::GatewayConnector* sender,
    ammo::gateway::PushData& pushData) {
  LOG_DEBUG("Push data received: uri: " << pushData.uri << " type: " << pushData.mimeType);

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

    string name = root["name"].asString();
    int32_t lat = atoi(root["lat"].asString().c_str());
    int32_t lon = atoi(root["lon"].asString().c_str());
    uint32_t time = atoi(root["created"].asString().c_str()) / 1000;

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
  }
}

void GatewayReceiver::appendString(ostringstream &stream, std::string &str) {
  if(str.length() > UINT16_MAX) {
    LOG_WARN("String too long, putting zero length string instead");
    appendUInt16(stream, 0);
  } else {
    appendUInt16(stream, str.length());
    stream << str;
  }
}

void GatewayReceiver::appendInt32(ostringstream &stream, int32_t val) {
  stream.write(reinterpret_cast<char *>(&val), sizeof(val));
}

void GatewayReceiver::appendUInt32(ostringstream &stream, uint32_t val) {
  stream.write(reinterpret_cast<char *>(&val), sizeof(val));
}

void GatewayReceiver::appendUInt16(ostringstream &stream, uint16_t val) {
  stream.write(reinterpret_cast<char *>(&val), sizeof(val));
}

void GatewayReceiver::appendBlob(ostringstream &stream, std::string &blob) {
  //blob format is currently exactly the same as a string, so treat it as such
  appendString(stream, blob);
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
