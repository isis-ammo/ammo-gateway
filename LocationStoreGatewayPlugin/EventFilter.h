#ifndef EVENT_FILTER_H
#define EVENT_FILTER_H

#include "ProjectionFilter.h"
#include "EventProjectionParser.h"

class EventFilter : public ProjectionFilter
{
public:
  EventFilter (const Json::Value &root,
               const std::string &projection);
               
  // Checks for match of projection items and parsed data blob.
  bool match (void);
  
private:
  EventProjectionParser parser_;
};

#endif // EVENT_FILTER_H
