/****************************************************************************\
*
*	Delete File Dialog
*
*   COPYRIGHT:  2008 Suyandi Wu
*
\****************************************************************************/

#pragma once

// CDeleteDlg dialog

class CDeleteDlg : public CDialog
{
	DECLARE_DYNAMIC(CDeleteDlg)

public:
	CDeleteDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	enum { IDD = IDD_DELETEDLG };

    // Constants
    enum e_delopt { DELTOBIN, DELPERMANENT };    // Options
    enum e_retvalue { DELETE_FILE, DELETE_ALL };    // Return values

    int Get_DelOpt () { return m_nDelOpt; }
    BOOL Is_DelLockedFiles () { return m_bDelLockedFiles; }
    void Set_FilePathName (LPCTSTR szFilePathName)
    {
        ::lstrcpy (m_szFilePathName, szFilePathName);
    }

    LPTSTR Get_FilePathName () { return m_szFilePathName; }

protected:
    int m_nDelOpt;
    BOOL m_bDelLockedFiles;
    TCHAR m_szFilePathName[MAX_PATH];

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
    afx_msg void OnBtndeleteClicked();
    afx_msg void OnBtnDeleteAllClicked();
    void OnOptionsClicked(UINT nID);
    afx_msg void OnChkDelLockedClicked();
    afx_msg void OnDestroy();
};

