#ifndef CHAT_MEDIA_PROJECTION_PARSER_H
#define CHAT_MEDIA_PROJECTION_PARSER_H

#include <string>

#include "StringParser.h"

class ChatMediaProjectionParser : public StringParser
{
public:
  ChatMediaProjectionParser (void);

  void parse (const std::string &params);

  std::string uuid_;
  std::string messageId_;
  std::string data_;
  std::string name_;
  std::string createdDate_min_;
  std::string createdDate_max_;
  std::string receipts_min_;
  std::string receipts_max_;
};

#endif // CHAT_MEDIA_PROJECTION_PARSER_H


