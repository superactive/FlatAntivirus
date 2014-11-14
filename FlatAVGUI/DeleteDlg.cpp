/****************************************************************************\
*
*	Delete File Dialog
*
*   COPYRIGHT:  2008 Suyandi Wu
*
\****************************************************************************/

// DeleteDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FlatAVGUI.h"
#include "DeleteDlg.h"
//#include ".\deletedlg.h"


// CDeleteDlg dialog

IMPLEMENT_DYNAMIC(CDeleteDlg, CDialog)
CDeleteDlg::CDeleteDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDeleteDlg::IDD, pParent)
{
    m_szFilePathName[0] = '\0';
}

void CDeleteDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDeleteDlg, CDialog)
    ON_BN_CLICKED(IDC_BTNDELETE, OnBtndeleteClicked)
    ON_BN_CLICKED(IDC_BTNDELETEALL, OnBtnDeleteAllClicked)
    ON_CONTROL_RANGE (BN_CLICKED, IDC_RBDELTOBIN, IDC_RBDELPERMANENT, OnOptionsClicked)
    ON_BN_CLICKED(IDC_CHKDELLOCKED, OnChkDelLockedClicked)
    ON_WM_DESTROY()
END_MESSAGE_MAP()


// CDeleteDlg message handlers

BOOL CDeleteDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    SetDlgItemText (IDC_LBLFILENAME, m_szFilePathName);

    // Read setting from registry
    //HUSKEY hUserKey;
    //int ret = ::SHRegCreateUSKey (_T("HKEY_CURRENT_USER\\Software\\SuyandiWu\\FlatAV"), KEY_ALL_ACCESS,
    //    NULL, &hUserKey, SHREGSET_FORCE_HKCU);
    //ret = ::SHRegCloseUSKey (hUserKey);

    DWORD dwData, dwDefData = MAKELONG(1, 1);
    DWORD dwDataSize = sizeof(dwData);

    //::SHRegQueryUSValue (m_hUserKey, _T("DeleteDlg"), &dwValType, &dwData, &dwDataSize,
    //    FALSE, NULL, 0);

    ::SHRegGetUSValue (REGKEY_FLATAV, _T("DeleteDlg"),
        NULL, &dwData, &dwDataSize, FALSE, &dwDefData, sizeof(dwDefData));

    m_nDelOpt = LOWORD(dwData);
    m_bDelLockedFiles = HIWORD(dwData);

    if (m_nDelOpt) {
        ((CButton*)GetDlgItem (IDC_RBDELPERMANENT))->SetCheck (1);
        ((CButton*)GetDlgItem (IDC_CHKDELLOCKED))->EnableWindow (1);
    }
    else {
        ((CButton*)GetDlgItem (IDC_RBDELTOBIN))->SetCheck (1);
        ((CButton*)GetDlgItem (IDC_CHKDELLOCKED))->EnableWindow (0);
    }

    if (m_bDelLockedFiles)
        ((CButton*)GetDlgItem (IDC_CHKDELLOCKED))->SetCheck (1);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CDeleteDlg::OnOptionsClicked(UINT nID)
{
    if (nID == IDC_RBDELTOBIN) {
        m_nDelOpt = DELTOBIN;
        m_bDelLockedFiles = FALSE;
        ((CButton*)GetDlgItem (IDC_CHKDELLOCKED))->EnableWindow (0);
    }
    else {
        m_nDelOpt = DELPERMANENT;
        m_bDelLockedFiles = ((CButton*)GetDlgItem (IDC_CHKDELLOCKED))->GetCheck ();
        ((CButton*)GetDlgItem (IDC_CHKDELLOCKED))->EnableWindow (1);
    }
}

void CDeleteDlg::OnBtndeleteClicked()
{
    EndDialog (DELETE_FILE);
}

void CDeleteDlg::OnBtnDeleteAllClicked()
{
    EndDialog (DELETE_ALL);
}

void CDeleteDlg::OnChkDelLockedClicked()
{
    m_bDelLockedFiles = ((CButton*)GetDlgItem (IDC_CHKDELLOCKED))->GetCheck ();
}

void CDeleteDlg::OnDestroy()
{
    CDialog::OnDestroy();

    DWORD dwData;;

    //::SHRegQueryUSValue (m_hUserKey, _T("DeleteDlg"), &dwValType, &dwData, &dwDataSize,
    //    FALSE, NULL, 0);

    dwData = MAKELONG(m_nDelOpt, ((CButton*)GetDlgItem (IDC_CHKDELLOCKED))->GetCheck ());

    ::SHRegSetUSValue (REGKEY_FLATAV, _T("DeleteDlg"),
        REG_DWORD, &dwData, sizeof(dwData), SHREGSET_FORCE_HKCU);
}
