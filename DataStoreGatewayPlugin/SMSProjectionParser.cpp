#include "SMSProjectionParser.h"

SMSProjectionParser::SMSProjectionParser (void)
  : StringParser ()
{
}

void
SMSProjectionParser::parse (const std::string &params)
{
  tokenize (sms_uri_, params);
  tokenize (sender_, params);
  tokenize (recipient_, params);
  tokenize (thread_min_, params);
  tokenize (thread_max_, params);
  tokenize (payload_, params);
  tokenize (createdDate_min_, params);
  tokenize (createdDate_max_, params);
  tokenize (modifiedDate_min_, params);
  tokenize (modifiedDate_max_, params);
  
  fini_check (params);
}

