#include "LdapConfigurationManager.h"

#include "json/reader.h"
#include "json/value.h"

#include <iostream>
#include <fstream>

#include "log.h"
#include <ace/OS_NS_sys_stat.h>

const char *CONFIG_DIRECTORY = "ammo-gateway";
const char *LDAP_CONFIG_FILE = "LdapPluginConfig.json";

using namespace std;

LdapConfigurationManager *LdapConfigurationManager::sharedInstance = NULL;

//============================================================
//
// Default constructor
//
// Use the default config file if none specified
//============================================================
LdapConfigurationManager::LdapConfigurationManager() : ldapServerAddress("localhost"), ldapServerPort(389), ldapUsername("cn=Manager,dc=ammo,dc=tdm"), ldapPassword("ammmo")
{
  string configFilename = findConfigFile(LDAP_CONFIG_FILE);
  
  if(configFilename != "") {
    configFromFile(configFilename);
  } else {
    LOG_WARN("Using default configuration.");
  }
}

//============================================================
//
// Constructor with filename
//
// Use the config file specified
//============================================================
LdapConfigurationManager::LdapConfigurationManager(string fileName)
{
  configFromFile(fileName);
}

//============================================================
//
// configFromFile()
//
//============================================================
void LdapConfigurationManager::configFromFile(string fileName)
{
  ifstream configFile(fileName.c_str());
  if(configFile)
    {
      Json::Value root;
      Json::Reader reader;

      bool parsingSuccessful = reader.parse(configFile, root);

      if(parsingSuccessful)
        {
          if(root["LdapServerAddress"].isString())
            {
              ldapServerAddress = root["LdapServerAddress"].asString();
            }
          else
            {
              cerr << "Error: LdapServerAddress is missing or wrong type (should be string)" << endl << flush;
            }

          if(root["LdapServerPort"].isInt())
            {
              ldapServerPort = root["LdapServerPort"].asInt();
            }
          else
            {
              cerr << "Error: LdapServerPort is missing or wrong type (should be integer)" << endl << flush;
            }

          if(root["LdapUsername"].isString())
            {
              ldapUsername = root["LdapUsername"].asString();
            }
          else
            {
              cerr << "Error: LdapUsername is missing or wrong type (should be string)" << endl << flush;
            }

          if(root["LdapPassword"].isString())
            {
              ldapPassword = root["LdapPassword"].asString();
            }
          else
            {
              cerr << "Error: LdapPassword is missing or wrong type (should be integer)" << endl << flush;
            }
        }
      else
        {
          cerr << "JSON parsing error in config file '" 
	       << fileName << "'.  Using defaults." << endl << flush;
        }
      configFile.close();
    }
  else
    {
      cout << "Could not read from config file '" 
	   << fileName << "'.  Using defaults." << endl << flush;
    }

}

string LdapConfigurationManager::findConfigFile(std::string defaultConfigFile) {
  string filePath;
  ACE_stat statStruct;
  
  string home, gatewayRoot;
  
  char *homeC = ACE_OS::getenv("HOME");
  if(homeC == NULL) {
    home = "";
  } else {
    home = homeC;
  }
  
  char *gatewayRootC = ACE_OS::getenv("GATEWAY_ROOT");
  if(gatewayRootC == NULL) {
    gatewayRoot = "";
  } else {
    gatewayRoot = gatewayRootC;
  }
  
  filePath = defaultConfigFile;
  //stat returns 0 if the file exists
  if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
    filePath = home + "/" + "." + CONFIG_DIRECTORY + "/" + LDAP_CONFIG_FILE;
    if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
      filePath = string("/etc/") + CONFIG_DIRECTORY + "/" + LDAP_CONFIG_FILE;
      if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
        filePath = gatewayRoot + "/etc/" + LDAP_CONFIG_FILE;
        if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
          filePath = gatewayRoot + "/build/etc/" + LDAP_CONFIG_FILE;
          if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
            filePath = string("../etc/") + LDAP_CONFIG_FILE;
            if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
              LOG_ERROR("No config file found.");
              return "";
            }
          }
        }
      }
    }
  }
  
  LOG_INFO("Using config file: " << filePath);
  return filePath;
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
    }
  return sharedInstance;
}
