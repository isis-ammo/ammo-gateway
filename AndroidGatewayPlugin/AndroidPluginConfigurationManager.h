#ifndef ANDROID_PLUGIN_CONFIGURATION_MANAGER_H
#define ANDROID_PLUGIN_CONFIGURATION_MANAGER_H

#include "ConfigurationManager.h"


class AndroidPluginConfigurationManager : public ConfigurationManager {
public:
  static AndroidPluginConfigurationManager* getInstance();
  static AndroidPluginConfigurationManager* getInstance(std::string configfile);
  
  unsigned int getHeartbeatTimeout();

protected:
  void init();
  void decode(const Json::Value& root);

private:
  AndroidPluginConfigurationManager(const char *configfile);
  
  static AndroidPluginConfigurationManager *sharedInstance;
  
  unsigned int heartbeatTimeout;
};


#endif //ANDROID_PLUGIN_CONFIGURATION_MANAGER_H
