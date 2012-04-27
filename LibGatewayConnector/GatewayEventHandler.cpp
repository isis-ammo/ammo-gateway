#include <string>

#include "GatewayEventHandler.h"
#include "GatewayConnector.h"
#include "log.h"

using namespace std;
using namespace ammo::gateway::internal;

GatewayEventHandler::GatewayEventHandler() : parent(NULL) {

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
