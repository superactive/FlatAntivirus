/****************************************************************************\
*
*	Splash Scan Result Dialog
*
*   COPYRIGHT:  2008 Suyandi Wu
*
\****************************************************************************/

// SplashScanResDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FlatAVGUI.h"
#include "SplashScanResDlg.h"
//#include ".\splashscanresdlg.h"
#include "DeleteDlg.h"
#include "MoveRenDlg.h"
#include "MemoryOp.h"
#include "FileOp.h"

// CSplashScanResDlg dialog

IMPLEMENT_DYNAMIC(CSplashScanResDlg, CDialog)
CSplashScanResDlg::CSplashScanResDlg(CMWObjPtrArray* pMWObjPtrArray, UINT uNumScanned, CWnd* pParent /*=NULL*/)
	: CDialog(CSplashScanResDlg::IDD, pParent)
{
    m_pMWObjPtrArray = pMWObjPtrArray;
    m_uNumScanned = uNumScanned;
}

void CSplashScanResDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LSTSCANRESULT, m_wndLvScanResult);
}


BEGIN_MESSAGE_MAP(CSplashScanResDlg, CDialog)
    ON_BN_CLICKED(IDC_BTNSTOPCLOSE, OnBtnStopCloseClicked)
    ON_BN_CLICKED(IDC_CHKSELALL, OnChkSelAllClicked)
    ON_NOTIFY(NM_CLICK, IDC_LSTSCANRESULT, OnLstScanResultNMClick)
    ON_NOTIFY(NM_DBLCLK, IDC_LSTSCANRESULT, OnLstScanResultNMDblClick)
    ON_NOTIFY(LVN_COLUMNCLICK, IDC_LSTSCANRESULT, OnLstScanResultLvnColumnClick)
    ON_BN_CLICKED(IDC_BTNDELFILES, OnBtnDelFilesClicked)
    ON_BN_CLICKED(IDC_BTNMOVERENFILES, OnBtnMoveRenFilesClicked)
END_MESSAGE_MAP()


// CSplashScanResDlg message handlers

void CSplashScanResDlg::OnBtnStopCloseClicked()
{
    //dbg_writelog(_T("C:\\a.log"), _T("01\n"));
    //EndDialog (0);
    OnCancel();
    //dbg_writelog(_T("C:\\a.log"), _T("02\n"));
}

