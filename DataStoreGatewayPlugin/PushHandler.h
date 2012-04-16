#ifndef PUSH_HANDLER_H
#define PUSH_HANDLER_H

#include <string>

#include "DataStoreUtils.h"

struct sqlite3;
struct sqlite3_stmt;

class ACE_Time_Value;

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
               const ACE_Time_Value *tv,
               const std::string &checksum);
               
  ~PushHandler (void);
  
protected:
  // This overload is used by ContactsPushHandler, and includes
  // the originUsername and the data blob in the checksum.
  void new_checksum (void);
  
  // This overload is used by OriginalPushHandler, and includes
  // the data blob and the generated timestamp in the checksum.
  void new_checksum (ACE_Time_Value &tv);
    
protected:
  sqlite3 *db_;
  sqlite3_stmt *stmt_;
  const ammo::gateway::PushData &pd_;
  std::string checksum_;
  const ACE_Time_Value *tv_;
  
private:
  void create_checksum (const std::string &ibuf);
};

#endif // #ifndef PUSH_HANDLER_H
