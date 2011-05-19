#ifndef MEDIA_FILTER_H
#define MEDIA_FILTER_H

#include "ProjectionFilter.h"
#include "MediaProjectionParser.h"

class MediaFilter : public ProjectionFilter
{
public:
  MediaFilter (const Json::Value &root,
               const std::string &projection);
               
  // Checks for match of projection items and parsed data blob.
  bool match (void);
  
private:
  MediaProjectionParser parser_;
};

#endif // MEDIA_FILTER_H
