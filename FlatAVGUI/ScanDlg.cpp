/****************************************************************************\
*
*	Scan Dialog
*
*   COPYRIGHT:  2008 Suyandi Wu
*
\****************************************************************************/

// ScanDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FlatAVGUI.h"
#include "ScanDlg.h"
//#include ".\scandlg.h"

#include "DeleteDlg.h"
#include "MoveRenDlg.h"

#include "FileOp.h"
#include "ArchiveOp.h"

static UINT ThreadFunc (LPVOID pParam)
{
    CScanDlg* scanDlg = (CScanDlg*) pParam;
    return scanDlg->ScanSelectedFiles ();
}

// CScanDlg dialog

IMPLEMENT_DYNAMIC(CScanDlg, CDialog)

CScanDlg::CScanDlg(LPFVD_BLOCK pFvdBlock, LPSCANOPT pScanOpt, CStringArray* pastrScanPathList,
                   CWnd* pParent /*=NULL*/)
	: CDialog(CScanDlg::IDD, pParent)
{
    m_pFvdBlock = pFvdBlock;
    m_pScanOpt = pScanOpt;
    m_pastrScanPathList = pastrScanPathList;
    m_bStopScan = FALSE;
}

void CScanDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LSTSCANRESULT, m_wndLvScanResult);
}

BEGIN_MESSAGE_MAP(CScanDlg, CDialog)
    ON_BN_CLICKED(IDC_BTNSTOPCLOSE, OnBtnStopCloseClicked)
    ON_BN_CLICKED(IDC_CHKSELALL, OnChkSelAllClicked)
    ON_NOTIFY(NM_CLICK, IDC_LSTSCANRESULT, OnLstScanResultNMClick)
    ON_NOTIFY(LVN_COLUMNCLICK, IDC_LSTSCANRESULT, OnLstScanResultLvnColumnClick)
    ON_BN_CLICKED(IDC_BTNDELFILES, OnBtnDelFilesClicked)
    ON_NOTIFY(NM_DBLCLK, IDC_LSTSCANRESULT, OnLstScanResultNMDblClick)
    ON_BN_CLICKED(IDC_BTNMOVERENFILES, OnBtnMoveRenFilesClicked)
    ON_WM_DESTROY()
END_MESSAGE_MAP()

// CScanDlg message handlers

BOOL CScanDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    if (m_pFvdBlock == NULL) {
        MessageBox (_T("Database error."), DEFCAPTION, MB_ICONERROR);
        EndDialog (0);
        return TRUE;
    }

    if (m_pastrScanPathList->GetSize() == 0) {
        MessageBox (_T("No file selected."), DEFCAPTION, MB_ICONERROR);
        EndDialog (0);
        return TRUE;
    }

    m_wndLvScanResult.SetExtendedStyle ( LVS_EX_CHECKBOXES | LVS_EX_FLATSB | LVS_EX_FULLROWSELECT |
        LVS_EX_GRIDLINES );

    int i = 0;
    m_wndLvScanResult.InsertColumn (i++, _T("File Name"), LVCFMT_LEFT, 250);
    m_wndLvScanResult.InsertColumn (i++, _T("Virus Name"), LVCFMT_LEFT, 160);
    m_wndLvScanResult.InsertColumn (i, _T("Information"), LVCFMT_LEFT, 220);

	// init the tooltip
	m_toolTipCtrl.Create(this);
	CWnd* pLblFilePath = GetDlgItem(IDC_LBLSCANFILE);
	RECT rect;
	pLblFilePath->GetClientRect(&rect);
	pLblFilePath->MapWindowPoints(this, &rect);
	m_toolTipCtrl.AddTool(this, _T(""), &rect, IDC_LBLSCANFILE);

    //SetTimer (1, 1, NULL);
    CScanDlg* pclsParam = this;
    m_pThread = ::AfxBeginThread (ThreadFunc, pclsParam);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CScanDlg::OnBtnStopCloseClicked()
{
    TCHAR szText[10];
    if (GetDlgItemText (IDC_BTNSTOPCLOSE, szText, sizeof(szText)/sizeof(TCHAR))) {
		if (::lstrcmpi (szText, _T("Stop")) == 0) {
            m_bStopScan = TRUE;
		}
        else {
            m_bStopScan = FALSE;
            EndDialog (0);
        }
    }
}

