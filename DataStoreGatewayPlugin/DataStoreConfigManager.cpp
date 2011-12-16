#include <iostream>
#include <fstream>

#include <ace/OS_NS_sys_stat.h>
#include <ace/DLL.h>
#include <ace/DLL_Manager.h>

#include "log.h"

#include "DataStoreConfigManager.h"
#include "DataStoreReceiver.h"
#include "DataStore_API.h"

#if defined (ACE_LD_DECORATOR_STR)
# define OBJ_SUFFIX ACE_LD_DECORATOR_STR ACE_DLL_SUFFIX
#else
# define OBJ_SUFFIX ACE_DLL_SUFFIX
#endif /* ACE_LD_DECORATOR_STR */

#if defined (ACE_WIN32) || defined (ACE_OPENVMS)
#  define OBJ_PREFIX ACE_DLL_PREFIX
#else
#  define OBJ_PREFIX "./" ACE_DLL_PREFIX
#endif /* ACE_WIN32 */

const char *CONFIG_DIRECTORY = "ammo-gateway";
const char *LOC_STORE_CONFIG_FILE = "DataStorePluginConfig.json";

using namespace std;
using namespace ammo::gateway;

DataStoreConfigManager *DataStoreConfigManager::sharedInstance_ = 0;

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
              // Keeps any loaded DLLs from unloading until the plugin
              // exits. On Windows, each DLL has its own heap, and it
              // goes away with the DLL, which would normally be unloaded
              // when the associated ACE_DLL object goes out of scope.
              ACE_DLL_Manager::instance ()->unload_policy (
                ACE_DLL_UNLOAD_POLICY_LAZY);

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
                  LOG_ERROR ("DatabasePath JSON string is missing "
                             "or malformed in config file");
                }
                
              if (! root_["DataStoreLibs"].empty ()
                  && root_["DataStoreLibs"].isObject ())
                {
                  for (Json::ValueIterator i = root_["DataStoreLibs"].begin ();
                       i != root_["DataStoreLibs"].end ();
                       ++i)
                    {
                      if (i.key ().isString ())
                        {
                          string lib_path (i.key ().asString ());
                          DataStore_API *obj = this->createObj (lib_path);
                          
                          if ((*i).isArray ())
                            {
                              for (Json::ValueIterator j = (*i).begin ();
                                   j != (*i).end ();
                                   ++j)
                                {
                                  LOG_DEBUG ("Associating object of " << lib_path
                                             << " with \"" << (*j).asString () << "\"");
                                  this->mapObj (obj, (*j).asString ());
                                }
                            }
                          else
                            {
                              LOG_ERROR ("User lib associated mime types string "
                                         "array is malformed in config file");
                            }
                        }
                      else
                        {
                          LOG_ERROR ("User lib JSON string is malformed "
                                     "in config file");
                        }
                    }
                }
              else
                {
                  LOG_ERROR ("DataStoreLibs string is malformed "
                             "in config file");
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
DataStoreConfigManager::create (DataStoreReceiver *receiver,
                                GatewayConnector *connector)
{
  if (sharedInstance_ == 0)
    {
      if (receiver == 0 || connector == 0)
        {
          LOG_ERROR ("DataStoreConfigManager::create - "
                     "Creation of DataStoreConfigManager requires"
                     "a receiver and a connector");
                     
          return 0;
        }
        
	    sharedInstance_ =
		    new DataStoreConfigManager (receiver, connector);
	  }
	
  return sharedInstance_;
}

DataStoreConfigManager *
DataStoreConfigManager::getInstance (void)
{
  if (sharedInstance_ == 0)
    {
      LOG_ERROR ("DataStoreConfigManager::getInstance - "
                 "Call create() first with appropriate arguments");
      
      return 0;
    }
    
  return sharedInstance_;
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

DataStoreConfigManager::OBJ_MAP const &
DataStoreConfigManager::obj_map (void) const
{
  return this->obj_map_;
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

DataStore_API *
DataStoreConfigManager::createObj (const std::string &lib_path)
{
  char delimiter = ACE_DIRECTORY_SEPARATOR_CHAR;
  std::string full_path (lib_path);
  
  // If the path has an environment variable, expand it first.
  if (lib_path[0] == '$')
    {
      char *expanded = ACE_OS::strenvdup (lib_path.c_str ());
      full_path = expanded;
      delete expanded;
      expanded = 0;
    }
    
  std::string::size_type pos = full_path.find_last_of (delimiter);
  std::string full_name, local_name;
  full_name.reserve (MAXPATHLEN);
  
  if (pos != std::string::npos)
    {
      full_name.append (full_path.substr (0, pos));
      full_name.append (1, delimiter);
      local_name = full_path.substr (pos + 1);
    }
  else
    {
      local_name = full_path;
    }
    
  // Make the DLL filename portable.
  full_name.append (OBJ_PREFIX);
  full_name.append (local_name);
  full_name.append (OBJ_SUFFIX);
  
  ACE_DLL dll;
  int retval = dll.open (full_name.c_str ());

  if (retval != 0)
    {
      char *dll_error = dll.error ();
      LOG_ERROR ("Error in DLL Open of "
                  << full_name
                  << ": "
                  << (dll_error != 0 ? dll_error : "unknown error"));
                  
      return 0;
    }
  else
    {
      LOG_DEBUG ("DLL " << full_name << " opened successfully");
    }

  // We require that the factory function name have this simple
  // relationship to the lib name.
  std::string symbol_str ("create_");
  symbol_str.append (local_name);
  void *vtmp = dll.symbol (symbol_str.c_str ());

  // ANSI C++ says you can't cast a void* to a function pointer.
  ptrdiff_t tmp = reinterpret_cast<ptrdiff_t> (vtmp);
  API_Factory factory = reinterpret_cast<API_Factory> (tmp);

  if (factory == 0)
    {
      LOG_ERROR ("Error resolving factory function: "
                 << dll.error ());
                 
      return 0;
    }
    
  DataStore_API *obj = factory ();
  
  // Store for deletion at cleanup time.
  obj_list_.push_back (obj);

  return obj;
}

void
DataStoreConfigManager::mapObj (DataStore_API *obj,
                                const std::string &mime_type)
{
  OBJ_MAP::iterator i = obj_map_.find (mime_type);

  if (i == obj_map_.end ())
    {
      OBJ_LIST per_mime;
      per_mime.push_back (obj);
      obj_map_.insert (OBJ_MAP_ELEM (mime_type, per_mime));
    }
  else
    {
      (*i).second.push_back (obj);
    }
}

