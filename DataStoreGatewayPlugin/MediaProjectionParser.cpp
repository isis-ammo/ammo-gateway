#include "MediaProjectionParser.h"

MediaProjectionParser::MediaProjectionParser (void)
  : StringParser ()
{
}

void
MediaProjectionParser::parse (const std::string &params)
{
  tokenize (eventId_, params);
  tokenize (dataType_, params);
  tokenize (data_, params);
  tokenize (createdDate_min_, params);
  tokenize (createdDate_max_, params);
  tokenize (modifiedDate_min_, params);
  tokenize (modifiedDate_max_, params);
  
  fini_check (params);
}


