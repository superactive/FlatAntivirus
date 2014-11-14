/****************************************************************************\
*
*	Miscellaneous functions
*
*   COPYRIGHT:  2008 Suyandi Wu
*
\****************************************************************************/

#include "stdafx.h"
#include <direct.h>
#include <Ntsecapi.h>

int MakeDir (const _TCHAR* szNewDir)
{
    _TCHAR szBuffer[260];
    _TCHAR* p;

    _tcscpy (szBuffer, szNewDir);
    p = szBuffer;
    while (*p != '\0') {
        if (*p == '\\' || *p == '/') {
            _TCHAR ch = *p;
            *p = '\0';
#ifdef _WIN32
            _tmkdir (szBuffer);
#else
			_tmkdir (szBuffer, 775);
#endif
            *p = ch;
        }
        ++p;
    }
#ifdef _WIN32
    return _tmkdir (szBuffer);
#else
    return _tmkdir (szBuffer, 775);
#endif
}

long GetFileLength (LPCTSTR szFilePathName)
{
    long ret = -1;
    int handle = _topen (szFilePathName, _O_RDONLY | _O_BINARY);
    if (handle != -1) {
        ret = _filelength (handle);
        _close (handle);
    }
    return ret;
}

BOOL CreateTempFilePathName (_TCHAR* szTempFilePathName)
{
    _TCHAR szTmpNam[13];
    BOOL ret = ::GetTempPath (MAX_PATH, szTempFilePathName);
    ::PathRemoveBackslash (szTempFilePathName);
    _ttmpnam (szTmpNam);
    _tcscat (szTempFilePathName, szTmpNam);
    return ret;
}

BOOL SetPrivilege(HANDLE hToken, LPCTSTR lpszPrivilegeName, BOOL bEnablePrivilege)
{
    TOKEN_PRIVILEGES tp;
    LUID luid;

    if (!LookupPrivilegeValue(NULL, lpszPrivilegeName, &luid))
        return FALSE;

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    if (bEnablePrivilege)
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    else
        tp.Privileges[0].Attributes = 0;

    if (AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL)) {
        if (GetLastError() == ERROR_SUCCESS)
            return TRUE;
    }
    return FALSE;
}

void PrintWinError(DWORD dwError, LPTSTR pszAPI, BOOL bExitProcess)
{
    LPVOID lpvMessageBuffer;
    FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 
                   NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                   (LPTSTR) &lpvMessageBuffer, 0, NULL);

    _tprintf (TEXT("ERROR: API        = %s\n"), pszAPI);
    _tprintf (TEXT("       error code = %u\n"), dwError);
    _tprintf (TEXT("       message    = %s\n"), (LPTSTR) lpvMessageBuffer);

    LocalFree (lpvMessageBuffer);

    if (bExitProcess)
        ExitProcess (dwError);
}

void MsgWinError(DWORD dwError, LPTSTR pszAPI, BOOL bExitProcess)
{
    LPVOID lpvMessageBuffer;
    ::FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 
                   NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                   (LPTSTR) &lpvMessageBuffer, 0, NULL);

    TCHAR szMsg[1024];
    wsprintf (szMsg, TEXT("ERROR: API        = %s\n")
                     TEXT("       error code = %u\n")
                     TEXT("       message    = %s\n"),
                     pszAPI, dwError, (LPTSTR) lpvMessageBuffer);

    ::MessageBox (NULL, szMsg, NULL, 0);

    LocalFree (lpvMessageBuffer);

    if (bExitProcess)
        ExitProcess (dwError);
}

LSA_HANDLE LsaGetPolicyHandle()
{
    LSA_HANDLE lsahPolicyHandle;
    LSA_OBJECT_ATTRIBUTES lsaObjectAttributes;

    ZeroMemory(&lsaObjectAttributes, sizeof(lsaObjectAttributes));

    if (LsaOpenPolicy(
        NULL,
        &lsaObjectAttributes,
        POLICY_ALL_ACCESS,
        &lsahPolicyHandle
    ) != ERROR_SUCCESS)
        return NULL;

    return lsahPolicyHandle;
}

