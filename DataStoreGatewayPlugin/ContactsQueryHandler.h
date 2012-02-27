#ifndef CONTACTS_QUERY_HANDLER_H
#define CONTACTS_QUERY_HANDLER_H

#include "QueryHandler.h"
#include "ContactsQueryStatementBuilder.h"

struct sqlite3_stmt;

class ContactsQueryHandler : public QueryHandler
{
public:
  ContactsQueryHandler (sqlite3 *db,
                        ammo::gateway::GatewayConnector *sender,
                        ammo::gateway::PullRequest &pr);
                        
  void handleQuery (void);
  
private:
  void encode_row (sqlite3_stmt *stmt,
                   std::string &output);

private:
  ContactsQueryStatementBuilder builder_;
};

#endif /* CONTACTS_QUERY_HANDLER_H */
