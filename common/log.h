#ifndef LOG_H
#define LOG_H

#include <sstream>
#include <ace/Time_Value.h>
#include <ace/OS_NS_time.h>
#include <ace/Log_Msg.h>



//ACE_Time_Value __ammo_current_time = ACE_OS::gettimeofday(); 
//std::cout << __ammo_current_time << msg << std::endl << std::flush; 

#define __PRIVATE_AMMO_LOG(logType, severity, msg) { \
std::ostringstream __ammo_logMessage; \
__ammo_logMessage << msg << std::endl; \
logType ((severity, ACE_TEXT("%D [%M] %s"), __ammo_logMessage.str().c_str())); \
}


#define LOG_TRACE(msg) __PRIVATE_AMMO_LOG(ACE_DEBUG, LM_TRACE, msg)
#define LOG_DEBUG(msg) __PRIVATE_AMMO_LOG(ACE_DEBUG, LM_DEBUG, msg)
#define LOG_INFO(msg)  __PRIVATE_AMMO_LOG(ACE_DEBUG, LM_INFO, msg)
#define LOG_WARN(msg)  __PRIVATE_AMMO_LOG(ACE_ERROR, LM_WARNING, msg)
#define LOG_ERROR(msg) __PRIVATE_AMMO_LOG(ACE_ERROR, LM_ERROR, msg)
#define LOG_FATAL(msg) __PRIVATE_AMMO_LOG(ACE_ERROR, LM_CRITICAL, msg)

#endif
