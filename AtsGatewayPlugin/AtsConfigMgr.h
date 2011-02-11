#ifndef ATS_GATEWAY_CONFIGURATION_MANAGER_H
#define ATS_GATEWAY_CONFIGURATION_MANAGER_H

#include <string>

#include "json/reader.h"
#include "json/value.h"

class AtsConfigMgr {
public:
  static AtsConfigMgr* getInstance();
  
  bool hasGatewayConfig() const;
  std::string getGatewayConfig() const;

  std::string getHost() const;
  int getPort() const;
  std::string getBaseDir() const;
  std::string getUrl() const;
  std::string getUrl(std::string suffix) const;
  std::string getUsername() const;
  std::string getPassword() const;
  std::string getHttpAuth() const;
  
  std::pair<std::string, std::string> getCredentialsForUser(std::string username);

private:
  AtsConfigMgr();
  int parsingSuccessful;
  Json::Value root;

  static AtsConfigMgr *sharedInstance;
};

#endif //ATS_GATEWAY_CONFIGURATION_MANAGER_H
