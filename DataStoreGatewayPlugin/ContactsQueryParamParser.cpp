#include "ContactsQueryParamParser.h"

ContactsQueryParamParser::ContactsQueryParamParser (void)
{
}

void
ContactsQueryParamParser::parse (const std::string &params)
{
  tokenize (contact_owner_, params);
  tokenize (first_name_, params);
  tokenize (middle_initial_, params);
  tokenize (last_name_, params);
  tokenize (rank_, params);
  tokenize (call_sign_, params);
  tokenize (branch_, params);
  tokenize (unit_, params);
  tokenize (email_, params);
  tokenize (phone_, params);
}

const std::string &
ContactsQueryParamParser::contact_owner (void) const
{
  return contact_owner_;
}

const std::string &
ContactsQueryParamParser::first_name (void) const
{
  return first_name_;
}

const std::string &
ContactsQueryParamParser::middle_initial (void) const
{
  return middle_initial_;
}

const std::string &
ContactsQueryParamParser::last_name (void) const
{
  return last_name_;
}

const std::string &
ContactsQueryParamParser::rank (void) const
{
  return rank_;
}

const std::string &
ContactsQueryParamParser::call_sign (void) const
{
  return call_sign_;
}

const std::string &
ContactsQueryParamParser::branch (void) const
{
  return branch_;
}

const std::string &
ContactsQueryParamParser::unit (void) const
{
  return unit_;
}

const std::string &
ContactsQueryParamParser::email (void) const
{
  return email_;
}

const std::string &
ContactsQueryParamParser::phone (void) const
{
  return phone_;
}


