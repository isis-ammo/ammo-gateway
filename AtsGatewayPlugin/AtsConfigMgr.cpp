#include "AtsConfigMgr.h"

#include "log.h"

#include <iostream>
#include <fstream>

const char *ATS_CONFIG_FILE = "AtsPluginConfig.json";

using namespace std;

AtsConfigMgr *AtsConfigMgr::sharedInstance = NULL;

AtsConfigMgr::AtsConfigMgr() : atsBaseAddress("http://ats/"), 
   atsUsername("ats_admin"), atsPassword("ats_pass"), atsBaseDir("") 
{
  //LOG_TRACE("Parsing config file...");
  ifstream configFile(ATS_CONFIG_FILE);
  if(configFile) {
    Json::Reader reader;
    
    bool parsingSuccessful = reader.parse(configFile, root);
    
    if(parsingSuccessful) {
      if(root["BaseAddress"].isString()) {
        atsBaseAddress = root["BaseAddress"].asString();
      } else {
        LOG_ERROR("Error: BaseAddress is missing or wrong type (should be string)");
      }
      
      if(root["Username"].isString()) {
        atsUsername = root["Username"].asString();
      } else {
        LOG_ERROR("Error: Username is missing or wrong type (should be string)");
      }
      
      if(root["BaseDir"].isString()) {
        atsBaseDir = root["BaseDir"].asString();
      } else {
        LOG_ERROR("Error: BaseDir is missing or wrong type (should be string)");
      }
      
      if(root["Password"].isString()) {
        atsPassword = root["Password"].asString();
      } else {
        LOG_ERROR("Error: Password is missing or wrong type (should be integer)");
      }
    } else {
      LOG_ERROR("JSON parsing error in config file '" << ATS_CONFIG_FILE << "'.  Using defaults.");
    }
    configFile.close();
  } else {
    LOG_WARN("Could not read from config file '" << ATS_CONFIG_FILE << "'.  Using defaults.");
  }
  
  LOG_INFO(" Connector Configuration: ");
  LOG_INFO("  Base Address: " << atsBaseAddress);
  LOG_INFO("  Address: " << atsUsername);
  LOG_INFO("  Password: " << atsPassword);
  LOG_INFO("  Base Path: " << atsBaseDir);
}

std::string AtsConfigMgr::getBaseAddress() {
  return atsBaseAddress;
}

std::string AtsConfigMgr::getBaseDir() {
  return atsBaseDir;
}

std::string AtsConfigMgr::getBasePath() {
  return atsBaseAddress + atsBaseDir;
}

std::string AtsConfigMgr::getPath(std::string suffix) const {
  return atsBaseAddress + atsBaseDir + suffix;
}

std::string AtsConfigMgr::getUsername() {
  return atsUsername;
}

std::string AtsConfigMgr::getPassword() {
  return atsPassword;
}

std::pair<std::string, std::string> AtsConfigMgr::getCredentialsForUser(std::string username) {
  std::pair<std::string, std::string> newCredentials;
  //default username in case of error or missing credentials
  newCredentials.first = getUsername();
  newCredentials.second = getPassword();
  
  if(username != "") {
    if (root["UsernameMap"].isObject() 
     && root["UsernameMap"][username].isObject() 
     && root["UsernameMap"][username]["Username"].isString() 
     && root["UsernameMap"][username]["Password"].isString()) 
    {
      newCredentials.first = root["UsernameMap"][username]["Username"].asString();
      newCredentials.second = root["UsernameMap"][username]["Password"].asString();
    }
  }
  return newCredentials;
}

AtsConfigMgr* AtsConfigMgr::getInstance() {
  if(sharedInstance == NULL) {
    sharedInstance = new AtsConfigMgr();
  }
  return sharedInstance;
}
