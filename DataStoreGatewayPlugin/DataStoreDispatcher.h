#ifndef DATA_STORE_DISPATCHER_H
#define DATA_STORE_DISPATCHER_H

class QueryHandler;
class sqlite3;
class DataStoreConfigManager;

namespace ammo
{
  namespace gateway
  {
    class PullRequest;
    class GatewayConnector;
  }
}

class DataStoreDispatcher
{
public:
  DataStoreDispatcher (void);
  
  void init (void);
  
  void dispatchPushData (sqlite3 *db,
                         ammo::gateway::PushData &pd);
                       
  void dispatchPullRequest (sqlite3 *db,
                            ammo::gateway::GatewayConnector *sender,
                            ammo::gateway::PullRequest &pr);
                            
private:
  DataStoreConfigManager *cfg_mgr_;
};

#endif /* DATA_STORE_DISPATCHER_H */
