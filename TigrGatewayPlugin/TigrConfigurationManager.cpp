#include "TigrConfigurationManager.h"

#include <log4cxx/logger.h>
#include <log4cxx/ndc.h>

#include <iostream>
#include <fstream>

const char *TIGR_CONFIG_FILE = "TigrPluginConfig.json";

using namespace std;
using namespace log4cxx;
using namespace log4cxx::helpers;

extern LoggerPtr logger;

TigrConfigurationManager *TigrConfigurationManager::sharedInstance = NULL;

TigrConfigurationManager::TigrConfigurationManager() : tigrBaseAddress("http://tigr/r4/"), tigrUsername("jwilliams"), tigrPassword("jwilliams"), tigrSecurityInfo("X") {
  NDC::push("config");
  //LOG4CXX_TRACE(logger, "Parsing config file...");
  ifstream configFile(TIGR_CONFIG_FILE);
  if(configFile) {
    Json::Reader reader;
    
    bool parsingSuccessful = reader.parse(configFile, root);
    
    if(parsingSuccessful) {
      if(root["TigrBaseAddress"].isString()) {
        tigrBaseAddress = root["TigrBaseAddress"].asString();
      } else {
        LOG4CXX_ERROR(logger, "Error: TigrBaseAddress is missing or wrong type (should be string)");
      }
      
      if(root["TigrUsername"].isString()) {
        tigrUsername = root["TigrUsername"].asString();
      } else {
        LOG4CXX_ERROR(logger, "Error: TigrUsername is missing or wrong type (should be string)");
      }
      
      if(root["TigrSecurityInfo"].isString()) {
        tigrSecurityInfo = root["TigrSecurityInfo"].asString();
      } else {
        LOG4CXX_ERROR(logger, "Error: TigrSecurityInfo is missing or wrong type (should be string)");
      }
      
      if(root["TigrPassword"].isString()) {
        tigrPassword = root["TigrPassword"].asString();
      } else {
        LOG4CXX_ERROR(logger, "Error: TigrPassword is missing or wrong type (should be integer)");
      }
    } else {
      LOG4CXX_ERROR(logger, "JSON parsing error in config file '" << TIGR_CONFIG_FILE << "'.  Using defaults.");
    }
    configFile.close();
  } else {
    LOG4CXX_WARN(logger, "Could not read from config file '" << TIGR_CONFIG_FILE << "'.  Using defaults.");
  }
  
  LOG4CXX_INFO(logger, "Tigr Connector Configuration: ");
  LOG4CXX_INFO(logger, "  Base Address: " << tigrBaseAddress);
  LOG4CXX_INFO(logger, "  Address: " << tigrUsername);
  LOG4CXX_INFO(logger, "  Password: " << tigrPassword);
  LOG4CXX_INFO(logger, "  SecurityInfo: " << tigrSecurityInfo);
  NDC::pop();
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
