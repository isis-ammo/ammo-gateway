#include <sqlite3.h>

#include "json/value.h"
#include "json/reader.h"
#include "log.h"

#include "PushHandler.h"

PushHandler::PushHandler (sqlite3 *db,
                          const ammo::gateway::PushData &pd)
  : db_ (db),
    stmt_ (0),
    pd_ (pd)
{
}

PushHandler::~PushHandler (void)
{
  // OK if stmt_ is 0.
  sqlite3_finalize (stmt_);
}

