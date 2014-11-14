/****************************************************************************\
*
*	Move/Rename Dialog
*
*   COPYRIGHT:  2008 Suyandi Wu
*
\****************************************************************************/

// MoveRenDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FlatAVGUI.h"
#include "MoveRenDlg.h"
//#include ".\moverendlg.h"

// CMoveRenDlg dialog

IMPLEMENT_DYNAMIC(CMoveRenDlg, CDialog)
CMoveRenDlg::CMoveRenDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMoveRenDlg::IDD, pParent)
{
    m_bRenFile = TRUE;
    m_bMoveFile = TRUE;
    m_bMoveRenLocked = TRUE;
    m_szFilePathName[0] = '\0';
}

void CMoveRenDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CMoveRenDlg, CDialog)
    ON_WM_DESTROY()
    ON_CONTROL_RANGE (BN_CLICKED, IDC_CHKRENFILE, IDC_CHKMOVERENLOCKED, OnOptionsClicked)
    ON_BN_CLICKED(IDC_BTNBROWSE, OnBtnBrowseClicked)
    ON_BN_CLICKED(IDC_BTNMOVEREN, OnBtnMoveRenClicked)
    ON_BN_CLICKED(IDC_BTNMOVERENALL, OnBtnMoveRenAllClicked)
END_MESSAGE_MAP()


// CMoveRenDlg message handlers

BOOL CMoveRenDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    SetDlgItemText (IDC_LBLFILENAME, m_szFilePathName);

    BYTE data[4], defData[4] = {1, 1, 1, 0};
    DWORD dwDataSize = sizeof(data);

    ::SHRegGetUSValue (REGKEY_FLATAV, _T("MoveRenDlg"),
        NULL, data, &dwDataSize, FALSE, defData, sizeof(defData));

    m_bRenFile = data[0];
    m_bMoveFile = data[1];
    m_bMoveRenLocked = data[2];

    if (m_bRenFile) ((CButton*)GetDlgItem (IDC_CHKRENFILE))->SetCheck (1);
    if (m_bMoveFile) ((CButton*)GetDlgItem (IDC_CHKMOVEFILE))->SetCheck (1);
    if (m_bMoveRenLocked) ((CButton*)GetDlgItem (IDC_CHKMOVERENLOCKED))->SetCheck (1);

    //if (m_bIsArchive) GetDlgItem (IDC_CHKRENFILE)->EnableWindow (FALSE);

    dwDataSize = MAX_PATH;
    TCHAR szDefData[MAX_PATH];
    ::GetCurrentDirectory (MAX_PATH, szDefData);
    ::lstrcat (szDefData, _T("\\moved"));

    ::SHRegGetUSValue (REGKEY_FLATAV, _T("FlatAVMovedFolderPath"),
        NULL, m_szDestFolderPath, &dwDataSize, TRUE, szDefData, MAX_PATH);

    SetDlgItemText (IDC_TXTDESTFOLDER, m_szDestFolderPath);
    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CMoveRenDlg::OnDestroy()
{
    CDialog::OnDestroy();

    BYTE data[4];
    data[0] = m_bRenFile;
    data[1] = m_bMoveFile;
    data[2] = m_bMoveRenLocked;

    ::SHRegSetUSValue (REGKEY_FLATAV, _T("MoveRenDlg"),
        REG_DWORD, data, sizeof(data), SHREGSET_FORCE_HKCU);

    //GetDlgItemText (IDC_TXTDESTFOLDER, m_szDestFolderPath, MAX_PATH);

    //::SHRegSetUSValue (REGKEY_FLATAV, _T("FlatAVMovedFolderPath"),
    //    REG_SZ, m_szDestFolderPath, MAX_PATH, SHREGSET_FORCE_HKLM);
}

void CMoveRenDlg::OnOptionsClicked(UINT nID)
{
    if (nID == IDC_CHKRENFILE) {
        m_bRenFile = ((CButton*)GetDlgItem (IDC_CHKRENFILE))->GetCheck ();
    }
    else if (nID == IDC_CHKMOVEFILE) {
        m_bMoveFile = ((CButton*)GetDlgItem (IDC_CHKMOVEFILE))->GetCheck ();
        GetDlgItem (IDC_TXTDESTFOLDER)->EnableWindow (m_bMoveFile);
    }
    else
        m_bMoveRenLocked = ((CButton*)GetDlgItem (IDC_CHKMOVERENLOCKED))->GetCheck ();

    if (m_bRenFile || m_bMoveFile) {
        ((CButton*)GetDlgItem (IDC_CHKMOVERENLOCKED))->EnableWindow (TRUE);
        ((CButton*)GetDlgItem (IDC_BTNMOVEREN))->EnableWindow (TRUE);
        //((CButton*)GetDlgItem (IDC_BTNMOVERENALL))->EnableWindow (TRUE);
    }
    else {
        ((CButton*)GetDlgItem (IDC_CHKMOVERENLOCKED))->EnableWindow (FALSE);
        ((CButton*)GetDlgItem (IDC_BTNMOVEREN))->EnableWindow (FALSE);
        //((CButton*)GetDlgItem (IDC_BTNMOVERENALL))->EnableWindow (FALSE);
    }
}

void CMoveRenDlg::OnBtnBrowseClicked()
{
    BROWSEINFO bi;
    ::ZeroMemory (&bi, sizeof(bi));
    bi.hwndOwner = m_hWnd;
    bi.lpszTitle = _T("Select folder");

    LPITEMIDLIST pIdl = ::SHBrowseForFolder (&bi);
    if (pIdl) {
        if (::SHGetPathFromIDList (pIdl, m_szDestFolderPath))
            SetDlgItemText (IDC_TXTDESTFOLDER, m_szDestFolderPath);
        theApp.SHMallocFree (pIdl);
    }
}

void CMoveRenDlg::OnBtnMoveRenClicked()
{
    EndDialog (MOVEREN_FILE);
}

void CMoveRenDlg::OnBtnMoveRenAllClicked()
{
    EndDialog (MOVEREN_ALL);
}
