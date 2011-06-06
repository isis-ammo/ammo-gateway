#ifndef PROJECTION_FILTER_H
#define PROJECTION_FILTER_H

#include <string>

namespace Json
{
  class Value;
}

class StringParser;

class ProjectionFilter
{
public:
  ProjectionFilter (const Json::Value &root);
                                        
protected:
  bool match_string (const std::string &token,
                     const std::string &data);
                     
  bool match_int (const std::string &token,
                  const long data,
                  const bool lower_bound);
                      
  bool match_real (const std::string &token,
                   const double data,
                   const bool lower_bound);
                       
protected:
  const Json::Value &root_;
  std::string digits_;
  std::string real_items_;
};

#endif // PROJECTION_FILTER_H
