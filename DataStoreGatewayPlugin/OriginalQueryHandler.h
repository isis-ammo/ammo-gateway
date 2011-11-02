#ifndef ORIGINAL_QUERY_HANDLER_H
#define ORIGINAL_QUERY_HANDLER_H

#include "QueryHandler.h"
#include "OriginalQueryStatementBuilder.h"

class OriginalQueryHandler : public QueryHandler
{
public:
  OriginalQueryHandler (sqlite3 *db,
                        ammo::gateway::GatewayConnector *sender,
                        ammo::gateway::PullRequest &pr);
                        
  ~OriginalQueryHandler (void);
                        
  void handleQuery (void);

protected:
  bool matchedData (const std::string &projection,
                    const std::string &data);

  virtual bool matchedProjection (const Json::Value &root,
                                  const std::string &projection) = 0;
protected:
  OriginalQueryStatementBuilder builder_;
};

#endif /* ORIGINAL_QUERY_HANDLER_H */
