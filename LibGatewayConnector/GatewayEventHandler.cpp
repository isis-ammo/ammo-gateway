#include <string>

#include "GatewayEventHandler.h"
#include "log.h"

using namespace std;
using namespace ammo::gateway::internal;

void GatewayEventHandler::onConnect(std::string &peerAddress) {
  LOG_TRACE("onConnect(" << peerAddress << ")");
}

void GatewayEventHandler::onDisconnect() {
  LOG_TRACE("onDisconnect()");
}

int GatewayEventHandler::onMessageAvailable(ammo::gateway::protocol::GatewayWrapper *msg) {
  LOG_TRACE("onMessageAvailable()");
  LOG_TRACE("  msg: " << msg->DebugString());
  return 0;
}

int GatewayEventHandler::onError(const char errorCode) {
  LOG_ERROR("onError(" << errorCode << ")");
  return 0;
}
