#include "GatewayConfigurationManager.h"

#include "json/reader.h"
#include "json/value.h"

#include "log.h"

#include <iostream>
#include <fstream>

#include <ace/OS_NS_sys_stat.h>

const char *CONFIG_DIRECTORY = "ammo-gateway";
const char *CONFIG_FILE = "GatewayConfig.json";

using namespace std;

GatewayConfigurationManager *GatewayConfigurationManager::sharedInstance = NULL;

GatewayConfigurationManager::GatewayConfigurationManager() : gatewayAddress("127.0.0.1"), gatewayInterface("0.0.0.0"), gatewayPort(12475) {
  LOG_DEBUG("Parsing config file...");
  string configFilename = findConfigFile();
  
  if(configFilename != "") {
    ifstream configFile(configFilename.c_str());
    if(configFile) {
      Json::Value root;
      Json::Reader reader;
      
      bool parsingSuccessful = reader.parse(configFile, root);
      
      if(parsingSuccessful) {
        if(root["GatewayInterface"].isString()) {
          gatewayInterface = root["GatewayInterface"].asString();
        } else {
          LOG_ERROR("GatewayInterface is missing or wrong type (should be string)");
        }
        
        if(root["GatewayAddress"].isString()) {
          gatewayAddress = root["GatewayAddress"].asString();
        } else {
          LOG_ERROR("GatewayAddress is missing or wrong type (should be string)");
        }
        
        if(root["GatewayPort"].isInt()) {
          gatewayPort = root["GatewayPort"].asInt();
        } else {
          LOG_ERROR("GatewayPort is missing or wrong type (should be integer)");
        }
      } else {
        LOG_ERROR("JSON parsing error in config file '" << CONFIG_FILE << "'.  Using defaults.");
      }
      configFile.close();
    } else {
      LOG_WARN("Could not read from config file '" << CONFIG_FILE << "'.  Using defaults.");
    }
  } else {
    LOG_WARN("Using default configuration.");
  }
  
  LOG_INFO("Gateway Configuration: ");
  LOG_INFO("  Interface: " << gatewayInterface);
  LOG_INFO("  Address: " << gatewayAddress);
  LOG_INFO("  Port: " << gatewayPort);
}

std::string GatewayConfigurationManager::getGatewayAddress() {
  return gatewayAddress;
}

std::string GatewayConfigurationManager::getGatewayInterface() {
  return gatewayInterface;
}

int GatewayConfigurationManager::getGatewayPort() {
  return gatewayPort;
}

/**
 * Searches for the gateway config file.  Search order:
 *   1) The current working directory
 *   2) ~/.ammo-gateway/
 *   3) /etc/ammo-gateway/
 *   Fallback locations (don't rely on these; they may change or disappear in a
 *   future release.  Gateway installation should put the config file into
 *   a location that's searched by default):
 *   4) $GATEWAY_ROOT/etc
 *   5) $GATEWAY_ROOT/build/etc
 *   6) ../etc
 */
string GatewayConfigurationManager::findConfigFile() {
  string filePath;
  ACE_stat statStruct;
  
  filePath = CONFIG_FILE;
  //stat returns 0 if the file exists
  if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
    filePath = string(ACE_OS::getenv("HOME")) + "/" + "." + CONFIG_DIRECTORY + "/" + CONFIG_FILE;
    if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
      filePath = string("/etc/") + CONFIG_DIRECTORY + "/" + CONFIG_FILE;
      if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
        filePath = string(ACE_OS::getenv("GATEWAY_ROOT")) + "/etc/" + CONFIG_FILE;
        if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
          filePath = string(ACE_OS::getenv("GATEWAY_ROOT")) + "/build/etc/" + CONFIG_FILE;
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

GatewayConfigurationManager* GatewayConfigurationManager::getInstance() {
  if(sharedInstance == NULL) {
    sharedInstance = new GatewayConfigurationManager();
  }
  return sharedInstance;
}
