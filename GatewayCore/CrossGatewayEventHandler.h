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
  
  bool sendPushedData(std::string uri, std::string mimeType, std::string encoding, const std::string &data, std::string originUser);
  
  bool sendPullRequest(std::string requestUid, std::string pluginId, std::string mimeType, std::string query, std::string projection, unsigned int maxResults, unsigned int startFromCount, bool liveQuery);
  bool sendPullResponse(std::string requestUid, std::string pluginId, std::string mimeType, std::string uri, std::string encoding, const std::string &data);
  
  bool sendRegisterPullInterest(std::string mimeType);
  bool sendUnregisterPullInterest(std::string mimeType);
  
private:
  std::string gatewayId;
  bool gatewayIdAuthenticated;
  
  bool registeredWithGateway;
  
  std::vector<std::string> registeredHandlers;
  std::vector<std::string> registeredPullHandlers;
  std::set<std::string> registeredPullResponsePluginIds;
};

#endif
