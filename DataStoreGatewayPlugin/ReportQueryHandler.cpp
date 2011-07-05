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
    match_string (parser.contentGuid (), root["contentGuid"].asString ())
    && match_int (parser.reportTime_min (), root["reportTime"].asInt (), true)
    && match_int (parser.reportTime_max (), root["reportTime"].asInt (),false)
    && match_string (parser.reportingUnit (), root["reportingUnit"].asString ())
    && match_int (parser.size_min (), root["size"].asInt (), true)
    && match_int (parser.size_max (), root["size"].asInt (), false)
    && match_string (parser.activity (), root["activity"].asString ())
    && match_string (parser.locationUtm (), root["locationUtm"].asString ())
    && match_string (parser.enemyUnit (), root["enemyUnit"].asString ())
    && match_int (parser.observationTime_min (), root["observationTime"].asInt (), true)
    && match_int (parser.observationTime_max (), root["observationTime"].asInt (), false)
    && match_string (parser.equipment (), root["equipment"].asString ())
    && match_string (parser.assessment (), root["assessment"].asString ())
    && match_string (parser.narrative (), root["narrative"].asString ())
    && match_string (parser.authentication (), root["authentication"].asString ());
}


