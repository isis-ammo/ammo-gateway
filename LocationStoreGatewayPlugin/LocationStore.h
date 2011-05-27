#ifndef LOCATION_STORE_H
#define LOCATION_STORE_H

#include "GatewayConnector.h"

class sqlite3;

namespace Json
{
  class Value;
}

class LocationStoreReceiver : public DataPushReceiverListener,
                              public GatewayConnectorDelegate,
                              public PullRequestReceiverListener
{
public:
  LocationStoreReceiver (void);
  ~LocationStoreReceiver (void);
	
  // GatewayConnectorDelegate methods
  virtual void onConnect (GatewayConnector *sender);
  virtual void onDisconnect (GatewayConnector *sender);
  
  // DataPushReceiverListener methods
  virtual void onDataReceived (GatewayConnector *sender,
                               std::string uri,
                               std::string mimeType,
                               std::vector<char> &data,
                               std::string originUser);
	
  // PullRequestReceiverListener methods
  virtual void onDataReceived (GatewayConnector *sender, 
                               std::string requestUid,
                               std::string pluginId,
                               std::string mimeType,
                               std::string query,
                               std::string projection,
                               unsigned int maxResults,
                               unsigned int startFromCount,
                               bool liveQuery);
                               
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

