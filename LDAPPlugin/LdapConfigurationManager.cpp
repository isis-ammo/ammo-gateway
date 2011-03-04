#include "LdapConfigurationManager.h"

#include "json/reader.h"
#include "json/value.h"

#include <iostream>
#include <fstream>

const char *LDAP_CONFIG_FILE = "LdapPluginConfig.json";

using namespace std;

LdapConfigurationManager *LdapConfigurationManager::sharedInstance = NULL;

//============================================================
//
// Default constructor
//
// Use the default config file if none specified
//============================================================
LdapConfigurationManager::LdapConfigurationManager() : ldapBaseAddress("localhost"), ldapUsername("cn=Manager,dc=transapps,dc=darpa,dc=mil"), ldapPassword("ammmo")
{
  configFromFile(LDAP_CONFIG_FILE);
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
          if(root["LdapBaseAddress"].isString())
            {
              ldapBaseAddress = root["LdapBaseAddress"].asString();
            }
          else
            {
              cerr << "Error: LdapBaseAddress is missing or wrong type (should be string)" << endl << flush;
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

//============================================================
//
// getLdapBaseAddress()
//
//============================================================
std::string LdapConfigurationManager::getLdapBaseAddress()
{
  return ldapBaseAddress;
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
