#ifndef CONTACTS_QUERY_PARAM_PARSER_H
#define CONTACTS_QUERY_PARAM_PARSER_H

#include <string>

#include "StringParser.h"

class ContactsQueryParamParser : public StringParser
{
public:
  ContactsQueryParamParser (void);

  void parse (const std::string &params);

  std::string contact_owner_;
  std::string uid_;
  std::string first_name_;
  std::string middle_initial_;
  std::string last_name_;
  std::string rank_;
  std::string call_sign_;
  std::string branch_;
  std::string unit_;
  std::string email_;
  std::string phone_;
};

#endif // CONTACTS_QUERY_PARAM_PARSER_H
