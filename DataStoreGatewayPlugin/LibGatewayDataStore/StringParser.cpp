/* Copyright (C) 2010-2013 Vanderbilt University
 *  
 * This software was developed by the Institute for Software Integrated
 * Systems (ISIS) at Vanderbilt University, Tennessee, USA for the
 * Transformative Apps program under DARPA, Contract # HR011-10-C-0175 and
 * Contract # HR0011-12-C-0109.
 *  
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this data, including any software or models in source or binary
 * form, as well as any drawings, specifications, and documentation
 * (collectively "the Data"), to deal in the Data without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Data, and to
 * permit persons to whom the Data is furnished to do so, subject to the
 * following conditions:
 *  
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Data.
 *  
 * THE DATA IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS, SPONSORS, DEVELOPERS, CONTRIBUTORS, OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE DATA OR THE USE OR OTHER DEALINGS IN THE DATA.
 */

#include <ace/OS_NS_sys_time.h>

#include "log.h"

#include "ace/Connector.h"
#include "ace/SOCK_Connector.h"

#include "StringParser.h"

StringParser::StringParser (void)
  : start_index_ (0),
    end_index_ (0),
    TOKEN_SEPARATOR_ (',')
{
}

void
StringParser::tokenize (std::string &token,
                        const std::string &token_string)
{
  if (end_index_ == std::string::npos)
    {
      LOG_ERROR ("projection string \""
                 << token_string.c_str ()
                 << "\" has too few fields");
      return;
    }
    
  end_index_ = token_string.find (TOKEN_SEPARATOR_, start_index_);
  token = token_string.substr (start_index_,
                               end_index_ - start_index_);
  start_index_ = end_index_ + 1;
}

void
StringParser::fini_check (const std::string &token_string) const
{
  if (end_index_ != std::string::npos)
    {
      LOG_ERROR ("projection string \""
                 << token_string.c_str ()
                 << "\" has too many fields");
    }
}

