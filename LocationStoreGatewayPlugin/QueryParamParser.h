#ifndef QUERY_PARAM_PARSER_H
#define QUERY_PARAM_PARSER_H

#include <string>

class QueryParamParser
{
public:
  QueryParamParser (void);

  void parse (const std::string &params);

  // Member accessors.
  const std::string &uri (void) const;
  const std::string &user (void) const;
  const std::string &time_begin (void) const;
  const std::string &time_end (void) const;

private:
  void tokenize (std::string &token,
                 const std::string &token_string);

private:
  std::string uri_;
  std::string user_;
  std::string time_begin_;
  std::string time_end_;
  size_t start_index_;
  size_t end_index_;
};

#endif // QUERY_PARAM_PARSER_H