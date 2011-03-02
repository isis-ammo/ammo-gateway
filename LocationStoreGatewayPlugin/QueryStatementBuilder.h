#ifndef QUERY_STATEMENT_BUILDER_H
#define QUERY_STATEMENT_BUILDER_H

#include "QueryParamParser.h"

struct sqlite3;
struct sqlite3_stmt;

class QueryStatementBuilder
{
public:
  QueryStatementBuilder (const std::string &mime_type,
                         const std::string &params,
                         sqlite3 *db);

  ~QueryStatementBuilder (void);

  // Prepares the SQL query statement and binds the param values.
  bool build (void);

  // Accesses the (finished) query statement.
  sqlite3_stmt *query (void) const;

private:
  bool bind (void);

  bool addFilter (const std::string &token,
                  const char *stub,
                  bool is_int);

  bool bindText (const std::string &token);
  bool bindInteger (const std::string &token);

private:
  QueryParamParser parser_;
  const std::string &mime_type_;
  const std::string &params_;
  sqlite3 *db_;
  sqlite3_stmt *stmt_;
  bool has_term_;
  std::string query_str_;
  std::string digits_;
  int bind_index_;
};

#endif // QUERY_STATEMENT_BUILDER_H