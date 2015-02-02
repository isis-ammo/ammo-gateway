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

#ifndef ORIGINAL_QUERY_STATEMENT_BUILDER_H
#define ORIGINAL_QUERY_STATEMENT_BUILDER_H

#include "QueryStatementBuilder.h"
#include "OriginalQueryParamParser.h"

class OriginalQueryStatementBuilder : public QueryStatementBuilder
{
public:
  OriginalQueryStatementBuilder (const std::string &mime_type,
                                 const std::string &params,
                                 sqlite3 *db);

  // Prepares the SQL query statement and binds the param values.
  bool build (void);
  
private:
  bool bind (void);

  // We need an extra step here so that negative time values can
  // be converted to offsets from time of day.
  bool bindInteger (const std::string &token);

private:
  OriginalQueryParamParser parser_;
  std::string mime_type_;
  unsigned int bind_index_;
};

#endif // ORIGINAL_QUERY_STATEMENT_BUILDER_H
