
// -*- C++ -*-
// $Id: Second_API_Export.h 35485 2000-10-10 22:16:08Z nanbor $
// Definition for Win32 Export directives.
// This file is generated automatically by generate_export_file.pl
// ------------------------------
#ifndef SECOND_API_EXPORT_H
#define SECOND_API_EXPORT_H

#include "ace/config-all.h"

#if defined (SECOND_API_AS_STATIC_LIBS)
#  if !defined (SECOND_API_HAS_DLL)
#    define SECOND_API_HAS_DLL 0
#  endif /* ! SECOND_API_HAS_DLL */
#else
#  if !defined (SECOND_API_HAS_DLL)
#    define SECOND_API_HAS_DLL 1
#  endif /* ! SECOND_API_HAS_DLL */
#endif

#if defined (SECOND_API_HAS_DLL) && (SECOND_API_HAS_DLL == 1)
#  if defined (SECOND_API_BUILD_DLL)
#    define Second_API_Export ACE_Proper_Export_Flag
#    define SECOND_API_SINGLETON_DECLARATION(T) ACE_EXPORT_SINGLETON_DECLARATION (T)
#    define SECOND_API_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_EXPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  else /* SECOND_API_BUILD_DLL */
#    define Second_API_Export ACE_Proper_Import_Flag
#    define SECOND_API_SINGLETON_DECLARATION(T) ACE_IMPORT_SINGLETON_DECLARATION (T)
#    define SECOND_API_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_IMPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  endif /* SECOND_API_BUILD_DLL */
#else /* SECOND_API_HAS_DLL == 1 */
#  define Second_API_Export
#  define SECOND_API_SINGLETON_DECLARATION(T)
#  define SECOND_API_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#endif /* SECOND_API_HAS_DLL == 1 */

#endif /* SECOND_API_EXPORT_H */

// End of auto generated file.
