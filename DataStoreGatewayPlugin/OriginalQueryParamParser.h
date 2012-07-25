#ifndef ORIGINAL_QUERY_PARAM_PARSER_H
#define ORIGINAL_QUERY_PARAM_PARSER_H

#include <string>

#include "StringParser.h"

class OriginalQueryParamParser : public StringParser
{
public:
  OriginalQueryParamParser (void);

  void parse (const std::string &params);

  std::string uri_;
  std::string user_;
  std::string time_begin_;
  std::string time_end_;
  std::string recipient_;
};

#endif // ORIGINAL_QUERY_PARAM_PARSER_H
