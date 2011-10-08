#ifndef QUERY_HANDLER_H
#define QUERY_HANDLER_H

#include <string>

#include "LibGatewayDataStore_Export.h"

class sqlite3;

namespace Json
{
  class Value;
}

namespace ammo
{
  namespace gateway
  {
    class PullRequest;
    class GatewayConnector;
  }
}

class LibGatewayDataStore_Export QueryHandler
{
public:
  QueryHandler (sqlite3 *db,
                ammo::gateway::GatewayConnector *sender,
                ammo::gateway::PullRequest &pr);
                
  ~QueryHandler (void);
                
protected:
  sqlite3 *db_;
  ammo::gateway::GatewayConnector *sender_;
  ammo::gateway::PullRequest &pr_;
};

#endif /* QUERY_HANDLER_H */
