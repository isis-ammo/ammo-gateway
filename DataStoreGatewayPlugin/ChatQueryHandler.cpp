#include "json/value.h"

#include "ChatQueryHandler.h"
#include "ChatProjectionParser.h"

ChatQueryHandler::ChatQueryHandler (
      sqlite3 *db,
      ammo::gateway::GatewayConnector *sender,
      ammo::gateway::PullRequest &pr)
  : OriginalQueryHandler (db, sender, pr)
{
}

bool
ChatQueryHandler::matchedProjection (const Json::Value &root,
                                     const std::string &projection)
{
  ChatProjectionParser parser;
  parser.parse (projection);
  
  return
    match_string (parser.originator_, root["originator"].asString ())
    && match_string (parser.group_id_, root["group_id"].asString ())
    && match_string (parser.text_, root["text"].asString ())
    && match_real (parser.latitude_min_, root["latitude"].asDouble (), true)
    && match_real (parser.latitude_max_, root["latitude"].asDouble (), false)
    && match_real (parser.longitude_min_, root["longitude"].asDouble (), true)
    && match_real (parser.longitude_max_, root["longitude"].asDouble (), false)
    && match_string (parser.uuid_, root["uuid"].asString ())
    && match_int (parser.pending_receipts_min_, root["pending_receipts"].asInt (), true)
    && match_int (parser.pending_receipts_max_, root["pending_receipts"].asInt (), false)
    && match_int (parser.media_count_min_, root["media_count"].asInt (), true)
    && match_int (parser.media_count_max_, root["media_count"].asInt (), false)
    && match_int (parser.status_min_, root["status"].asInt (), true)
    && match_int (parser.status_max_, root["status"].asInt (), false)
    && match_int (parser.created_date_min_, root["created_date"].asInt (), true)
    && match_int (parser.created_date_max_, root["created_date"].asInt (), false)
    && match_int (parser.modified_date_min_, root["modified_date"].asInt (), true)
    && match_int (parser.modified_date_max_, root["modified_date"].asInt (), false);
}


