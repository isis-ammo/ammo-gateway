#ifndef QUERY_HANDLER_H
#define QUERY_HANDLER_H

#include <string>

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

class QueryHandler
{
public:
  QueryHandler (sqlite3 *db,
                ammo::gateway::GatewayConnector *sender,
                ammo::gateway::PullRequest &pr);
                
  virtual ~QueryHandler (void);
                
  virtual void handleQuery (void) = 0;
  
protected:
  bool parseJson (const std::string &input,
                  Json::Value& root);
  
protected:
  sqlite3 *db_;
  ammo::gateway::GatewayConnector *sender_;
  ammo::gateway::PullRequest &pr_;
};

#endif /* QUERY_HANDLER_H */
