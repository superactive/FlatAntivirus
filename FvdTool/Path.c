/****************************************************************************\
*
*	File/Folder Path Functions
*
*   COPYRIGHT:  2008 Suyandi Wu
*
\****************************************************************************/

#include <tchar.h>
#include <stdlib.h>

/*
	PathAddExt
	Add extension to a path string, if there is already a file extension present,
	no extension will be added.
	@PARAMIO szPath : path string that will be appended with extension.
	@PARAMI szExt   : extension string to be appended to szPath (should be prefixed
	with '.'.
	@RETURN         : 1 if an extension was added, or 0 otherwise.
	*/
int PathAddExt(_TCHAR* szPath, _TCHAR* szExt)
{
	_TCHAR* p;
	p = _tcsrchr(szPath, '.');
	if (p == NULL) {
		_tcscat_s(szPath, _MAX_PATH, szExt);
		return 1;
	}
	else
		return 0;
}

/*
	PathRemoveExt
	Remove a file extension from a path, if there is no file extension present,
	no extension will be removed.
	@PARAMIO szPath : path string that its extension will be removed.
	@RETURN         : 1 if an extension was removed, or 0 otherwise.
	*/
int PathRemoveExt(_TCHAR* szPath)
{
	_TCHAR* p;
	p = _tcsrchr(szPath, '.');
	if (p != NULL) {
		szPath[p - szPath] = '\0';
		return 1;
	}
	return 0;
}

/*
	PathFindExt
	Searches a path for an extension.
	@PARAMIO szPath : path string.
	@RETURN         : address of the "." from extension found, or NULL if no extension found.
	*/
_TCHAR* PathFindExt(const _TCHAR* szPath)
{
	_TCHAR* p;
	p = _tcsrchr(szPath, '.');
	if (p != NULL) {
		return p;
	}
	else
		return NULL;
}

/*
	PathAddBackslash
	Adds a backslash to the end of a string to create the correct syntax for a path.
	If the source path already has a trailing backslash, no backslash will be added.
	@PARAMIO szPath : path string.
	@RETURN         : 1 if an backslash was added, or 0 otherwise.
	*/
int PathAddBkSlash(_TCHAR* szPath)
{
	size_t n = _tcslen(szPath) - 1;
	if (n >= 0 && szPath[n] != '\\') {
		szPath[n + 1] = '\\';
		szPath[n + 2] = '\0';
		return 1;
	}
	return 0;
}

/*
	PathTrimSpaces
	Trims all leading and trailing spaces from a string.
	@PARAMIO szPath : path string.
	*/
void PathTrimSpaces(_TCHAR* szPath)
{
	_TCHAR buff[_MAX_PATH];
	int foundSpace = 0;
	size_t n;
	_TCHAR* p;

	n = _tcslen(szPath);
	p = szPath + n - 1;
	while (*p == ' ') {
		--p;
	}
	p[1] = '\0';

	p = szPath;
	if (*p++ == ' ') foundSpace = 1;
	while (*p == ' ') {
		++p;
	}
	if (foundSpace) {
		_tcscpy_s(buff, _MAX_PATH, p);
		_tcscpy_s(szPath, _MAX_PATH, buff);
	}
}

/*
	PathQuote
	Enclose path with quotation marks, if spaces are found.
	@PARAMIO szPath : path string.
	@RETURN         : 1 if an quotation marks was added, or 0 otherwise.
	*/
int PathQuote(_TCHAR* szPath)
{
	size_t n;
	n = _tcslen(szPath);
	if (_tcschr(szPath, ' ') != NULL) {
		size_t i;
		szPath[n + 2] = '\0';
		szPath[n + 1] = '"';
		for (i = n; i > 0; --i)
			szPath[i] = szPath[i - 1];
		szPath[i] = '"';
		return 1;
	}
	return 0;
}


/*
	PathUnquote
	Remove quotation marks from a string.
	@PARAMIO szPath : path string.
	@RETURN         : 1 if an quotation marks was removed, or 0 otherwise.
	*/
int PathUnquote(_TCHAR* szPath)
{
	_TCHAR buff[_MAX_PATH];
	size_t n;
	n = _tcslen(szPath);
	if (szPath[0] == '"' && szPath[n - 1] == '"') {
		_tcscpy_s(buff, _MAX_PATH, &szPath[1]);
		buff[n - 2] = '\0';
		_tcscpy_s(szPath, _MAX_PATH, buff);
		return 1;
	}
	return 0;
}