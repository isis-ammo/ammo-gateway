#include "ChatProjectionParser.h"

ChatProjectionParser::ChatProjectionParser (void)
  : StringParser ()
{
}

void
ChatProjectionParser::parse (const std::string &params)
{
  tokenize (originator_, params);
  tokenize (group_id_, params);
  tokenize (text_, params);
  tokenize (latitude_min_, params);
  tokenize (latitude_max_, params);
  tokenize (longitude_min_, params);
  tokenize (longitude_max_, params);
  tokenize (uuid_, params);
  tokenize (pending_receipts_min_, params);
  tokenize (pending_receipts_max_, params);
  tokenize (media_count_min_, params);
  tokenize (media_count_max_, params);
  tokenize (status_min_, params);
  tokenize (status_max_, params);
  tokenize (created_date_min_, params);
  tokenize (created_date_max_, params);
  tokenize (modified_date_min_, params);
  tokenize (modified_date_max_, params);
  
  fini_check (params);
}

