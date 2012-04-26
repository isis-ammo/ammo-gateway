#ifndef PUSH_HANDLER_H
#define PUSH_HANDLER_H

#include <string>

#include <ace/Time_Value.h>

#include "DataStoreUtils.h"

struct sqlite3;
struct sqlite3_stmt;

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

class PushHandler
{
public:
  PushHandler (sqlite3 *db,
               const ammo::gateway::PushData &pd);
               
  PushHandler (sqlite3 *db,
               const ammo::gateway::PushData &pd,
               const ACE_Time_Value &tv,
               const std::string &checksum);
               
  ~PushHandler (void);
  
protected:
  void create_checksum (void);
    
protected:
  sqlite3 *db_;
  sqlite3_stmt *stmt_;
  const ammo::gateway::PushData &pd_;
  std::string checksum_;
  ACE_Time_Value tv_;
};

#endif // #ifndef PUSH_HANDLER_H
