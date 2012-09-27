#include "SerialConfigurationManager.h"
#include "json/value.h"
#include "log.h"
#include <sstream>

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
  std::string listenPort;

#ifdef WIN32
  listenPort = "COM1";
#else
  listenPort = "/dev/ttyUSB0";
#endif

  listenPorts.clear();
  listenPorts.push_back(listenPort);

#ifdef WIN32
  gpsPort = "COM2";
#else
  gpsPort = "/dev/ttyUSB1";
#endif

  sendEnabled = false;

  baudRate = 9600;

  slotDuration = 750;
  slotNumber = 1;
  numberOfSlots = 16;
  transmitDuration = 500;
  gpsTimeOffset = 0;

  pliRelayPerCycle = 4;

  pliRelayEnabled = true;
  pliSendFrequency = 2; //every other slot
}

void SerialConfigurationManager::decode(const Json::Value& root)
{
  std::string listenPort;

  listenPorts.clear();

  CM_DecodeString ( root, "listenPort", listenPort );
  listenPorts.push_back(listenPort);
  CM_DecodeString ( root, "gpsPort", gpsPort);
  CM_DecodeBool   ( root, "sendEnabled", sendEnabled);
  CM_DecodeInt    ( root, "baudRate", baudRate);
  CM_DecodeInt    ( root, "slotDuration", slotDuration);
  CM_DecodeInt    ( root, "slotNumber", slotNumber);
  CM_DecodeInt    ( root, "numberOfSlots", numberOfSlots);
  CM_DecodeInt    ( root, "transmitDuration", transmitDuration);
  CM_DecodeInt    ( root, "gpsTimeOffset", gpsTimeOffset);

  CM_DecodeInt    ( root, "pliRelayPerCycle", pliRelayPerCycle);

  CM_DecodeBool    ( root, "pliRelayEnabled", pliRelayEnabled);
  CM_DecodeInt    ( root, "pliSendFrequency", pliSendFrequency);

  bool findMorePorts = true;
  for (int i = 1; findMorePorts; i++) {
    std::stringstream ss;
    ss << "listenPort" << i;
    std::string portKey = ss.str();

	if(!root[portKey].isString()) {
      findMorePorts = false;
      continue;
	}

    listenPort = root[portKey].asString();
	listenPorts.push_back(listenPort);
  }

  LOG_INFO("Serial Plugin Configuration: ");
  LOG_INFO("  Listen Port: " << listenPorts[0]);
  for (int i = 1; i < listenPorts.size(); i++) {
    std::stringstream ss;
    ss << "listenPort" << i;
    std::string portKey = ss.str();
	LOG_INFO("  Listen Port " << i << ": " << listenPorts[i]);
  }
  LOG_INFO("  GPS Port: " << gpsPort);
  LOG_INFO("  Send Enabled: " << sendEnabled);
  LOG_INFO("  Baud Rate: " << baudRate);
  LOG_INFO("  Slot Duration: " << slotDuration);
  LOG_INFO("  Slot Number: " << slotNumber);
  LOG_INFO("  Number Of Slots: " << numberOfSlots);
  LOG_INFO("  Transmit Duration: " << transmitDuration);
  LOG_INFO("  GPS Time Offset: " << gpsTimeOffset);
  LOG_INFO("  Pli Relay Per Cycle: " << pliRelayPerCycle);
}
