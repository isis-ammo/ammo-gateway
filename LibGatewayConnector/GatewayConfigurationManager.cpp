#include "GatewayConfigurationManager.h"

#include "json/reader.h"
#include "json/value.h"

#include <iostream>
#include <fstream>

const char *CONFIG_FILE = "GatewayConfig.json";

using namespace std;

GatewayConfigurationManager *GatewayConfigurationManager::sharedInstance = NULL;

GatewayConfigurationManager::GatewayConfigurationManager() : gatewayAddress("127.0.0.1"), gatewayInterface("0.0.0.0"), gatewayPort(12475) {
  //cout << "Parsing config file..." << endl << flush;
  ifstream configFile(CONFIG_FILE);
  if(configFile) {
    Json::Value root;
    Json::Reader reader;
    
    bool parsingSuccessful = reader.parse(configFile, root);
    
    if(parsingSuccessful) {
      if(root["GatewayInterface"].isString()) {
        gatewayInterface = root["GatewayInterface"].asString();
      } else {
        cout << "Error: GatewayInterface is missing or wrong type (should be string)" << endl << flush;
      }
      
      if(root["GatewayAddress"].isString()) {
        gatewayAddress = root["GatewayAddress"].asString();
      } else {
        cout << "Error: GatewayAddress is missing or wrong type (should be string)" << endl << flush;
      }
      
      if(root["GatewayPort"].isInt()) {
        gatewayPort = root["GatewayPort"].asInt();
      } else {
        cout << "Error: GatewayPort is missing or wrong type (should be integer)" << endl << flush;
      }
    } else {
      cout << "JSON parsing error in config file '" << CONFIG_FILE << "'.  Using defaults." << endl << flush;
    }
    configFile.close();
  } else {
    cout << "Could not read from config file '" << CONFIG_FILE << "'.  Using defaults." << endl << flush;
  }
  
  cout << endl;
  cout << "Gateway Configuration: " << endl;
  cout << "  Interface: " << gatewayInterface << endl;
  cout << "  Address: " << gatewayAddress << endl;
  cout << "  Port: " << gatewayPort << endl << flush;
  cout << endl;
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
