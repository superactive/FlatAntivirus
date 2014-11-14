/****************************************************************************\
*
*	FlatAV Core Engine
*
*   COPYRIGHT:  2008 Suyandi Wu
*
\****************************************************************************/

#ifndef FLATAV_CONFIG_H_SDIWU_
#define FLATAV_CONFIG_H_SDIWU_

#ifndef bool_t
#   define bool_t int
#endif

#if defined(WINDOWS) || defined(WIN32)
#  ifdef LIBFLATAV_DLL
#    if defined(_WIN32) && (!defined(__BORLANDC__) || (__BORLANDC__ >= 0x500))
#      ifdef LIBFLATAV_EXPORTS
#        define LIBEXTERN extern __declspec(dllexport)
#      else
#        define LIBEXTERN extern __declspec(dllimport)
#      endif
#    endif
#  endif

#  ifdef LIBFLATAV_WINAPI
#    ifdef FAR
#      undef FAR
#    endif
#    include <windows.h>
#    define LIBEXPORT WINAPI
#    ifdef WIN32
#      define LIBEXPORTVA WINAPIV
#    else
#      define LIBEXPORTVA FAR CDECL
#    endif
#  endif
#endif

#ifndef LIBEXTERN
#  define LIBEXTERN extern
#endif
#ifndef LIBEXPORT
#  define LIBEXPORT
#endif
#ifndef LIBEXPORTVA
#  define LIBEXPORTVA
#endif

#ifndef FAR
#  define FAR
#endif

#endif  /* FLATAV_CONFIG_H_SDIWU_ */