#include "SMSProjectionParser.h"

SMSProjectionParser::SMSProjectionParser (void)
  : StringParser ()
{
}

void
SMSProjectionParser::parse (const std::string &params)
{
  tokenize (sms_uri_, params);
  tokenize (sender_, params);
  tokenize (recipient_, params);
  tokenize (thread_min_, params);
  tokenize (thread_max_, params);
  tokenize (payload_, params);
  tokenize (createdDate_min_, params);
  tokenize (createdDate_max_, params);
  tokenize (modifiedDate_min_, params);
  tokenize (modifiedDate_max_, params);
  
  fini_check (params);
}

const std::string &
SMSProjectionParser::sms_uri (void) const
{
  return sms_uri_;
}

const std::string &
SMSProjectionParser::sender (void) const
{
  return sender_;
}

const std::string &
SMSProjectionParser::recipient (void) const
{
  return recipient_;
}

const std::string &
SMSProjectionParser::thread_min (void) const
{
  return thread_min_;
}

const std::string &
SMSProjectionParser::thread_max (void) const
{
  return thread_max_;
}

const std::string &
SMSProjectionParser::payload (void) const
{
  return payload_;
}

const std::string &
SMSProjectionParser::createdDate_min (void) const
{
  return createdDate_min_;
}

const std::string &
SMSProjectionParser::createdDate_max (void) const
{
  return createdDate_max_;
}

const std::string &
SMSProjectionParser::modifiedDate_min (void) const
{
  return modifiedDate_min_;
}

const std::string &
SMSProjectionParser::modifiedDate_max (void) const
{
  return modifiedDate_max_;
}


