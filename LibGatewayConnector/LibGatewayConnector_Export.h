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


// -*- C++ -*-
// $Id$
// Definition for Win32 Export directives.
// This file is generated automatically by generate_export_file.pl LibGatewayConnector
// ------------------------------
#ifndef LIBGATEWAYCONNECTOR_EXPORT_H
#define LIBGATEWAYCONNECTOR_EXPORT_H

#include "ace/config-all.h"

#if defined (ACE_AS_STATIC_LIBS) && !defined (LIBGATEWAYCONNECTOR_HAS_DLL)
#  define LIBGATEWAYCONNECTOR_HAS_DLL 0
#endif /* ACE_AS_STATIC_LIBS && LIBGATEWAYCONNECTOR_HAS_DLL */

#if !defined (LIBGATEWAYCONNECTOR_HAS_DLL)
#  define LIBGATEWAYCONNECTOR_HAS_DLL 1
#endif /* ! LIBGATEWAYCONNECTOR_HAS_DLL */

#if defined (LIBGATEWAYCONNECTOR_HAS_DLL) && (LIBGATEWAYCONNECTOR_HAS_DLL == 1)
#  if defined (LIBGATEWAYCONNECTOR_BUILD_DLL)
#    define LibGatewayConnector_Export ACE_Proper_Export_Flag
#    define LIBGATEWAYCONNECTOR_SINGLETON_DECLARATION(T) ACE_EXPORT_SINGLETON_DECLARATION (T)
#    define LIBGATEWAYCONNECTOR_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_EXPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  else /* LIBGATEWAYCONNECTOR_BUILD_DLL */
#    define LibGatewayConnector_Export ACE_Proper_Import_Flag
#    define LIBGATEWAYCONNECTOR_SINGLETON_DECLARATION(T) ACE_IMPORT_SINGLETON_DECLARATION (T)
#    define LIBGATEWAYCONNECTOR_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_IMPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  endif /* LIBGATEWAYCONNECTOR_BUILD_DLL */
#else /* LIBGATEWAYCONNECTOR_HAS_DLL == 1 */
#  define LibGatewayConnector_Export
#  define LIBGATEWAYCONNECTOR_SINGLETON_DECLARATION(T)
#  define LIBGATEWAYCONNECTOR_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#endif /* LIBGATEWAYCONNECTOR_HAS_DLL == 1 */

// Set LIBGATEWAYCONNECTOR_NTRACE = 0 to turn on library specific tracing even if
// tracing is turned off for ACE.
#if !defined (LIBGATEWAYCONNECTOR_NTRACE)
#  if (ACE_NTRACE == 1)
#    define LIBGATEWAYCONNECTOR_NTRACE 1
#  else /* (ACE_NTRACE == 1) */
#    define LIBGATEWAYCONNECTOR_NTRACE 0
#  endif /* (ACE_NTRACE == 1) */
#endif /* !LIBGATEWAYCONNECTOR_NTRACE */

#if (LIBGATEWAYCONNECTOR_NTRACE == 1)
#  define LIBGATEWAYCONNECTOR_TRACE(X)
#else /* (LIBGATEWAYCONNECTOR_NTRACE == 1) */
#  if !defined (ACE_HAS_TRACE)
#    define ACE_HAS_TRACE
#  endif /* ACE_HAS_TRACE */
#  define LIBGATEWAYCONNECTOR_TRACE(X) ACE_TRACE_IMPL(X)
#  include "ace/Trace.h"
#endif /* (LIBGATEWAYCONNECTOR_NTRACE == 1) */

#endif /* LIBGATEWAYCONNECTOR_EXPORT_H */

// End of auto generated file.
