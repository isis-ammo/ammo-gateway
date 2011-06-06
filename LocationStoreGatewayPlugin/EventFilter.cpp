#include "log.h"

#include "json/reader.h"
#include "json/value.h"

#include "EventProjectionParser.h"

#include "EventFilter.h"

EventFilter::EventFilter (
      const Json::Value &root,
      const std::string &projection)
  : ProjectionFilter (root)
{
  parser_.parse (projection);
}

bool
EventFilter::match (void)
{
  return
    match_string (parser_.uuid (), root_["uuid"].asString ())
    && match_int (parser_.mediaCount_min (), root_["mediaCount"].asInt (), true)
    && match_int (parser_.mediaCount_max (), root_["mediaCount"].asInt (), false)
    && match_string (parser_.displayName (), root_["displayName"].asString ())
    && match_string (parser_.categoryId (), root_["categoryId"].asString ())
    && match_string (parser_.title (), root_["title"].asString ())
    && match_string (parser_.description (), root_["description"].asString ())
    && match_real (parser_.longitude_min (), root_["longitude"].asDouble (), true)
    && match_real (parser_.longitude_max (), root_["longitude"].asDouble (), false)
    && match_real (parser_.latitude_min (), root_["latitude"].asDouble (), true)
    && match_real (parser_.latitude_max (), root_["latitude"].asDouble (), false)
    && match_int (parser_.createdDate_min (), root_["createdDate"].asInt (), true)
    && match_int (parser_.createdDate_max (), root_["createdDate"].asInt (), false)
    && match_int (parser_.modifiedDate_min (), root_["modifiedDate"].asInt (), true)
    && match_string (parser_.cid (), root_["cid"].asString ())
    && match_string (parser_.category (), root_["category"].asString ())
    && match_string (parser_.unit (), root_["unit"].asString ())
    && match_int (parser_.size_min (), root_["size"].asInt (), true)
    && match_int (parser_.size_max (), root_["size"].asInt (), false)
    && match_string (parser_.dest_group_name (), root_["dest_group_name"].asString ())
    && match_string (parser_.dest_group_type (), root_["dest_group_type"].asString ())
    && match_int (parser_.status_min (), root_["status"].asInt (), true)
    && match_int (parser_.status_max (), root_["statuc"].asInt (), false);
}


