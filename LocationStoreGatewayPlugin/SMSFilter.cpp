#include "log.h"

#include "json/reader.h"
#include "json/value.h"

#include "SMSFilter.h"

SMSFilter::SMSFilter (
      const Json::Value &root,
      const std::string &projection)
  : ProjectionFilter (root)
{
  parser_.parse (projection);
}

bool
SMSFilter::match (void)
{
  return
    match_string (parser_.sms_uri (), root_["sms_uri"].asString ())
    && match_string (parser_.sender (), root_["sender"].asString ())
    && match_string (parser_.recipient (), root_["recipient"].asString ())
    && match_int (parser_.thread_min (), root_["thread"].asInt (), true)
    && match_int (parser_.thread_max (), root_["thread"].asInt (), false)
    && match_string (parser_.payload (), root_["payload"].asString ())
    && match_int (parser_.createdDate_min (), root_["createdDate"].asInt (), true)
    && match_int (parser_.createdDate_max (), root_["createdDate"].asInt (), false)
    && match_int (parser_.modifiedDate_min (), root_["modifiedDate"].asInt (), true)
    && match_int (parser_.modifiedDate_max (), root_["modifiedDate"].asInt (), false);
}


