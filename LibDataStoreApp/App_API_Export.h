
// -*- C++ -*-
// $Id: App_API_Export.h 35485 2000-10-10 22:16:08Z nanbor $
// Definition for Win32 Export directives.
// This file is generated automatically by generate_export_file.pl
// ------------------------------
#ifndef APP_API_EXPORT_H
#define APP_API_EXPORT_H

#include "ace/config-all.h"

#if defined (APP_API_AS_STATIC_LIBS)
#  if !defined (APP_API_HAS_DLL)
#    define APP_API_HAS_DLL 0
#  endif /* ! APP_API_HAS_DLL */
#else
#  if !defined (APP_API_HAS_DLL)
#    define APP_API_HAS_DLL 1
#  endif /* ! APP_API_HAS_DLL */
#endif

#if defined (APP_API_HAS_DLL) && (APP_API_HAS_DLL == 1)
#  if defined (APP_API_BUILD_DLL)
#    define App_API_Export ACE_Proper_Export_Flag
#    define APP_API_SINGLETON_DECLARATION(T) ACE_EXPORT_SINGLETON_DECLARATION (T)
#    define APP_API_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_EXPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  else /* APP_API_BUILD_DLL */
#    define App_API_Export ACE_Proper_Import_Flag
#    define APP_API_SINGLETON_DECLARATION(T) ACE_IMPORT_SINGLETON_DECLARATION (T)
#    define APP_API_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_IMPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  endif /* APP_API_BUILD_DLL */
#else /* APP_API_HAS_DLL == 1 */
#  define App_API_Export
#  define APP_API_SINGLETON_DECLARATION(T)
#  define APP_API_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#endif /* APP_API_HAS_DLL == 1 */

#endif /* APP_API_EXPORT_H */

// End of auto generated file.
