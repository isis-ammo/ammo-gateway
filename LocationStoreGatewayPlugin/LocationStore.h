#ifndef LOCATION_STORE_H
#define LOCATION_STORE_H

#include "GatewayConnector.h"

class sqlite3;

namespace Json
{
  class Value;
}

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
  virtual void onPullRequestReceived (ammo::gateway::GatewayConnector *sender,
                                      ammo::gateway::PullRequest &pullReq);
                               
  void db_filepath (const std::string &path);
  bool init (void);

private:
  bool matchedData (const std::string &mimeType,
                    const std::string &projection,
                    const std::vector<char> &data);
                    
  bool matchedEvent (const Json::Value &root,
                     const std::string &projection);
                     
  bool matchedMedia (const Json::Value &root,
                     const std::string &projection);
                     
  bool matchedSMS (const Json::Value &root,
                   const std::string &projection);
                   
  bool matchedReport (const Json::Value &root,
                      const std::string &projection);
	
private:
  // Pointer to open database.
  sqlite3 *db_;
	
  // Prefix to error messages from data push callbacks
  // to this class, used repeatedly in overridden method.
  const char * const err_prefix_;
  
  // Set by the config manager.
  std::string db_filepath_;
};

#endif        //  #ifndef LOCATION_STORE_H

