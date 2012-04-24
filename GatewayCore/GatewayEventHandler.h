#ifndef GATEWAY_EVENT_HANDLER_H
#define GATEWAY_EVENT_HANDLER_H

#include <string>
#include <set>
#include <queue>
#include <vector>

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
  
  bool sendPushedData(std::string uid, std::string mimeType, std::string encoding, const std::string &data, std::string originUser, std::string originDevice, MessageScope scope, bool ackDeviceDelivered, bool ackPluginDelivered, char priority);
  bool sendPullRequest(std::string requestUid, std::string pluginId, std::string mimeType, std::string query, std::string projection,
		       unsigned int maxResults, unsigned int startFromCount, bool liveQuery, char priority);
  bool sendPullResponse(std::string requestUid, std::string pluginId, std::string mimeType, std::string uid, std::string encoding, const std::string& data, char priority);
  
  bool sendPointToPointMessage(std::string uid, std::string destinationGateway, std::string destinationPluginName, std::string destinationInstanceId, std::string sourceGateway, std::string sourcePluginName, std::string sourceInstanceId, std::string mimeType, std::string encoding, std::string data, char priority);
  
  bool sendPluginAuthenticationResponse(bool success, std::string message);
  
  bool sendRemoteGatewayConnectedNotification(std::string gatewayId, std::vector<std::pair<std::string, std::string> > connectedPlugins);
  bool sendPluginConnectedNotification(std::string pluginName, std::string instanceId, bool remotePlugin, std::string gatewayId);
  
  
  
private:
  std::string username;
  bool usernameAuthenticated;
  
  std::string pluginName;
  std::string instanceId;
  
  std::vector<std::string> registeredHandlers;
  std::vector<std::string> registeredPullRequestHandlers;
  std::set<std::string> registeredPullResponsePluginIds;
};

#endif
