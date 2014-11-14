/****************************************************************************\
*
*	FlatAV GUI Main Apps
*
*   COPYRIGHT:  2008 Suyandi Wu
*
\****************************************************************************/

// FlatAVGUI.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "..\libflatav\flatav.h"
#include "misc.h"

#define REGKEY_FLATAV    _T("Software\\SuyandiWu\\FlatAV")

typedef struct ScanOpt_t
{
    struct archive_opt_t
    {
        BOOL bScanArchive;      /* 1 = scan archive, 0 = no scan archive */
        int dwMaxFileExtract;   /* Max files to be extracted for scanning */
        int dwMaxSubArcExtract; /* Max sub-archive to be extracted for scanning */
        int dwMaxScanArcSize;   /* Max archive size for scanning */
    } archive_opt;
    BOOL bScanSubFolders;
} SCANOPT, *LPSCANOPT;

typedef struct MWObj_t
{
    DWORD dwType;
    TCHAR szFilePathName[MAX_PATH];
    union {
        //int nArchiveType;
        DWORD dwProcessId;
    };
    union {
        TCHAR szFileNameInArchive[MAX_PATH];
        TCHAR szServiceName[MAX_PATH];
    };
    int nScanResult;
    char szMalwareName[31+1];
    int nMalwareType;
} MWOBJ, *LPMWOBJ;

typedef CTypedPtrArray<CPtrArray, LPMWOBJ> CMWObjPtrArray;

#define MWT_FILE        0
#define MWT_ARCHIVE     1
#define MWT_PROCESS     2
#define MWT_SERVICE     3

// CFlatAVGUIApp:
// See FlatAVGUI.cpp for the implementation of this class
//

class CFlatAVGUIApp : public CWinApp
{
public:
	CFlatAVGUIApp();
    ~CFlatAVGUIApp() 
    {
        if (m_pMalloc) m_pMalloc->Release();
    }
    
// Overrides
public:
	CEvent g_event;
    BOOL m_bCanDebug;
    void SHMallocFree (void* pv)
    {
        m_pMalloc->Free (pv);
    }

	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
protected:
    LPMALLOC m_pMalloc;
};

extern CFlatAVGUIApp theApp;