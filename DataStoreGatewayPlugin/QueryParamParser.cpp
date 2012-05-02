#include "OriginalQueryParamParser.h"

OriginalQueryParamParser::OriginalQueryParamParser (void)
{
}

void
OriginalQueryParamParser::parse (const std::string &params)
{
  tokenize (uid_, params);
  tokenize (user_, params);
  tokenize (time_begin_, params);
  tokenize (time_end_, params);
  tokenize (directed_user_, params);
}

const std::string &
OriginalQueryParamParser::uid (void) const
{
  return uid_;
}

const std::string &
OriginalQueryParamParser::user (void) const
{
  return user_;
}

const std::string &
OriginalQueryParamParser::time_begin (void) const
{
  return time_begin_;
}

const std::string &
OriginalQueryParamParser::time_end (void) const
{
  return time_end_;
}

const std::string &
OriginalQueryParamParser::directed_user (void) const
{
  return directed_user_;
}

