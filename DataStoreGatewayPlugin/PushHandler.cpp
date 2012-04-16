#include <sstream>
#include <sqlite3.h>
#include <openssl/sha.h>

#include <ace/OS_NS_sys_time.h>

#include "json/value.h"
#include "json/reader.h"
#include "GatewayConnector.h"
#include "log.h"

#include "PushHandler.h"

PushHandler::PushHandler (sqlite3 *db,
                          const ammo::gateway::PushData &pd)
  : db_ (db),
    stmt_ (0),
    pd_ (pd),
    tv_ (0)
{
}

PushHandler::PushHandler (sqlite3 *db,
                          const ammo::gateway::PushData &pd,
                          const ACE_Time_Value *tv,
                          const std::string &checksum)
  : db_ (db),
    stmt_ (0),
    pd_ (pd),
    checksum_ (checksum),
    tv_ (tv)
{
}

PushHandler::~PushHandler (void)
{
  // OK if stmt_ is 0.
  sqlite3_finalize (stmt_);
}

void
PushHandler::new_checksum (void)
{
  std::string ibuf (pd_.data.data ());
  ibuf.append (pd_.originUsername);
  this->create_checksum (ibuf);
}

void
PushHandler::new_checksum (ACE_Time_Value &tv)
{
  std::ostringstream o (pd_.data.data ());
  o << tv.sec () << tv.usec () << std::ends;
  this->create_checksum (o.str ());
}

void
PushHandler::create_checksum (const std::string &ibuf)
{
  unsigned char buf[DataStoreUtils::CS_SIZE + 1];
  
  (void) SHA1 (reinterpret_cast<const unsigned char *> (ibuf.c_str ()),
               ibuf.length (),
               buf);
               
  buf[DataStoreUtils::CS_SIZE] = '\0';
  checksum_ = reinterpret_cast<char *> (buf);
}

