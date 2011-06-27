#ifndef MEDIA_QUERY_HANDLER_H
#define MEDIA_QUERY_HANDLER_H

#include "OriginalQueryHandler.h"
#include "ProjectionFilter.h"

class MediaQueryHandler : public OriginalQueryHandler,
                          public ProjectionFilter
{
public:
  MediaQueryHandler (sqlite3 *db,
                     ammo::gateway::GatewayConnector *sender,
                     ammo::gateway::PullRequest &pr);
                     
protected:
  virtual bool matchedProjection (const Json::Value &root,
                                  const std::string &projection);
};

#endif /* MEDIA_QUERY_HANDLER_H */
