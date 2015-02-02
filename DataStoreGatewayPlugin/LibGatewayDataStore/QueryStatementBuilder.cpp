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

#include <sqlite3.h>

#include "ace/OS_NS_stdlib.h"
#include "ace/OS_NS_sys_time.h"

#include "log.h"

#include "QueryStatementBuilder.h"

QueryStatementBuilder::QueryStatementBuilder (
      const std::string &params,
      sqlite3 *db,
      const char *query_stub)
  : params_ (params),
    db_ (db),
    stmt_ (0),
    has_term_ (false),
    query_str_ (query_stub),
    digits_ ("0123456789-.")
{
}

QueryStatementBuilder::~QueryStatementBuilder (void)
{
  // OK if stmt_ is 0.
  sqlite3_finalize (stmt_);
}

sqlite3_stmt *
QueryStatementBuilder::query (void) const
{
  return this->stmt_;
}

bool
QueryStatementBuilder::addFilter (const std::string &token,
                                  const char *stub,
                                  bool is_numeric,
                                  bool is_lower_bound,
                                  bool is_blob)
{
  // Not handling binary BLOBs at the moment, except to store them in the
  // database as an empty placeholder.
  if (!token.empty () && !is_blob)
    {
      if (is_numeric && token.find_first_not_of (digits_) != std::string::npos)
        {
          LOG_ERROR ("token " << token.c_str () << " is malformed");

          return false;
        }

      if (has_term_)
        {
          query_str_ += " AND ";
        }
      else
        {
          has_term_ = true;
        }

      query_str_ += stub;

      if (!is_numeric)
        {
          if (token.find ('%') == std::string::npos)
            {
              query_str_ += "=?";
            }
          else
            {
              query_str_ += " LIKE ?";
            }
        }
      else
        {
          if (is_lower_bound)
            {
              query_str_ += ">=?";
            }
          else
            {
              query_str_ += "<=?";
            }
        }
    }

  return true;
}


