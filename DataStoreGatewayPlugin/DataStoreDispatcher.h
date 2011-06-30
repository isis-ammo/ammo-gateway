#ifndef DATA_STORE_DISPATCHER_H
#define DATA_STORE_DISPATCHER_H

class QueryHandler;
class sqlite3;

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
};

#endif /* DATA_STORE_DISPATCHER_H */
