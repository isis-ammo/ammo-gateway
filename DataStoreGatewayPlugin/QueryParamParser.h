#ifndef ORIGINAL_QUERY_PARAM_PARSER_H
#define ORIGINAL_QUERY_PARAM_PARSER_H

#include <string>

#include "StringParser.h"

class OriginalQueryParamParser : public StringParser
{
public:
  OriginalQueryParamParser (void);

  void parse (const std::string &params);

  // Member accessors.
  const std::string &uri (void) const;
  const std::string &user (void) const;
  const std::string &time_begin (void) const;
  const std::string &time_end (void) const;
  const std::string &directed_user (void) const;

private:
  std::string uri_;
  std::string user_;
  std::string time_begin_;
  std::string time_end_;
  std::string directed_user_;
};

#endif // ORIGINAL_QUERY_PARAM_PARSER_H
