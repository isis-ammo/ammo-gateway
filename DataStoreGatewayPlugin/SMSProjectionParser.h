#ifndef SMS_PROJECTION_PARSER_H
#define SMS_PROJECTION_PARSER_H

#include <string>

#include "StringParser.h"

class SMSProjectionParser : public StringParser
{
public:
  SMSProjectionParser (void);

  void parse (const std::string &params);

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


