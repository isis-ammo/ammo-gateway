#ifndef LDAP_CONFIGURATION_MANAGER_H
#define LDAP_CONFIGURATION_MANAGER_H

#include <string>

class LdapConfigurationManager 
{
public:
  static LdapConfigurationManager* getInstance();  

  std::string getLdapBaseAddress();
  std::string getLdapUsername();
  std::string getLdapPassword();

private:
  LdapConfigurationManager();
  static LdapConfigurationManager *sharedInstance;

  std::string ldapBaseAddress;
  std::string ldapUsername;
  std::string ldapPassword;
};

#endif //LDAP_CONFIGURATION_MANAGER_H
