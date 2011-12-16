#ifndef DATA_STORE_DISPATCHER_H
#define DATA_STORE_DISPATCHER_H

#include "DataStoreConfigManager.h"

class QueryHandler;
class sqlite3;

namespace ammo
{
  namespace gateway
  {
    class PushData;
    class PullRequest;
    class GatewayConnector;
  }
}

class DataStoreDispatcher
{
public:
  DataStoreDispatcher (void);
  
  void
  init (void);
  
  void
  dispatchPushData (sqlite3 *db,
                    ammo::gateway::GatewayConnector *sender,
                    ammo::gateway::PushData &pd);
                       
  void
  dispatchPullRequest (sqlite3 *db,
                       ammo::gateway::GatewayConnector *sender,
                       ammo::gateway::PullRequest &pr);
                            
private:
  void
  appsPushData (ammo::gateway::GatewayConnector *sender,
                ammo::gateway::PushData &pd);
                   
  void
  appsPullRequest (ammo::gateway::GatewayConnector *sender,
                   ammo::gateway::PullRequest &pr);
                        
  DataStoreConfigManager::OBJ_MAP::const_iterator
  findObjList (std::string const &mime_type);
                        
private:
  DataStoreConfigManager *cfg_mgr_;
};

#endif /* DATA_STORE_DISPATCHER_H */
