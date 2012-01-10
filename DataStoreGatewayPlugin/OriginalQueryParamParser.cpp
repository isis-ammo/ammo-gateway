#include "OriginalQueryParamParser.h"

OriginalQueryParamParser::OriginalQueryParamParser (void)
{
}

void
OriginalQueryParamParser::parse (const std::string &params)
{
  tokenize (uri_, params);
  tokenize (user_, params);
  tokenize (time_begin_, params);
  tokenize (time_end_, params);
  tokenize (directed_user_, params);
  
  fini_check (params);
}


