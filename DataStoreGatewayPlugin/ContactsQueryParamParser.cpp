#include "ContactsQueryParamParser.h"

ContactsQueryParamParser::ContactsQueryParamParser (void)
{
}

void
ContactsQueryParamParser::parse (const std::string &params)
{
  tokenize (uri_, params);
  tokenize (contact_owner_, params);
  tokenize (time_begin_, params);
  tokenize (time_end_, params);
  tokenize (first_name_, params);
  tokenize (middle_initial_, params);
  tokenize (last_name_, params);
  tokenize (rank_, params);
  tokenize (call_sign_, params);
  tokenize (branch_, params);
  tokenize (unit_, params);
  tokenize (email_, params);
  tokenize (phone_, params);
  
  fini_check (params);
}


