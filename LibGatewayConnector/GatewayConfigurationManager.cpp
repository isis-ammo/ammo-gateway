#include "GatewayConfigurationManager.h"
#include "json/value.h"
#include "log.h"

using namespace std;

const char *CONFIG_FILE = "GatewayConfig.json";

ammo::gateway::internal::GatewayConfigurationManager *ammo::gateway::internal::GatewayConfigurationManager::sharedInstance = NULL;

ammo::gateway::internal::GatewayConfigurationManager::GatewayConfigurationManager(const char* configFileName) : ConfigurationManager(configFileName) {
  init();
}

void ammo::gateway::internal::GatewayConfigurationManager::init()
{
  gatewayAddress = "127.0.0.1";
  gatewayInterface = "0.0.0.0";
  gatewayPort = 12475;
}

void ammo::gateway::internal::GatewayConfigurationManager::decode(const Json::Value& root)
{
  CM_DecodeString( root, "GatewayInterface", gatewayInterface );
  CM_DecodeString( root, "GatewayAddress",   gatewayAddress   );
  CM_DecodeInt   ( root, "GatewayPort",      gatewayPort      );

  LOG_INFO("Gateway Configuration: ");
  LOG_INFO("  Interface: " << gatewayInterface);
  LOG_INFO("  Address: " << gatewayAddress);
  LOG_INFO("  Port: " << gatewayPort);
}

std::string ammo::gateway::internal::GatewayConfigurationManager::getGatewayAddress() {
  return gatewayAddress;
}

std::string ammo::gateway::internal::GatewayConfigurationManager::getGatewayInterface() {
  return gatewayInterface;
}

int ammo::gateway::internal::GatewayConfigurationManager::getGatewayPort() {
  return gatewayPort;
}

ammo::gateway::internal::GatewayConfigurationManager* ammo::gateway::internal::GatewayConfigurationManager::getInstance() {
  if(sharedInstance == NULL) {
    sharedInstance = new GatewayConfigurationManager(CONFIG_FILE);
	sharedInstance->populate();
  }
  return sharedInstance;
}
ammo::gateway::internal::GatewayConfigurationManager* ammo::gateway::internal::GatewayConfigurationManager::getInstance(std::string configfile) {
  if(sharedInstance == NULL) {
    sharedInstance = new GatewayConfigurationManager(configfile.c_str());
	sharedInstance->populate();
  }
  return sharedInstance;
}
