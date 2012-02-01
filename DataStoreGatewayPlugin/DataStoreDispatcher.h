#ifndef DATA_STORE_DISPATCHER_H
#define DATA_STORE_DISPATCHER_H

class QueryHandler;
struct sqlite3;
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
  
  void dispatchPushData (sqlite3 *db,
                         ammo::gateway::PushData &pd);
                       
  void dispatchPullRequest (sqlite3 *db,
                            ammo::gateway::GatewayConnector *sender,
                            ammo::gateway::PullRequest &pr);
                            
  void set_cfg_mgr (DataStoreConfigManager *cfg_mgr);
                            
private:
  DataStoreConfigManager *cfg_mgr_;
};

#endif /* DATA_STORE_DISPATCHER_H */
