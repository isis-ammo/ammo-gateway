#ifndef CONTACTS_PUSH_HANDLER_H
#define CONTACTS_PUSH_HANDLER_H

#include "PushHandler.h"

class ContactsPushHandler : public PushHandler
{
public:
  ContactsPushHandler (sqlite3 *db,
                       const ammo::gateway::PushData &pd);
  
  ContactsPushHandler (sqlite3 *db,
                       const ammo::gateway::PushData &pd,
                       const ACE_Time_Value &tv,
                       const std::string &checksum);
  
  bool handlePush (void);
};

#endif // #ifndef CONTACTS_PUSH_HANDLER_H


