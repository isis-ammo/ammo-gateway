#include "log.h"

#include "json/reader.h"
#include "json/value.h"

#include "ReportFilter.h"

ReportFilter::ReportFilter (
      const Json::Value &root,
      const std::string &projection)
  : ProjectionFilter (root)
{
  parser_.parse (projection);
}

bool
ReportFilter::match (void)
{
  return
    match_string (parser_.contentGuid (), root_["contentGuid"].asString ())
    && match_int (parser_.reportTime_min (), root_["reportTime"].asInt (), true)
    && match_int (parser_.reportTime_max (), root_["reportTime"].asInt (),false)
    && match_string (parser_.reportingUnit (), root_["reportingUnit"].asString ())
    && match_int (parser_.size_min (), root_["size"].asInt (), true)
    && match_int (parser_.size_max (), root_["size"].asInt (), false)
    && match_string (parser_.activity (), root_["activity"].asString ())
    && match_string (parser_.locationUtm (), root_["locationUtm"].asString ())
    && match_string (parser_.enemyUnit (), root_["enemyUnit"].asString ())
    && match_int (parser_.observationTime_min (), root_["observationTime"].asInt (), true)
    && match_int (parser_.observationTime_max (), root_["observationTime"].asInt (), false)
    && match_string (parser_.equipment (), root_["equipment"].asString ())
    && match_string (parser_.assessment (), root_["assessment"].asString ())
    && match_string (parser_.narrative (), root_["narrative"].asString ())
    && match_string (parser_.authentication (), root_["authentication"].asString ());
}


