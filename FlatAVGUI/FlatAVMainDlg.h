/****************************************************************************\
*
*	FlatAV GUI Main Dialog
*
*   COPYRIGHT:  2008 Suyandi Wu
*
\****************************************************************************/

// FlatAVMainDlg.h : header file
//

#pragma once
#include "shellfoldertree.h"
#include "..\libflatav\flatav.h"
#include "shellfoldertree.h"

// CFlatAVMainDlg dialog
class CFlatAVMainDlg : public CDialog
{
// Construction
public:
	CFlatAVMainDlg(CWnd* pParent = NULL);	// standard constructor

	static LPCTSTR IDD_CLASS;

    //~CFlatAVMainDlg()
    //{
    //    //if (m_pMalloc) m_pMalloc->Release();
    //    //if (m_pShellFolder) m_pShellFolder->Release();
    //}

// Dialog Data
	enum { IDD = IDD_MAINDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
    CShellfoldertree m_wndSfTree;
    // Fields
    FVD_BLOCK m_fvdBlock;
    SCANOPT m_ScanOpt;

    // Methods
    BOOL IterateCheckedItem (HTREEITEM hItemRoot, CStringArray& strArray);
    void GetDriveStringList (CStringArray& strArray);
    void AddCheckedItem (BSTR bstrPath, CStringArray& strArray);
    BOOL ReadRegScanOptions ();
    BOOL WriteRegScanOptions ();

	DECLARE_MESSAGE_MAP()
public:
	BOOL Initialize();

    DECLARE_EVENTSINK_MAP()
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnBtnScanClicked();
    afx_msg void OnDestroy();
    afx_msg void OnChkTestArchivesClicked();
    afx_msg void OnChkScanSubFoldersClicked();
    afx_msg void OnCmdScanMemory();
    afx_msg void OnAboutbox();
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
