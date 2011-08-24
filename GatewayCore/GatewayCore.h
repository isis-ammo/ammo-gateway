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

struct LocalSubscriptionInfo {
  GatewayServiceHandler *handler;
  MessageScope scope;
};

class GatewayCore {
public:
  GatewayCore();
  
  static GatewayCore* getInstance();
  
  bool registerDataInterest(std::string mime_type, MessageScope messageScope,  GatewayServiceHandler *handler);
  bool unregisterDataInterest(std::string mime_type, MessageScope messageScope, GatewayServiceHandler *handler);
  
  bool registerPullInterest(std::string mime_type, GatewayServiceHandler *handler);
  bool unregisterPullInterest(std::string mime_type, GatewayServiceHandler *handler);
  
  bool pushData(std::string uri, std::string mimeType, const std::string &data, std::string originUser, MessageScope messageScope);
  
  bool pullRequest(std::string requestUid, std::string pluginId, std::string mimeType, std::string query, std::string projection,
                   unsigned int maxResults, unsigned int startFromCount, bool liveQuery, GatewayServiceHandler *originatingPlugin);
  bool pullResponse(std::string requestUid, std::string pluginId, std::string mimeType, std::string uri, const std::string &data);
  bool unregisterPullResponsePluginId(std::string pluginId, GatewayServiceHandler *handler);
  
  bool directedMessage(const std::string &uri, const std::string &destinationUser, const std::string &mimeType, const std::string &data, const std::string &originUser, MessageScope messageScope);
  
  //shouldn't be called until a user is fully authenticated (they'll get directed messages)
  bool registerUser(std::string &username, GatewayServiceHandler *handler);
  bool unregisterUser(std::string &username, GatewayServiceHandler *handler);
  
  //Methods for cross-gateway communication
  void initCrossGateway();
  
  bool registerCrossGatewayConnection(std::string handlerId, CrossGatewayServiceHandler *handler);
  bool unregisterCrossGatewayConnection(std::string handlerId);
  
  bool subscribeCrossGateway(std::string mimeType, std::string originHandlerId);
  bool unsubscribeCrossGateway(std::string mimeType, std::string originHandlerId);
  
  bool pushCrossGateway(std::string uri, std::string mimeType, const std::string &data, std::string originUser, std::string originHandlerId);
  
private:
  std::set<GatewayServiceHandler *> getPushHandlersForType(std::string mimeType);
  
  static GatewayCore* sharedInstance;
  
  typedef std::multimap<std::string, LocalSubscriptionInfo> PushHandlerMap;
  PushHandlerMap pushHandlers;
  
  std::multimap<std::string, GatewayServiceHandler *> pullHandlers;
  
  std::map<std::string, GatewayServiceHandler *> plugins;
  
  typedef std::map<std::string, GatewayServiceHandler *> UserMap;
  UserMap authenticatedUsers;
  
  std::map<std::string, CrossGatewayServiceHandler *> crossGatewayHandlers;
  std::multimap<std::string, SubscriptionInfo> subscriptions;
  
  ACE_Connector<CrossGatewayServiceHandler, ACE_SOCK_Connector> *parentConnector;
  CrossGatewayServiceHandler *parentHandler;
  
  ACE_Acceptor<CrossGatewayServiceHandler, ACE_SOCK_Acceptor> *crossGatewayAcceptor;
};

#endif //#ifndef GATEWAY_CORE_H
