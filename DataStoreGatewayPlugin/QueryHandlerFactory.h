#ifndef QUERY_HANDLER_FACTORY_H
#define QUERY_HANDLER_FACTORY

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

class QueryHandlerFactory
{
public:
  QueryHandlerFactory (void);
                       
  QueryHandler *createHandler (sqlite3 *db,
                               ammo::gateway::GatewayConnector *sender,
                               ammo::gateway::PullRequest &pr);
};

#endif /* QUERY_HANDLER_FACTORY */
