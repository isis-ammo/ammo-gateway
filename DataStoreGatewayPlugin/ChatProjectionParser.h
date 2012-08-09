#ifndef CHAT_PROJECTION_PARSER_H
#define CHAT_PROJECTION_PARSER_H

#include <string>

#include "StringParser.h"

class ChatProjectionParser : public StringParser
{
public:
  ChatProjectionParser (void);

  void parse (const std::string &params);

  std::string originator_;
  std::string group_id_;
  std::string text_;
  std::string latitude_min_;
  std::string latitude_max_;
  std::string longitude_min_;
  std::string longitude_max_;
  std::string uuid_;
  std::string pending_receipts_min_;
  std::string pending_receipts_max_;
  std::string media_count_min_;
  std::string media_count_max_;
  std::string status_min_;
  std::string status_max_;
  std::string created_date_min_;
  std::string created_date_max_;
  std::string modified_date_min_;
  std::string modified_date_max_;
};

#endif // CHAT_PROJECTION_PARSER_H


