/****************************************************************************\
*
*   FILE:       debug.h
*
*   PURPOSE:    Functions for debugging.
*
*   FUNCTIONS:
*       EXPORTS:
*           dbg_printf  - Print debug info from arguments.
*
*   COPYRIGHT:  2008 Suyandi Wu
*
\****************************************************************************/
#ifndef DEBUG_H_SIDWU
#define DEBUG_H_SIDWU

#ifdef _DEBUG

void dbg_printf (const _TCHAR* fmt, ...);
void dbg_error (const _TCHAR* fmt, ...);
void dbg_writelog (const TCHAR* fn, const TCHAR* fmt, ...);
void dbg_writelogln (const TCHAR* fn, const TCHAR* fmt, ...);

#else

#define dbg_printf(fmt, ...)
#define dbg_error(fmt, ...)
#define dbg_writelog(f, fmt, ...)
#define dbg_writelogln(f, fmt, ...)
//void dbg_printf (const _TCHAR* fmt, ...);
//void dbg_error (const _TCHAR* fmt, ...);
//void dbg_writelog (const TCHAR* fn, const TCHAR* fmt, ...);
//void dbg_writelogln (const TCHAR* fn, const TCHAR* fmt, ...);
#endif

#endif