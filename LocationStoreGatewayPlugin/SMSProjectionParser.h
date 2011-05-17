#ifndef SMS_PROJECTION_PARSER_H
#define SMS_PROJECTION_PARSER_H

#include <string>

#include "StringParser.h"

class SMSProjectionParser : public StringParser
{
public:
  SMSProjectionParser (void);

  void parse (const std::string &params);

  // Member accessors.
  const std::string &sms_uri (void) const;
  const std::string &sender (void) const;
  const std::string &recipient (void) const;
  const std::string &thread_min (void) const;
  const std::string &thread_max (void) const;
  const std::string &payload (void) const;
  const std::string &createdDate_min (void) const;
  const std::string &createdDate_max (void) const;
  const std::string &modifiedDate_min (void) const;
  const std::string &modifiedDate_max (void) const;

private:
  std::string sms_uri_;
  std::string sender_;
  std::string recipient_;
  std::string thread_min_;
  std::string thread_max_;
  std::string payload_;
  std::string createdDate_min_;
  std::string createdDate_max_;
  std::string modifiedDate_min_;
  std::string modifiedDate_max_;
};

#endif // SMS_PROJECTION_PARSER_H


