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

#include "OriginalQueryParamParser.h"

OriginalQueryParamParser::OriginalQueryParamParser (void)
{
}

void
OriginalQueryParamParser::parse (const std::string &params)
{
  tokenize (uri_, params);
  tokenize (user_, params);
  tokenize (time_begin_, params);
  tokenize (time_end_, params);
  tokenize (directed_user_, params);
}

const std::string &
OriginalQueryParamParser::uri (void) const
{
  return uri_;
}

const std::string &
OriginalQueryParamParser::user (void) const
{
  return user_;
}

const std::string &
OriginalQueryParamParser::time_begin (void) const
{
  return time_begin_;
}

const std::string &
OriginalQueryParamParser::time_end (void) const
{
  return time_end_;
}

const std::string &
OriginalQueryParamParser::directed_user (void) const
{
  return directed_user_;
}

