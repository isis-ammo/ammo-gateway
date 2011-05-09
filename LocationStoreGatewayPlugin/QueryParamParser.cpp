#include "QueryParamParser.h"

QueryParamParser::QueryParamParser (void)
  : start_index_ (0),
    end_index_ (0)
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

void
QueryParamParser::tokenize (std::string &token,
                            const std::string &token_string)
{
  end_index_ = token_string.find (',', start_index_);
  token = token_string.substr (start_index_,
                               end_index_ - start_index_);
  start_index_ = end_index_ + 1;
}

