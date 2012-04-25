#include <iostream>
#include <fstream>

#include <ace/OS_NS_sys_stat.h>

#include "log.h"

#include "DataStoreConfigManager.h"
#include "DataStoreReceiver.h"

#ifdef WIN32
#include <Windows.h>
#include <shlwapi.h>
#include <ShlObj.h>
#endif


const char *CONFIG_DIRECTORY = "ammo-gateway";
const char *LOC_STORE_CONFIG_FILE = "DataStorePluginConfig.json";

using namespace std;
using namespace ammo::gateway;

DataStoreConfigManager *DataStoreConfigManager::sharedInstance = 0;

DataStoreConfigManager::DataStoreConfigManager (
      DataStoreReceiver *receiver,
      GatewayConnector *connector)
  : receiver_ (receiver),
    connector_ (connector),
    sync_reach_back_secs_ (0)
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
                  "PrivateContactsMimeType",
                  "ReqCsumMimeType",
                  "SendCsumMimeType",
                  "ReqObjsMimeType",
                  "SendObjsMimeType"
                };
                
              const unsigned long CONTRACT_ARRAY_SIZE = 6;
              const unsigned long TOTAL_ARRAY_SIZE = 10;
                
              void (DataStoreConfigManager::*store_ops[TOTAL_ARRAY_SIZE]) (const string &) =
                {
                  &DataStoreConfigManager::setEventMimeType,
                  &DataStoreConfigManager::setMediaMimeType,
                  &DataStoreConfigManager::setSMSMimeType,
                  &DataStoreConfigManager::setReportMimeType,
                  &DataStoreConfigManager::setLocationsMimeType,
                  &DataStoreConfigManager::setPrivateContactsMimeType,
                  &DataStoreConfigManager::setReqCsumsMimeType,
                  &DataStoreConfigManager::setSendCsumsMimeType,
                  &DataStoreConfigManager::setReqObjsMimeType,
                  &DataStoreConfigManager::setSendObjsMimeType,
                };
                
              for (unsigned long i = 0; i < TOTAL_ARRAY_SIZE; ++i)
                {
                  if (! root_[ mime_type_labels[i] ].isString ())
                    {
                      LOG_ERROR (mime_type_labels[i]
                                 << " is missing or malformed in config file");
                                 
                      continue;
                    }
                    
                  string mime_type (root_[ mime_type_labels[i] ].asString ());
                  (this->*store_ops[i])(mime_type);
                  
                  if (i < CONTRACT_ARRAY_SIZE)
                    {
                      LOG_DEBUG ("Registering interest in " << mime_type);
                      connector_->registerDataInterest (mime_type, receiver_);
                      connector_->registerPullInterest (mime_type, receiver_);
                    }
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
                
              if (root_["SyncReachBackSecs"].isInt ())
                {
                  sync_reach_back_secs_ = root_["SyncReachBackSecs"].asInt ();
                }
              else
                {
                  LOG_ERROR ("Gateway sync time offset is missing "
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

const std::string &
DataStoreConfigManager::getReqCsumsMimeType (void) const
{
  return req_csums_mime_type_;
}

void
DataStoreConfigManager::setReqCsumsMimeType (const std::string &val)
{
  req_csums_mime_type_ = val;
}

const std::string &
DataStoreConfigManager::getSendCsumsMimeType (void) const
{
  return send_csums_mime_type_;
}

void
DataStoreConfigManager::setSendCsumsMimeType (const std::string &val)
{
  send_csums_mime_type_ = val;
}

const std::string &
DataStoreConfigManager::getReqObjsMimeType (void) const
{
  return req_objs_mime_type_;
}

void
DataStoreConfigManager::setReqObjsMimeType (const std::string &val)
{
   req_objs_mime_type_ = val;
}

const std::string &
DataStoreConfigManager::getSendObjsMimeType (void) const
{
   return send_objs_mime_type_;
}

void
DataStoreConfigManager::setSendObjsMimeType (const std::string &val)
{
  send_objs_mime_type_ = val;
}
	
long
DataStoreConfigManager::getSyncReachBackSecs (void) const
{
  return sync_reach_back_secs_;
}

#ifndef WIN32
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
string DataStoreConfigManager::findConfigFile() {
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
  
  filePath = LOC_STORE_CONFIG_FILE;
  //stat returns 0 if the file exists
  if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
    filePath = string(userConfigPath) + "\\" + CONFIG_DIRECTORY + "\\" + LOC_STORE_CONFIG_FILE;
    if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
      filePath = string(systemConfigPath) + "\\" + CONFIG_DIRECTORY + "\\" + LOC_STORE_CONFIG_FILE;
      if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
        filePath = gatewayRoot + "\\etc\\" + LOC_STORE_CONFIG_FILE;
        if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
          filePath = gatewayRoot + "\\build\\etc\\" + LOC_STORE_CONFIG_FILE;
          if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
            filePath = string("..\\etc\\") + LOC_STORE_CONFIG_FILE;
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



