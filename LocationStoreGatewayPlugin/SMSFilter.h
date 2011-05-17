#ifndef SMS_FILTER_H
#define SMS_FILTER_H

#include "ProjectionFilter.h"

class SMSFilter : public ProjectionFilter
{
public:
  SMSFilter (const Json::Value &root,
             const std::string &projection);
               
  // Checks for match of projection items and parsed data blob.
  bool match (void);
  
private:
  SMSProjectionParser parser_;
};

#endif // SMS_FILTER_H
