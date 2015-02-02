/* Copyright (c) 2010-2015 Vanderbilt University
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

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
                     

