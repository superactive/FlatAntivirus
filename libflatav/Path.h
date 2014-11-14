/****************************************************************************\
*
*	File/Folder Path Functions
*
*   COPYRIGHT:  2008 Suyandi Wu
*
\****************************************************************************/

#ifndef PATH_H_
#define PATH_H_

#include <tchar.h>

#ifdef __cplusplus
extern "C" {
#endif

int PathAddExt (_TCHAR* szPath, _TCHAR* szExt);
int PathRemoveExt (_TCHAR* szPath);
_TCHAR* PathFindExt (const _TCHAR* szPath);
int PathAddBkSlash (_TCHAR* szPath);
void PathTrimSpaces(_TCHAR* szPath);
int PathQuote (_TCHAR* szPath);
int PathUnquote (_TCHAR* szPath);

#ifdef __cplusplus
}
#endif


#endif
