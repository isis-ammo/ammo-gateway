#ifndef MEDIA_PROJECTION_PARSER_H
#define MEDIA_PROJECTION_PARSER_H

#include <string>

#include "StringParser.h"

class MediaProjectionParser : public StringParser
{
public:
  MediaProjectionParser (void);

  void parse (const std::string &params);

  // Member accessors.
  const std::string &eventId (void) const;
  const std::string &dataType (void) const;
  const std::string &data (void) const;
  const std::string &createdDate_min (void) const;
  const std::string &createdDate_max (void) const;
  const std::string &modifiedDate_min (void) const;
  const std::string &modifiedDate_max (void) const;

private:
  std::string eventId_;
  std::string dataType_;
  std::string data_;
  std::string createdDate_min_;
  std::string createdDate_max_;
  std::string modifiedDate_min_;
  std::string modifiedDate_max_;
};

#endif // MEDIA_PROJECTION_PARSER_H


