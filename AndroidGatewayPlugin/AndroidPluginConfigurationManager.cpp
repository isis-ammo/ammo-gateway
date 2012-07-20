#include "AndroidPluginConfigurationManager.h"
#include "json/value.h"
#include "log.h"

using namespace std;

const char *CONFIG_FILE = "AndroidPluginConfig.json";

AndroidPluginConfigurationManager *AndroidPluginConfigurationManager::sharedInstance = NULL;

AndroidPluginConfigurationManager::AndroidPluginConfigurationManager(const char* configFileName) : ConfigurationManager(configFileName) {
  init();
}

void AndroidPluginConfigurationManager::init()
{
  heartbeatTimeout = 100;
}

void AndroidPluginConfigurationManager::decode(const Json::Value& root)
{
  //TODO:  This really ought to be reading a uint, but LibJSON apparently isn't
  //       handling uints in any kind of a sane, predictable way.
  CM_DecodeInt( root, "HeartbeatTimeout", heartbeatTimeout);

  LOG_INFO("Android Plugin Configuration: ");
  LOG_INFO("  Heartbeat Timeout: " << heartbeatTimeout);
}

unsigned int AndroidPluginConfigurationManager::getHeartbeatTimeout() {
  return heartbeatTimeout;
}

AndroidPluginConfigurationManager* AndroidPluginConfigurationManager::getInstance() {
  if(sharedInstance == NULL) {
    sharedInstance = new AndroidPluginConfigurationManager(CONFIG_FILE);
    sharedInstance->populate();
  }
  return sharedInstance;
}
AndroidPluginConfigurationManager* AndroidPluginConfigurationManager::getInstance(std::string configfile) {
  if(sharedInstance == NULL) {
    sharedInstance = new AndroidPluginConfigurationManager(configfile.c_str());
    sharedInstance->populate();
  }
  return sharedInstance;
}
