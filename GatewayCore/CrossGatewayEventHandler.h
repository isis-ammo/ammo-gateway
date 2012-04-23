#ifndef CROSS_GATEWAY_EVENT_HANDLER_H
#define CROSS_GATEWAY_EVENT_HANDLER_H

#include <string>
#include <set>
#include <queue>

#include "NetworkEventHandler.h"
#include "protocol/GatewayPrivateMessages.pb.h"
#include "Enumerations.h"

class CrossGatewayEventHandler : public ammo::gateway::internal::NetworkEventHandler<ammo::gateway::protocol::GatewayWrapper, ammo::gateway::internal::SYNC_MULTITHREADED, 0x8badf00d> {
public:
  virtual ~CrossGatewayEventHandler();
  
  virtual void onConnect(std::string &peerAddress);
  virtual void onDisconnect();
  virtual int onMessageAvailable(ammo::gateway::protocol::GatewayWrapper *msg);
  virtual int onError(const char errorCode);
  
  bool sendSubscribeMessage(std::string mime_type);
  bool sendUnsubscribeMessage(std::string mime_type);
  
  bool sendPushedData(std::string uid, std::string mimeType, std::string encoding, const std::string &data, std::string originUser, char priority);
  
  bool sendPullRequest(std::string requestUid, std::string pluginId, std::string mimeType, std::string query, std::string projection, unsigned int maxResults, unsigned int startFromCount, bool liveQuery, char priority);
  bool sendPullResponse(std::string requestUid, std::string pluginId, std::string mimeType, std::string uid, std::string encoding, const std::string &data, char priority);
  
  bool sendRegisterPullInterest(std::string mimeType);
  bool sendUnregisterPullInterest(std::string mimeType);
  
  bool sendPointToPointMessage(std::string uid, std::string destinationGateway, std::string destinationPluginName, std::string destinationInstanceId, std::string sourceGateway, std::string sourcePluginName, std::string sourceInstanceId, std::string mimeType, std::string encoding, std::string data, char priority);
  
  bool sendRemoteGatewayConnectedNotification(std::string gatewayId, std::vector<std::pair<std::string, std::string> > connectedPlugins);
  bool sendRemoteGatewayDisconnectedNotification(std::string gatewayId);
  
  bool sendPluginConnectedNotification(std::string pluginName, std::string instanceId, bool remotePlugin, std::string gatewayId);
  
private:
  std::string gatewayId;
  bool gatewayIdAuthenticated;
  
  bool registeredWithGateway;
  
  std::vector<std::string> registeredHandlers;
  std::vector<std::string> registeredPullHandlers;
  std::set<std::string> registeredPullResponsePluginIds;
};

#endif
