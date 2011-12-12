#ifndef DATASTORE_CONFIG_MANAGER_H
#define DATASTORE_CONFIG_MANAGER_H

#include <vector>
#include <map>

#include "json/reader.h"
#include "json/value.h"

class DataStoreReceiver;
class DataStore_API;

namespace ammo
{
  namespace gateway
  {
    class GatewayConnector;
  }
}

class DataStoreConfigManager
{
public:
  static
  DataStoreConfigManager *create (
	  DataStoreReceiver *receiver,
	  ammo::gateway::GatewayConnector *connector);
	  
	static
	DataStoreConfigManager *getInstance (void);
	  
	const std::string &getEventMimeType (void) const;
	void setEventMimeType (const std::string &val);
	
	const std::string &getMediaMimeType (void) const;
	void setMediaMimeType (const std::string &val);
	
	const std::string &getSMSMimeType (void) const;
	void setSMSMimeType (const std::string &val);
	
	const std::string &getReportMimeType (void) const;
	void setReportMimeType (const std::string &val);
	
	const std::string &getLocationsMimeType (void) const;
	void setLocationsMimeType (const std::string &val);
	
	const std::string &getPrivateContactsMimeType (void) const;
	void setPrivateContactsMimeType (const std::string &val);
	
  typedef std::vector<DataStore_API *> OBJ_LIST;
  typedef std::map<std::string, OBJ_LIST> OBJ_MAP;
	
private:
  DataStoreConfigManager (
	  DataStoreReceiver *receiver,
	  ammo::gateway::GatewayConnector *connector);
	
  std::string findConfigFile (void);
  
  DataStore_API *createObj (const std::string &lib_name);
  void mapObj (DataStore_API *obj, const std::string &mime_type);
	
private:
  typedef std::pair<std::string, OBJ_LIST> OBJ_MAP_ELEM;
  typedef std::vector<std::string> MIME_TYPES;
  
  static DataStoreConfigManager *sharedInstance_;
	
  Json::Value root_;
	
  DataStoreReceiver *receiver_;
  ammo::gateway::GatewayConnector *connector_;
  
  std::string event_mime_type_;
  std::string media_mime_type_;
  std::string sms_mime_type_;
  std::string report_mime_type_;
  std::string locations_mime_type_;
  std::string private_contacts_mime_type_;
  
  OBJ_LIST obj_list_;
  OBJ_MAP obj_map_;
};

#endif // DATASTORE_CONFIG_MANAGER_H
