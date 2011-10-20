#ifndef MEDIA_PROJECTION_PARSER_H
#define MEDIA_PROJECTION_PARSER_H

#include <string>

#include "StringParser.h"

class MediaProjectionParser : public StringParser
{
public:
  MediaProjectionParser (void);

  void parse (const std::string &params);

  std::string eventId_;
  std::string dataType_;
  std::string data_;
  std::string createdDate_min_;
  std::string createdDate_max_;
  std::string modifiedDate_min_;
  std::string modifiedDate_max_;
};

#endif // MEDIA_PROJECTION_PARSER_H


