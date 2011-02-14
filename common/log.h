#ifndef LOG_H
#define LOG_H

#include <iostream>

#define LOG_TRACE(msg) std::cout << "TRACE " << msg << std::endl << std::flush
#define LOG_DEBUG(msg) std::cout << "DEBUG " << msg << std::endl << std::flush
#define LOG_INFO(msg) std::cout << "INFO  " << msg << std::endl << std::flush
#define LOG_WARN(msg) std::cout << "WARN  " << msg << std::endl << std::flush
#define LOG_ERROR(msg) std::cout << "ERROR " << msg << std::endl << std::flush
#define LOG_FATAL(msg) std::cout << "FATAL " << msg << std::endl << std::flush

#endif
