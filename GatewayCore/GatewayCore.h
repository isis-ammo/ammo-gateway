#ifndef GATEWAY_CORE_H
#define GATEWAY_CORE_H

#include <map>
#include <set>
#include <string>

#include "NetworkAcceptor.h"
#include "CrossGatewayEventHandler.h"
#include "NetworkEnumerations.h"
#include "protocol/GatewayPrivateMessages.pb.h"

#include "CrossGatewayEventHandler.h"
#include "SubscriptionDataStore.h"
#include "Enumerations.h"

class GatewayEventHandler;
class CrossGatewayConnectionManager;

struct SubscriptionInfo {
  std::string handlerId;
  unsigned int references;
};

struct PullRequestHandlerInfo {
  std::string handlerId;
  unsigned int references;
};

struct LocalSubscriptionInfo {
  GatewayEventHandler *handler;
  MessageScope scope;
};

struct LocalPullHandlerInfo {
  GatewayEventHandler *handler;
  MessageScope scope;
};

class GatewayCore {
public:
  GatewayCore();
  
  static GatewayCore* getInstance();
  
  
  bool registerDataInterest(std::string mime_type, MessageScope messageScope,  GatewayEventHandler *handler);
  bool unregisterDataInterest(std::string mime_type, MessageScope messageScope, GatewayEventHandler *handler);
  
  bool registerPullInterest(std::string mime_type, MessageScope scope, GatewayEventHandler *handler);
  bool unregisterPullInterest(std::string mime_type, MessageScope scope, GatewayEventHandler *handler);
  
  bool pushData(GatewayEventHandler *sender, std::string uri, std::string mimeType, std::string encoding, const std::string &data, std::string originUser, std::string originDevice, MessageScope messageScope, bool ackDeviceDelivered, bool ackPluginDelivered, char priority);
  
  bool pushAcknowledgement(GatewayEventHandler *sender, std::string uid, std::string destinationDevice, std::string acknowledgingDevice, std::string destinationUser, std::string acknowledgingUser, bool deviceDelivered, bool pluginDelivered, PushStatus status);
  
  bool pullRequest(GatewayEventHandler *sender, std::string requestUid, std::string pluginId, std::string mimeType, std::string query, std::string projection,
                   unsigned int maxResults, unsigned int startFromCount, bool liveQuery, MessageScope scope, char priority);
  bool pullResponse(std::string requestUid, std::string pluginId, std::string mimeType, std::string uri, std::string encoding, const std::string &data, char priority);
  
  bool unregisterPullResponsePluginId(std::string pluginId, GatewayEventHandler *handler);
  
  //Methods for cross-gateway communication
  void initCrossGateway();
  
  void setParentHandler(CrossGatewayEventHandler *handler);
  
  bool registerCrossGatewayConnection(std::string handlerId, CrossGatewayEventHandler *handler);
  bool unregisterCrossGatewayConnection(std::string handlerId);
  
  bool subscribeCrossGateway(std::string mimeType, std::string originHandlerId);
  bool unsubscribeCrossGateway(std::string mimeType, std::string originHandlerId);
  
  bool registerPullInterestCrossGateway(std::string mimeType, std::string originHandlerId);
  bool unregisterPullInterestCrossGateway(std::string mimeType, std::string originHandlerId);
  
  bool pushCrossGateway(std::string uri, std::string mimeType, std::string encoding, const std::string &data, std::string originUser, std::string originHandlerId, char priority);
  bool pullRequestCrossGateway(std::string requestUid, std::string pluginId, std::string mimeType, std::string query, std::string projection, unsigned int maxResults, unsigned int startFromCount, bool liveQuery, std::string originHandlerId, char priority);
  bool pullResponseCrossGateway(std::string requestUid, std::string pluginId, std::string mimeType, std::string uri, std::string encoding, const std::string &data, std::string originHandlerId, char priority);
  
  bool unregisterPullResponsePluginIdCrossGateway(std::string pluginId, std::string handler);
  
  void terminate();
  
  virtual ~GatewayCore();
  
private:
  std::set<GatewayEventHandler *> getPushHandlersForType(std::string mimeType);
  
  static GatewayCore* sharedInstance;
  
  typedef std::multimap<std::string, LocalSubscriptionInfo> PushHandlerMap;
  PushHandlerMap pushHandlers;
  
  typedef std::multimap<std::string, LocalPullHandlerInfo> PullHandlerMap;
  PullHandlerMap pullHandlers;
  
  std::map<std::string, GatewayEventHandler *> plugins;
  
  std::map<std::string, CrossGatewayEventHandler *> crossGatewayHandlers;
  SubscriptionDataStore crossGatewaySubscriptions;
  
  typedef std::multimap<std::string, PullRequestHandlerInfo> CrossGatewayPullRequestHandlerMap;
  CrossGatewayPullRequestHandlerMap crossGatewayPullRequestHandlers;
  
  typedef std::map<std::string, std::string> PullRequestReturnIdMap;
  PullRequestReturnIdMap cgPullRequestReturnIds;

  CrossGatewayConnectionManager *connectionManager;
  CrossGatewayEventHandler *parentHandler;
  
  ammo::gateway::internal::NetworkAcceptor<ammo::gateway::protocol::GatewayWrapper, CrossGatewayEventHandler, ammo::gateway::internal::SYNC_MULTITHREADED, 0x8badf00d> *crossGatewayAcceptor;
};

#endif //#ifndef GATEWAY_CORE_H
