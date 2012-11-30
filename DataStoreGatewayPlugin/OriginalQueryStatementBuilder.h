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
