/****************************************************************************\
*
*	Miscellaneous functions
*
*   COPYRIGHT:  2008 Suyandi Wu
*
\****************************************************************************/

#ifndef MISC_H_8945152
#define MISC_H_8945152

int MakeDir (const _TCHAR* szNewDir);
long GetFileLength (LPCTSTR szFilePathName);
BOOL CreateTempFilePathName (_TCHAR* szTempFilePathName);
BOOL SetPrivilege(HANDLE hToken, LPCTSTR lpszPrivName, BOOL bEnablePrivilege);
void PrintWinError(DWORD dwError, LPTSTR pszAPI, BOOL bExitProcess);
void MsgWinError(DWORD dwError, LPTSTR pszAPI, BOOL bExitProcess);
void PumpMessage ();

#endif