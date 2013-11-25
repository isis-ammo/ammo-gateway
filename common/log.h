/* Copyright (C) 2010-2013 Vanderbilt University
 *  
 * This software was developed by the Institute for Software Integrated
 * Systems (ISIS) at Vanderbilt University, Tennessee, USA for the
 * Transformative Apps program under DARPA, Contract # HR011-10-C-0175 and
 * Contract # HR0011-12-C-0109.
 *  
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this data, including any software or models in source or binary
 * form, as well as any drawings, specifications, and documentation
 * (collectively "the Data"), to deal in the Data without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Data, and to
 * permit persons to whom the Data is furnished to do so, subject to the
 * following conditions:
 *  
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Data.
 *  
 * THE DATA IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS, SPONSORS, DEVELOPERS, CONTRIBUTORS, OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE DATA OR THE USE OR OTHER DEALINGS IN THE DATA.
 */

#ifndef LOG_H
#define LOG_H

#include <sstream>
#include <boost/log/trivial.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/severity_logger.hpp>

BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(ammoInternalLogger, boost::log::sources::severity_logger_mt<boost::log::trivial::severity_level>);

//ACE_Time_Value __ammo_current_time = ACE_OS::gettimeofday(); 
//std::cout << __ammo_current_time << msg << std::endl << std::flush; 

#define __PRIVATE_AMMO_LOG(severity, msg) { \
BOOST_LOG_SEV(ammoInternalLogger::get(), severity) << msg; \
}


#define LOG_TRACE(msg) __PRIVATE_AMMO_LOG(boost::log::trivial::trace, msg)
#define LOG_DEBUG(msg) __PRIVATE_AMMO_LOG(boost::log::trivial::debug, msg)
#define LOG_INFO(msg)  __PRIVATE_AMMO_LOG(boost::log::trivial::info, msg)
#define LOG_WARN(msg)  __PRIVATE_AMMO_LOG(boost::log::trivial::warning, msg)
#define LOG_ERROR(msg) __PRIVATE_AMMO_LOG(boost::log::trivial::error, msg)
#define LOG_FATAL(msg) __PRIVATE_AMMO_LOG(boost::log::trivial::fatal, msg)

#endif
