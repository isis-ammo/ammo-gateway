#ifndef CHAT_MEDIA_QUERY_HANDLER_H
#define CHAT_MEDIA_QUERY_HANDLER_H

#include "OriginalQueryHandler.h"
#include "ProjectionFilter.h"

class ChatMediaQueryHandler : public OriginalQueryHandler,
                              public ProjectionFilter
{
public:
  ChatMediaQueryHandler (sqlite3 *db,
                         ammo::gateway::GatewayConnector *sender,
                         ammo::gateway::PullRequest &pr);
                     
protected:
  bool matchedProjection (const Json::Value &root,
                          const std::string &projection);
};

#endif /* CHAT_MEDIA_QUERY_HANDLER_H */
