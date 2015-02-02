/* Copyright (c) 2010-2015 Vanderbilt University
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <string>

#include "GatewayEventHandler.h"
#include "GatewayConnector.h"
#include "log.h"

using namespace std;
using namespace ammo::gateway::internal;

GatewayEventHandler::GatewayEventHandler() : parent(NULL) {

}

GatewayEventHandler::~GatewayEventHandler() {
  LOG_TRACE("In ~GatewayEventHandler");
}

void GatewayEventHandler::onConnect(std::string &peerAddress) {
  LOG_TRACE("GatewayEventHandler::onConnect(" << peerAddress << ")");
  if(parent) {
    parent->onConnectReceived();
  }
}

void GatewayEventHandler::onDisconnect() {
  LOG_TRACE("GatewayEventHandler::onDisconnect()");
  if(parent) {
    parent->onDisconnectReceived();
  }
}

int GatewayEventHandler::onMessageAvailable(ammo::gateway::protocol::GatewayWrapper *msg) {
  LOG_TRACE("GatewayEventHandler::onMessageAvailable()");
  
  if(msg->type() == ammo::gateway::protocol::GatewayWrapper_MessageType_ASSOCIATE_RESULT) {
    LOG_DEBUG("Received Associate Result...");
    parent->onAssociateResultReceived(msg->associate_result());
  } else if(msg->type() == ammo::gateway::protocol::GatewayWrapper_MessageType_PUSH_DATA) {
    LOG_DEBUG("Received Push Data...");
    parent->onPushDataReceived(msg->push_data(), msg->message_priority());
  } else if(msg->type() == ammo::gateway::protocol::GatewayWrapper_MessageType_PUSH_ACKNOWLEDGEMENT) {
    LOG_DEBUG("Received Push Acknowledgement...");
    parent->onPushAcknowledgementReceived(msg->push_acknowledgement());
  } else if(msg->type() == ammo::gateway::protocol::GatewayWrapper_MessageType_PULL_REQUEST) {
    LOG_DEBUG("Received Pull Request...");
    parent->onPullRequestReceived(msg->pull_request(), msg->message_priority());
  } else if(msg->type() == ammo::gateway::protocol::GatewayWrapper_MessageType_PULL_RESPONSE) {
    LOG_DEBUG("Received Pull Response...");
    parent->onPullResponseReceived(msg->pull_response(), msg->message_priority());
  }
  
  delete msg;
  
  return 0;
}

int GatewayEventHandler::onError(const char errorCode) {
  LOG_ERROR("GatewayEventHandler::onError(" << errorCode << ")");
  return 0;
}

void GatewayEventHandler::setParentConnector(ammo::gateway::GatewayConnector *parent) {
  this->parent = parent;
  parent->onConnectReceived(); //we have to call this here, because parent isn't
                              //set yet on initial connect
}
