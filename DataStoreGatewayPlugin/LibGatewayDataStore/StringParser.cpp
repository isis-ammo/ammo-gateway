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

