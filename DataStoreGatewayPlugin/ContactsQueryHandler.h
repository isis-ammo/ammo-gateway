#ifndef CONTACTS_QUERY_HANDLER_H
#define CONTACTS_QUERY_HANDLER_H

#include "QueryHandler.h"
#include "ContactsQueryStatementBuilder.h"

class ContactsQueryHandler : public QueryHandler
{
public:
  ContactsQueryHandler (sqlite3 *db,
                        ammo::gateway::GatewayConnector *sender,
                        ammo::gateway::PullRequest &pr);
                        
  virtual void handleQuery (void);

protected:
  ContactsQueryStatementBuilder builder_;
};

#endif /* CONTACTS_QUERY_HANDLER_H */
