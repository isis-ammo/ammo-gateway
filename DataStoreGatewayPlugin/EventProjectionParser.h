#ifndef EVENT_PROJECTION_PARSER_H
#define EVENT_PROJECTION_PARSER_H

#include <string>

#include "StringParser.h"

class EventProjectionParser : public StringParser
{
public:
  EventProjectionParser (void);

  void parse (const std::string &params);

  std::string uuid_;
  std::string mediaCount_min_;
  std::string mediaCount_max_;
  std::string displayName_;
  std::string categoryId_;
  std::string title_;
  std::string description_;
  std::string longitude_min_;
  std::string longitude_max_;
  std::string latitude_min_;
  std::string latitude_max_;
  std::string createdDate_min_;
  std::string createdDate_max_;
  std::string modifiedDate_min_;
  std::string modifiedDate_max_;
  std::string cid_;
  std::string category_;
  std::string unit_;
  std::string size_min_;
  std::string size_max_;
  std::string dest_group_type_;
  std::string dest_group_name_;
  std::string status_min_;
  std::string status_max_;
};

#endif // EVENT_PROJECTION_PARSER_H
