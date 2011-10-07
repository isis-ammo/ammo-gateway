#include <iostream>
#include <fstream>

#include <ace/OS_NS_sys_stat.h>

#include "log.h"

#include "DataStoreConfigManager.h"
#include "DataStoreReceiver.h"

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
              // This dummy type is still hanging around in some old test scripts.  
              string test_mime_type ("text/plain");  
              connector_->registerDataInterest (test_mime_type, receiver_);
              connector_->registerPullInterest (test_mime_type, receiver_);
                
              const char *mime_type_labels[] =
                {
                  "EventMimeType",
                  "MediaMimeType",
                  "SMSMimeType",
                  "ReportMimeType",
                  "LocationsMimeType",
                  "PrivateContactsMimeType"
                };
                
              const unsigned long ARRAY_SIZE = 6;
                
              void (DataStoreConfigManager::*store_ops[ARRAY_SIZE]) (const string &) =
                {
                  &DataStoreConfigManager::setEventMimeType,
                  &DataStoreConfigManager::setMediaMimeType,
                  &DataStoreConfigManager::setSMSMimeType,
                  &DataStoreConfigManager::setReportMimeType,
                  &DataStoreConfigManager::setLocationsMimeType,
                  &DataStoreConfigManager::setPrivateContactsMimeType
                };
                
              for (unsigned long i = 0; i < ARRAY_SIZE; ++i)
                {
                  if (! root_[ mime_type_labels[i] ].isString ())
                    {
                      LOG_ERROR (mime_type_labels[i]
                                 << " is missing or malformed in config file");
                                 
                      continue;
                    }
                    
                  string mime_type (root_[ mime_type_labels[i] ].asString ());
                  LOG_DEBUG ("Registering interest in " << mime_type);
                  connector_->registerDataInterest (mime_type, receiver_);
                  connector_->registerPullInterest (mime_type, receiver_);
                  (this->*store_ops[i])(mime_type);
                }
                
              if (root_["DatabasePath"].isString ())
                {
                  receiver_->db_filepath (root_["DatabasePath"].asString ());
                }
              else
                {
                  LOG_ERROR ("DatabasePath string is missing "
                             << "or malformed in config file");
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

DataStoreConfigManager *
DataStoreConfigManager::getInstance (DataStoreReceiver *receiver,
                                     GatewayConnector *connector)
{
  if (sharedInstance == 0)
    {
      if (receiver == 0 || connector == 0)
        {
          LOG_ERROR ("First call to getInstance() must pass in"
                     "a receiver and a connector");
                     
          return 0;
        }
        
	    sharedInstance =
		    new DataStoreConfigManager (receiver, connector);
	  }
	
  return sharedInstance;
}

const std::string &
DataStoreConfigManager::getEventMimeType (void) const
{
  return event_mime_type_;
}

void
DataStoreConfigManager::setEventMimeType (const std::string &val)
{
  event_mime_type_ = val;
}

const std::string &
DataStoreConfigManager::getMediaMimeType (void) const
{
  return media_mime_type_;
}

void
DataStoreConfigManager::setMediaMimeType (const std::string &val)
{
  media_mime_type_ = val;
}

const std::string &
DataStoreConfigManager::getSMSMimeType (void) const
{
  return sms_mime_type_;
}

void
DataStoreConfigManager::setSMSMimeType (const std::string &val)
{
  sms_mime_type_ = val;
}

const std::string &
DataStoreConfigManager::getReportMimeType (void) const
{
  return report_mime_type_;
}

void
DataStoreConfigManager::setReportMimeType (const std::string &val)
{
  report_mime_type_ = val;
}

const std::string &
DataStoreConfigManager::getLocationsMimeType (void) const
{
  return locations_mime_type_;
}

void
DataStoreConfigManager::setLocationsMimeType (const std::string &val)
{
  locations_mime_type_ = val;
}

const std::string &
DataStoreConfigManager::getPrivateContactsMimeType (void) const
{
  return private_contacts_mime_type_;
}

void
DataStoreConfigManager::setPrivateContactsMimeType (const std::string &val)
{
  private_contacts_mime_type_ = val;
}

string
DataStoreConfigManager::findConfigFile (void)
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
  
  LOG_INFO ("Using config file: " << filePath);
  return filePath;
}


