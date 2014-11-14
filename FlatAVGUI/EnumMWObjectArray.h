/****************************************************************************\
*
*	For enumerating OS objects (memory process, start up items, registry keys, etc)
*
*   COPYRIGHT:  2008 Suyandi Wu
*
\****************************************************************************/

#pragma once

#include "FlatAVGUI.h"

// dwEnumType value
#define ET_ALL      0
#define ET_MEMORY   1
#define ET_STARTUP  2

class CEnumMWObjectArray
{
public:
    CEnumMWObjectArray(/*BOOL bEnumObj = FALSE*/);
    ~CEnumMWObjectArray();

    virtual BOOL EnumObjects (DWORD dwEnumType, BOOL bRemoveAllExistingObj = FALSE);
    void RemoveAllObjects ();

    BOOL AddMWObject (LPTSTR szFPN, DWORD dwType = MWT_FILE, DWORD dwPID = 0, 
        LPTSTR szServiceName = NULL);
    void AddMWObjectEx (LPTSTR szPath); // only accept if hasn't exist in array
    BOOL IsObjAlreadyExists (LPCTSTR szFPN);

    int EnumArgs (LPTSTR szPath);
    LPTSTR GetAbsPathFromFileSpec (LPTSTR szFileSpec, LPTSTR szOutput);

    INT_PTR GetSize () { return m_aptrMWObj.GetSize(); }
    LPMWOBJ GetAt (int nIndex) { return m_aptrMWObj.GetAt (nIndex); }

protected:
    BOOL EnumMemoryObjects();
    BOOL EnumStartupObjects();

private:
    CMWObjPtrArray m_aptrMWObj;
    void EnumRegKeyRun (HKEY hRootKey, LPCTSTR szSubKey);
    void EnumRegKeyWinlogon();
    void EnumRegKeySafeBoot (LPCTSTR szSubKey);
    BOOL IsDebugPrivEnabled();
};
