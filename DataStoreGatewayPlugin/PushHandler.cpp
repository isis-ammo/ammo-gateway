#include <strstream>
#include <sqlite3.h>
#include <openssl/sha.h>

#include <ace/OS_NS_sys_time.h>

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

void
PushHandler::new_checksum (ACE_Time_Value &tv)
{
  std::ostrstream o;
  o << tv.sec () << tv.usec ();
  
  (void) SHA1 (reinterpret_cast<unsigned char *> (o.str ()),
               PushHandler::CS_SIZE,
               checksum_);
}

