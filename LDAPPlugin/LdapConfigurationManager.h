#ifndef LDAP_CONFIGURATION_MANAGER_H
#define LDAP_CONFIGURATION_MANAGER_H

#include "ConfigurationManager.h"

class LdapConfigurationManager : public ConfigurationManager
{
public:
  static LdapConfigurationManager* getInstance();  

  std::string getLdapServerAddress();
  int getLdapServerPort();
  std::string getLdapUsername();
  std::string getLdapPassword();

protected:
  void init();
  void decode(const Json::Value& root);

private:
  LdapConfigurationManager();
  
  static LdapConfigurationManager *sharedInstance;

  std::string ldapServerAddress;
  int ldapServerPort;
  std::string ldapUsername;
  std::string ldapPassword;
};

#endif //LDAP_CONFIGURATION_MANAGER_H
