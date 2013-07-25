#ifndef SATCOM_CONFIGURATION_MANAGER_H
#define SATCOM_CONFIGURATION_MANAGER_H

#include <vector>

#include <ace/Copy_Disabled.h>

#include "ConfigurationManager.h"

class SatcomConfigurationManager : public ConfigurationManager, public ACE_Copy_Disabled {
public:
  static SatcomConfigurationManager &getInstance();
  
  std::string getListenPort() { return listenPort; }
  
  int getBaudRate() { return baudRate; }

  int getTokenTimeout() { return tokenTimeout; }
  int getDataTimeout() { return dataTimeout; }

  int getPliRelayRangeScale() { return pliRelayRangeScale; }
  int getPliRelayTimeScale() { return pliRelayTimeScale; }

protected:
  void init();
  void decode(const Json::Value& root);
  
private:
  SatcomConfigurationManager();
  
  std::string listenPort;
  
  int baudRate;
  int tokenTimeout;
  int dataTimeout;

  int pliRelayRangeScale;
  int pliRelayTimeScale;
};

#endif //SATCOM_CONFIGURATION_MANAGER_H
