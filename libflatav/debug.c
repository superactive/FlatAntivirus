/****************************************************************************\
*
*   FILE:       debug.c
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
#include <tchar.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

void dbg_printf(const _TCHAR* fmt, ...)
{
	va_list vargs;
	_TCHAR buffer[BUFSIZ];
	int n = sizeof(_T("FlatAV Debug: ")) / sizeof(_TCHAR) - 1;
	_tcsncpy(buffer, _T("FlatAV Debug: "), n);

	va_start(vargs, fmt);
	_vsntprintf(buffer + n, BUFSIZ - n, fmt, vargs);
	buffer[BUFSIZ - 1] = '\0';
	_fputts(buffer, stderr);
	va_end(vargs);
}

void dbg_error(const _TCHAR* fmt, ...)
{
	va_list vargs;
	TCHAR buffer[BUFSIZ];
	int n = sizeof(_T("FlatAV Error: ")) / sizeof(_TCHAR) - 1;
	_tcsncpy(buffer, _T("FlatAV Error: "), n);

	va_start(vargs, fmt);
	_vsntprintf(buffer + n, BUFSIZ - n, fmt, vargs);
	buffer[BUFSIZ - 1] = '\0';
	_fputts(buffer, stderr);
	va_end(vargs);
}

void dbg_writelog(const TCHAR* fn, const TCHAR* fmt, ...)
{
	FILE* file;
	va_list vargs;
	_TCHAR buffer[BUFSIZ];

	va_start(vargs, fmt);
	_vsntprintf(buffer, BUFSIZ, fmt, vargs);
	buffer[BUFSIZ - 1] = '\0';

	file = _tfopen(fn, _T("a+"));
	if (file == NULL)
		return;

	_fputts(buffer, file);

	fclose(file);
	va_end(vargs);
}

void dbg_writelogln(const TCHAR* fn, const TCHAR* fmt, ...)
{
	FILE* file;
	va_list vargs;
	_TCHAR buffer[BUFSIZ];

	va_start(vargs, fmt);
	_vsntprintf(buffer, BUFSIZ, fmt, vargs);
	buffer[BUFSIZ - 1] = '\0';

	file = _tfopen(fn, _T("a+"));
	if (file == NULL)
		return;

	_ftprintf(file, _T("%s\n"), buffer);

	fclose(file);
	va_end(vargs);
}
