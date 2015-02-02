/* Copyright (c) 2010-2015 Vanderbilt University
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

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
