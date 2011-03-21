#ifndef GATEWAY_CONFIGURATION_MANAGER_H
#define GATEWAY_CONFIGURATION_MANAGER_H

#include <string>

class GatewayConfigurationManager {
public:
  static GatewayConfigurationManager* getInstance();
  
  std::string getGatewayAddress();
  std::string getGatewayInterface();
  std::string getCrossGatewayId();
  int getGatewayPort();
  std::string getCrossGatewayServerInterface();
  int getCrossGatewayServerPort();
  std::string getCrossGatewayParentAddress();
  int getCrossGatewayParentPort();
  
private:
  GatewayConfigurationManager();
  
  std::string findConfigFile();
  
  static GatewayConfigurationManager *sharedInstance;
  
  std::string gatewayAddress;
  std::string gatewayInterface;
  int gatewayPort;
  
  std::string crossGatewayId;
  std::string crossGatewayServerInterface;
  int crossGatewayServerPort;
  std::string crossGatewayParentAddress;
  int crossGatewayParentPort;
};

#endif //GATEWAY_CONFIGURATION_MANAGER_H