BOOL LsaAddPrivilege(LPTSTR szAccountName, LPWSTR wszPrivName)
{
    BOOL bRet = FALSE;
    DWORD cbSID = 0, cbDomainName = 0;
    LPTSTR pszDomainName = NULL;
    SID_NAME_USE snu;
    PSID pSID = NULL;

    LookupAccountName(NULL, szAccountName, NULL, &cbSID, pszDomainName, &cbDomainName, &snu);
    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        return FALSE;

    pSID = (PSID) malloc(cbSID);
    if (pSID == NULL)
        return FALSE;

    pszDomainName = (LPTSTR) malloc(cbDomainName);
    if (pszDomainName == NULL) {
        free (pSID);
        return FALSE;
    }

    if (LookupAccountName(NULL, szAccountName, pSID, &cbSID, pszDomainName, &cbDomainName, &snu)) {
        HANDLE lsahPolicyHandle = LsaGetPolicyHandle();
        if (lsahPolicyHandle) {
            LSA_UNICODE_STRING lusPrivilege;
            USHORT wPrivNameLen = (USHORT) wcslen(wszPrivName);
            lusPrivilege.Buffer = wszPrivName;
            lusPrivilege.Length =  wPrivNameLen * sizeof(WCHAR);
            lusPrivilege.MaximumLength= (wPrivNameLen+1) * sizeof(WCHAR);
            if (LsaAddAccountRights(lsahPolicyHandle, pSID, &lusPrivilege, 1) == ERROR_SUCCESS)
                bRet = TRUE;

            LsaClose(lsahPolicyHandle);
        }
    }
    free(pszDomainName);
    free(pSID);
    return bRet;
}

BOOL LsaIsPrivAssignedToAccount(LPTSTR szAccountName, LPWSTR wszPrivName)
{
    BOOL bRet = FALSE;
    DWORD cbSID = 0, cbDomainName = 0;
    LPTSTR pszDomainName = NULL;
    SID_NAME_USE snu;
    PSID pSID = NULL;

    LookupAccountName(NULL, szAccountName, NULL, &cbSID, pszDomainName, &cbDomainName, &snu);
    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        return FALSE;

    pSID = (PSID) malloc(cbSID);
    if (pSID == NULL)
        return FALSE;

    pszDomainName = (LPTSTR) malloc(cbDomainName);
    if (pszDomainName == NULL) {
        free (pSID);
        return FALSE;
    }

    if (LookupAccountName(NULL, szAccountName, pSID, &cbSID, pszDomainName, &cbDomainName, &snu)) {
        HANDLE lsahPolicyHandle = LsaGetPolicyHandle();
        if (lsahPolicyHandle) {
            PLSA_UNICODE_STRING pLsaUString;
            ULONG uRightCount;
            if (LsaEnumerateAccountRights(lsahPolicyHandle, pSID, &pLsaUString, &uRightCount) == ERROR_SUCCESS) {
                ULONG i;
                for (i = 0; i < uRightCount; ++i) {
                    if (lstrcmpW(pLsaUString[i].Buffer, wszPrivName) == 0) {
                        bRet = TRUE;
                        break;
                    }
                }
                LsaFreeMemory(pLsaUString);
            }
            LsaClose(lsahPolicyHandle);
        }
    }
    return bRet;
}

void PumpMessage ()
{
    MSG msg;
    while (::PeekMessage (&msg, NULL, 0, 0, PM_REMOVE) && msg.message != WM_QUIT) {
    	::TranslateMessage (&msg);
    	::DispatchMessage (&msg);
    }
}

// Static functions

//static int GetCurFileTime(
//    _TCHAR *filename,                /* name of file to get info on */
//    tm_zip *tmzip,             /* return value: access, modific. and creation times */
//    uLong *dt             /* dostime */
//)
//{
//    int ret = 0;
//#ifdef _WIN32
//    FILETIME ftLocal;
//    HANDLE hFind;
//    WIN32_FIND_DATA wfd;
//
//    hFind = FindFirstFile(filename, &wfd);
//    if (hFind != INVALID_HANDLE_VALUE) {
//        FileTimeToLocalFileTime(&(wfd.ftLastWriteTime), &ftLocal);
//        FileTimeToDosDateTime(&ftLocal, ((LPWORD)dt)+1, ((LPWORD)dt)+0);
//        FindClose(hFind);
//        ret = 1;
//    }
//#endif
//#ifdef unix
//    struct stat s;        /* results of stat() */
//    struct tm* filedate;
//    time_t tm_t = 0;
//
//    if (strcmp(filename, _T("-")) != 0) {
//        _THCHAR name[_MAX_PATH+1];
//        int len = strlen(f);
//        if (len > _MAX_PATH)
//            len = _MAX_PATH;
//
//        strncpy(name, f, _MAX_PATH-1);
//        /* strncpy doesnt append the trailing NULL, of the string is too long. */
//        name[_MAX_PATH] = '\0';
//
//        if (name[len - 1] == '/')
//            name[len - 1] = '\0';
//        /* not all systems allow stat'ing a file with / appended */
//        if (stat(name,&s) == 0) {
//            tm_t = s.st_mtime;
//            ret = 1;
//        }
//    }
//    filedate = localtime(&tm_t);
//
//    tmzip->tm_sec  = filedate->tm_sec;
//    tmzip->tm_min  = filedate->tm_min;
//    tmzip->tm_hour = filedate->tm_hour;
//    tmzip->tm_mday = filedate->tm_mday;
//    tmzip->tm_mon  = filedate->tm_mon ;
//    tmzip->tm_year = filedate->tm_year;
//#endif
//    return ret;
//}