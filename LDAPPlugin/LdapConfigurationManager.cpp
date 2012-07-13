#include "LdapConfigurationManager.h"
#include "json/value.h"
#include "log.h"

static const char *LDAP_CONFIG_FILE = "LdapPluginConfig.json";

using namespace std;

LdapConfigurationManager *LdapConfigurationManager::sharedInstance = NULL;

//============================================================
//
// Default constructor
//
// Use the default config file if none specified
//============================================================
LdapConfigurationManager::LdapConfigurationManager() : ConfigurationManager(LDAP_CONFIG_FILE)
{
  init();
}

void LdapConfigurationManager::init()
{
  ldapServerAddress = "localhost";
  ldapServerPort = 389;
  ldapUsername = "cn=Manager,dc=ammo,dc=tdm";
  ldapPassword = "ammmo";
}

void LdapConfigurationManager::decode(const Json::Value& root)
{
  CM_DecodeString ( root, "LdapServerAddress", ldapServerAddress );
  CM_DecodeInt    ( root, "LdapServerPort",    ldapServerPort    );
  CM_DecodeString ( root, "LdapUsername",      ldapUsername      );
  CM_DecodeString ( root, "LdapPassword",      ldapPassword      );

  LOG_INFO("LDAP Plugin Configuration: ");
  LOG_INFO("  Ldap Server Address: " << ldapServerAddress);
  LOG_INFO("  Ldap Server Port: " << ldapServerPort);
  LOG_INFO("  Ldap Username: " << ldapUsername);
  LOG_INFO("  Ldap Password: " << ldapPassword);
}

//============================================================
//
// getLdapServerAddress()
//
//============================================================
std::string LdapConfigurationManager::getLdapServerAddress()
{
  return ldapServerAddress;
}

//============================================================
//
// getLdapServerPort()
//
//============================================================
int LdapConfigurationManager::getLdapServerPort()
{
  return ldapServerPort;
}

//============================================================
//
// getLdapUsername()
//
//============================================================
std::string LdapConfigurationManager::getLdapUsername()
{
  return ldapUsername;
}

//============================================================
//
// getLdapPassword()
//
//============================================================
std::string LdapConfigurationManager::getLdapPassword()
{
  return ldapPassword;
}

//============================================================
//
// getInstance()
//
//============================================================
LdapConfigurationManager* LdapConfigurationManager::getInstance()
{
  if(sharedInstance == NULL)
    {
      sharedInstance = new LdapConfigurationManager();
	  sharedInstance->populate();
    }
  return sharedInstance;
}
