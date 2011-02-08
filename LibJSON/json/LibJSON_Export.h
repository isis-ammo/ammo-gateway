
// -*- C++ -*-
// $Id$
// Definition for Win32 Export directives.
// This file is generated automatically by generate_export_file.pl LibJSON
// ------------------------------
#ifndef LIBJSON_EXPORT_H
#define LIBJSON_EXPORT_H

#include "ace/config-all.h"

#if defined (ACE_AS_STATIC_LIBS) && !defined (LIBJSON_HAS_DLL)
#  define LIBJSON_HAS_DLL 0
#endif /* ACE_AS_STATIC_LIBS && LIBJSON_HAS_DLL */

#if !defined (LIBJSON_HAS_DLL)
#  define LIBJSON_HAS_DLL 1
#endif /* ! LIBJSON_HAS_DLL */

#if defined (LIBJSON_HAS_DLL) && (LIBJSON_HAS_DLL == 1)
#  if defined (LIBJSON_BUILD_DLL)
#    define LibJSON_Export ACE_Proper_Export_Flag
#    define LIBJSON_SINGLETON_DECLARATION(T) ACE_EXPORT_SINGLETON_DECLARATION (T)
#    define LIBJSON_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_EXPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  else /* LIBJSON_BUILD_DLL */
#    define LibJSON_Export ACE_Proper_Import_Flag
#    define LIBJSON_SINGLETON_DECLARATION(T) ACE_IMPORT_SINGLETON_DECLARATION (T)
#    define LIBJSON_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_IMPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  endif /* LIBJSON_BUILD_DLL */
#else /* LIBJSON_HAS_DLL == 1 */
#  define LibJSON_Export
#  define LIBJSON_SINGLETON_DECLARATION(T)
#  define LIBJSON_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#endif /* LIBJSON_HAS_DLL == 1 */

// Set LIBJSON_NTRACE = 0 to turn on library specific tracing even if
// tracing is turned off for ACE.
#if !defined (LIBJSON_NTRACE)
#  if (ACE_NTRACE == 1)
#    define LIBJSON_NTRACE 1
#  else /* (ACE_NTRACE == 1) */
#    define LIBJSON_NTRACE 0
#  endif /* (ACE_NTRACE == 1) */
#endif /* !LIBJSON_NTRACE */

#if (LIBJSON_NTRACE == 1)
#  define LIBJSON_TRACE(X)
#else /* (LIBJSON_NTRACE == 1) */
#  if !defined (ACE_HAS_TRACE)
#    define ACE_HAS_TRACE
#  endif /* ACE_HAS_TRACE */
#  define LIBJSON_TRACE(X) ACE_TRACE_IMPL(X)
#  include "ace/Trace.h"
#endif /* (LIBJSON_NTRACE == 1) */

#endif /* LIBJSON_EXPORT_H */

// End of auto generated file.