BOOL CSplashScanResDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_wndLvScanResult.SetExtendedStyle ( LVS_EX_CHECKBOXES | LVS_EX_FLATSB | LVS_EX_FULLROWSELECT |
        LVS_EX_GRIDLINES );
    int i = 0;
    m_wndLvScanResult.InsertColumn (i++, _T("File Name"), LVCFMT_LEFT, 250);
    m_wndLvScanResult.InsertColumn (i++, _T("Virus Name"), LVCFMT_LEFT, 160);
    m_wndLvScanResult.InsertColumn (i, _T("Information"), LVCFMT_LEFT, 220);

    SetDlgItemInt (IDC_LBLSCANNUM, m_uNumScanned, FALSE);
    SetDlgItemInt (IDC_LBLINFFILES, (UINT)m_pMWObjPtrArray->GetSize(), FALSE);

    for (i = 0; i < m_pMWObjPtrArray->GetSize(); ++i) {
        LPMWOBJ pMWObj = m_pMWObjPtrArray->GetAt(i);
        InsertLvScanResultItem (pMWObj->szFilePathName, pMWObj->szMalwareName, pMWObj->nMalwareType,
            (LPARAM)pMWObj);
    }

    ((CButton*)GetDlgItem (IDC_CHKSELALL))->SetCheck (1);
    OnChkSelAllClicked ();
    GetDlgItem (IDC_BTNMOVERENFILES)->EnableWindow (TRUE);
    GetDlgItem (IDC_BTNDELFILES)->EnableWindow (TRUE);
    GetDlgItem (IDC_BTNSTOPCLOSE)->SetWindowText (_T("Close"));

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CSplashScanResDlg::InsertLvScanResultItem (LPCTSTR szFilePath, LPCSTR szMalwareName, 
                                                int nMalwareType, LPARAM lParam)
{
    int n = m_wndLvScanResult.GetItemCount ();
    int nScanRet = ((LPMWOBJ)lParam)->nScanResult;

    m_wndLvScanResult.InsertItem (LVIF_TEXT | LVIF_PARAM, n, szFilePath, 0, 0, 0, lParam);
#ifdef _UNICODE
    WCHAR wszMalwareName[MAX_PATH];
    ::MultiByteToWideChar (CP_ACP, 0, szMalwareName, -1, wszMalwareName,
        MAX_PATH);
    m_wndLvScanResult.SetItemText (n, 1, wszMalwareName);
#else
    m_wndLvScanResult.SetItemText (n, 1, szMalwareName);
#endif

    if (nScanRet == FL_VIRUS) {
        TCHAR szInfo[24];
        switch (nMalwareType)
        {
        case 0:
            ::lstrcpy (szInfo, _T("Worm"));
            break;
        case 1:
            ::lstrcpy (szInfo, _T("Trojan"));
            break;
        case 2:
            ::lstrcpy (szInfo, _T("Virus"));
            break;
        case 3:
            ::lstrcpy (szInfo, _T("Script"));
            break;
        case 4:
            ::lstrcpy (szInfo, _T("Adware"));
            break;
        case 5:
            ::lstrcpy (szInfo, _T("Spyware"));
            break;
        case 6:
            ::lstrcpy (szInfo, _T("Joke/Hoax"));
            break;
        default:
            ::lstrcpy (szInfo, _T("Unknown"));
        }
        m_wndLvScanResult.SetItemText (n, 2, szInfo);
    }
    else if (nScanRet == FL_SUSPECT)
        m_wndLvScanResult.SetItemText (n, 2, _T("Please submit this file to FlatAV team."));
}


void CSplashScanResDlg::OnChkSelAllClicked()
{
    int i;
    BOOL fCheck = ((CButton*)GetDlgItem (IDC_CHKSELALL))->GetCheck ();
    int nCount = m_wndLvScanResult.GetItemCount ();
    for (i = 0; i < nCount; ++i)
        m_wndLvScanResult.SetCheck (i, fCheck);
}

void CSplashScanResDlg::OnLstScanResultNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
    POINT pt;
    UINT uFlags;
    int nItem, nCount;
    nCount = m_wndLvScanResult.GetItemCount ();
    if (nCount == 0)
        return;
    ::GetCursorPos (&pt);
    ::ScreenToClient (m_wndLvScanResult.m_hWnd, &pt);
    nItem = m_wndLvScanResult.HitTest (pt, &uFlags);
    if ((uFlags & LVHT_ONITEMSTATEICON) == LVHT_ONITEMSTATEICON && 
        (uFlags & LVHT_ONITEMLABEL) == 0) {
        BOOL fCheck = m_wndLvScanResult.GetCheck (nItem);
        if (fCheck)
            ((CButton*)GetDlgItem (IDC_CHKSELALL))->SetCheck (0);
        else {
            int i, n;
            for (i = 0, n = 0; i < nCount; ++i) {
                BOOL fCheck = m_wndLvScanResult.GetCheck (i);
                if (!fCheck)
                    ++n;
            }
            if (n == 1)
                ((CButton*)GetDlgItem (IDC_CHKSELALL))->SetCheck (1);
        }
    }
    *pResult = 0;
}

void CSplashScanResDlg::OnLstScanResultNMDblClick(NMHDR *pNMHDR, LRESULT *pResult)
{
    OnLstScanResultNMClick (pNMHDR, pResult);
    *pResult = 0;
}

