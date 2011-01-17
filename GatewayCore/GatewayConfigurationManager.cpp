#include "GatewayConfigurationManager.h"

#include "json/reader.h"
#include "json/value.h"

#include <log4cxx/logger.h>
#include <log4cxx/ndc.h>

#include <iostream>
#include <fstream>

const char *CONFIG_FILE = "GatewayConfig.json";

using namespace std;
using namespace log4cxx;
using namespace log4cxx::helpers;

extern LoggerPtr logger; //statically declared in main.cpp

GatewayConfigurationManager *GatewayConfigurationManager::sharedInstance = NULL;

GatewayConfigurationManager::GatewayConfigurationManager() : gatewayAddress("127.0.0.1"), gatewayInterface("0.0.0.0"), gatewayPort(12475) {
  NDC::push("GatewayConfig");
  LOG4CXX_DEBUG(logger, "Parsing config file...");
  ifstream configFile(CONFIG_FILE);
  if(configFile) {
    Json::Value root;
    Json::Reader reader;
    
    bool parsingSuccessful = reader.parse(configFile, root);
    
    if(parsingSuccessful) {
      if(root["GatewayInterface"].isString()) {
        gatewayInterface = root["GatewayInterface"].asString();
      } else {
        LOG4CXX_ERROR(logger, "GatewayInterface is missing or wrong type (should be string)");
      }
      
      if(root["GatewayAddress"].isString()) {
        gatewayAddress = root["GatewayAddress"].asString();
      } else {
        LOG4CXX_ERROR(logger, "GatewayAddress is missing or wrong type (should be string)");
      }
      
      if(root["GatewayPort"].isInt()) {
        gatewayPort = root["GatewayPort"].asInt();
      } else {
        LOG4CXX_ERROR(logger, "GatewayPort is missing or wrong type (should be integer)");
      }
    } else {
      LOG4CXX_ERROR(logger, "JSON parsing error in config file '" << CONFIG_FILE << "'.  Using defaults.");
    }
    configFile.close();
  } else {
    LOG4CXX_WARN(logger, "Could not read from config file '" << CONFIG_FILE << "'.  Using defaults.");
  }
  
  LOG4CXX_INFO(logger, "Gateway Configuration: ");
  LOG4CXX_INFO(logger, "  Interface: " << gatewayInterface);
  LOG4CXX_INFO(logger, "  Address: " << gatewayAddress);
  LOG4CXX_INFO(logger, "  Port: " << gatewayPort);
  NDC::pop();
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

GatewayConfigurationManager* GatewayConfigurationManager::getInstance() {
  if(sharedInstance == NULL) {
    sharedInstance = new GatewayConfigurationManager();
  }
  return sharedInstance;
}
