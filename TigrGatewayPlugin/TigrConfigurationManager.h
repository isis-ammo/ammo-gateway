#ifndef GATEWAY_CONFIGURATION_MANAGER_H
#define GATEWAY_CONFIGURATION_MANAGER_H

#include <string>

#include "json/reader.h"
#include "json/value.h"

class TigrConfigurationManager {
public:
  static TigrConfigurationManager* getInstance();
  
  std::string getTigrBaseAddress();
  
  
  std::pair<std::string, std::string> getTigrCredentialsForUser(std::string username);
private:
  TigrConfigurationManager();
  std::string getTigrUsername();
  std::string getTigrPassword();
  static TigrConfigurationManager *sharedInstance;
  
  std::string tigrBaseAddress;
  std::string tigrUsername;
  std::string tigrPassword;
  
  Json::Value root;
};

#endif //GATEWAY_CONFIGURATION_MANAGER_H
