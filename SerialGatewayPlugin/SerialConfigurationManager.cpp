#include "SerialConfigurationManager.h"
#include "json/value.h"
#include "log.h"

static const char* CONFIG_FILE = "SerialPluginConfig.json";

SerialConfigurationManager* SerialConfigurationManager::sharedInstance = NULL;

SerialConfigurationManager* SerialConfigurationManager::getInstance()
{
  if (!sharedInstance) {
    sharedInstance = new SerialConfigurationManager();
	sharedInstance->populate();
  }

  return sharedInstance;
}

SerialConfigurationManager::SerialConfigurationManager() : ConfigurationManager(CONFIG_FILE)
{
  init();
}

void SerialConfigurationManager::init()
{
#ifdef WIN32
  listenPort = "COM1";
#else
  listenPort = "/dev/ttyUSB0";
#endif
}

void SerialConfigurationManager::decode(const Json::Value& root)
{
  CM_DecodeString ( root, "listenPort", listenPort );

  LOG_INFO("Serial Plugin Configuration: ");
  LOG_INFO("  Listen Port: " << listenPort);
}