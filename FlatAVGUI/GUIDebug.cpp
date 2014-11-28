#include "stdafx.h"

void dbg_msgbox(const TCHAR* fmt, ...)
{
	va_list vargs;
	TCHAR buffer[BUFSIZ];

	va_start(vargs, fmt);
	_vsntprintf_s(buffer, BUFSIZ, _TRUNCATE, fmt, vargs);
	buffer[BUFSIZ - 1] = '\0';

	::MessageBox(NULL, buffer, NULL, 0);

	va_end(vargs);
}

void dbg_writelog(const TCHAR* fn, const TCHAR* fmt, ...)
{
	FILE* file;
	va_list vargs;
	_TCHAR buffer[BUFSIZ];

	va_start(vargs, fmt);
	_vsntprintf_s(buffer, BUFSIZ, _TRUNCATE, fmt, vargs);
	buffer[BUFSIZ - 1] = '\0';

	file = _tfopen(fn, _T("a+"));
	if (file == NULL)
		return;

	_fputts(buffer, file);
	//_ftprintf (file, _T("%s\n"), buffer);

	fclose(file);
	va_end(vargs);
}