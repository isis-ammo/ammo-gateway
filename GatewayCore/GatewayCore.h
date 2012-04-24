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

struct PluginInfo {
  GatewayEventHandler *handler;
  std::string pluginName;
};

class GatewayCore {
public:
  GatewayCore();
  
  static GatewayCore* getInstance();
  
  bool registerPlugin(std::string pluginId, std::string instanceId, GatewayEventHandler *handler);
  bool unregisterPlugin(std::string pluginId, std::string instanceId, GatewayEventHandler *handler);
  
  bool registerDataInterest(std::string mime_type, MessageScope messageScope,  GatewayEventHandler *handler);
  bool unregisterDataInterest(std::string mime_type, MessageScope messageScope, GatewayEventHandler *handler);
  
  bool registerPullInterest(std::string mime_type, MessageScope scope, GatewayEventHandler *handler);
  bool unregisterPullInterest(std::string mime_type, MessageScope scope, GatewayEventHandler *handler);
  
  bool pushData(GatewayEventHandler *sender, std::string uid, std::string mimeType, std::string encoding, const std::string &data, std::string originUser, std::string originDevice, MessageScope messageScope, bool ackDeviceDelivered, bool ackPluginDelivered, char priority);
  
  bool pullRequest(GatewayEventHandler *sender, std::string requestUid, std::string pluginId, std::string mimeType, std::string query, std::string projection,
                   unsigned int maxResults, unsigned int startFromCount, bool liveQuery, MessageScope scope, char priority);
  bool pullResponse(std::string requestUid, std::string pluginId, std::string mimeType, std::string uid, std::string encoding, const std::string &data, char priority);
  
  bool pointToPointMessage(GatewayEventHandler *sender, std::string uid, std::string destinationGateway, std::string destinationPluginName, std::string destinationInstanceId, std::string sourcePluginName, std::string sourceInstanceId, std::string mimeType, std::string encoding, std::string data, char priority);
  
  bool unregisterPullResponsePluginId(std::string pluginId, GatewayEventHandler *handler);
  
  //Methods for cross-gateway communication
  void initCrossGateway();
  
  void setParentHandler(CrossGatewayEventHandler *handler);
  
  bool registerCrossGatewayConnection(std::string handlerId, CrossGatewayEventHandler *handler, std::vector<std::pair<std::string, std::string> >);
  bool unregisterCrossGatewayConnection(std::string handlerId);
  
  bool pluginConnectedCrossGateway(std::string gatewayId, std::string localHandlerId, std::string pluginName, std::string instanceId);
  
  bool gatewayConnectedCrossGateway(std::string gatewayId, std::string localHandlerId, std::vector<std::pair<std::string, std::string> > connectedPlugins);
  bool gatewayDisconnectedCrossGateway(std::string gatewayId, std::string localHandlerId);
  
  bool subscribeCrossGateway(std::string mimeType, std::string originHandlerId);
  bool unsubscribeCrossGateway(std::string mimeType, std::string originHandlerId);
  
  bool registerPullInterestCrossGateway(std::string mimeType, std::string originHandlerId);
  bool unregisterPullInterestCrossGateway(std::string mimeType, std::string originHandlerId);
  
  bool pushCrossGateway(std::string uid, std::string mimeType, std::string encoding, const std::string &data, std::string originUser, std::string originHandlerId, char priority);
  bool pullRequestCrossGateway(std::string requestUid, std::string pluginId, std::string mimeType, std::string query, std::string projection, unsigned int maxResults, unsigned int startFromCount, bool liveQuery, std::string originHandlerId, char priority);
  bool pullResponseCrossGateway(std::string requestUid, std::string pluginId, std::string mimeType, std::string uri, std::string encoding, const std::string &data, std::string originHandlerId, char priority);  
  
  bool pointToPointMessageCrossGateway(std::string originHandlerId, std::string uid, std::string destinationGateway, std::string destinationPluginName, std::string destinationInstanceId, std::string sourceGateway, std::string sourcePluginName, std::string sourceInstanceId, std::string mimeType, std::string encoding, std::string data, char priority);
  
  bool unregisterPullResponsePluginIdCrossGateway(std::string pluginId, std::string handler);
  
  std::vector<std::pair<std::string, std::string> > getLocalPlugins();
  
  void terminate();
  
  virtual ~GatewayCore();
  
private:
  std::set<GatewayEventHandler *> getPushHandlersForType(std::string mimeType);
  
  std::string localGatewayId;
  
  static GatewayCore* sharedInstance;
  
  typedef std::map<std::string, PluginInfo> PluginInstanceMap; //key:  plugin instance ID
  PluginInstanceMap pluginInstances;
  
  typedef std::multimap<std::string, LocalSubscriptionInfo> PushHandlerMap;
  PushHandlerMap pushHandlers;
  
  typedef std::multimap<std::string, LocalPullHandlerInfo> PullHandlerMap;
  PullHandlerMap pullHandlers;
  
  std::map<std::string, GatewayEventHandler *> plugins; //for pull requests (don't currently use authenticated plugin name)
  
  typedef std::map<std::string, CrossGatewayEventHandler *> CrossGatewayHandlerMap;
  CrossGatewayHandlerMap crossGatewayHandlers;
  
  typedef std::map<std::string, std::string> GatewayRouteMap;
  GatewayRouteMap gatewayRoutes; //key: gateway ID, value: handler ID through which that gateway is accessible
  
  typedef std::multimap<std::string, SubscriptionInfo> CrossGatewaySubscriptionMap;
  CrossGatewaySubscriptionMap subscriptions;
  
  typedef std::multimap<std::string, PullRequestHandlerInfo> CrossGatewayPullRequestHandlerMap;
  CrossGatewayPullRequestHandlerMap crossGatewayPullRequestHandlers;
  
  typedef std::map<std::string, std::string> PullRequestReturnIdMap;
  PullRequestReturnIdMap cgPullRequestReturnIds;

  CrossGatewayConnectionManager *connectionManager;
  CrossGatewayEventHandler *parentHandler;
  
  ammo::gateway::internal::NetworkAcceptor<ammo::gateway::protocol::GatewayWrapper, CrossGatewayEventHandler, ammo::gateway::internal::SYNC_MULTITHREADED, 0x8badf00d> *crossGatewayAcceptor;
};

#endif //#ifndef GATEWAY_CORE_H
