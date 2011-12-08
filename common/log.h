#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <ace/Time_Value.h>
#include <ace/OS_NS_time.h>

#define __PRIVATE_AMMO_LOG(msg) { \
ACE_Time_Value __ammo_current_time = ACE_OS::gettimeofday(); \
std::cout << __ammo_current_time << msg << std::endl << std::flush; \
}


#define LOG_TRACE(msg) __PRIVATE_AMMO_LOG(" TRACE " << msg)
#define LOG_DEBUG(msg) __PRIVATE_AMMO_LOG(" DEBUG " << msg)
#define LOG_INFO(msg)  __PRIVATE_AMMO_LOG(" INFO  " << msg)
#define LOG_WARN(msg)  __PRIVATE_AMMO_LOG(" WARN  " << msg)
#define LOG_ERROR(msg) __PRIVATE_AMMO_LOG(" ERROR " << msg)
#define LOG_FATAL(msg) __PRIVATE_AMMO_LOG(" FATAL " << msg)

#endif
