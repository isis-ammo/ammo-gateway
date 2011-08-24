#ifndef REPORT_QUERY_HANDLER_H
#define REPORT_QUERY_HANDLER_H

#include "OriginalQueryHandler.h"
#include "ProjectionFilter.h"

class ReportQueryHandler : public OriginalQueryHandler,
                           public ProjectionFilter
{
public:
  ReportQueryHandler (sqlite3 *db,
                      ammo::gateway::GatewayConnector *sender,
                      ammo::gateway::PullRequest &pr);
                     
protected:
  virtual bool matchedProjection (const Json::Value &root,
                                  const std::string &projection);
};

#endif /* REPORT_QUERY_HANDLER_H */
