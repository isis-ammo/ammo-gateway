#ifndef EVENT_PROJECTION_PARSER_H
#define EVENT_PROJECTION_PARSER_H

#include <string>

#include "StringParser.h"

class EventProjectionParser : public StringParser
{
public:
  EventProjectionParser (void);

  void parse (const std::string &params);

  // Member accessors.
  const std::string &uuid (void) const;
  const std::string &mediaCount_min (void) const;
  const std::string &mediaCount_max (void) const;
  const std::string &displayName (void) const;
  const std::string &categoryId (void) const;
  const std::string &title (void) const;
  const std::string &description (void) const;
  const std::string &longitude_min (void) const;
  const std::string &longitude_max (void) const;
  const std::string &latitude_min (void) const;
  const std::string &latitude_max (void) const;
  const std::string &createdDate_min (void) const;
  const std::string &createdDate_max (void) const;
  const std::string &modifiedDate_min (void) const;
  const std::string &modifiedDate_max (void) const;

private:
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
};

#endif // EVENT_PROJECTION_PARSER_H
