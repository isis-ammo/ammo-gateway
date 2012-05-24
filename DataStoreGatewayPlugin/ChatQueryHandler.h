#ifndef CHAT_QUERY_HANDLER_H
#define CHAT_QUERY_HANDLER_H

#include "OriginalQueryHandler.h"
#include "ProjectionFilter.h"

class ChatQueryHandler : public OriginalQueryHandler,
                         public ProjectionFilter
{
public:
  ChatQueryHandler (sqlite3 *db,
                    ammo::gateway::GatewayConnector *sender,
                    ammo::gateway::PullRequest &pr);
                     
protected:
  virtual bool matchedProjection (const Json::Value &root,
                                  const std::string &projection);
};

#endif /* CHAT_QUERY_HANDLER_H */
