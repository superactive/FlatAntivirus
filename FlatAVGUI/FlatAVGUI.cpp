/****************************************************************************\
*
*	FlatAV GUI Main Apps
*
*   COPYRIGHT:  2008 Suyandi Wu
*
\****************************************************************************/

// FlatAVGUI.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "FlatAVGUI.h"
#include "FlatAVMainDlg.h"

//#include "SplashWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFlatAVGUIApp

BEGIN_MESSAGE_MAP(CFlatAVGUIApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


// CFlatAVGUIApp construction

CFlatAVGUIApp::CFlatAVGUIApp()
{
	// TODO: add construction code here,
    ::SHGetMalloc (&m_pMalloc);
	// Place all significant initialization in InitInstance
}


// The one and only CFlatAVGUIApp object

CFlatAVGUIApp theApp;


// CFlatAVGUIApp initialization

BOOL CFlatAVGUIApp::InitInstance()
{
	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	// Enable the splash screen component based on the command line info.
	//CCommandLineInfo cmdInfo;
	//ParseCommandLine(cmdInfo);
	//CSplashWnd::EnableSplashScreen(cmdInfo.m_bShowSplash);

	CWinApp::InitInstance();
    AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	//SetRegistryKey(_T("Local AppWizard-Generated Applications"));

    // Enable debug privilege, so the program can terminate service process
    HANDLE hProcess = ::GetCurrentProcess ();
    if (hProcess != NULL) {
        HANDLE hToken;
        if (::OpenProcessToken (hProcess, TOKEN_ADJUST_PRIVILEGES, &hToken)) {
            m_bCanDebug = ::SetPrivilege (hToken, SE_DEBUG_NAME, TRUE);
            ::CloseHandle (hToken);
        }
        ::CloseHandle (hProcess);
    }

	CFlatAVMainDlg dlg;
	m_pMainWnd = &dlg;

	//LPTSTR szAppName = MAKEINTRESOURCE(dlg.IDD);
	//MSG          msg ;
	//WNDCLASS     wndclass ;

	//memset(&wndclass, 0, sizeof(WNDCLASS)); 

	//wndclass.style         = CS_HREDRAW | CS_VREDRAW;
	//wndclass.lpfnWndProc   = ::DefWindowProc;
	//wndclass.cbWndExtra    = DLGWINDOWEXTRA;
	//wndclass.hInstance     = AfxGetInstanceHandle();
	//wndclass.hIcon         = LoadIcon(IDR_MAINFRAME);
	//wndclass.hCursor       = LoadCursor(IDC_ARROW);
	//wndclass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	//wndclass.lpszClassName = dlg.IDD_CLASS;

	//if (!AfxRegisterClass(&wndclass)) {
	//	TRACE("Class Registration Failed\n");
	//	MessageBox (NULL, TEXT ("This program requires Windows NT!"), szAppName, MB_ICONERROR);
	//	return 0 ;
	//}

	
	//dlg.Create(dlg.IDD);
	//dlg.ShowWindow(SW_SHOW);

	int nResponse;
	if (dlg.Initialize() == TRUE) {
		nResponse = dlg.DoModal();
	}
	else {
		AfxMessageBox(_T("Error starting application"), MB_ICONERROR);
	}
	//BOOL b = TRUE;
	//while (GetMessage(&msg, NULL, 0, 0)) {
	//	if (!IsDialogMessage(dlg.m_hWnd, &msg)) {
	//		TranslateMessage(&msg) ;
	//		DispatchMessage(&msg) ;
	//	}
	//}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
