/****************************************************************************\
*
*	Scan Memory Dialog
*
*   COPYRIGHT:  2008 Suyandi Wu
*
\****************************************************************************/

#pragma once
#include "afxcmn.h"

#include "..\libflatav\flatav.h"
#include "EnumMWObjectArray.h"
//#include "EnumMemoryObjectArray.h"
//#include "EnumStartupObjectArray.h"

// CScanMemDlg dialog

class CScanMemDlg : public CDialog
{
	DECLARE_DYNAMIC(CScanMemDlg)

public:
	CScanMemDlg(LPFVD_BLOCK pFvdBlock, LPSCANOPT pScanOpt, /*CEnumMemoryObjectArray* pMemObjArray,*/
        CWnd* pParent = NULL);

    // Dialog Data
	enum { IDD = IDD_SCANDLG };

    int ScanMemory ();
protected:
    //CEnumMemoryObjectArray m_enumMemObjArray;
    //CEnumStartupObjectArray m_enumStObjArray;
    CEnumMWObjectArray m_enumMWObjArray;
    LPFVD_BLOCK m_pFvdBlock;
    LPSCANOPT m_pScanOpt;

    BOOL m_bStopScan;
    CWinThread* m_pThread;
    UINT m_uNumScanned;
    UINT m_uNumInfected;

	CToolTipCtrl m_toolTipCtrl;

protected:
    int ScanMemoryObject (LPMWOBJ pMemoryObj);
    void UpdateScanUI (LPCTSTR szFilePath);//, BOOL bInfected);
    void InsertLvScanResultItem (LPCTSTR szFilePath, LPARAM lParam);
    //BOOL KillProcess (LPMWOBJ pMemoryObj);
    //BOOL StopAndDeleteService (LPMWOBJ pMemoryObj);
    //BOOL SuspendResumeProcess (DWORD dwProcessId, BOOL bResume = FALSE);

    //int DeleteMalware (LPCTSTR szFilePathName, BOOL bDelPermanent, BOOL bDelLockedFiles);
    //int MoveRenameMalware (LPCTSTR szFilePathName, TCHAR* szDestFolderPath, BOOL bRenFile, 
    //    BOOL bMoveFile, BOOL bMoveRenLockedFiles);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
public:
    //void InsertLvScanResultItem (CMemoryObjPtrArray* pMemObjPtrArray);
    CListCtrl m_wndLvScanResult;
    virtual BOOL OnInitDialog();
    afx_msg void OnBtnStopCloseClicked();
    afx_msg void OnChkSelAllClicked();
    afx_msg void OnLstScanResultNMClick(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnLstScanResultNMDblClick(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnLstScanResultLvnColumnClick(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnBtnDelFilesClicked();
    afx_msg void OnBtnMoveRenFilesClicked();
    afx_msg void OnDestroy();
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
