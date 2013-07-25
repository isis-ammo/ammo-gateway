#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#ifndef WIN32
#include <tr1/memory>
#include <tr1/unordered_map>
#else
#include <memory>
#include <unordered_map>
#endif

#include <stdint.h>

#include <ace/Thread_Mutex.h>
#include <ace/Guard_T.h>

typedef ACE_Guard<ACE_Thread_Mutex> ThreadMutexGuard;

#endif
