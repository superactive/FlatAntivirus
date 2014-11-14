/****************************************************************************\
*
*	Move/Rename Dialog
*
*   COPYRIGHT:  2008 Suyandi Wu
*
\****************************************************************************/

#pragma once


// CMoveRenDlg dialog

class CMoveRenDlg : public CDialog
{
	DECLARE_DYNAMIC(CMoveRenDlg)

public:
	CMoveRenDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	enum { IDD = IDD_MOVERENDLG };
    enum e_retvalue { MOVEREN_FILE, MOVEREN_ALL };    // Return values

    BOOL Is_RenFile () { return m_bRenFile; }
    BOOL Is_MoveFile () { return m_bMoveFile; }
    BOOL Is_MoveRenLocked () { return m_bMoveRenLocked; }
    void Set_FilePathName (LPCTSTR szFilePathName) //, BOOL bIsArchive)
    {
        ::lstrcpy (m_szFilePathName, szFilePathName);
        //m_bIsArchive = bIsArchive;
    }

    LPTSTR Get_FilePathName () { return m_szFilePathName; }
    LPTSTR Get_DestFolderPath () { return m_szDestFolderPath; }
protected:
    //BOOL m_bIsArchive;
    BOOL m_bRenFile;
    BOOL m_bMoveFile;
    BOOL m_bMoveRenLocked;
    TCHAR m_szFilePathName[MAX_PATH];
    TCHAR m_szDestFolderPath[MAX_PATH];
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
    afx_msg void OnDestroy();
    void OnOptionsClicked(UINT nID);
    afx_msg void OnBtnBrowseClicked();
    afx_msg void OnBtnMoveRenClicked();
    afx_msg void OnBtnMoveRenAllClicked();
};
