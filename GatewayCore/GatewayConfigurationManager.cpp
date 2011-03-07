#include "GatewayConfigurationManager.h"

#include "json/reader.h"
#include "json/value.h"

#include "log.h"

#include <iostream>
#include <fstream>

const char *CONFIG_FILE = "GatewayConfig.json";

using namespace std;

GatewayConfigurationManager *GatewayConfigurationManager::sharedInstance = NULL;

GatewayConfigurationManager::GatewayConfigurationManager() : 
gatewayAddress("127.0.0.1"),
gatewayInterface("0.0.0.0"),
gatewayPort(12475),
crossGatewayId("DefaultGateway"),
crossGatewayServerInterface("127.0.0.1"),
crossGatewayServerPort(47543),
crossGatewayParentAddress(""),
crossGatewayParentPort(47543) {
  LOG_DEBUG("Parsing config file...");
  ifstream configFile(CONFIG_FILE);
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
      
      if(root["CrossGatewayId"].isString()) {
        crossGatewayId = root["CrossGatewayId"].asString();
      } else {
        LOG_ERROR("CrossGatewayId is missing or wrong type (should be string)");
      }
      
      if(root["CrossGatewayServerInterface"].isString()) {
        crossGatewayServerInterface = root["CrossGatewayServerInterface"].asString();
      } else {
        LOG_ERROR("CrossGatewayServerInterface is missing or wrong type (should be string)");
      }
      
      if(root["CrossGatewayServerPort"].isInt()) {
        crossGatewayServerPort = root["CrossGatewayServerPort"].asInt();
      } else {
        LOG_ERROR("CrossGatewayServerPort is missing or wrong type (should be int)");
      }
      
      if(root["CrossGatewayParentAddress"].isString()) {
        crossGatewayParentAddress = root["CrossGatewayParentAddress"].asString();
      } else {
        LOG_ERROR("CrossGatewayParentAddress is missing or wrong type (should be string)");
      }
      
      if(root["CrossGatewayParentPort"].isInt()) {
        crossGatewayParentPort = root["CrossGatewayParentPort"].asInt();
      } else {
        LOG_ERROR("CrossGatewayParentPort is missing or wrong type (should be int)");
      }
    } else {
      LOG_ERROR("JSON parsing error in config file '" << CONFIG_FILE << "'.  Using defaults.");
    }
    configFile.close();
  } else {
    LOG_WARN("Could not read from config file '" << CONFIG_FILE << "'.  Using defaults.");
  }
  
  LOG_INFO("Gateway Configuration: ");
  LOG_INFO("  Interface: " << gatewayInterface);
  LOG_INFO("  Address: " << gatewayAddress);
  LOG_INFO("  Port: " << gatewayPort);
  LOG_INFO("  CrossGatewayId: " << crossGatewayId);
  LOG_INFO("  CrossGatewayServerInterface: " << crossGatewayServerInterface);
  LOG_INFO("  CrossGatewayServerPort: " << crossGatewayServerPort);
  LOG_INFO("  CrossGatewayParentAddress: " << crossGatewayParentAddress);
  LOG_INFO("  CrossGatewayParentPort: " << crossGatewayParentPort);
}

std::string GatewayConfigurationManager::getGatewayAddress() {
  return gatewayAddress;
}

std::string GatewayConfigurationManager::getGatewayInterface() {
  return gatewayInterface;
}

std::string GatewayConfigurationManager::getCrossGatewayId() {
  return crossGatewayId;
}

int GatewayConfigurationManager::getGatewayPort() {
  return gatewayPort;
}

std::string GatewayConfigurationManager::getCrossGatewayServerInterface() {
  return crossGatewayServerInterface;
}

int GatewayConfigurationManager::getCrossGatewayServerPort() {
  return crossGatewayServerPort;
}

std::string GatewayConfigurationManager::getCrossGatewayParentAddress() {
  return crossGatewayParentAddress;
}

int GatewayConfigurationManager::getCrossGatewayParentPort() {
  return crossGatewayParentPort;
}

GatewayConfigurationManager* GatewayConfigurationManager::getInstance() {
  if(sharedInstance == NULL) {
    sharedInstance = new GatewayConfigurationManager();
  }
  return sharedInstance;
}
