#include "ace/OS_NS_stdlib.h"

#include "log.h"

#include "ProjectionFilter.h"

ProjectionFilter::ProjectionFilter (
      const Json::Value &root)
  : root_ (root),
    digits_ ("0123456789"),
    real_items_ (digits_ + '.')
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
    
  if (token.find_first_not_of (digits_) != std::string::npos)
    {
      LOG_ERROR ("projection field '"
                 << token.c_str ()
                 << "' is malformed");

      return false;
    }
    
  long val = ACE_OS::atol (token.c_str ());
  return (lower_bound ? val <= data : val >= data);
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
    
  if (token.find_first_not_of (real_items_) != std::string::npos)
    {
      LOG_ERROR ("projection field '"
                 << token.c_str ()
                 << "' is malformed");

      return false;
    }
    
  double val = ACE_OS::atof (token.c_str ());
  return (lower_bound ? val <= data : val >= data);
}
                     

