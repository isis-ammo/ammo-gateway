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
	
	const std::string &getReqCsumsMimeType (void) const;
	void setReqCsumsMimeType (const std::string &val);
	
	const std::string &getSendCsumsMimeType (void) const;
	void setSendCsumsMimeType (const std::string &val);
	
	const std::string &getReqObjsMimeType (void) const;
	void setReqObjsMimeType (const std::string &val);
	
	const std::string &getSendObjsMimeType (void) const;
	void setSendObjsMimeType (const std::string &val);
	
	unsigned long getSyncReachBackSecs (void) const;
	
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
  
  std::string req_csums_mime_type_;
  std::string send_csums_mime_type_;
  std::string req_objs_mime_type_;
  std::string send_objs_mime_type_;
  
  unsigned long sync_reach_back_secs_;
};

#endif // DATASTORE_CONFIG_MANAGER_H
