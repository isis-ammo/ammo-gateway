#ifndef ORIGINAL_PUSH_HANDLER_H
#define ORIGINAL_PUSH_HANDLER_H

#include "PushHandler.h"

class OriginalPushHandler : public PushHandler
{
public:
  OriginalPushHandler (sqlite3 *db,
                       const ammo::gateway::PushData &pd);
  
  bool handlePush (void);
};

#endif // #ifndef ORIGINAL_PUSH_HANDLER_H


