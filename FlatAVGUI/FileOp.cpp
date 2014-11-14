/****************************************************************************\
*
*	File operation functions
*
*   COPYRIGHT:  2008 Suyandi Wu
*
\****************************************************************************/

#include "stdafx.h"

#include "..\libflatav\flatav.h"
#include "Misc.h"

BOOL DeleteFileToRecycleBin (LPCTSTR szFilePathName)
{
    TCHAR _szFilePathName[MAX_PATH];
    ::lstrcpy (_szFilePathName, szFilePathName);
    _szFilePathName[::lstrlen(_szFilePathName)+1] = '\0';
    SHFILEOPSTRUCT sfo;
    sfo.hwnd = NULL;
    sfo.wFunc = FO_DELETE;
    sfo.pFrom = _szFilePathName;
    sfo.pTo = NULL;
    sfo.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
    sfo.fAnyOperationsAborted = 0;
    sfo.hNameMappings = NULL;
    sfo.lpszProgressTitle = NULL;

    return (::SHFileOperation (&sfo) == 0);
}

int DeleteFileEx (LPCTSTR szFilePathName, BOOL bDelPermanent, BOOL bDelLockedFiles)
{
    int ret = FL_SUCCESS;
    BOOL bSuccess;
    if (::GetFileAttributes (szFilePathName) == 0xffffffff)
        return ret;//FL_ENOFILE;

    ::SetFileAttributes (szFilePathName, FILE_ATTRIBUTE_NORMAL);
    if (bDelPermanent)
        bSuccess = ::DeleteFile (szFilePathName);
    else {
        // Delete file to recycle bin
        bSuccess = DeleteFileToRecycleBin (szFilePathName);
    }
    if (!bSuccess) {
        ret = FL_EWRITE;
        if (bDelLockedFiles) {
            // The file might be locked, try delete it at next system restart.
            ::MoveFileEx(szFilePathName, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
        }
    }
    return ret;
}

int MoveRenameFileEx (LPCTSTR szFilePathName, LPCTSTR szDestFolderPath, 
                       BOOL bRenFile, BOOL bMoveFile, BOOL bMoveRenLockedFiles)
{
    TCHAR szNewPathName[MAX_PATH], szResult[MAX_PATH];

    int ret = FL_SUCCESS;
    if (::GetFileAttributes (szFilePathName) == 0xffffffff)
        return ret;//FL_ENOFILE;

    ::SetFileAttributes (szFilePathName, FILE_ATTRIBUTE_NORMAL);
    ::lstrcpy (szResult, szFilePathName);

    BOOL bSuccess = TRUE;
    if (bRenFile) {
        ::lstrcpy (szNewPathName, szFilePathName);
        ::lstrcat (szNewPathName, _T(".vir"));
        bSuccess = ::MoveFileEx (szResult, szNewPathName, MOVEFILE_REPLACE_EXISTING);
        if (bSuccess)
            ::lstrcpy (szResult, szNewPathName);
    }

    if (bMoveFile && bSuccess) {
        if (::GetFileAttributes (szDestFolderPath) == 0xffffffff && MakeDir (szDestFolderPath) == -1)
            return FL_EACCESS;
        ::lstrcpy (szNewPathName, szDestFolderPath);
        ::PathAddBackslash (szNewPathName);
        ::lstrcat (szNewPathName, ::PathFindFileName (szResult));
        bSuccess = ::MoveFileEx (szResult, szNewPathName, MOVEFILE_REPLACE_EXISTING);
    }

    if (!bSuccess) {
        ret = FL_EWRITE;
        if (bMoveRenLockedFiles) {
            // The file might be locked, try move/rename it at next system restart.
            ::MoveFileEx (szResult, szNewPathName, MOVEFILE_DELAY_UNTIL_REBOOT);
        }
    }

    return ret;
}