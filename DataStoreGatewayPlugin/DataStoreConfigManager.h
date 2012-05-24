#ifndef DATASTORE_CONFIG_MANAGER_H
#define DATASTORE_CONFIG_MANAGER_H

#include "json/reader.h"
#include "json/value.h"

class DataStoreReceiver;

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
  DataStoreConfigManager *getInstance (
	  DataStoreReceiver *receiver = 0,
	  ammo::gateway::GatewayConnector *connector = 0);
	  
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
	
	const std::string &getChatMimeType (void) const;
	void setChatMimeType (const std::string &val);
	
private:
  DataStoreConfigManager (
	  DataStoreReceiver *receiver,
	  ammo::gateway::GatewayConnector *connector);
	
  std::string findConfigFile (void);
	
  static DataStoreConfigManager *sharedInstance;
	
private:
  Json::Value root_;
	
  DataStoreReceiver *receiver_;
  ammo::gateway::GatewayConnector *connector_;
  
  std::string event_mime_type_;
  std::string media_mime_type_;
  std::string sms_mime_type_;
  std::string report_mime_type_;
  std::string locations_mime_type_;
  std::string private_contacts_mime_type_;
  std::string chat_mime_type_;
};

#endif // DATASTORE_CONFIG_MANAGER_H
