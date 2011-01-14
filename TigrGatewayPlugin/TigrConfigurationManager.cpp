#include "TigrConfigurationManager.h"



#include <iostream>
#include <fstream>

const char *TIGR_CONFIG_FILE = "TigrPluginConfig.json";

using namespace std;

TigrConfigurationManager *TigrConfigurationManager::sharedInstance = NULL;

TigrConfigurationManager::TigrConfigurationManager() : tigrBaseAddress("http://tigr/r4/"), tigrUsername("jwilliams"), tigrPassword("jwilliams"), tigrSecurityInfo("X") {
  //cout << "Parsing config file..." << endl << flush;
  ifstream configFile(TIGR_CONFIG_FILE);
  if(configFile) {
    Json::Reader reader;
    
    bool parsingSuccessful = reader.parse(configFile, root);
    
    if(parsingSuccessful) {
      if(root["TigrBaseAddress"].isString()) {
        tigrBaseAddress = root["TigrBaseAddress"].asString();
      } else {
        cout << "Error: TigrBaseAddress is missing or wrong type (should be string)" << endl << flush;
      }
      
      if(root["TigrUsername"].isString()) {
        tigrUsername = root["TigrUsername"].asString();
      } else {
        cout << "Error: TigrUsername is missing or wrong type (should be string)" << endl << flush;
      }
      
      if(root["TigrSecurityInfo"].isString()) {
        tigrSecurityInfo = root["TigrSecurityInfo"].asString();
      } else {
        cout << "Error: TigrSecurityInfo is missing or wrong type (should be string)" << endl << flush;
      }
      
      if(root["TigrPassword"].isString()) {
        tigrPassword = root["TigrPassword"].asString();
      } else {
        cout << "Error: TigrPassword is missing or wrong type (should be integer)" << endl << flush;
      }
    } else {
      cout << "JSON parsing error in config file '" << TIGR_CONFIG_FILE << "'.  Using defaults." << endl << flush;
    }
    configFile.close();
  } else {
    cout << "Could not read from config file '" << TIGR_CONFIG_FILE << "'.  Using defaults." << endl << flush;
  }
  
  cout << endl;
  cout << "Tigr Connector Configuration: " << endl;
  cout << "  Base Address: " << tigrBaseAddress << endl;
  cout << "  Address: " << tigrUsername << endl;
  cout << "  Password: " << tigrPassword << endl;
  cout << "  SecurityInfo: " << tigrSecurityInfo << endl << flush;
  cout << endl;
}

std::string TigrConfigurationManager::getTigrBaseAddress() {
  return tigrBaseAddress;
}

std::string TigrConfigurationManager::getTigrSecurityInfo() {
  return tigrSecurityInfo;
}

std::string TigrConfigurationManager::getTigrUsername() {
  return tigrUsername;
}

std::string TigrConfigurationManager::getTigrPassword() {
  return tigrPassword;
}

std::pair<std::string, std::string> TigrConfigurationManager::getTigrCredentialsForUser(std::string username) {
  std::pair<std::string, std::string> newCredentials;
  //default username in case of error or missing credentials
  newCredentials.first = getTigrUsername();
  newCredentials.second = getTigrPassword();
  
  if(username != "") {
    if(root["UsernameMap"].isObject() && root["UsernameMap"][username].isObject() && root["UsernameMap"][username]["TigrUsername"].isString() && root["UsernameMap"][username]["TigrPassword"].isString()) {
      newCredentials.first = root["UsernameMap"][username]["TigrUsername"].asString();
      newCredentials.second = root["UsernameMap"][username]["TigrPassword"].asString();
    }
  }
  return newCredentials;
}

TigrConfigurationManager* TigrConfigurationManager::getInstance() {
  if(sharedInstance == NULL) {
    sharedInstance = new TigrConfigurationManager();
  }
  return sharedInstance;
}
