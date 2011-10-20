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
    match_string (parser.eventId_, root["eventId"].asString ())
    && match_string (parser.dataType_, root["dataType"].asString ())
    && match_string (parser.data_, root["data"].asString ())
    && match_int (parser.createdDate_min_, root["createdDate"].asInt (), true)
    && match_int (parser.createdDate_max_, root["createdDate"].asInt (), false)
    && match_int (parser.modifiedDate_min_, root["modifiedDate"].asInt (), true)
    && match_int (parser.modifiedDate_max_, root["modifiedDate"].asInt (), false);
}


