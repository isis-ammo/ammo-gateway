#ifndef GATEWAY_CORE_H
#define GATEWAY_CORE_H

#include <map>
#include <string>

class GatewayServiceHandler;
class CrossGatewayServiceHandler;

typedef struct _subscription_info {
  std::string handlerId;
  unsigned int references;
} SubscriptionInfo;

class GatewayCore {
public:
  static GatewayCore* getInstance();
  
  bool registerDataInterest(std::string mime_type, GatewayServiceHandler *handler);
  bool unregisterDataInterest(std::string mime_type, GatewayServiceHandler *handler);
  
  bool registerPullInterest(std::string mime_type, GatewayServiceHandler *handler);
  bool unregisterPullInterest(std::string mime_type, GatewayServiceHandler *handler);
  
  bool pushData(std::string uri, std::string mimeType, const std::string &data, std::string originUser);
  
  bool pullRequest(std::string requestUid, std::string pluginId, std::string mimeType, std::string query, std::string projection,
                   unsigned int maxResults, unsigned int startFromCount, bool liveQuery, GatewayServiceHandler *originatingPlugin);
  bool pullResponse(std::string requestUid, std::string pluginId, std::string mimeType, std::string uri, const std::string &data);
  
  //Methods for cross-gateway communication
  void initCrossGateway();
  
  bool registerCrossGatewayConnection(std::string handlerId, CrossGatewayServiceHandler *handler);
  bool unregisterCrossGatewayConnection(std::string handlerId);
  
  bool subscribeCrossGateway(std::string mimeType, std::string originHandlerId);
  bool unsubscribeCrossGateway(std::string mimeType, std::string originHandlerId);
  
  bool pushCrossGateway(std::string uri, std::string mimeType, const std::string &data, std::string originUser, std::string originHandlerId);
  
private:
  static GatewayCore* sharedInstance;
  
  std::multimap<std::string, GatewayServiceHandler *> pushHandlers;
  std::multimap<std::string, GatewayServiceHandler *> pullHandlers;
  
  std::map<std::string, GatewayServiceHandler *> plugins;
  
  std::map<std::string, CrossGatewayServiceHandler *> crossGatewayHandlers;
  std::multimap<std::string, SubscriptionInfo> subscriptions;
};

#endif //#ifndef GATEWAY_CORE_H
