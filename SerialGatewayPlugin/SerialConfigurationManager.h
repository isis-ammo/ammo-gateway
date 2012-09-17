#ifndef SERIAL_CONFIGURATION_MANAGER_H
#define SERIAL_CONFIGURATION_MANAGER_H

#include "ConfigurationManager.h"

class SerialConfigurationManager : public ConfigurationManager {
public:
  static SerialConfigurationManager* getInstance();
  
  std::string getListenPort() { return listenPort; }
  std::string getGpsPort() { return gpsPort; }

  bool getSendEnabled() { return sendEnabled; }

  int getSlotDuration() { return slotDuration; }
  int getSlotNumber() { return slotNumber; }
  int getNumberOfSlots() { return numberOfSlots; }
  int getTransmitDuration() { return transmitDuration; }
  int getGpsTimeOffset() { return gpsTimeOffset; }

protected:
  void init();
  void decode(const Json::Value& root);
  
private:
  SerialConfigurationManager();
  
  static SerialConfigurationManager *sharedInstance;
  std::string listenPort;
  std::string gpsPort;

  bool sendEnabled;

  int slotDuration;
  int slotNumber;
  int numberOfSlots;
  int transmitDuration;

  int gpsTimeOffset;
};

#endif //SERIAL_CONFIGURATION_MANAGER_H
