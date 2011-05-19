#ifndef REPORT_FILTER_H
#define REPORT_FILTER_H

#include "ProjectionFilter.h"
#include "ReportProjectionParser.h"

class ReportFilter : public ProjectionFilter
{
public:
  ReportFilter (const Json::Value &root,
                const std::string &projection);
               
  // Checks for match of projection items and parsed data blob.
  bool match (void);
  
private:
  ReportProjectionParser parser_;
};

#endif // REPORT_FILTER_H
