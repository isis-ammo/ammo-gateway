#ifndef ENUMERATIONS_H
#define ENUMERATIONS_H

enum MessageScope {
  SCOPE_GLOBAL = 0,
  SCOPE_LOCAL = 1,
  SCOPE_ALL = 2 //used by unregisterPullInterest, to remove a subscription regardless of scope
};

#endif //ENUMERATIONS_H
