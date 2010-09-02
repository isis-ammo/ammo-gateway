#ifndef GATEWAY_CORE_H
#define GATEWAY_CORE_H

#include <map>
#include <string>

class GatewayServiceHandler;

class GatewayCore {
public:
  static GatewayCore* getInstance();
  
  bool registerDataInterest(std::string uri, GatewayServiceHandler *handler);
  bool unregisterDataInterest(std::string uri, GatewayServiceHandler *handler);
  
  bool pushData(std::string uri, std::string mimeType, const std::string &data);
  
private:
  static GatewayCore* sharedInstance;
  
  std::multimap<std::string, GatewayServiceHandler *> pushHandlers;
};

#endif //#ifndef GATEWAY_CORE_H