int CScanDlg::ScanSelectedFiles ()
{
    int i;
    m_uNumScanned = 0;
    m_uNumInfected = 0;
    for (i = 0; i < m_pastrScanPathList->GetSize (); ++i)
        FindAndScan (m_pastrScanPathList->GetAt (i));

    if (m_wndLvScanResult.GetItemCount () > 0) {
        //m_wndLvScanResult.EnableWindow (TRUE);
        GetDlgItem (IDC_CHKSELALL)->EnableWindow (TRUE);
        ((CButton*)GetDlgItem (IDC_CHKSELALL))->SetCheck (1);
        OnChkSelAllClicked ();
        GetDlgItem (IDC_BTNMOVERENFILES)->EnableWindow (TRUE);
        GetDlgItem (IDC_BTNDELFILES)->EnableWindow (TRUE);
    }
    GetDlgItem (IDC_BTNSTOPCLOSE)->SetWindowText (_T("Close"));
    GetDlgItem (IDC_LBLSCANFILE)->SetWindowText (_T(""));
    return 0;
}

int CScanDlg::FindAndScan (CString strFile)
{
    if (m_bStopScan)
        return 0;

    CFileFind fileFind;

    DWORD dwAttr = ::GetFileAttributes ((LPCTSTR) strFile);
    if (dwAttr == 0xFFFFFFFF)
        return 0;

    int ret = FL_CLEAN;
    if ( (dwAttr & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY ) {
        ret = ScanFile (strFile);//, NULL);//, strFile.GetLength ());
        return ret;
    }

    CString strWildCardPath (strFile);
    strWildCardPath += _T("\\*.*");

    BOOL bFind = fileFind.FindFile (strWildCardPath, 0);

    while (!m_bStopScan && bFind) {
        bFind = fileFind.FindNextFile ();

        if (fileFind.IsDots ())
            continue;

        if (fileFind.IsDirectory () && m_pScanOpt->bScanSubFolders)
            FindAndScan (fileFind.GetFilePath ());
        else
            ret = ScanFile (fileFind.GetFilePath ());//, NULL);//, ::lstrlen (fileFind.GetFilePath ()));
    }
    fileFind.Close();
    return ret;
}

int CScanDlg::ScanFile (LPCTSTR szScanFPN)//, LPCTSTR szArchiveFPN, LPCTSTR szFileNameInArchive)//FileInArchiveInfo* pfia_info)
{
    int nRet = FL_CLEAN;
    UpdateScanUI (szScanFPN);//, FALSE);
    if (m_pScanOpt->archive_opt.bScanArchive && (CArchiveOp::IsArchive(szScanFPN) != ARC_NONE)) {
        long lFileLen = GetFileLength (szScanFPN);
        if (lFileLen > m_pScanOpt->archive_opt.dwMaxScanArcSize)
            return FL_CLEAN;

        CArchiveOp archiveOp(m_pFvdBlock);
        archiveOp.ScanArchiveFile (szScanFPN);
        m_uNumScanned += archiveOp.GetScanCount();
        UpdateScanUI (szScanFPN);
        //m_uNumInfected += archiveOp.GetScannedMWCount();
        for (int i = 0; i < archiveOp.GetScannedMWCount(); ++i) {
            TCHAR szComboName[MAX_PATH*2];
            LPMWOBJ pMWObj = archiveOp.GetScannedMW(i);
            lstrcpy (szComboName, pMWObj->szFilePathName);
            lstrcat (szComboName, TEXT("\\"));
            lstrcat (szComboName, pMWObj->szFileNameInArchive);
            LPTSTR p = szComboName;
            while (*p) {
                if (*p == '/') *p = '\\';
                ++p;
            }
            ++m_uNumInfected;
            UpdateScanUI (szComboName);//, pMWObj->nScanResult);
            InsertLvScanResultItem (szComboName, (LPARAM)pMWObj);
        }
    }
    else {
        nRet = Sca_ScanFile (szScanFPN, m_pFvdBlock);
        if (nRet == FL_VIRUS || nRet == FL_SUSPECT) {
            ++m_uNumInfected;
            LPMWOBJ pMWObj = new MWOBJ;
            pMWObj->nScanResult = nRet;
            pMWObj->dwType = MWT_FILE;
            lstrcpy (pMWObj->szFilePathName, szScanFPN);
            InsertLvScanResultItem (szScanFPN, (LPARAM)pMWObj);
        }
        ++m_uNumScanned;
        UpdateScanUI (szScanFPN);//, nRet);
    }
    return nRet;
}

void CScanDlg::UpdateScanUI (LPCTSTR szFilePath)
{
    //int n = GetDlgItemInt (IDC_LBLSCANNUM, NULL, FALSE) + 1;
    SetDlgItemInt (IDC_LBLSCANNUM, m_uNumScanned, FALSE);

    SetDlgItemText (IDC_LBLSCANFILE, szFilePath);

    //if (bInfected) {
        //n = GetDlgItemInt (IDC_LBLINFFILES, NULL, FALSE) + 1;
    SetDlgItemInt (IDC_LBLINFFILES, m_uNumInfected, FALSE);
    //}

	m_toolTipCtrl.UpdateTipText(szFilePath, this, IDC_LBLSCANFILE);
}

void CScanDlg::InsertLvScanResultItem (LPCTSTR szFilePath, /*int nScanRet,*/ LPARAM lParam)
{
    int n = m_wndLvScanResult.GetItemCount ();
    int nScanRet = ((LPMWOBJ)lParam)->nScanResult;

    m_wndLvScanResult.InsertItem (LVIF_TEXT | LVIF_PARAM, n, szFilePath, 0, 0, 0, lParam);
#ifdef _UNICODE
    WCHAR wszMalwareName[MAX_PATH];
    ::MultiByteToWideChar (CP_ACP, 0, m_pFvdBlock->szMalwareName, -1, wszMalwareName,
        MAX_PATH);
    m_wndLvScanResult.SetItemText (n, 1, wszMalwareName);
#else
    m_wndLvScanResult.SetItemText (n, 1, m_pFvdBlock->szMalwareName);
#endif

    if (nScanRet == FL_VIRUS) {
        TCHAR szInfo[24];
        switch (m_pFvdBlock->bMalwareType)
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

void CScanDlg::OnChkSelAllClicked()
{
    int i;
    BOOL fCheck = ((CButton*)GetDlgItem (IDC_CHKSELALL))->GetCheck ();
    int nCount = m_wndLvScanResult.GetItemCount ();
    for (i = 0; i < nCount; ++i)
        m_wndLvScanResult.SetCheck (i, fCheck);
}

void CScanDlg::OnLstScanResultNMClick(NMHDR *pNMHDR, LRESULT *pResult)
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

void CScanDlg::OnLstScanResultNMDblClick(NMHDR *pNMHDR, LRESULT *pResult)
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

void CScanDlg::OnLstScanResultLvnColumnClick(NMHDR *pNMHDR, LRESULT *pResult)
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

void CScanDlg::OnBtnDelFilesClicked()
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
            LPMWOBJ pMWObj = (LPMWOBJ)m_wndLvScanResult.GetItemData (i);
            if (pMWObj->dwType == MWT_ARCHIVE) {
                CArchiveOp archiveOp(m_pFvdBlock);
                ret = archiveOp.DeleteFileInArchive (pMWObj->szFilePathName, deleteDlg.Get_DelOpt(),
                    deleteDlg.Is_DelLockedFiles(), pMWObj->szFileNameInArchive);
            }
            else {
                ret = DeleteFileEx (pMWObj->szFilePathName, deleteDlg.Get_DelOpt(), 
                    deleteDlg.Is_DelLockedFiles());
            }
            
            //if (LOWORD(lParam) == 0)
            //    ret = DeleteFileEx (m_wndLvScanResult.GetItemText (i, 0), deleteDlg.Get_DelOpt (), 
            //        deleteDlg.Is_DelLockedFiles ());
            //else
            //    ret = DeleteMalwareInArchive (m_wndLvScanResult.GetItemText (i, 0), 
            //        deleteDlg.Get_DelOpt (), deleteDlg.Is_DelLockedFiles (), 
            //        LOWORD(lParam), HIWORD(lParam));

            if (ret == FL_SUCCESS) {
                // File was successfully deleted, so delete the item in list-view
                delete pMWObj;
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
        }// if (m_wndLvScanResult.GetCheck (i))
    }// for (int i = 0; i < nItemCount; ++i)

    if (m_wndLvScanResult.GetItemCount () == 0) {
        ((CButton*) GetDlgItem (IDC_CHKSELALL))->EnableWindow (FALSE);
        ((CButton*) GetDlgItem (IDC_BTNDELFILES))->EnableWindow (FALSE);
        ((CButton*) GetDlgItem (IDC_BTNMOVERENFILES))->EnableWindow (FALSE);
    }
}


void CScanDlg::OnBtnMoveRenFilesClicked()
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
            LPMWOBJ pMWObj = (LPMWOBJ)m_wndLvScanResult.GetItemData (i);
            if (pMWObj->dwType == MWT_ARCHIVE) {
                CArchiveOp archiveOp(m_pFvdBlock);
                ret = archiveOp.MoveRenameFileInArchive (pMWObj->szFilePathName, 
                    moveRenDlg.Get_DestFolderPath(), moveRenDlg.Is_RenFile(),
                    moveRenDlg.Is_MoveRenLocked(), pMWObj->szFileNameInArchive);
            }
            else {
                ret = MoveRenameFileEx (pMWObj->szFilePathName, moveRenDlg.Get_DestFolderPath(), 
                    moveRenDlg.Is_RenFile(), moveRenDlg.Is_MoveFile(), moveRenDlg.Is_MoveRenLocked());
            }
            //if (LOWORD(lParam) == 0)
            //    ret = MoveRenameMalware (m_wndLvScanResult.GetItemText (i, 0), moveRenDlg.Get_DestFolderPath (),
            //        moveRenDlg.Is_RenFile (), moveRenDlg.Is_MoveFile (), moveRenDlg.Is_MoveRenLocked ());
            //else
            //    ret = MoveRenameMalwareInArchive (m_wndLvScanResult.GetItemText (i, 0), 
            //        moveRenDlg.Get_DestFolderPath (), moveRenDlg.Is_RenFile (), 
            //        moveRenDlg.Is_MoveFile (), moveRenDlg.Is_MoveRenLocked (), 
            //        LOWORD(lParam), HIWORD(lParam));

            if (ret == FL_SUCCESS) {
                // File was successfully moved/renamed, so delete the item in list-view
                delete pMWObj;
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

void CScanDlg::FreeInsertedMWObj ()
{
    int nItemCount = m_wndLvScanResult.GetItemCount();
    for (int i = 0; i < nItemCount; ++i) {
        LPMWOBJ pMWObj = (LPMWOBJ) m_wndLvScanResult.GetItemData(i);
        delete pMWObj;
    }
}

void CScanDlg::OnDestroy()
{
    CDialog::OnDestroy();
    FreeInsertedMWObj();
}

BOOL CScanDlg::PreTranslateMessage(MSG* pMsg)
{
	if (!m_bStopScan) {
		m_toolTipCtrl.RelayEvent(pMsg);
	}

	return CDialog::PreTranslateMessage(pMsg);
}
