#ifndef CONTACTS_QUERY_STATEMENT_BUILDER_H
#define CONTACTS_QUERY_STATEMENT_BUILDER_H

#include "QueryStatementBuilder.h"
#include "ContactsQueryParamParser.h"

class ContactsQueryStatementBuilder : public QueryStatementBuilder
{
public:
  ContactsQueryStatementBuilder (const std::string &params,
                                 sqlite3 *db);

  // Prepares the SQL query statement and binds the param values.
  bool build (void);
  
private:
  bool bind (void);

private:
  ContactsQueryParamParser parser_;
  unsigned int bind_index_;
};

#endif // CONTACTS_QUERY_STATEMENT_BUILDER_H
