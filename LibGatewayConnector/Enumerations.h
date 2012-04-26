#ifndef ENUMERATIONS_H
#define ENUMERATIONS_H

namespace ammo {
  namespace gateway {
    enum MessageScope {
      SCOPE_GLOBAL = 0,
      SCOPE_LOCAL = 1
    };
    
    enum MessagePriority {
      PRIORITY_AUTH = 127,
      PRIORITY_CTRL = 112,
      PRIORITY_FLASH = 96,
      PRIORITY_URGENT = 64,
      PRIORITY_IMPORTANT = 32,
      PRIORITY_NORMAL = 0,
      PRIORITY_BACKGROUND = -32
    };
    
    enum PushStatus {
      PUSH_RECEIVED  = 0,
      PUSH_SUCCESS   = 1,
      PUSH_FAIL      = 2,
      PUSH_REJECTED  = 3
    };
  }
}

#endif //ENUMERATIONS_H
