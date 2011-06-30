#include "json/value.h"

#include "SMSQueryHandler.h"
#include "SMSProjectionParser.h"

SMSQueryHandler::SMSQueryHandler (
      sqlite3 *db,
      ammo::gateway::GatewayConnector *sender,
      ammo::gateway::PullRequest &pr)
  : OriginalQueryHandler (db, sender, pr)
{
}

bool
SMSQueryHandler::matchedProjection (const Json::Value &root,
                                    const std::string &projection)
{
  SMSProjectionParser parser;
  parser.parse (projection);
  
  return
    match_string (parser.sms_uri (), root["sms_uri"].asString ())
    && match_string (parser.sender (), root["sender"].asString ())
    && match_string (parser.recipient (), root["recipient"].asString ())
    && match_int (parser.thread_min (), root["thread"].asInt (), true)
    && match_int (parser.thread_max (), root["thread"].asInt (), false)
    && match_string (parser.payload (), root["payload"].asString ())
    && match_int (parser.createdDate_min (), root["createdDate"].asInt (), true)
    && match_int (parser.createdDate_max (), root["createdDate"].asInt (), false)
    && match_int (parser.modifiedDate_min (), root["modifiedDate"].asInt (), true)
    && match_int (parser.modifiedDate_max (), root["modifiedDate"].asInt (), false);
}


