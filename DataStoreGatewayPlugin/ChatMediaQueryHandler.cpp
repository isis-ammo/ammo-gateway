#include "json/value.h"

#include "ChatMediaQueryHandler.h"
#include "ChatMediaProjectionParser.h"

ChatMediaQueryHandler::ChatMediaQueryHandler (
      sqlite3 *db,
      ammo::gateway::GatewayConnector *sender,
      ammo::gateway::PullRequest &pr)
  : OriginalQueryHandler (db, sender, pr)
{
}

bool
ChatMediaQueryHandler::matchedProjection (const Json::Value &root,
                                          const std::string &projection)
{
  ChatMediaProjectionParser parser;
  parser.parse (projection);
  
  return
    match_string (parser.uuid_, root["uuid"].asString ())
    && match_string (parser.messageId_, root["messageId"].asString ())
    && match_string (parser.name_, root["name"].asString ())
    && match_string (parser.data_, root["data"].asString ())
    && match_int (parser.createdDate_min_, root["createdDate"].asInt (), true)
    && match_int (parser.createdDate_max_, root["createdDate"].asInt (), false)
    && match_int (parser.receipts_min_, root["receipts"].asInt (), true)
    && match_int (parser.receipts_max_, root["receipts"].asInt (), false);
}


