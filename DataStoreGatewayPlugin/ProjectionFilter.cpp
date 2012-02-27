#include "log.h"

#include "ProjectionFilter.h"
#include "DataStoreUtils.h"

ProjectionFilter::ProjectionFilter (void)
{
}

bool
ProjectionFilter::match_string (const std::string &token,
                                const std::string &data)
{
  return token.empty () || token == data;
}
                   
bool
ProjectionFilter::match_int (const std::string &token,
                             const long data,
                             const bool lower_bound)
{
  if (token.empty ())
    {
      return true;
    }
    
  long val = 0;
  
  if (DataStoreUtils::safe_atol (token, val))
    {
      return (lower_bound ? val <= data : val >= data);
    }
    
  // Error msg output by safe_atol().
  return false;
}
                    
bool
ProjectionFilter::match_real (const std::string &token,
                              const double data,
                              const bool lower_bound)
{
  if (token.empty ())
    {
      return true;
    }
    
  double val = 0.0;
  
  if (DataStoreUtils::safe_atof (token, val))
    {
      return (lower_bound ? val <= data : val >= data);
    }
    
  // Error msg output by safe_atof().
  return false;
}
                     

