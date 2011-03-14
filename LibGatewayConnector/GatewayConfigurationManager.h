#ifndef GATEWAY_CONFIGURATION_MANAGER_H
#define GATEWAY_CONFIGURATION_MANAGER_H

#include <string>


class GatewayConfigurationManager {
public:
  static GatewayConfigurationManager* getInstance();
  static GatewayConfigurationManager* getInstance(std::string configfile);
  
  std::string getGatewayAddress();
  std::string getGatewayInterface();
  int getGatewayPort();
private:
  GatewayConfigurationManager(const char *configfile);
  
  std::string findConfigFile(std::string defaultConfigFile);
  
  static GatewayConfigurationManager *sharedInstance;
  
  std::string gatewayAddress;
  std::string gatewayInterface;
  int gatewayPort;
};

#endif //GATEWAY_CONFIGURATION_MANAGER_H
