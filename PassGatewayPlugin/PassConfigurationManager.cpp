#include "PassConfigurationManager.h"

#include "json/reader.h"
#include "json/value.h"

#include <iostream>
#include <fstream>

#include <ace/OS_NS_sys_stat.h>
#include "log.h"

const char *CONFIG_DIRECTORY = "ammo-gateway";
const char *PASS_CONFIG_FILE = "PassPluginConfig.json";

using namespace std;

PassConfigurationManager *PassConfigurationManager::sharedInstance = NULL;

PassConfigurationManager::PassConfigurationManager() : 
passServerAddress("http://pass.:8888/PASS"),
passTopic("POS-RPT:MCS:US/ARMY/BFT"),
passSubscriberId("ammosub@isis"),
passSubscriberInterface("0.0.0.0"),
passSubscriberPort(8045),
passSubscriberAddress("http:/192.168.5.2:8045") {
  //cout << "Parsing config file..." << endl << flush;
  string configFilename = findConfigFile();
  if(configFilename != "") {
    ifstream configFile(configFilename.c_str());
    if(configFile) {
      Json::Value root;
      Json::Reader reader;
      
      bool parsingSuccessful = reader.parse(configFile, root);
      
      if(parsingSuccessful) {
        if(root["PassServerAddress"].isString()) {
          passServerAddress = root["PassServerAddress"].asString();
        } else {
          cout << "Error: PassServerAddress is missing or wrong type (should be string)" << endl << flush;
        }
        
        if(root["PassTopic"].isString()) {
          passTopic = root["PassTopic"].asString();
        } else {
          cout << "Error: PassTopic is missing or wrong type (should be string)" << endl << flush;
        }
        
        if(root["PassSubscriberId"].isString()) {
          passSubscriberId = root["PassSubscriberId"].asString();
        } else {
          cout << "Error: PassSubscriberId is missing or wrong type (should be string)" << endl << flush;
        }
        
        if(root["PassSubscriberInterface"].isString()) {
          passSubscriberInterface = root["PassSubscriberInterface"].asString();
        } else {
          cout << "Error: PassSubscriberInterface is missing or wrong type (should be string)" << endl << flush;
        }
        
        if(root["PassSubscriberPort"].isInt()) {
          passSubscriberPort = root["PassSubscriberPort"].asInt();
        } else {
          cout << "Error: PassSubscriberPort is missing or wrong type (should be integer)" << endl << flush;
        }
        
        if(root["PassSubscriberAddress"].isString()) {
          passSubscriberAddress = root["PassSubscriberAddress"].asString();
        } else {
          cout << "Error: PassSubscriberAddress is missing or wrong type (should be string)" << endl << flush;
        }
      } else {
        cout << "JSON parsing error in config file '" << PASS_CONFIG_FILE << "'.  Using defaults." << endl << flush;
      }
      configFile.close();
    } else {
      cout << "Could not read from config file '" << PASS_CONFIG_FILE << "'.  Using defaults." << endl << flush;
    }
  } else {
    LOG_WARN("Using default configuration.");
  }
  
  cout << endl;
  cout << "Pass Plugin Configuration: " << endl;
  cout << "  Pass Server Address: " << passServerAddress << endl << flush;
  cout << "  Pass Topic: " << passTopic << endl << flush;
  cout << "  Pass Subscriber ID: " << passSubscriberId << endl << flush;
  cout << "  Pass Subscriber Interface: " << passSubscriberInterface << endl << flush;
  cout << "  Pass Subscriber Port: " << passSubscriberPort << endl << flush;
  cout << "  Pass Subscriber Address: " << passSubscriberAddress << endl << flush;
  cout << endl;
}

std::string PassConfigurationManager::getPassServerAddress() {
  return passServerAddress;  
}

std::string PassConfigurationManager::getPassTopic() {
  return passTopic;
}

std::string PassConfigurationManager::getPassSubscriberId() {
  return passSubscriberId;
}

std::string PassConfigurationManager::getPassSubscriberInterface() {
  return passSubscriberInterface;
}

int PassConfigurationManager::getPassSubscriberPort() {
  return passSubscriberPort;
}

std::string PassConfigurationManager::getPassSubscriberAddress() {
  return passSubscriberAddress;
}

string PassConfigurationManager::findConfigFile() {
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
  
  filePath = PASS_CONFIG_FILE;
  //stat returns 0 if the file exists
  if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
    filePath = home + "/" + "." + CONFIG_DIRECTORY + "/" + PASS_CONFIG_FILE;
    if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
      filePath = string("/etc/") + CONFIG_DIRECTORY + "/" + PASS_CONFIG_FILE;
      if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
        filePath = gatewayRoot + "/etc/" + PASS_CONFIG_FILE;
        if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
          filePath = gatewayRoot + "/build/etc/" + PASS_CONFIG_FILE;
          if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
            filePath = string("../etc/") + PASS_CONFIG_FILE;
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

PassConfigurationManager* PassConfigurationManager::getInstance() {
  if(sharedInstance == NULL) {
    sharedInstance = new PassConfigurationManager();
  }
  return sharedInstance;
}
