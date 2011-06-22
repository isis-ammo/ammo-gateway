#include "DataStoreConfigManager.h"

#include "log.h"
#include "DataStore.h"

#include <iostream>
#include <fstream>

#include <ace/OS_NS_sys_stat.h>

const char *CONFIG_DIRECTORY = "ammo-gateway";
const char *LOC_STORE_CONFIG_FILE = "DataStorePluginConfig.json";

using namespace std;
using namespace ammo::gateway;

DataStoreConfigManager *DataStoreConfigManager::sharedInstance = 0;

DataStoreConfigManager::DataStoreConfigManager (
      DataStoreReceiver *receiver,
      GatewayConnector *connector)
  : receiver_ (receiver),
    connector_ (connector)
{
  LOG_TRACE ("Parsing config file...");
	
  string configFilename = findConfigFile ();
  
  if (configFilename != "")
    {
      ifstream configFile (configFilename.c_str ());
	
      if (configFile)
        {
          Json::Reader reader;
        
          bool parsingSuccessful = reader.parse (configFile, root_);
      
          if (parsingSuccessful)
            {
              if (root_["MimeTypes"].isArray ())
                {
		              for (Json::Value::iterator i = root_["MimeTypes"].begin ();
                       i != root_["MimeTypes"].end ();
                       ++i)
                    {
                      const char *mime_type = (*i).asString ().c_str ();
                      LOG_DEBUG ("Registering interest in " << mime_type);
                      connector_->registerDataInterest (mime_type, receiver_);
                      connector_->registerPullInterest (mime_type, receiver_, SCOPE_GLOBAL);
                    }
                }
              else
                {
                  LOG_ERROR ("MimeTypes string array is malformed in config file");
                }
                
              if (root_["DatabasePath"].isString ())
                {
                  receiver_->db_filepath (root_["DatabasePath"].asString ());
                }
              else
                {
                  LOG_ERROR ("DatabasePath string is malformed in config file");
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
	}
  else
    {
      LOG_WARN ("Using default configuration.");
    }
}

string DataStoreConfigManager::findConfigFile ()
{
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

DataStoreConfigManager *
DataStoreConfigManager::getInstance (DataStoreReceiver *receiver,
                                     GatewayConnector *connector)
{
  if (sharedInstance == 0)
    {
	    sharedInstance =
		    new DataStoreConfigManager (receiver, connector);
	  }
	
  return sharedInstance;
}