static int CALLBACK CompareLvItemText(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    CListCtrl* pListCtrl = (CListCtrl*) *((LPARAM*)lParamSort);
    int iColIdx = (int) *((LPARAM*)(lParamSort+4));
    BOOL bAscSort = (BOOL) *((LPARAM*)(lParamSort+8));

    CString strItem1 = pListCtrl->GetItemText ((int)lParam1, iColIdx);
    CString strItem2 = pListCtrl->GetItemText ((int)lParam2, iColIdx);

    if (bAscSort)
        return ::lstrcmpi((LPCTSTR) strItem1, (LPCTSTR) strItem2);
    else
        return ::lstrcmpi((LPCTSTR) strItem2, (LPCTSTR) strItem1);
}

void CSplashScanResDlg::OnLstScanResultLvnColumnClick(NMHDR *pNMHDR, LRESULT *pResult)
{
    if (m_wndLvScanResult.GetItemCount () == 0)
        return;

    LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
    static BOOL bAscSort = TRUE;
    static int iLastColIdx = -1;

    if (iLastColIdx != pNMLV->iSubItem)
        bAscSort = TRUE;

    LPARAM dwParam[3];
    dwParam[0] = (LPARAM) &m_wndLvScanResult;
    dwParam[1] = (LPARAM) pNMLV->iSubItem;
    dwParam[2] = bAscSort;
    ListView_SortItemsEx (m_wndLvScanResult.m_hWnd, CompareLvItemText, (LPARAM) &dwParam);
    bAscSort = !bAscSort;
    iLastColIdx = pNMLV->iSubItem;

    *pResult = 0;
}

void CSplashScanResDlg::OnBtnDelFilesClicked()
{
    CDeleteDlg deleteDlg;
    int i;
    INT_PTR ret;
    int nItemCount = m_wndLvScanResult.GetItemCount ();

    TCHAR szMsg[MAX_PATH];
    int nSelItemCount = 0;
    for (i = 0; i < nItemCount; ++i) {
        if (m_wndLvScanResult.GetCheck (i))
            ++nSelItemCount;
    }
    if (nSelItemCount == 0) {
        MessageBox(TEXT("No item selected."), DEFCAPTION, MB_ICONWARNING);
        return;
    }
    ::wsprintf (szMsg, _T("%d objects will be deleted."), nSelItemCount);

    deleteDlg.Set_FilePathName (szMsg);
    ret = deleteDlg.DoModal ();
    if (ret == IDCANCEL)
        return;

    for (i = 0; i < nItemCount; ++i) {
        if (m_wndLvScanResult.GetCheck (i)) {
            CWaitCursor waitCursor;
            LPMWOBJ pMemoryObj = (LPMWOBJ) m_wndLvScanResult.GetItemData (i);
            if (pMemoryObj->dwType == MWT_PROCESS)
                KillProcess (pMemoryObj->dwProcessId);
            else
                StopAndDeleteService (pMemoryObj->szServiceName, pMemoryObj->dwProcessId > 0);

            // Delete the malware
            ret = DeleteFileEx (m_wndLvScanResult.GetItemText (i, 0), deleteDlg.Get_DelOpt(), 
                deleteDlg.Is_DelLockedFiles());
            if (ret == FL_SUCCESS) {
                // File was successfully deleted, so delete the item in list-view
                m_wndLvScanResult.DeleteItem (i--);
                --nItemCount;
                //m_wndLvScanResult.SetItemText (i, 2, _T("File was successfully deleted."));
            }
            else if (ret == FL_EWRITE) {
                if (deleteDlg.Is_DelLockedFiles ())
                    m_wndLvScanResult.SetItemText (i, 2, _T("File will be deleted at next restart."));
                else
                    m_wndLvScanResult.SetItemText (i, 2, _T("Cannot delete file, file is locked by another process."));
            }
            //else if (ret == FL_ENOFILE)
            //    m_wndLvScanResult.SetItemText (i, 2, _T("Cannot delete file, file doesn't exist."));
            else if (ret == FL_ENOTSUPT)
                m_wndLvScanResult.SetItemText (i, 2, _T("Cannot delete file, the operation is not supported."));
            else
                m_wndLvScanResult.SetItemText (i, 2, _T("Cannot delete file."));

        }   // if (m_wndLvScanResult.GetCheck (i))
    }   // for (int i = 0; i < nItemCount; ++i)

    if (m_wndLvScanResult.GetItemCount () == 0) {
        ((CButton*) GetDlgItem (IDC_CHKSELALL))->EnableWindow (FALSE);
        ((CButton*) GetDlgItem (IDC_BTNDELFILES))->EnableWindow (FALSE);
        ((CButton*) GetDlgItem (IDC_BTNMOVERENFILES))->EnableWindow (FALSE);
    }
}

