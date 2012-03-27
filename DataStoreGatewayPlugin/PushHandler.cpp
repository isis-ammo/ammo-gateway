#include <sstream>
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
  std::basic_ostringstream<char> o;
  o << tv.sec () << tv.usec () << std::ends;
  
  (void) SHA1 (reinterpret_cast<const unsigned char *> (o.str ().c_str ()),
               PushHandler::CS_SIZE,
               checksum_);
}

