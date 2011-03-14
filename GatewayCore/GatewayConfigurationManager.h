#ifndef GATEWAY_CONFIGURATION_MANAGER_H
#define GATEWAY_CONFIGURATION_MANAGER_H

#include <string>

class GatewayConfigurationManager {
public:
  static GatewayConfigurationManager* getInstance();
  
  std::string getGatewayAddress();
  std::string getGatewayInterface();
  int getGatewayPort();
private:
  GatewayConfigurationManager();
  
  std::string findConfigFile();
  
  static GatewayConfigurationManager *sharedInstance;
  
  std::string gatewayAddress;
  std::string gatewayInterface;
  int gatewayPort;
};

#endif //GATEWAY_CONFIGURATION_MANAGER_H
