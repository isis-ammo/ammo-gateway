#include "LdapConfigurationManager.h"

#include "json/reader.h"
#include "json/value.h"

#include <iostream>
#include <fstream>

const char *LDAP_CONFIG_FILE = "LdapPluginConfig.json";

using namespace std;

LdapConfigurationManager *LdapConfigurationManager::sharedInstance = NULL;

LdapConfigurationManager::LdapConfigurationManager() : ldapBaseAddress("localhost"), ldapUsername("cn=Manager,dc=transapps,dc=darpa,dc=mil"), ldapPassword("ammmo") {
  //cout << "Parsing config file..." << endl << flush;
  ifstream configFile(LDAP_CONFIG_FILE);
  if(configFile) {
    Json::Value root;
    Json::Reader reader;
    
    bool parsingSuccessful = reader.parse(configFile, root);
    
    if(parsingSuccessful) {
      if(root["LdapBaseAddress"].isString()) {
        ldapBaseAddress = root["LdapBaseAddress"].asString();
      } else {
        cout << "Error: LdapBaseAddress is missing or wrong type (should be string)" << endl << flush;
      }
      
      if(root["LdapUsername"].isString()) {
        ldapUsername = root["LdapUsername"].asString();
      } else {
        cout << "Error: LdapUsername is missing or wrong type (should be string)" << endl << flush;
      }
      
      if(root["LdapPassword"].isString()) {
        ldapPassword = root["LdapPassword"].asString();
      } else {
        cout << "Error: LdapPassword is missing or wrong type (should be integer)" << endl << flush;
      }
    } else {
      cout << "JSON parsing error in config file '" << LDAP_CONFIG_FILE << "'.  Using defaults." << endl << flush;
    }
    configFile.close();
  } else {
    cout << "Could not read from config file '" << LDAP_CONFIG_FILE << "'.  Using defaults." << endl << flush;
  }
  
  cout << endl;
  cout << "Ldap Connector Configuration: " << endl;
  cout << "  Base Address: " << ldapBaseAddress << endl;
  cout << "  Username: " << ldapUsername << endl;
  cout << "  Password: " << ldapPassword << endl << flush;
  cout << endl;
}

std::string LdapConfigurationManager::getLdapBaseAddress() {
  return ldapBaseAddress;
}

std::string LdapConfigurationManager::getLdapUsername() {
  return ldapUsername;
}

std::string LdapConfigurationManager::getLdapPassword() {
  return ldapPassword;
}

LdapConfigurationManager* LdapConfigurationManager::getInstance() {
  if(sharedInstance == NULL) {
    sharedInstance = new LdapConfigurationManager();
  }
  return sharedInstance;
}
