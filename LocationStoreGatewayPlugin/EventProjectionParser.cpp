#include "EventProjectionParser.h"

EventProjectionParser::EventProjectionParser (void)
  : StringParser ()
{
}

void
EventProjectionParser::parse (const std::string &params)
{
  tokenize (uuid_, params);
  tokenize (mediaCount_min_, params);
  tokenize (mediaCount_max_, params);
  tokenize (displayName_, params);
  tokenize (categoryId_, params);
  tokenize (title_, params);
  tokenize (description_, params);
  tokenize (longitude_min_, params);
  tokenize (longitude_max_, params);
  tokenize (latitude_min_, params);
  tokenize (latitude_max_, params);
  tokenize (createdDate_min_, params);
  tokenize (createdDate_max_, params);
  tokenize (modifiedDate_min_, params);
  tokenize (modifiedDate_max_, params);
  
  fini_check (params);
}

const std::string &
EventProjectionParser::uuid (void) const
{
  return uuid_;
}

const std::string &
EventProjectionParser::mediaCount_min (void) const
{
  return mediaCount_min_;
}

const std::string &
EventProjectionParser::mediaCount_max (void) const
{
  return mediaCount_max_;
}

const std::string &
EventProjectionParser::displayName (void) const
{
  return displayName_;
}

const std::string &
EventProjectionParser::categoryId (void) const
{
  return categoryId_;
}

const std::string &
EventProjectionParser::title (void) const
{
  return title_;
}

const std::string &
EventProjectionParser::description (void) const
{
  return description_;
}

const std::string &
EventProjectionParser::longitude_min (void) const
{
  return longitude_min_;
}

const std::string &
EventProjectionParser::longitude_max (void) const
{
  return longitude_max_;
}

const std::string &
EventProjectionParser::latitude_min (void) const
{
  return latitude_min_;
}

const std::string &
EventProjectionParser::latitude_max (void) const
{
  return latitude_max_;
}

const std::string &
EventProjectionParser::createdDate_min (void) const
{
  return createdDate_min_;
}

const std::string &
EventProjectionParser::createdDate_max (void) const
{
  return createdDate_max_;
}

const std::string &
EventProjectionParser::modifiedDate_min (void) const
{
  return modifiedDate_min_;
}

const std::string &
EventProjectionParser::modifiedDate_max (void) const
{
  return modifiedDate_max_;
}


