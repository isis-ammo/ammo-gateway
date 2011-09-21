#ifndef GATEWAY_CORE_H
#define GATEWAY_CORE_H

#include <map>
#include <set>
#include <string>

#include "ace/INET_Addr.h"
#include "ace/SOCK_Connector.h"
#include "ace/SOCK_Stream.h"
#include "ace/SOCK_Acceptor.h"

#include "ace/Acceptor.h"
#include "ace/Connector.h"
#include "ace/Reactor.h"
#include "CrossGatewayServiceHandler.h"
#include "Enumerations.h"

class GatewayServiceHandler;

struct SubscriptionInfo {
  std::string handlerId;
  unsigned int references;
};

struct PullRequestHandlerInfo {
  std::string handlerId;
  unsigned int references;
};

struct LocalSubscriptionInfo {
  GatewayServiceHandler *handler;
  MessageScope scope;
};

struct LocalPullHandlerInfo {
  GatewayServiceHandler *handler;
  MessageScope scope;
};

class GatewayCore {
public:
  GatewayCore();
  
  static GatewayCore* getInstance();
  
  bool registerDataInterest(std::string mime_type, MessageScope messageScope,  GatewayServiceHandler *handler);
  bool unregisterDataInterest(std::string mime_type, MessageScope messageScope, GatewayServiceHandler *handler);
  
  bool registerPullInterest(std::string mime_type, MessageScope scope, GatewayServiceHandler *handler);
  bool unregisterPullInterest(std::string mime_type, MessageScope scope, GatewayServiceHandler *handler);
  
  bool pushData(std::string uri, std::string mimeType, std::string encoding, const std::string &data, std::string originUser, MessageScope messageScope);
  
  bool pullRequest(std::string requestUid, std::string pluginId, std::string mimeType, std::string query, std::string projection,
                   unsigned int maxResults, unsigned int startFromCount, bool liveQuery, MessageScope scope, GatewayServiceHandler *originatingPlugin);
  bool pullResponse(std::string requestUid, std::string pluginId, std::string mimeType, std::string uri, std::string encoding, const std::string &data);
  
  bool unregisterPullResponsePluginId(std::string pluginId, GatewayServiceHandler *handler);
  
  //Methods for cross-gateway communication
  void initCrossGateway();
  
  bool registerCrossGatewayConnection(std::string handlerId, CrossGatewayServiceHandler *handler);
  bool unregisterCrossGatewayConnection(std::string handlerId);
  
  bool subscribeCrossGateway(std::string mimeType, std::string originHandlerId);
  bool unsubscribeCrossGateway(std::string mimeType, std::string originHandlerId);
  
  bool registerPullInterestCrossGateway(std::string mimeType, std::string originHandlerId);
  bool unregisterPullInterestCrossGateway(std::string mimeType, std::string originHandlerId);
  
  bool pushCrossGateway(std::string uri, std::string mimeType, std::string encoding, const std::string &data, std::string originUser, std::string originHandlerId);
  bool pullRequestCrossGateway(std::string requestUid, std::string pluginId, std::string mimeType, std::string query, std::string projection, unsigned int maxResults, unsigned int startFromCount, bool liveQuery, std::string originHandlerId);
  bool pullResponseCrossGateway(std::string requestUid, std::string pluginId, std::string mimeType, std::string uri, std::string encoding, const std::string &data, std::string originHandlerId);
  
  bool unregisterPullResponsePluginIdCrossGateway(std::string pluginId, std::string handler);
  
private:
  std::set<GatewayServiceHandler *> getPushHandlersForType(std::string mimeType);
  
  static GatewayCore* sharedInstance;
  
  typedef std::multimap<std::string, LocalSubscriptionInfo> PushHandlerMap;
  PushHandlerMap pushHandlers;
  
  typedef std::multimap<std::string, LocalPullHandlerInfo> PullHandlerMap;
  PullHandlerMap pullHandlers;
  
  std::map<std::string, GatewayServiceHandler *> plugins;
  
  std::map<std::string, CrossGatewayServiceHandler *> crossGatewayHandlers;
  std::multimap<std::string, SubscriptionInfo> subscriptions;
  
  typedef std::multimap<std::string, PullRequestHandlerInfo> CrossGatewayPullRequestHandlerMap;
  CrossGatewayPullRequestHandlerMap crossGatewayPullRequestHandlers;
  
  typedef std::map<std::string, std::string> PullRequestReturnIdMap;
  PullRequestReturnIdMap cgPullRequestReturnIds;
  
  ACE_Connector<CrossGatewayServiceHandler, ACE_SOCK_Connector> *parentConnector;
  CrossGatewayServiceHandler *parentHandler;
  
  ACE_Acceptor<CrossGatewayServiceHandler, ACE_SOCK_Acceptor> *crossGatewayAcceptor;
};

#endif //#ifndef GATEWAY_CORE_H
