#include <string>
#include "json/reader.h"
#include "json/value.h"

#include "log.h"

#include <iostream>
#include <fstream>
#include <algorithm>

#include "ace/Signal.h"
#include <ace/OS_NS_sys_stat.h>

const char *LOG_CONFIG_DIRECTORY = "ammo-gateway";
const char *LOG_CONFIG_FILE = "LoggingConfig.json";

std::string findConfigFile();

/**
* This function reads the logging config file and sets up logging accordingly.
* This should be included in main.cpp and called exactly once when the
* application is initialized.
*/
void setupLogging() {
  std::string configFilename = findConfigFile();
  
  std::string logLevel = "trace";
  
  if(configFilename != "") {
    std::ifstream configFile(configFilename.c_str());
    if(configFile) {
      Json::Value root;
      Json::Reader reader;
      
      bool parsingSuccessful = reader.parse(configFile, root);
      
      if(parsingSuccessful) {
        if(root["LogLevel"].isString()) {
          logLevel = root["LogLevel"].asString();
          std::transform(logLevel.begin(), logLevel.end(), logLevel.begin(), tolower);
        } else {
          LOG_ERROR("LogLevel is missing or wrong type (should be string)");
        }
      } else {
        LOG_ERROR("JSON parsing error in config file '" << LOG_CONFIG_FILE << "'.  Using defaults.");
      }
    } else {
      LOG_WARN("Could not read from config file '" << LOG_CONFIG_FILE << "'.  Using defaults.");
    }
  } else {
    LOG_WARN("Using default configuration.");
  }
  
  if(logLevel == "trace") {
    ACE_LOG_MSG->priority_mask (LM_TRACE | LM_DEBUG | LM_INFO | LM_WARNING | LM_ERROR | LM_CRITICAL, ACE_Log_Msg::PROCESS);
  } else if(logLevel == "debug") {
    ACE_LOG_MSG->priority_mask (LM_DEBUG | LM_INFO | LM_WARNING | LM_ERROR | LM_CRITICAL, ACE_Log_Msg::PROCESS);
  } else if(logLevel == "info") {
    ACE_LOG_MSG->priority_mask (LM_INFO | LM_WARNING | LM_ERROR | LM_CRITICAL, ACE_Log_Msg::PROCESS);
  } else if(logLevel == "warning") {
    ACE_LOG_MSG->priority_mask (LM_WARNING | LM_ERROR | LM_CRITICAL, ACE_Log_Msg::PROCESS);
  } else if(logLevel == "error") {
    ACE_LOG_MSG->priority_mask (LM_ERROR | LM_CRITICAL, ACE_Log_Msg::PROCESS);
  } else if(logLevel == "critical") {
    ACE_LOG_MSG->priority_mask (LM_CRITICAL, ACE_Log_Msg::PROCESS);
  } else {
    LOG_ERROR("Unknown logging level... using default configuration.");
  }
}

std::string findConfigFile() {
  std::string filePath;
  ACE_stat statStruct;
  
  std::string home, gatewayRoot;
  
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
  
  filePath = LOG_CONFIG_FILE;
  //stat returns 0 if the file exists
  if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
    filePath = home + "/" + "." + LOG_CONFIG_DIRECTORY + "/" + LOG_CONFIG_FILE;
    if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
      filePath = std::string("/etc/") + LOG_CONFIG_DIRECTORY + "/" + LOG_CONFIG_FILE;
      if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
        filePath = gatewayRoot + "/etc/" + LOG_CONFIG_FILE;
        if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
          filePath = gatewayRoot + "/build/etc/" + LOG_CONFIG_FILE;
          if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
            filePath = std::string("../etc/") + LOG_CONFIG_FILE;
            if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
              LOG_ERROR("No logging config file found.");
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

//:mode=c++: (jEdit modeline for syntax highlighting)
