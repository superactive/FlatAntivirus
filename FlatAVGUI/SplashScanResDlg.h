/****************************************************************************\
*
*	Splash Scan Result Dialog
*
*   COPYRIGHT:  2008 Suyandi Wu
*
\****************************************************************************/

#pragma once


// CSplashScanResDlg dialog

class CSplashScanResDlg : public CDialog
{
	DECLARE_DYNAMIC(CSplashScanResDlg)

public:
	CSplashScanResDlg(CMWObjPtrArray* pMWObjPtrArray, UINT uNumScanned, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	enum { IDD = IDD_SCANDLG };

protected:
    CMWObjPtrArray* m_pMWObjPtrArray;
    UINT m_uNumScanned;
    void InsertLvScanResultItem (LPCTSTR szFilePath, LPCSTR szMalwareName, int nMalwareType,
        LPARAM lParam);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    CListCtrl m_wndLvScanResult;
    virtual BOOL OnInitDialog();
    void OnBtnStopCloseClicked();
    afx_msg void OnChkSelAllClicked();
    afx_msg void OnLstScanResultNMClick(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnLstScanResultNMDblClick(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnLstScanResultLvnColumnClick(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnBtnDelFilesClicked();
    afx_msg void OnBtnMoveRenFilesClicked();
};
