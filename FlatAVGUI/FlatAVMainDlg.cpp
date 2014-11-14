/****************************************************************************\
*
*	FlatAV GUI Main Dialog
*
*   COPYRIGHT:  2008 Suyandi Wu
*
\****************************************************************************/

// FlatAVMainDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FlatAVGUI.h"
#include "FlatAVMainDlg.h"
//#include ".\flatavmaindlg.h"

#include "ScanDlg.h"
#include "ScanMemDlg.h"

//#include "uniquestringarray.h"
//#include "SplashWnd.h"
#include "SplashScanMemDlg.h"
#include ".\flatavmaindlg.h"

//#include "SplashScanMemThread.h"
#include "SplashScanMemDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define REGKEY_SCANOPT    _T("Software\\SuyandiWu\\FlatAV\\ScanOpt")

UINT SplashDlgThreadFunc(LPVOID pParam) {
	CSplashScanMemDlg* pDlg = (CSplashScanMemDlg*) pParam;
	return pDlg->DoModal();
}

UINT ScanThreadFunc(LPVOID pParam) {
	CSplashScanMemDlg* pDlg = (CSplashScanMemDlg*) pParam;
	//while(pDlg->m_bActive == FALSE) {
	//	if (pDlg->m_bStopScan == TRUE) {
	//		pDlg->EndDialog(0);
	//		return 0;
	//	}
	//}
	if (pDlg->m_bStopScan == FALSE) {
		pDlg->ScanMemory();
		pDlg->EndDialog(0);
	}
	return 0;
}

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CFlatAVMainDlg dialog

LPCTSTR CFlatAVMainDlg::IDD_CLASS = _T("FAMain");

CFlatAVMainDlg::CFlatAVMainDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFlatAVMainDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

    m_ScanOpt.archive_opt.bScanArchive = TRUE;
    m_ScanOpt.archive_opt.dwMaxFileExtract = 100;
    m_ScanOpt.archive_opt.dwMaxScanArcSize = 10000000;
    m_ScanOpt.archive_opt.dwMaxSubArcExtract = 5;
    m_ScanOpt.bScanSubFolders = TRUE;

    //::SHGetMalloc (&m_pMalloc);
    //::SHGetDesktopFolder (&m_pShellFolder);
}

void CFlatAVMainDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_SHELLFOLDERTREE, m_wndSfTree);
}

BEGIN_MESSAGE_MAP(CFlatAVMainDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_BTNSCAN, OnBtnScanClicked)
    ON_WM_DESTROY()
    ON_BN_CLICKED(IDC_CHKTESTARCHIVES, OnChkTestArchivesClicked)
    ON_BN_CLICKED(IDC_CHKSCANSUBFOLDERS, OnChkScanSubFoldersClicked)
    ON_COMMAND(IDM_SCAN, OnBtnScanClicked)
    ON_COMMAND(IDM_SCANMEMORY, OnCmdScanMemory)
    ON_COMMAND(IDM_ABOUTBOX, OnAboutbox)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CFlatAVMainDlg, CDialog)
END_EVENTSINK_MAP()

// CFlatAVMainDlg message handlers

BOOL CFlatAVMainDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	//SetIcon(m_hIcon, TRUE);			// Set big icon
	//SetIcon(m_hIcon, FALSE);		// Set small icon

   // Display database header information
    FVD_HEADER fvdHdr;
    char szDispText[16];
    Fvd_ParseHeader (&fvdHdr, &m_fvdBlock);
    ::wsprintfA (szDispText, "%d-%d, %s", fvdHdr.dwDbMayorVersion, fvdHdr.bDbMinorVersion,
        fvdHdr.szReleaseDate);
    ::SetDlgItemTextA (m_hWnd, IDC_LBLDBVER, szDispText);
    SetDlgItemInt (IDC_LBLSIGCOUNT, fvdHdr.dwSignCount, FALSE);

    // Expand folder "My Computer" on shell tree-view
    LPITEMIDLIST pIdl;
    ::SHGetSpecialFolderLocation (m_hWnd, CSIDL_DRIVES, &pIdl);
    m_wndSfTree.Expand ((int*) pIdl, TVE_EXPAND);
    theApp.SHMallocFree (pIdl);

    // Read scan options
    ReadRegScanOptions ();

    ((CButton*)GetDlgItem (IDC_CHKTESTARCHIVES))->SetCheck (m_ScanOpt.archive_opt.bScanArchive);
    ((CButton*)GetDlgItem (IDC_CHKSCANSUBFOLDERS))->SetCheck (m_ScanOpt.bScanSubFolders);

	SetForegroundWindow();

    m_wndSfTree.SetFocus ();

	return FALSE;  // return TRUE  unless you set the focus to a control
}

