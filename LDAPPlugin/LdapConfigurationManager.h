#ifndef LDAP_CONFIGURATION_MANAGER_H
#define LDAP_CONFIGURATION_MANAGER_H

#include <string>

class LdapConfigurationManager 
{
public:
  LdapConfigurationManager(std::string fileName);
  static LdapConfigurationManager* getInstance();  

  std::string getLdapBaseAddress();
  std::string getLdapUsername();
  std::string getLdapPassword();

private:
  LdapConfigurationManager();
  void configFromFile(std::string fileName);
  
  static LdapConfigurationManager *sharedInstance;

  std::string ldapBaseAddress;
  std::string ldapUsername;
  std::string ldapPassword;
};

#endif //LDAP_CONFIGURATION_MANAGER_H
