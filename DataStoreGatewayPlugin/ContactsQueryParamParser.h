#ifndef CONTACTS_QUERY_PARAM_PARSER_H
#define CONTACTS_QUERY_PARAM_PARSER_H

#include <string>

#include "StringParser.h"

class ContactsQueryParamParser : public StringParser
{
public:
  ContactsQueryParamParser (void);

  void parse (const std::string &params);

  // Member accessors.
  const std::string &contact_owner (void) const;
  const std::string &first_name (void) const;
  const std::string &middle_initial (void) const;
  const std::string &last_name (void) const;
  const std::string &rank (void) const;
  const std::string &call_sign (void) const;
  const std::string &branch (void) const;
  const std::string &unit (void) const;
  const std::string &email (void) const;
  const std::string &phone (void) const;

private:
  std::string contact_owner_;
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
