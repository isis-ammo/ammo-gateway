#include "GatewayConfigurationManager.h"
#include "GatewayConnector.h"

#include "json/reader.h"
#include "json/value.h"

#include <ace/OS_NS_sys_stat.h>
#include <ace/OS_NS_sys_time.h>

#include <iostream>
#include <fstream>

#ifdef WIN32
#include <Windows.h>
#include <shlwapi.h>
#include <ShlObj.h>
#endif

#include "log.h"

using namespace std;

const char *CONFIG_DIRECTORY = "ammo-gateway";
const char *CONFIG_FILE = "GatewayConfig.json";

ammo::gateway::internal::GatewayConfigurationManager *ammo::gateway::internal::GatewayConfigurationManager::sharedInstance = NULL;

ammo::gateway::internal::GatewayConfigurationManager::GatewayConfigurationManager(const char* configFileName) : gatewayAddress("127.0.0.1"), gatewayInterface("0.0.0.0"), gatewayPort(12475) {
  //LOG_INFO("Parsing config file...");
  
  string filename = findConfigFile(configFileName);
  
  if(filename != "") {
  ifstream configFile(filename.c_str());
  if(configFile) {
    Json::Value root;
    Json::Reader reader;
    
    bool parsingSuccessful = reader.parse(configFile, root);
    
    if(parsingSuccessful) {
      if(root["GatewayInterface"].isString()) {
        gatewayInterface = root["GatewayInterface"].asString();
      } else {
        LOG_ERROR("Error: GatewayInterface is missing or wrong type (should be string)");
      }
      
      if(root["GatewayAddress"].isString()) {
        gatewayAddress = root["GatewayAddress"].asString();
      } else {
        LOG_ERROR("Error: GatewayAddress is missing or wrong type (should be string)");
      }
      
      if(root["GatewayPort"].isInt()) {
        gatewayPort = root["GatewayPort"].asInt();
      } else {
        LOG_ERROR("Error: GatewayPort is missing or wrong type (should be integer)");
      }
    } else {
      LOG_ERROR("JSON parsing error in config file '" << configFileName << "'.  Using defaults.");
    }
    configFile.close();
  } else {
    LOG_WARN("Could not read from config file '" << configFileName << "'.  Using defaults.");
  }
  } else {
    LOG_WARN("Using default configuration");
  }
  
  LOG_INFO("Gateway Configuration: ");
  LOG_INFO("  Interface: " << gatewayInterface);
  LOG_INFO("  Address: " << gatewayAddress);
  LOG_INFO("  Port: " << gatewayPort);
}

std::string ammo::gateway::internal::GatewayConfigurationManager::getGatewayAddress() {
  return gatewayAddress;
}

std::string ammo::gateway::internal::GatewayConfigurationManager::getGatewayInterface() {
  return gatewayInterface;
}

int ammo::gateway::internal::GatewayConfigurationManager::getGatewayPort() {
  return gatewayPort;
}

#ifndef WIN32

string ammo::gateway::internal::GatewayConfigurationManager::findConfigFile(std::string defaultConfigFile) {
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
    filePath = home + "/" + "." + CONFIG_DIRECTORY + "/" + CONFIG_FILE;
    if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
      filePath = string("/etc/") + CONFIG_DIRECTORY + "/" + CONFIG_FILE;
      if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
        filePath = gatewayRoot + "/etc/" + CONFIG_FILE;
        if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
          filePath = gatewayRoot + "/build/etc/" + CONFIG_FILE;
          if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
            filePath = string("../etc/") + CONFIG_FILE;
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

#else
/**
 * Searches for the gateway config file.  Search order:
 *   1) The current working directory
 *   2) The user's configuration directory (Roaming appdata directory/ammo-gateway)
 *   3) The all users configuration directory (i.e. C:\ProgramData\ammo-gateway on Vista/7)
 *   Fallback locations (don't rely on these; they may change or disappear in a
 *   future release.  Gateway installation should put the config file into
 *   a location that's searched by default):
 *   4) $GATEWAY_ROOT/etc
 *   5) $GATEWAY_ROOT/build/etc
 *   6) ../etc
 */
string ammo::gateway::internal::GatewayConfigurationManager::findConfigFile(std::string defaultConfigFile) {
  string filePath;
  ACE_stat statStruct;
  
  string gatewayRoot;
  
  TCHAR userConfigPath[MAX_PATH];
  TCHAR systemConfigPath[MAX_PATH];

  SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, userConfigPath);
  SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, 0, systemConfigPath);
  
  char *gatewayRootC = ACE_OS::getenv("GATEWAY_ROOT");
  if(gatewayRootC == NULL) {
    gatewayRoot = "";
  } else {
    gatewayRoot = gatewayRootC;
  }
  
  filePath = defaultConfigFile;
  //stat returns 0 if the file exists
  if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
    filePath = string(userConfigPath) + "\\" + CONFIG_DIRECTORY + "\\" + CONFIG_FILE;
    if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
      filePath = string(systemConfigPath) + "\\" + CONFIG_DIRECTORY + "\\" + CONFIG_FILE;
      if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
        filePath = gatewayRoot + "\\etc\\" + CONFIG_FILE;
        if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
          filePath = gatewayRoot + "\\build\\etc\\" + CONFIG_FILE;
          if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
            filePath = string("..\\etc\\") + CONFIG_FILE;
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
#endif

ammo::gateway::internal::GatewayConfigurationManager* ammo::gateway::internal::GatewayConfigurationManager::getInstance() {
  if(sharedInstance == NULL) {
    sharedInstance = new GatewayConfigurationManager(CONFIG_FILE);
  }
  return sharedInstance;
}
ammo::gateway::internal::GatewayConfigurationManager* ammo::gateway::internal::GatewayConfigurationManager::getInstance(std::string configfile) {
  if(sharedInstance == NULL) {
    sharedInstance = new GatewayConfigurationManager(configfile.c_str());
  }
  return sharedInstance;
}