void CSplashScanResDlg::OnBtnMoveRenFilesClicked()
{
    CMoveRenDlg moveRenDlg;
    int i;
    INT_PTR ret;
    int nItemCount = m_wndLvScanResult.GetItemCount ();

    TCHAR szMsg[MAX_PATH];
    int nSelItemCount = 0;
    for (i = 0; i < nItemCount; ++i) {
        if (m_wndLvScanResult.GetCheck (i))
            ++nSelItemCount;
    }
    if (nSelItemCount == 0) {
        MessageBox(TEXT("No item selected."), DEFCAPTION, MB_ICONWARNING);
        return;
    }
    ::wsprintf (szMsg, _T("%d objects will be moved/renamed."), nSelItemCount);

    moveRenDlg.Set_FilePathName (szMsg);
    ret = moveRenDlg.DoModal ();
    if (ret == IDCANCEL)
        return;

    for (i = 0; i < nItemCount; ++i) {
        if (m_wndLvScanResult.GetCheck (i)) {
            CWaitCursor waitCursor;
            LPMWOBJ pMemoryObj = (LPMWOBJ) m_wndLvScanResult.GetItemData (i);
            if (pMemoryObj->dwType == MWT_PROCESS)
                KillProcess (pMemoryObj->dwProcessId);
            else
                StopAndDeleteService (pMemoryObj->szServiceName, pMemoryObj->dwProcessId > 0);

            ret = MoveRenameFileEx (m_wndLvScanResult.GetItemText (i, 0), moveRenDlg.Get_DestFolderPath(),
                moveRenDlg.Is_RenFile(), moveRenDlg.Is_MoveFile(), moveRenDlg.Is_MoveRenLocked());

            if (ret == FL_SUCCESS) {
                // File was successfully moved/renamed, so delete the item in list-view
                m_wndLvScanResult.DeleteItem (i--);
                --nItemCount;
            }
            else if (ret == FL_EWRITE) {
                if (moveRenDlg.Is_MoveRenLocked ())
                    m_wndLvScanResult.SetItemText (i, 2, _T("File will be moved/renamed at next restart."));
                else
                    m_wndLvScanResult.SetItemText (i, 2, _T("Cannot move/rename file, file is locked by another process."));
            }
            //else if (ret == FL_ENOFILE)
            //    m_wndLvScanResult.SetItemText (i, 2, _T("Cannot move/rename file, file doesn't exist."));
            else if (ret == FL_EACCESS)
                m_wndLvScanResult.SetItemText (i, 2, _T("Cannot move/rename file, destination path not exist."));
            else if (ret == FL_ENOTSUPT)
                m_wndLvScanResult.SetItemText (i, 2, _T("Cannot move/rename file, the operation is not supported."));
            else
                m_wndLvScanResult.SetItemText (i, 2, _T("Cannot move/rename file."));
        }// END if (m_wndLvScanResult.GetCheck (i))
    }// for (int i = 0; i < nItemCount; ++i)

    if (m_wndLvScanResult.GetItemCount () == 0) {
        ((CButton*) GetDlgItem (IDC_CHKSELALL))->EnableWindow (FALSE);
        ((CButton*) GetDlgItem (IDC_BTNDELFILES))->EnableWindow (FALSE);
        ((CButton*) GetDlgItem (IDC_BTNMOVERENFILES))->EnableWindow (FALSE);
    }
}