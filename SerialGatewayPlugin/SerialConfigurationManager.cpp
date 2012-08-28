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

#ifdef WIN32
  gpsPort = "COM2";
#else
  gpsPort = "/dev/ttyUSB1";
#endif

  slotDuration = 750;
  slotNumber = 1;
  numberOfSlots = 16;
  transmitDuration = 500;
  gpsTimeOffset = 0;
}

void SerialConfigurationManager::decode(const Json::Value& root)
{
  CM_DecodeString ( root, "listenPort", listenPort );
  CM_DecodeString ( root, "gpsPort", gpsPort);
  CM_DecodeInt    ( root, "slotDuration", slotDuration);
  CM_DecodeInt    ( root, "slotNumber", slotNumber);
  CM_DecodeInt    ( root, "numberOfSlots", numberOfSlots);
  CM_DecodeInt    ( root, "transmitDuration", transmitDuration);
  CM_DecodeInt    ( root, "gpsTimeOffset", gpsTimeOffset);

  LOG_INFO("Serial Plugin Configuration: ");
  LOG_INFO("  Listen Port: " << listenPort);
  LOG_INFO("  GPS Port: " << gpsPort);
  LOG_INFO("  Slot Duration: " << slotDuration);
  LOG_INFO("  Slot Number: " << slotNumber);
  LOG_INFO("  Number Of Slots: " << numberOfSlots);
  LOG_INFO("  Transmit Duration: " << transmitDuration);
  LOG_INFO("  GPS Time Offset: " << gpsTimeOffset);
}
