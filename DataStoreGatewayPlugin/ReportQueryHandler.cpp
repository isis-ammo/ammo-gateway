#include "json/value.h"

#include "ReportQueryHandler.h"
#include "ReportProjectionParser.h"

ReportQueryHandler::ReportQueryHandler (
      sqlite3 *db,
      ammo::gateway::GatewayConnector *sender,
      ammo::gateway::PullRequest &pr)
  : OriginalQueryHandler (db, sender, pr)
{
}

bool
ReportQueryHandler::matchedProjection (const Json::Value &root,
                                       const std::string &projection)
{
  ReportProjectionParser parser;
  parser.parse (projection);
  
  return
    match_string (parser.contentGuid_, root["contentGuid"].asString ())
    && match_int (parser.reportTime_min_, root["reportTime"].asInt (), true)
    && match_int (parser.reportTime_max_, root["reportTime"].asInt (),false)
    && match_string (parser.reportingUnit_, root["reportingUnit"].asString ())
    && match_int (parser.size_min_, root["size"].asInt (), true)
    && match_int (parser.size_max_, root["size"].asInt (), false)
    && match_string (parser.activity_, root["activity"].asString ())
    && match_string (parser.locationUtm_, root["locationUtm"].asString ())
    && match_string (parser.enemyUnit_, root["enemyUnit"].asString ())
    && match_int (parser.observationTime_min_, root["observationTime"].asInt (), true)
    && match_int (parser.observationTime_max_, root["observationTime"].asInt (), false)
    && match_string (parser.equipment_, root["equipment"].asString ())
    && match_string (parser.assessment_, root["assessment"].asString ())
    && match_string (parser.narrative_, root["narrative"].asString ())
    && match_string (parser.authentication_, root["authentication"].asString ());
}


