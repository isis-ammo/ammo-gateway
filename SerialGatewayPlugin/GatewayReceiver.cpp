/*
 * GatewayReceiver.cpp
 *
 *  Created on: Aug 28, 2012
 *      Author: jwilliams
 */

#include "GatewayReceiver.h"
#include <sstream>

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
    ostringstream tersePayload;
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