BOOL CFlatAVMainDlg::Initialize() {
	// Load database
    int ret = Fvd_LoadDb (_T("main.fvd"), &m_fvdBlock);
    if (ret != FL_SUCCESS) {
        MessageBox (_T("Load database failed."), DEFCAPTION, MB_ICONERROR);
        EndDialog (-1);
        return FALSE;
    }

    // Validate database
    ret = Fvd_ValidateDb (&m_fvdBlock, 1, 0);
    if (ret != FL_SUCCESS) {
        MessageBox (_T("Validate database failed."), DEFCAPTION, MB_ICONERROR);
        EndDialog (-1);
        return FALSE;
    }

    // Show splash
    //CSplashScanMemDlg splashScanMemDlg(IDB_SPLASH, &m_fvdBlock, &m_ScanOpt);
    //splashScanMemDlg.ShowSplash();
	//splashScanMemDlg.DoModal();

	//CSplashScanMemThread* pspt = (CSplashScanMemThread*) AfxBeginThread (
	//	RUNTIME_CLASS(CSplashScanMemThread), THREAD_PRIORITY_NORMAL,
	//	0, CREATE_SUSPENDED);
	//pspt->m_uBitmapID = IDB_SPLASH;
	//pspt->m_pFvdBlock = &m_fvdBlock;
	//pspt->m_pScanOpt = &m_ScanOpt;
	
	//pspt->ScanMemory();

	//CSplashScanMemDlg dlg;
	//dlg.Create(IDB_SPLASH);
	//pspt->m_pSplashScanMemDlg = &dlg;

	//pspt->ShowSplash();
	//::WaitForSingleObject(pspt->m_hThread, INFINITE);
	//while (!pspt->m_bFinishedScan) {
	//	Sleep(5000);
	//}
	//::PostThreadMessage(pspt->m_nThreadID, WM_QUIT, 0, 0);

	//CWinThread* pThread = ::AfxBeginThread(ThreadFunc, this);

	//DWORD dw = ::WaitForSingleObject(pspt->m_hThread, INFINITE);

	CSplashScanMemDlg splashDlg(&m_fvdBlock, &m_ScanOpt);

	CWinThread* pSplashDlgThread = ::AfxBeginThread(SplashDlgThreadFunc, &splashDlg);

	//WINDOWPLACEMENT wp;
	//while (!IsWindow(splashDlg.m_hWnd) || (!splashDlg.IsWindowEnabled())) {

	//}
	//Sleep(500);
	theApp.g_event.Lock();
	CWinThread* pScanThread = ::AfxBeginThread(ScanThreadFunc, &splashDlg);
	::WaitForSingleObject(pScanThread->m_hThread, INFINITE);

	return TRUE;
}

void CFlatAVMainDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
        OnAboutbox();
	else
		CDialog::OnSysCommand(nID, lParam);
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CFlatAVMainDlg::OnPaint() 
{
	if (IsIconic()) {
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else {
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CFlatAVMainDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CFlatAVMainDlg::OnDestroy()
{
    Fvd_UnloadDb(&m_fvdBlock);

    // Write scan options
    WriteRegScanOptions ();

    CDialog::OnDestroy();
}

void CFlatAVMainDlg::OnBtnScanClicked()
{
    CStringArray astrScanPathList;

    // Get selected filepaths from shell tree-view
    int ret = IterateCheckedItem (NULL, astrScanPathList);
    if (ret) {
        if (astrScanPathList.GetSize() == 0) {
            TCHAR buff[MAX_PATH];
            ::GetWindowsDirectory (buff, MAX_PATH);
            astrScanPathList.Add (buff);

            ::GetSystemDirectory (buff, MAX_PATH);
            astrScanPathList.Add (buff);
        }

        CScanDlg scanDlg(&m_fvdBlock, &m_ScanOpt, &astrScanPathList);
        scanDlg.DoModal();

        astrScanPathList.RemoveAll ();
    }
    else {
        MessageBox (_T("No file selected."), DEFCAPTION, MB_ICONWARNING);
    }
    /*for (int i=0; i < scanDlg.m_astrScanPathList.GetSize (); ++i)
        MessageBox (scanDlg.m_astrScanPathList.GetAt (i), "Iterate Results");*/
}

void CFlatAVMainDlg::GetDriveStringList (CStringArray& strArray)
{
    TCHAR szBuff[MAX_PATH];
    int n = ::GetLogicalDriveStrings (MAX_PATH, szBuff);
    TCHAR* p;
    p = szBuff;
    strArray.Add (p);
    for (int i=0; i<n; ++i) {
        if (p[i] == '\0') {
            ++i;
            if (p[i]) strArray.Add (&p[i]);
        }
    }
}

void CFlatAVMainDlg::AddCheckedItem (BSTR bstrPath, CStringArray& strArray)
{
#ifndef _UNICODE
    char _str01_[MAX_PATH];
    ::WideCharToMultiByte (CP_ACP, 0, bstrPath, -1, _str01_, MAX_PATH, NULL, NULL);
#else
#   define _str01_ bstrPath
#endif

    if (!::PathIsFileSpec (_str01_)) {
        strArray.Add (_str01_);
        return;
    }

    LPITEMIDLIST pIdl;
    SHFILEINFO sfi;
    TCHAR buff[MAX_PATH];

    // check for folder "My Computer"
    ::SHGetSpecialFolderLocation (m_hWnd, CSIDL_DRIVES, &pIdl);
    ::SHGetFileInfo ((LPTSTR) pIdl, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_DISPLAYNAME);
    theApp.SHMallocFree (pIdl);
    if (::lstrcmpi (_str01_, sfi.szDisplayName) == 0) {
        GetDriveStringList (strArray);
        return;
    }

    // check for folder "Recycle Bin"
    ::SHGetSpecialFolderLocation (m_hWnd, CSIDL_BITBUCKET, &pIdl);
    ::SHGetFileInfo ((LPTSTR) pIdl, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_DISPLAYNAME);
    theApp.SHMallocFree (pIdl);
    if (::lstrcmpi (_str01_, sfi.szDisplayName) == 0) {
        ::GetWindowsDirectory (buff, MAX_PATH);
        buff[3] = '\0';
        ::lstrcat (buff, _T("Recycled"));
        strArray.Add (buff);
        return;
    }

    // check for folder "internet explorer"
    ::SHGetSpecialFolderLocation (m_hWnd, CSIDL_INTERNET, &pIdl);
    ::SHGetFileInfo ((LPTSTR) pIdl, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_DISPLAYNAME);
    theApp.SHMallocFree (pIdl);
    if (::lstrcmpi (_str01_, sfi.szDisplayName) == 0) {
        TCHAR szName[] = _T("Path");
        DWORD dwType, dwBuffSize = MAX_PATH;
        if (::SHRegGetUSValue (_T("SOFTWARE\\Microsoft\\IE Setup\\Setup"), szName, &dwType,
            buff, &dwBuffSize, TRUE, NULL, 0) == ERROR_SUCCESS)
            strArray.Add (buff);
        return;
    }

    // check for file "my network places" and its subdirectories
    ::SHGetSpecialFolderLocation (m_hWnd, CSIDL_NETWORK, &pIdl);
    ::SHGetFileInfo ((LPTSTR) pIdl, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_DISPLAYNAME);
    theApp.SHMallocFree (pIdl);
    if (::lstrcmpi (_str01_, sfi.szDisplayName) == 0 ||
        ::lstrcmpi (_str01_, _T("Add Network Place")) == 0 || 
        ::lstrcmpi (_str01_, _T("Entire Network")) == 0) 
    {
        ::SHGetSpecialFolderLocation (m_hWnd, CSIDL_NETHOOD, &pIdl);
        ::SHGetPathFromIDList (pIdl, buff);
        theApp.SHMallocFree (pIdl);
        strArray.Add (buff);
        return;
    }       
}

BOOL CFlatAVMainDlg::IterateCheckedItem (HTREEITEM hItemRoot, CStringArray& strArray)
{
    BSTR bstrPath = ::SysAllocString (L"");
    int nState;
    HTREEITEM hItem = (HTREEITEM) m_wndSfTree.GetFirstCheckedItem ((int*) hItemRoot, 
        &bstrPath, &nState);
    if (hItem) {
        if ( nState == m_wndSfTree.CHECKED ) {
            AddCheckedItem (bstrPath, strArray);
        }
        else if (nState == m_wndSfTree.CHILD_CHECKED)
            IterateCheckedItem (hItem, strArray);
        else
            return 1;
    }
    else
        return 0;

    while (hItem) {
        hItem = (HTREEITEM) m_wndSfTree.GetNextCheckedItem ((int*) hItem, &bstrPath, &nState);
        if (hItem) {
            if (nState == m_wndSfTree.CHECKED) {
                AddCheckedItem (bstrPath, strArray);
            }
            else if (nState == m_wndSfTree.CHILD_CHECKED)
                IterateCheckedItem (hItem, strArray);
        }
    }

    if (bstrPath)
        ::SysFreeString (bstrPath);
    return 1;
}


void CFlatAVMainDlg::OnChkTestArchivesClicked()
{
    m_ScanOpt.archive_opt.bScanArchive = ((CButton*)GetDlgItem (IDC_CHKTESTARCHIVES))->GetCheck ();
}

void CFlatAVMainDlg::OnChkScanSubFoldersClicked()
{
    m_ScanOpt.bScanSubFolders = ((CButton*)GetDlgItem (IDC_CHKSCANSUBFOLDERS))->GetCheck ();
}

BOOL CFlatAVMainDlg::ReadRegScanOptions ()
{
    HKEY hKey;
    int ret = ::RegOpenKeyEx (HKEY_CURRENT_USER, REGKEY_SCANOPT, 0, KEY_READ, &hKey);
    if (ret == ERROR_SUCCESS) {
        DWORD dwDataSize = sizeof(DWORD);
        ::RegQueryValueEx (hKey, _T("ScanSubFolder"), NULL, NULL, 
            (LPBYTE) &m_ScanOpt.bScanSubFolders, &dwDataSize);

        ::RegQueryValueEx (hKey, _T("ScanArchive"), NULL, NULL,
            (LPBYTE) &m_ScanOpt.archive_opt.bScanArchive, &dwDataSize);

        ::RegQueryValueEx (hKey, _T("MaxFileExtract"), NULL, NULL,
            (LPBYTE) &m_ScanOpt.archive_opt.dwMaxFileExtract, &dwDataSize);

        ::RegQueryValueEx (hKey, _T("MaxScanArcSize"), NULL, NULL,
            (LPBYTE) &m_ScanOpt.archive_opt.dwMaxScanArcSize, &dwDataSize);

        ::RegQueryValueEx (hKey, _T("MaxSubArcExtract"), NULL, NULL,
            (LPBYTE) &m_ScanOpt.archive_opt.dwMaxSubArcExtract, &dwDataSize);

        ::RegCloseKey (hKey);
        return TRUE;
    }
    return FALSE;
}

BOOL CFlatAVMainDlg::WriteRegScanOptions ()
{
    HKEY hKey;
    int ret = ::RegCreateKeyEx (HKEY_CURRENT_USER, REGKEY_SCANOPT, 0, NULL, 0,
        KEY_ALL_ACCESS, NULL, &hKey, NULL);
    if (ret == ERROR_SUCCESS) {
        ::RegSetValueEx (hKey, _T("ScanSubFolder"), 0, REG_DWORD, 
            (LPBYTE) &m_ScanOpt.bScanSubFolders, sizeof(DWORD));

        ::RegSetValueEx (hKey, _T("ScanArchive"), 0, REG_DWORD, 
            (LPBYTE) &m_ScanOpt.archive_opt.bScanArchive, sizeof(DWORD));

        ::RegSetValueEx (hKey, _T("MaxFileExtract"), 0, REG_DWORD, 
            (LPBYTE) &m_ScanOpt.archive_opt.dwMaxFileExtract, sizeof(DWORD));

        ::RegSetValueEx (hKey, _T("MaxScanArcSize"), 0, REG_DWORD, 
            (LPBYTE) &m_ScanOpt.archive_opt.dwMaxScanArcSize, sizeof(DWORD));

        ::RegSetValueEx (hKey, _T("MaxSubArcExtract"), 0, REG_DWORD, 
            (LPBYTE) &m_ScanOpt.archive_opt.dwMaxSubArcExtract, sizeof(DWORD));

        ::RegCloseKey (hKey);
        return TRUE;
    }
    return FALSE;
}

// HKLM
#define REGKEY_HKLM_RUN _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run")
#define REGKEY_WINLOGON _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon")
#define REGKEY_CCS_SAFEBOOT _T("SYSTEM\CurrentControlSet\\Control\\SafeBoot")
// HKCU
#define REGKEY_HKCU_RUN _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run")

/*
void CFlatAVMainDlg::OnCmdScanMemory()
{
    //CUniqueStringArray astrScanPathList;
    CScanMemoryPtrArray aptrScanMem;
    BOOL bAlreadyExists;
    LPSCANMEMORY pScanMemory;
    int i, j;

    // Get all application path in process
    HANDLE hProcessSnap, hModuleSnap;
    PROCESSENTRY32 procEntry32;
    MODULEENTRY32 moduleEntry32;

    hProcessSnap = ::CreateToolhelp32Snapshot (TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        MessageBox (_T("Cannot scan memory."), DEFCAPTION, MB_ICONERROR);
        return;
    }

    // Get the application files first
    procEntry32.dwSize = sizeof(PROCESSENTRY32);
    if (::Process32First (hProcessSnap, &procEntry32)) {
        while (::Process32Next (hProcessSnap, &procEntry32)) {
            hModuleSnap = ::CreateToolhelp32Snapshot (TH32CS_SNAPMODULE, 
                procEntry32.th32ProcessID);
            if (hModuleSnap != INVALID_HANDLE_VALUE) {
                moduleEntry32.dwSize = sizeof(MODULEENTRY32);
                if (::Module32First (hModuleSnap, &moduleEntry32)) {
                    // Make sure the filepath is not in array yet
                    bAlreadyExists = FALSE;
                    for (i = 0; i < aptrScanMem.GetSize (); ++i) {
                        pScanMemory = aptrScanMem.GetAt (i);
                        if (::lstrcmpi (pScanMemory->szFilePathName, moduleEntry32.szExePath) == 0) {
                            bAlreadyExists = TRUE;
                            break;
                        }
                    }
                    if (!bAlreadyExists) {
                        pScanMemory = new SCANMEMORY;
                        pScanMemory->dwType = SMT_PROCESS;
                        ::lstrcpy (pScanMemory->szFilePathName, moduleEntry32.szExePath);
                        pScanMemory->dwProcessId = procEntry32.th32ProcessID;
                        aptrScanMem.Add (pScanMemory);
                    }
                }
                ::CloseHandle (hModuleSnap);
            }
        }
    }
    // Next, get the loaded module files
    procEntry32.dwSize = sizeof(PROCESSENTRY32);
    if (::Process32First (hProcessSnap, &procEntry32)) {
        while (::Process32Next (hProcessSnap, &procEntry32)) {
            hModuleSnap = ::CreateToolhelp32Snapshot (TH32CS_SNAPMODULE, 
                procEntry32.th32ProcessID);
            if (hModuleSnap != INVALID_HANDLE_VALUE) {
                moduleEntry32.dwSize = sizeof(MODULEENTRY32);
                if (::Module32First (hModuleSnap, &moduleEntry32)) {
                    do {
                        // Make sure the filepath is not in array yet
                        bAlreadyExists = FALSE;
                        for (i = 0; i < aptrScanMem.GetSize (); ++i) {
                            pScanMemory = aptrScanMem.GetAt (i);
                            if (::lstrcmpi (pScanMemory->szFilePathName, moduleEntry32.szExePath) == 0) {
                                bAlreadyExists = TRUE;
                                break;
                            }
                        }
                        if (!bAlreadyExists) {
                            pScanMemory = new SCANMEMORY;
                            pScanMemory->dwType = SMT_PROCESS;
                            ::lstrcpy (pScanMemory->szFilePathName, moduleEntry32.szExePath);
                            pScanMemory->dwProcessId = procEntry32.th32ProcessID;
                            aptrScanMem.Add (pScanMemory);
                        }
                    } while (::Module32Next (hModuleSnap, &moduleEntry32));
                }
                ::CloseHandle (hModuleSnap);
            }
        }
    }

    ::CloseHandle (hProcessSnap);

    // Get all active and inactive service path
    SC_HANDLE hSCManager = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hSCManager == NULL) {
        //MessageBox (_T("Cannot scan memory."), DEFCAPTION, MB_ICONERROR);
        //return;
        goto LShowScanDlg;
    }

	DWORD dwBytesNeeded, dwServicesNum, dwResumeHandle = 0;
    BOOL ret = ::EnumServicesStatus (hSCManager, SERVICE_WIN32, SERVICE_ACTIVE | SERVICE_INACTIVE, 
		NULL, 0, &dwBytesNeeded, &dwServicesNum, &dwResumeHandle);

    if (!ret && GetLastError () != ERROR_MORE_DATA) {
        //MessageBox (_T("Cannot scan memory."), DEFCAPTION, MB_ICONERROR);
        //return;
        ::CloseServiceHandle (hSCManager);
        goto LShowScanDlg;
    }

    LPENUM_SERVICE_STATUS pest = new ENUM_SERVICE_STATUS[dwBytesNeeded];

	dwResumeHandle = 0;
    ret = ::EnumServicesStatus (hSCManager, SERVICE_WIN32, SERVICE_ACTIVE | SERVICE_INACTIVE, 
		pest, dwBytesNeeded, &dwBytesNeeded, &dwServicesNum, &dwResumeHandle);

    if (!ret) {
        delete pest;
        ::CloseServiceHandle (hSCManager);
        //MessageBox (_T("Cannot scan memory."), DEFCAPTION, MB_ICONERROR);
        //return;
        goto LShowScanDlg;
    }

	LPQUERY_SERVICE_CONFIG pqsc = new QUERY_SERVICE_CONFIG[4096];
    // Iterate each service name and get its binary path
	for (i = 0; (unsigned)i < dwServicesNum; ++i) {
        SC_HANDLE hService = ::OpenService (hSCManager, pest[i].lpServiceName, SERVICE_ALL_ACCESS);
		if (hService) {
            ret = ::QueryServiceConfig (hService, pqsc, 4096, &dwBytesNeeded);
			if (ret) {
                // Check if lpBinaryPathName contains argument or not by using PathFileExists.
                // If there are any args, the PathFileExists will return FALSE and the args
                //  should be removed.
                if (!::PathFileExists (pqsc->lpBinaryPathName))
                    ::PathRemoveArgs (pqsc->lpBinaryPathName);
                ::PathUnquoteSpaces (pqsc->lpBinaryPathName);

                // Make sure the filepath is not in array yet
                bAlreadyExists = FALSE;
                for (j = 0; j < aptrScanMem.GetSize (); ++j) {
                    pScanMemory = aptrScanMem.GetAt (j);
                    if (::lstrcmpi (pScanMemory->szFilePathName, pqsc->lpBinaryPathName) == 0) {
                        bAlreadyExists = TRUE;
                        break;
                    }
                }
                if (!bAlreadyExists) {
                    pScanMemory = new SCANMEMORY;
                    pScanMemory->dwType = SMT_SERVICE;
                    ::lstrcpy (pScanMemory->szFilePathName, pqsc->lpBinaryPathName);
                    ::lstrcpy (pScanMemory->szServiceName, pest[i].lpServiceName);
                    aptrScanMem.Add (pScanMemory);
                }
			}
            ::CloseServiceHandle (hService);
		}
	}

    delete pqsc;
    delete pest;
    ::CloseServiceHandle (hSCManager);

LShowScanDlg:

    HKEY hKey;
    ret = ::RegOpenKeyEx (HKEY_LOCAL_MACHINE, REGKEY_HKLM_RUN, 0, KEY_READ, &hKey);
    if (ret == ERROR_SUCCESS) {
        TCHAR szValName[MAX_PATH], szData[MAX_PATH];
        DWORD dwValNameSize = MAX_PATH, dwDataSize = MAX_PATH, dwIndex = 0;
        while ((ret = ::RegEnumValue (hKey, dwIndex++, szValName, &dwValNameSize, NULL, NULL, 
            (LPBYTE) szData, &dwDataSize)) == ERROR_SUCCESS) {

            if (!::PathFileExists (szData))
                ::PathRemoveArgs (szData);
            ::PathUnquoteSpaces (szData);

            if (::PathIsFileSpec (szData)) {
                //TODO
            }
            // Make sure the filepath is not in array yet
            bAlreadyExists = FALSE;
            for (i = 0; i < aptrScanMem.GetSize (); ++i) {
                pScanMemory = aptrScanMem.GetAt (i);
                if (::lstrcmpi (pScanMemory->szFilePathName, szData) == 0) {
                    bAlreadyExists = TRUE;
                    break;
                }
            }
            if (!bAlreadyExists) {
                pScanMemory = new SCANMEMORY;
                pScanMemory->dwType = SMT_FILE;
                ::lstrcpy (pScanMemory->szFilePathName, szData);
                ::lstrcpy (pScanMemory->szValName, szValName);
                aptrScanMem.Add (pScanMemory);
            }
            dwValNameSize = dwDataSize = MAX_PATH;
        }
        ::RegCloseKey (hKey);
    }

    if (aptrScanMem.GetSize () > 0) {
        CScanMemDlg scanMemDlg(&m_fvdBlock, &m_ScanOpt, &aptrScanMem);
        scanMemDlg.DoModal();
    }

    for (i = 0; i < aptrScanMem.GetSize (); ++i)
        delete aptrScanMem.GetAt (i);
    aptrScanMem.RemoveAll ();
}
*/

void CFlatAVMainDlg::OnCmdScanMemory()
{
    //CEnumMemoryObjectArray memObjArray;

    //memObjArray.EnumMemoryObjects ();


//LShowScanDlg:
    //DBG_WRITELOG(_T("scanmem.log"), _T("Count = %d\n\n"), aptrScanMem.GetSize());
    //for (i = 0; i < aptrScanMem.GetSize (); ++i) {
    //    pScanMemory = aptrScanMem.GetAt (i);
    //    DBG_WRITELOG (_T("scanmem.log"), 
    //        _T("Process type = %s\n")
    //        _T("Process id = %d\n")
    //        _T("File path = %s\n")
    //        _T("Service name = %s\n\n"),
    //        pScanMemory->dwType == SMT_PROCESS ? _T("PROCESS") : _T("SERVICE"),
    //        pScanMemory->dwProcessId,
    //        pScanMemory->szFilePathName,
    //        pScanMemory->szServiceName
    //    );
    //}
    
    //if (memObjArray.GetSize () > 0) {
        CScanMemDlg scanMemDlg(&m_fvdBlock, &m_ScanOpt/*, &memObjArray*/);
        scanMemDlg.DoModal();
    //}

    //for (i = 0; i < aptrScanMem.GetSize (); ++i)
    //    delete aptrScanMem.GetAt (i);
    //aptrScanMem.RemoveAll ();
}
void CFlatAVMainDlg::OnAboutbox()
{
    CAboutDlg dlgAbout;
    dlgAbout.DoModal();
}

BOOL CFlatAVMainDlg::PreCreateWindow(CREATESTRUCT& cs)
{
	//cs.lpszClass = IDD_CLASS;
	return CDialog::PreCreateWindow(cs);
}


BOOL CFlatAVMainDlg::PreTranslateMessage(MSG* pMsg)
{
	//if (IsDialogMessage(pMsg)) {
	//	return TRUE;
	//}

	return CDialog::PreTranslateMessage(pMsg);
}
