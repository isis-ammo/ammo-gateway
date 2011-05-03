#ifndef LOCATION_STORE_H
#define LOCATION_STORE_H

#include "GatewayConnector.h"

class sqlite3;

class LocationStoreReceiver : public ammo::gateway::DataPushReceiverListener,
							                public ammo::gateway::GatewayConnectorDelegate,
                              public ammo::gateway::PullRequestReceiverListener
{
public:
  LocationStoreReceiver (void);
  ~LocationStoreReceiver (void);
	
  // GatewayConnectorDelegate methods
  virtual void onConnect (ammo::gateway::GatewayConnector *sender);
  virtual void onDisconnect (ammo::gateway::GatewayConnector *sender);
  
  // DataPushReceiverListener methods
  virtual void onPushDataReceived (ammo::gateway::GatewayConnector *sender,
							   std::string uri,
							   std::string mimeType,
							   std::vector<char> &data,
							   std::string originUser);
	
  // PullRequestReceiverListener methods
  virtual void onPullRequestReceived (ammo::gateway::GatewayConnector *sender, 
							   std::string requestUid,
							   std::string pluginId,
							   std::string mimeType,
							   std::string query,
							   std::string projection,
							   unsigned int maxResults,
							   unsigned int startFromCount,
							   bool liveQuery);
	
private:
  // Pointer to open database.
  sqlite3 *db_;
	
  // Prefix to error messages from data push callbacks
  // to this class, used repeatedly in overridden method.
  const char * const err_prefix_;
};

#endif        //  #ifndef LOCATION_STORE_H

