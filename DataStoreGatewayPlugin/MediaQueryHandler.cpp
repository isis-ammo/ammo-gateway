#include "json/value.h"

#include "MediaQueryHandler.h"
#include "MediaProjectionParser.h"

MediaQueryHandler::MediaQueryHandler (
      sqlite3 *db,
      ammo::gateway::GatewayConnector *sender,
      ammo::gateway::PullRequest &pr)
  : OriginalQueryHandler (db, sender, pr)
{
}

bool
MediaQueryHandler::matchedProjection (const Json::Value &root,
                                      const std::string &projection)
{
  MediaProjectionParser parser;
  parser.parse (projection);
  
  return
    match_string (parser.eventId (), root["eventId"].asString ())
    && match_string (parser.dataType (), root["dataType"].asString ())
    && match_string (parser.data (), root["data"].asString ())
    && match_int (parser.createdDate_min (), root["createdDate"].asInt (), true)
    && match_int (parser.createdDate_max (), root["createdDate"].asInt (), false)
    && match_int (parser.modifiedDate_min (), root["modifiedDate"].asInt (), true)
    && match_int (parser.modifiedDate_max (), root["modifiedDate"].asInt (), false);
}


