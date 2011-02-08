#ifndef ATS_GATEWAY_CONFIGURATION_MANAGER_H
#define ATS_GATEWAY_CONFIGURATION_MANAGER_H

#include <string>

#include "json/reader.h"
#include "json/value.h"

class AtsConfigMgr {
public:
  static AtsConfigMgr* getInstance();
  
  std::string getBaseAddress();
  std::string getBaseDir();
  std::string getBasePath();
  std::string getPath(std::string suffix) const;
  
  std::pair<std::string, std::string> getCredentialsForUser(std::string username);

private:
  AtsConfigMgr();
  std::string getUsername();
  std::string getPassword();
  static AtsConfigMgr *sharedInstance;
  
  std::string atsBaseAddress;
  std::string atsUsername;
  std::string atsPassword;
  std::string atsBaseDir;
  
  Json::Value root;
};

#endif //ATS_GATEWAY_CONFIGURATION_MANAGER_H
