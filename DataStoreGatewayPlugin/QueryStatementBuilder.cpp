#include <sqlite3.h>

#include "ace/OS_NS_stdlib.h"
#include "ace/OS_NS_sys_time.h"

#include "QueryStatementBuilder.h"
#include "DataStoreConstants.h"
#include "log.h"

QueryStatementBuilder::QueryStatementBuilder (
      const std::string &params,
      sqlite3 *db,
      const char *query_stub)
  : params_ (params),
    db_ (db),
    stmt_ (0),
    has_term_ (false),
    query_str_ (query_stub),
    digits_ ("0123456789-"),
    bind_index_ (1)
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
                                  bool is_int)
{
  if (!token.empty ())
    {
      if (is_int && token.find_first_not_of (digits_) != std::string::npos)
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

      if (!is_int)
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
    }

  return true;
}


