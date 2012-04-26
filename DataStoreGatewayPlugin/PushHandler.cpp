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
    tv_ (ACE_Time_Value::zero)
{
}

PushHandler::PushHandler (sqlite3 *db,
                          const ammo::gateway::PushData &pd,
                          const ACE_Time_Value &tv,
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
PushHandler::create_checksum (void)
{
  // This means a checksum and timestamp were passed to
  // the constructor, copied from the remote
  // datastore object we are getting via gateway sync.
  if (!checksum_.empty ())
    {
      return;
    }
    
  tv_ = ACE_OS::gettimeofday ();
    
  std::ostringstream o (pd_.data.data ());
  o << tv_.sec () << tv_.usec () << std::ends;
  
  unsigned char buf[DataStoreUtils::CS_SIZE];
  
  (void) SHA1 (reinterpret_cast<const unsigned char *> (o.str ().c_str ()),
               o.str ().length (),
               buf);
        
  // 2 characters to represent each byte in hex.         
  char strbuf[DataStoreUtils::CS_SIZE * 2];
  char *bufptr = strbuf;
  
  for (unsigned long i = 0; i < DataStoreUtils::CS_SIZE; ++i, bufptr += 2)
    {
      sprintf (bufptr, "%02x", buf[i]);
    }
               
  checksum_ = strbuf;
}

