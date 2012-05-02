#include "EventProjectionParser.h"

EventProjectionParser::EventProjectionParser (void)
  : StringParser ()
{
}

void
EventProjectionParser::parse (const std::string &params)
{
  tokenize (uuri_, params);
  tokenize (mediaCount_min_, params);
  tokenize (mediaCount_max_, params);
  tokenize (displayName_, params);
  tokenize (categoryId_, params);
  tokenize (title_, params);
  tokenize (description_, params);
  tokenize (longitude_min_, params);
  tokenize (longitude_max_, params);
  tokenize (latitude_min_, params);
  tokenize (latitude_max_, params);
  tokenize (createdDate_min_, params);
  tokenize (createdDate_max_, params);
  tokenize (modifiedDate_min_, params);
  tokenize (modifiedDate_max_, params);
  tokenize (cid_, params);
  tokenize (category_, params);
  tokenize (unit_, params);
  tokenize (size_min_, params);
  tokenize (size_max_, params);
  tokenize (dest_group_name_, params);
  tokenize (dest_group_type_, params);
  tokenize (status_min_, params);
  tokenize (status_max_, params);
  
  fini_check (params);
}


