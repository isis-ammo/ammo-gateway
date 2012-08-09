#include "ChatMediaProjectionParser.h"

ChatMediaProjectionParser::ChatMediaProjectionParser (void)
  : StringParser ()
{
}

void
ChatMediaProjectionParser::parse (const std::string &params)
{
  tokenize (uuid_, params);
  tokenize (messageId_, params);
  tokenize (name_, params);
  tokenize (data_, params);
  tokenize (createdDate_min_, params);
  tokenize (createdDate_max_, params);
  tokenize (receipts_min_, params);
  tokenize (receipts_max_, params);
  
  fini_check (params);
}


