#ifndef DATA_STORE_RECEIVER_H
#define DATA_STORE_RECEIVER_H

#include "GatewayConnector.h"

#include "DataStoreDispatcher.h"

struct sqlite3;

class DataStoreReceiver : public ammo::gateway::DataPushReceiverListener,
					                public ammo::gateway::GatewayConnectorDelegate,
                          public ammo::gateway::PullRequestReceiverListener
{
public:
  DataStoreReceiver (void);
  ~DataStoreReceiver (void);
	
  // GatewayConnectorDelegate methods
  // ========================================================
  
  virtual void
  onConnect (
    ammo::gateway::GatewayConnector *sender);
  
  virtual void
  onDisconnect (
    ammo::gateway::GatewayConnector *sender);
  
  virtual void
  onRemoteGatewayConnected (
    ammo::gateway::GatewayConnector *sender,
    const std::string &gatewayId,
    const PluginList &connectedPlugins);
  
  virtual void
  onPluginConnected (
    ammo::gateway::GatewayConnector *sender,
    const ammo::gateway::PluginInstanceId &pluginId,
    const bool remotePlugin,
    const std::string &gatewayId);
  
  virtual void
  onPointToPointMessageReceived (
    ammo::gateway::GatewayConnector *sender,
    const ammo::gateway::PointToPointMessage &message);
                                              
  //============================================================
  
  // DataPushReceiverListener methods
  virtual void
  onPushDataReceived (
    ammo::gateway::GatewayConnector *sender,
		ammo::gateway::PushData &pushData);
	
  // PullRequestReceiverListener methods
  virtual void
  onPullRequestReceived (
    ammo::gateway::GatewayConnector *sender,
    ammo::gateway::PullRequest &pullReq);
                               
  void db_filepath (const std::string &path);
  bool init (void);

private:
  // Create all directories that don't exist, expanding
  // environment variables if necessary.
  bool check_path (void);
  
  // Match entries stored later than tv's value, and store
  // their checksums in the class member.
  bool fetch_recent_checksums (const ACE_Time_Value &tv);
  
  // Match entries whose checksums are equal to any in the list arg.
  bool match_requested_checksums (const std::vector<std::string> &checksums);
  
  // Identify which checksums from a list are not in the local db.
  bool collect_missing_checksums (const std::vector<std::string> &checksums);

private:
  // Dispatcher for pushes and pulls.
  DataStoreDispatcher dispatcher_;
  
  // Pointer to open database.
  sqlite3 *db_;
  
  // Set by the config manager.
  std::string db_filepath_;
  
  // Persistent container for checksums.
  std::vector<std::string> checksums_;
};

#endif        //  #ifndef DATA_STORE_RECEIVER_H

