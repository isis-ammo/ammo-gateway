#ifndef GATEWAY_CORE_H
#define GATEWAY_CORE_H

#include <map>
#include <string>

class GatewayServiceHandler;

struct LocalSubscriptionInfo {
  GatewayServiceHandler *handler;
  unsigned int references;
};

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
  
private:
  static GatewayCore* sharedInstance;
  
  std::multimap<std::string, LocalSubscriptionInfo> pushHandlers;
  std::multimap<std::string, GatewayServiceHandler *> pullHandlers;
  
  std::map<std::string, GatewayServiceHandler *> plugins;
};

#endif //#ifndef GATEWAY_CORE_H
