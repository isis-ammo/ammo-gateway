#include "LocationStoreConfigManager.h"

#include "log.h"
#include "LocationStore.h"

#include <iostream>
#include <fstream>

#include <ace/OS_NS_sys_stat.h>

const char *CONFIG_DIRECTORY = "ammo-gateway";
const char *LOC_STORE_CONFIG_FILE = "LocationStorePluginConfig.json";

using namespace std;

LocationStoreConfigManager *LocationStoreConfigManager::sharedInstance = 0;

LocationStoreConfigManager::LocationStoreConfigManager (
      LocationStoreReceiver *push_receiver,
      GatewayConnector *the_connector)
  : receiver (push_receiver),
    connector (the_connector)
{
  LOG_TRACE ("Parsing config file...");
	
  string configFilename = findConfigFile();
  
  if(configFilename != "") {
    ifstream configFile (configFilename.c_str());
	
    if (configFile)
      {
      Json::Reader reader;
      
      bool parsingSuccessful = reader.parse (configFile, root);
      
      if (parsingSuccessful)
        {
          if (root["MimeTypes"].isArray ())
          {
          for (Json::Value::iterator i = root["MimeTypes"].begin ();
             i != root["MimeTypes"].end ();
             ++i)
            {
            string mime_type = (*i).asString ();
            LOG_DEBUG ("Registering interest in " << mime_type.c_str ());
            connector->registerDataInterest (mime_type.c_str (), receiver);
            }
          }
      }
      else
        {
        LOG_ERROR ("JSON parsing error in config file '"
             << LOC_STORE_CONFIG_FILE
             << "'.");
        }
      
		
      configFile.close ();
    }
    else
      {
      LOG_WARN ("Could not read from config file '"
          << LOC_STORE_CONFIG_FILE
          << "'.  Using defaults.");
      }
	} else {
    LOG_WARN("Using default configuration.");
  }
}

string LocationStoreConfigManager::findConfigFile() {
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
  
  filePath = LOC_STORE_CONFIG_FILE;
  //stat returns 0 if the file exists
  if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
    filePath = home + "/" + "." + CONFIG_DIRECTORY + "/" + LOC_STORE_CONFIG_FILE;
    if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
      filePath = string("/etc/") + CONFIG_DIRECTORY + "/" + LOC_STORE_CONFIG_FILE;
      if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
        filePath = gatewayRoot + "/etc/" + LOC_STORE_CONFIG_FILE;
        if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
          filePath = gatewayRoot + "/build/etc/" + LOC_STORE_CONFIG_FILE;
          if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
            filePath = string("../etc/") + LOC_STORE_CONFIG_FILE;
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

LocationStoreConfigManager *
LocationStoreConfigManager::getInstance (
  LocationStoreReceiver *push_receiver,
  GatewayConnector *the_connector)
{
  if (sharedInstance == 0)
    {
	  sharedInstance =
		new LocationStoreConfigManager (push_receiver, the_connector);
	}
	
  return sharedInstance;
}
