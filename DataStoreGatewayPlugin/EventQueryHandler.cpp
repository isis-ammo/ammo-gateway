#include "json/value.h"

#include "EventQueryHandler.h"
#include "EventProjectionParser.h"

EventQueryHandler::EventQueryHandler (
      sqlite3 *db,
      ammo::gateway::GatewayConnector *sender,
      ammo::gateway::PullRequest &pr)
  : OriginalQueryHandler (db, sender, pr)
{
}

bool
EventQueryHandler::matchedProjection (const Json::Value &root,
                                      const std::string &projection)
{
  EventProjectionParser parser;
  parser.parse (projection);
  
  return
    match_string (parser.uuid (), root["uuid"].asString ())
    && match_int (parser.mediaCount_min (), root["mediaCount"].asInt (), true)
    && match_int (parser.mediaCount_max (), root["mediaCount"].asInt (), false)
    && match_string (parser.displayName (), root["displayName"].asString ())
    && match_string (parser.categoryId (), root["categoryId"].asString ())
    && match_string (parser.title (), root["title"].asString ())
    && match_string (parser.description (), root["description"].asString ())
    && match_real (parser.longitude_min (), root["longitude"].asDouble (), true)
    && match_real (parser.longitude_max (), root["longitude"].asDouble (), false)
    && match_real (parser.latitude_min (), root["latitude"].asDouble (), true)
    && match_real (parser.latitude_max (), root["latitude"].asDouble (), false)
    && match_int (parser.createdDate_min (), root["createdDate"].asInt (), true)
    && match_int (parser.createdDate_max (), root["createdDate"].asInt (), false)
    && match_int (parser.modifiedDate_min (), root["modifiedDate"].asInt (), true)
    && match_string (parser.cid (), root["cid"].asString ())
    && match_string (parser.category (), root["category"].asString ())
    && match_string (parser.unit (), root["unit"].asString ())
    && match_int (parser.size_min (), root["size"].asInt (), true)
    && match_int (parser.size_max (), root["size"].asInt (), false)
    && match_string (parser.dest_group_name (), root["destGroupName"].asString ())
    && match_string (parser.dest_group_type (), root["destGroupType"].asString ())
    && match_int (parser.status_min (), root["status"].asInt (), true)
    && match_int (parser.status_max (), root["status"].asInt (), false);
}


