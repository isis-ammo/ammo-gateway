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
							   ammo::gateway::PushData &pushData);
	
  // PullRequestReceiverListener methods
  virtual void onPullRequestReceived (ammo::gateway::GatewayConnector *sender, ammo::gateway::PullRequest &pullReq);
	
private:
  // Pointer to open database.
  sqlite3 *db_;
	
  // Prefix to error messages from data push callbacks
  // to this class, used repeatedly in overridden method.
  const char * const err_prefix_;
};

#endif        //  #ifndef LOCATION_STORE_H

