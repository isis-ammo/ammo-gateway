#ifndef GATEWAY_EVENT_HANDLER_H
#define GATEWAY_EVENT_HANDLER_H

#include <string>
#include <set>
#include <queue>

#include "NetworkEventHandler.h"
#include "protocol/GatewayPrivateMessages.pb.h"
#include "Enumerations.h"

class GatewayEventHandler : public ammo::gateway::internal::NetworkEventHandler<ammo::gateway::protocol::GatewayWrapper, ammo::gateway::internal::SYNC_MULTITHREADED, 0xdeadbeef> {
public:
  virtual ~GatewayEventHandler();
  
  virtual void onConnect(std::string &peerAddress);
  virtual void onDisconnect();
  virtual int onMessageAvailable(ammo::gateway::protocol::GatewayWrapper *msg);
  virtual int onError(const char errorCode);
  
  bool sendPushedData(std::string uri, std::string mimeType, std::string encoding, const std::string &data, std::string originUser, MessageScope scope, char priority);
  bool sendPullRequest(std::string requestUid, std::string pluginId, std::string mimeType, std::string query, std::string projection,
		       unsigned int maxResults, unsigned int startFromCount, bool liveQuery, char priority);
  bool sendPullResponse(std::string requestUid, std::string pluginId, std::string mimeType, std::string uri, std::string encoding, const std::string& data, char priority);
  
private:
  std::string username;
  bool usernameAuthenticated;
  
  std::vector<std::string> registeredHandlers;
  std::vector<std::string> registeredPullRequestHandlers;
  std::set<std::string> registeredPullResponsePluginIds;
};

#endif
