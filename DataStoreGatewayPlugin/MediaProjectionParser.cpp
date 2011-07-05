#include "MediaProjectionParser.h"

MediaProjectionParser::MediaProjectionParser (void)
  : StringParser ()
{
}

void
MediaProjectionParser::parse (const std::string &params)
{
  tokenize (eventId_, params);
  tokenize (dataType_, params);
  tokenize (data_, params);
  tokenize (createdDate_min_, params);
  tokenize (createdDate_max_, params);
  tokenize (modifiedDate_min_, params);
  tokenize (modifiedDate_max_, params);
  
  fini_check (params);
}

const std::string &
MediaProjectionParser::eventId (void) const
{
  return eventId_;
}

const std::string &
MediaProjectionParser::dataType (void) const
{
  return dataType_;
}

const std::string &
MediaProjectionParser::data (void) const
{
  return data_;
}

const std::string &
MediaProjectionParser::createdDate_min (void) const
{
  return createdDate_min_;
}

const std::string &
MediaProjectionParser::createdDate_max (void) const
{
  return createdDate_max_;
}

const std::string &
MediaProjectionParser::modifiedDate_min (void) const
{
  return modifiedDate_min_;
}

const std::string &
MediaProjectionParser::modifiedDate_max (void) const
{
  return modifiedDate_max_;
}


