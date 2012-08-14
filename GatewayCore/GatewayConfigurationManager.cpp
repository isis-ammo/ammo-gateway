#include "GatewayConfigurationManager.h"
#include "json/value.h"
#include "log.h"

static const char *CONFIG_FILE = "GatewayConfig.json";

using namespace std;

GatewayConfigurationManager *GatewayConfigurationManager::sharedInstance = NULL;

GatewayConfigurationManager::GatewayConfigurationManager() : ConfigurationManager(CONFIG_FILE) {
  init();
}

void GatewayConfigurationManager::init()
{
  gatewayAddress = "127.0.0.1";
  gatewayInterface = "0.0.0.0";
  gatewayPort = 12475;
  crossGatewayId = "DefaultGateway";
  crossGatewayServerInterface = "127.0.0.1";
  crossGatewayServerPort = 47543;
  crossGatewayParentAddress = "";
  crossGatewayParentPort = 47543;
}

void GatewayConfigurationManager::decode(const Json::Value& root)
{
  CM_DecodeString ( root, "GatewayInterface",             gatewayInterface            );
  CM_DecodeString ( root, "GatewayAddress",               gatewayAddress              );
  CM_DecodeInt    ( root, "GatewayPort",                  gatewayPort                 );
  CM_DecodeString ( root, "CrossGatewayId",               crossGatewayId              );
  CM_DecodeString ( root, "CrossGatewayServerInterface",  crossGatewayServerInterface );
  CM_DecodeInt    ( root, "CrossGatewayServerPort",       crossGatewayServerPort      );
  CM_DecodeString ( root, "CrossGatewayParentAddress",    crossGatewayParentAddress   );
  CM_DecodeInt    ( root, "CrossGatewayParentPort",       crossGatewayParentPort      );

  LOG_INFO("Gateway Configuration: ");
  LOG_INFO("  Interface: " << gatewayInterface);
  LOG_INFO("  Address: " << gatewayAddress);
  LOG_INFO("  Port: " << gatewayPort);
  LOG_INFO("  CrossGatewayId: " << crossGatewayId);
  LOG_INFO("  CrossGatewayServerInterface: " << crossGatewayServerInterface);
  LOG_INFO("  CrossGatewayServerPort: " << crossGatewayServerPort);
  LOG_INFO("  CrossGatewayParentAddress: " << crossGatewayParentAddress);
  LOG_INFO("  CrossGatewayParentPort: " << crossGatewayParentPort);
}

std::string GatewayConfigurationManager::getGatewayAddress() {
  return gatewayAddress;
}

std::string GatewayConfigurationManager::getGatewayInterface() {
  return gatewayInterface;
}

std::string GatewayConfigurationManager::getCrossGatewayId() {
  return crossGatewayId;
}

int GatewayConfigurationManager::getGatewayPort() {
  return gatewayPort;
}

std::string GatewayConfigurationManager::getCrossGatewayServerInterface() {
  return crossGatewayServerInterface;
}

int GatewayConfigurationManager::getCrossGatewayServerPort() {
  return crossGatewayServerPort;
}

std::string GatewayConfigurationManager::getCrossGatewayParentAddress() {
  return crossGatewayParentAddress;
}

int GatewayConfigurationManager::getCrossGatewayParentPort() {
  return crossGatewayParentPort;
}

GatewayConfigurationManager* GatewayConfigurationManager::getInstance() {
  if(sharedInstance == NULL) {
    sharedInstance = new GatewayConfigurationManager();
	sharedInstance->populate();
  }
  return sharedInstance;
}
