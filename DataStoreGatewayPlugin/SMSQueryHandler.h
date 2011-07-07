#ifndef SMS_MESSAGE_QUERY_HANDLER_H
#define SMS_MESSAGE_QUERY_HANDLER_H

#include "OriginalQueryHandler.h"
#include "ProjectionFilter.h"

class SMSQueryHandler : public OriginalQueryHandler,
                        public ProjectionFilter
{
public:
  SMSQueryHandler (sqlite3 *db,
                   ammo::gateway::GatewayConnector *sender,
                   ammo::gateway::PullRequest &pr);
                     
protected:
  virtual bool matchedProjection (const Json::Value &root,
                                  const std::string &projection);
};

#endif /* SMS_MESSAGE_QUERY_HANDLER_H */
