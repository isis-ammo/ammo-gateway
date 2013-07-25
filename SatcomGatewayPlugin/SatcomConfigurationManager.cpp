#include "SatcomConfigurationManager.h"
#include "json/value.h"
#include "log.h"
#include <sstream>

static const char* CONFIG_FILE = "SerialPluginConfig.json";

SatcomConfigurationManager &SatcomConfigurationManager::getInstance()
{
  static SatcomConfigurationManager sharedInstance;
  
  return sharedInstance;
}

SatcomConfigurationManager::SatcomConfigurationManager() : ConfigurationManager(CONFIG_FILE)
{
  init();
  populate();
}

void SatcomConfigurationManager::init()
{
#ifdef WIN32
  listenPort = "COM1";
#else
  listenPort = "/dev/ttyUSB0";
#endif

  baudRate = 2400;
  tokenTimeout = 5000;
  dataTimeout = 5000;
  pliRelayRangeScale = 4;
  pliRelayTimeScale = 4;
}

void SatcomConfigurationManager::decode(const Json::Value& root)
{
  CM_DecodeString ( root, "listenPort", listenPort );
  CM_DecodeInt    ( root, "baudRate", baudRate);
  CM_DecodeInt    ( root, "tokenTimeout", tokenTimeout);
  CM_DecodeInt    ( root, "dataTimeout", dataTimeout);
  CM_DecodeInt    ( root, "pliRelayRangeScale", pliRelayRangeScale);
  CM_DecodeInt    ( root, "pliRelayTimeScale", pliRelayTimeScale);

  LOG_INFO("Serial Plugin Configuration: ");
  LOG_INFO("  Listen Port: " << listenPort);
  LOG_INFO("  Baud Rate: " << baudRate);
  LOG_INFO("  Token Timeout: " << tokenTimeout);
  LOG_INFO("  Data Timeout: " << dataTimeout);
  LOG_INFO("  PLI Relay Range Scale: " << pliRelayRangeScale);
  LOG_INFO("  PLI Relay Time Scale: " << pliRelayTimeScale);
}
