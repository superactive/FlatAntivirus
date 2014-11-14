/****************************************************************************\
*
*	Scan Dialog
*
*   COPYRIGHT:  2008 Suyandi Wu
*
\****************************************************************************/

#pragma once
#include "afxcmn.h"

//#include "..\libflatav\flatav.h"

// Used by CScanDlg::ScanFile function
//typedef struct fileinarchiveinfo_t
//{
//    int nArchiveType;
//    TCHAR* pszCombinedFileName; // Combination of archive filename and filename in zip.
//                                // i.e., archive filename = C:\archive.zip, and 
//                                // filename in zip = file.txt, would become
//                                // C:\archive.zip\file.txt.
//    int nArchiveFileNameLen;    // Length of archive filename (without trailing filename in zip)
//                                // i.e., C:\archive.zip\file.txt, the archive filename length
//                                // would be 14.
//                                // It needed for SplitCombinedArcFileName's nArcFileNameLen parameter
//} FileInArchiveInfo;

// CScanDlg dialog

class CScanDlg : public CDialog
{
	DECLARE_DYNAMIC(CScanDlg)

public:
	CScanDlg(LPFVD_BLOCK pFvdBlock, LPSCANOPT pScanOpt, CStringArray* pastrScanPathList,
        CWnd* pParent = NULL);

	//CScanDlg(LPFVD_BLOCK pFvdBlock, LPSCANOPT pScanOpt, CScanMemoryPtrArray* paptrScanParam,
 //       CWnd* pParent = NULL);

    // Dialog Data
	enum { IDD = IDD_SCANDLG };

    int ScanSelectedFiles ();   // Public access, so the thread can access it
protected:
    // Input Params
    CStringArray* m_pastrScanPathList;
    LPFVD_BLOCK m_pFvdBlock;
    LPSCANOPT m_pScanOpt;

    BOOL m_bStopScan;
    CWinThread* m_pThread;
    UINT m_uNumScanned;
    UINT m_uNumInfected;

	CToolTipCtrl m_toolTipCtrl;
protected:
    int FindAndScan (CString strFile);
    int ScanFile (LPCTSTR szScanFPN);//, LPCTSTR szArchiveFPN, LPCTSTR szFileNameInArchive);//FileInArchiveInfo* pfia_info);// _TCHAR* szDispName, int nFileNameLen);
    void UpdateScanUI (LPCTSTR szFilePath);//, BOOL bInfected);
    void InsertLvScanResultItem (LPCTSTR szFilePath, /*int nScanRet,*/ LPARAM lParam);
    void FreeInsertedMWObj();
    //int DeleteMalware (LPCTSTR szFilePathName, BOOL bDelPermanent, BOOL bDelLockedFiles,
    //    LPCTSTR szFileNameInArchive);

    // General archive support functions
    //int GetArchiveType (LPCTSTR szFilePathName);
    //int ScanArchiveFile (LPCTSTR szFilePathName, int nArcType);

    //int DeleteMalwareInArchive (LPCTSTR szArchiveFPN, BOOL bDelPermanent, 
    //    BOOL bDelLockedFiles, LPCTSTR szFileNameInArchive);
    //int MoveRenameMalwareInArchive (LPCTSTR szFilePathName, LPCTSTR szDestFolderPath, 
    //    BOOL bRenFile, BOOL bMoveFile, BOOL bMoveRenLockedFiles,
    //    int nArcType, int nArcNameLen);

    //void BuildCombinedArcFileName (TCHAR* szCombinedName, LPCTSTR szArchiveFPN, 
    //    char* szFileNameInZip);
    //void SplitCombinedArcFileName (LPCTSTR szCombinedName, int nArcFileNameLen, 
    //    TCHAR* szArchiveFPN, TCHAR* szFileNameInArchive);

    // Zip archive support functions
    //int ScanZipFile (LPCTSTR szFilePathName);
    //int DeleteFileInZipArchive (LPCTSTR szArchiveFilePathName, const char* szDelFileNameInZip,
    //    BOOL bDelPermanent);
    //int ExtractFileInZipArchive (LPCTSTR szArchiveFilePathName, const char* szRenFileNameInZip,
    //                         LPCTSTR szDestFilePathName);

    // Move/Delete functions
    //int DeleteMalware (LPCTSTR szFilePathName, BOOL bDelPermanent, BOOL bDelLockedFiles, 
    //    LPARAM lParam);
    //int MoveRenameMalware (LPCTSTR szFilePathName, _TCHAR* szDestFolderPath, BOOL bRenFile, 
    //    BOOL bMoveFile, BOOL bMoveRenLockedFiles, LPARAM lParam);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
public:
    CListCtrl m_wndLvScanResult;
    virtual BOOL OnInitDialog();
    afx_msg void OnBtnStopCloseClicked();
    afx_msg void OnChkSelAllClicked();
    afx_msg void OnLstScanResultNMClick(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnLstScanResultLvnColumnClick(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnBtnDelFilesClicked();
    afx_msg void OnLstScanResultNMDblClick(NMHDR *pNMHDR, LRESULT *pResult);
//    afx_msg void OnDestroy();
    afx_msg void OnBtnMoveRenFilesClicked();
    afx_msg void OnDestroy();
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
