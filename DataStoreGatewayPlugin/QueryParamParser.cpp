#include "QueryParamParser.h"

QueryParamParser::QueryParamParser (void)
  : StringParser ()
{
}

void
QueryParamParser::parse (const std::string &params)
{
  tokenize (uri_, params);
  tokenize (user_, params);
  tokenize (time_begin_, params);
  tokenize (time_end_, params);
  tokenize (directed_user_, params);
}

const std::string &
QueryParamParser::uri (void) const
{
  return uri_;
}

const std::string &
QueryParamParser::user (void) const
{
  return user_;
}

const std::string &
QueryParamParser::time_begin (void) const
{
  return time_begin_;
}

const std::string &
QueryParamParser::time_end (void) const
{
  return time_end_;
}

const std::string &
QueryParamParser::directed_user (void) const
{
  return directed_user_;
}

