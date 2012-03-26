#ifndef PUSH_HANDLER_H
#define PUSH_HANDLER_H

#include <string>

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
               
  ~PushHandler (void);
  
  // Size of sha1 checksum, defined here so it can be
  // changed easily if some other checksum creator is needed.
  static const unsigned long CS_SIZE = 20U;
  
protected:
  void new_checksum (ACE_Time_Value &tv);
    
protected:
  sqlite3 *db_;
  sqlite3_stmt *stmt_;
  const ammo::gateway::PushData &pd_;
  unsigned char checksum_[CS_SIZE];
};

#endif // #ifndef PUSH_HANDLER_H
