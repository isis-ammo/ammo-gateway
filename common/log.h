#ifndef LOG_H
#define LOG_H

#include <sstream>
#include <boost/log/trivial.hpp>



//ACE_Time_Value __ammo_current_time = ACE_OS::gettimeofday(); 
//std::cout << __ammo_current_time << msg << std::endl << std::flush; 

#define __PRIVATE_AMMO_LOG(severity, msg) { \
BOOST_LOG_TRIVIAL(severity) << msg; \
}


#define LOG_TRACE(msg) __PRIVATE_AMMO_LOG(trace, msg)
#define LOG_DEBUG(msg) __PRIVATE_AMMO_LOG(debug, msg)
#define LOG_INFO(msg)  __PRIVATE_AMMO_LOG(info, msg)
#define LOG_WARN(msg)  __PRIVATE_AMMO_LOG(warning, msg)
#define LOG_ERROR(msg) __PRIVATE_AMMO_LOG(error, msg)
#define LOG_FATAL(msg) __PRIVATE_AMMO_LOG(fatal, msg)

#endif
