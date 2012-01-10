#include "json/reader.h"
#include "json/value.h"

#include "log.h"

#include "QueryHandler.h"

QueryHandler::QueryHandler (sqlite3 *db,
                            ammo::gateway::GatewayConnector *sender,
                            ammo::gateway::PullRequest &pr)
  : db_ (db),
    sender_ (sender),
    pr_ (pr)
{
}

QueryHandler::~QueryHandler (void)
{
}


