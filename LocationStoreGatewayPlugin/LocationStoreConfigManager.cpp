#include "LocationStoreConfigManager.h"

#include "log.h"
#include "LocationStore.h"

#include <iostream>
#include <fstream>

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
	
  ifstream configFile (LOC_STORE_CONFIG_FILE);
	
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
