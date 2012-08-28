#ifndef QUERY_STATEMENT_BUILDER_H
#define QUERY_STATEMENT_BUILDER_H

#include <string>

#include "LibGatewayDataStore_Export.h"

struct sqlite3;
struct sqlite3_stmt;

class LibGatewayDataStore_Export QueryStatementBuilder
{
public:
  QueryStatementBuilder (const std::string &params,
                         sqlite3 *db,
                         const char *query_stub);

  ~QueryStatementBuilder (void);

  // Accesses the (finished) query statement.
  sqlite3_stmt *query (void) const;

protected:
  bool addFilter (const std::string &token,
                  const char *stub,
                  bool is_numeric,
                  bool is_lower_bound = true,
                  bool is_blob = false);
                  
  bool bindText (const std::string &token);
  bool bindInteger (const std::string &token);

protected:
  const std::string &params_;
  sqlite3 *db_;
  sqlite3_stmt *stmt_;
  bool has_term_;
  std::string query_str_;
  std::string digits_;
  int bind_index_;
};

#endif // QUERY_STATEMENT_BUILDER_H
