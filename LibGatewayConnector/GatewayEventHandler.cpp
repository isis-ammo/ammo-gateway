/* Copyright (C) 2010-2013 Vanderbilt University
 *  
 * This software was developed by the Institute for Software Integrated
 * Systems (ISIS) at Vanderbilt University, Tennessee, USA for the
 * Transformative Apps program under DARPA, Contract # HR011-10-C-0175 and
 * Contract # HR0011-12-C-0109.
 *  
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this data, including any software or models in source or binary
 * form, as well as any drawings, specifications, and documentation
 * (collectively "the Data"), to deal in the Data without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Data, and to
 * permit persons to whom the Data is furnished to do so, subject to the
 * following conditions:
 *  
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Data.
 *  
 * THE DATA IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS, SPONSORS, DEVELOPERS, CONTRIBUTORS, OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE DATA OR THE USE OR OTHER DEALINGS IN THE DATA.
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
