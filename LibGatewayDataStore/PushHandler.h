#ifndef PUSH_HANDLER_H
#define PUSH_HANDLER_H

#include <string>

#include "LibGatewayDataStore_Export.h"

class sqlite3;
class sqlite3_stmt;

namespace ammo
{
  namespace gateway
  {
    class PushData;
  }
}

namespace Json
{
  class Value;
}

class LibGatewayDataStore_Export PushHandler
{
public:
  PushHandler (sqlite3 *db,
               const ammo::gateway::PushData &pd);
               
  ~PushHandler (void);
    
protected:
  sqlite3 *db_;
  sqlite3_stmt *stmt_;
  const ammo::gateway::PushData &pd_;
};

#endif // #ifndef PUSH_HANDLER_H
