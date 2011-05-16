#include "log.h"

#include "json/reader.h"
#include "json/value.h"

#include "MediaProjectionParser.h"

#include "MediaFilter.h"

MediaFilter::MediaFilter (
      const Json::Value &root,
      const std::string &projection)
  : ProjectionFilter (root)
{
  parser_.parse (projection);
}

bool
MediaFilter::match (void)
{
  return
    match_string (parser_.eventId (), root_["eventId"].asString ())
    && match_string (parser_.dataType (), root_["dataType"].asString ())
    && match_string (parser_.data (), root_["data"].asString ())
    && match_int (parser_.createdDate_min (), root_["createdDate"].asInt (), true)
    && match_int (parser_.createdDate_max (), root_["createdDate"].asInt (), false)
    && match_int (parser_.modifiedDate_min (), root_["modifiedDate"].asInt (), true)
    && match_int (parser_.modifiedDate_max (), root_["modifiedDate"].asInt (), false);
}


