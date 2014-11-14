/****************************************************************************\
*
*	Scan Memory Dialog
*
*   COPYRIGHT:  2008 Suyandi Wu
*
\****************************************************************************/

// ScanMemDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FlatAVGUI.h"
#include "ScanMemDlg.h"
//#include ".\scanmemdlg.h"

#include "DeleteDlg.h"
#include "MoveRenDlg.h"

#include "MemoryOp.h"
#include "FileOp.h"

static UINT ThreadMemFunc (LPVOID pParam)
{
    CScanMemDlg* scanMemDlg = (CScanMemDlg*) pParam;
    return scanMemDlg->ScanMemory ();
}

// CScanMemDlg dialog

IMPLEMENT_DYNAMIC(CScanMemDlg, CDialog)

CScanMemDlg::CScanMemDlg(LPFVD_BLOCK pFvdBlock, LPSCANOPT pScanOpt, /*CEnumMemoryObjectArray* pMemObjArray,*/
    CWnd* pParent)
    : CDialog(CScanMemDlg::IDD, pParent)
{
    m_pFvdBlock = pFvdBlock;
    m_pScanOpt = pScanOpt;
    m_bStopScan = FALSE;
    //m_pMemObjArray = pMemObjArray;
}

void CScanMemDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LSTSCANRESULT, m_wndLvScanResult);
}


BEGIN_MESSAGE_MAP(CScanMemDlg, CDialog)
    ON_BN_CLICKED(IDC_BTNSTOPCLOSE, OnBtnStopCloseClicked)
    ON_BN_CLICKED(IDC_CHKSELALL, OnChkSelAllClicked)
    ON_NOTIFY(NM_CLICK, IDC_LSTSCANRESULT, OnLstScanResultNMClick)
    ON_NOTIFY(NM_DBLCLK, IDC_LSTSCANRESULT, OnLstScanResultNMDblClick)
    ON_NOTIFY(LVN_COLUMNCLICK, IDC_LSTSCANRESULT, OnLstScanResultLvnColumnClick)
    ON_BN_CLICKED(IDC_BTNDELFILES, OnBtnDelFilesClicked)
    ON_BN_CLICKED(IDC_BTNMOVERENFILES, OnBtnMoveRenFilesClicked)
    ON_WM_DESTROY()
END_MESSAGE_MAP()


// CScanMemDlg message handlers

BOOL CScanMemDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    if (m_pFvdBlock == NULL) {
        MessageBox (_T("Database error."), DEFCAPTION, MB_ICONERROR);
        EndDialog (0);
        return TRUE;
    }

    //m_enumMemObjArray.EnumObjects ();
    //if (m_enumMemObjArray.GetSize() == 0) {
    //    EndDialog (0);
    //    return TRUE;
    //}
    //m_enumStObjArray.EnumObjects ();
    m_enumMWObjArray.EnumObjects(ET_ALL);
    if (m_enumMWObjArray.GetSize() == 0) {
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

    CScanMemDlg* pclsParam = this;
    m_pThread = ::AfxBeginThread (ThreadMemFunc, pclsParam);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

int CScanMemDlg::ScanMemory ()
{
    int i;
    m_uNumScanned = m_uNumInfected = 0;
    //for (i = 0; i < m_enumMemObjArray.GetSize (); ++i) {
    //    if (m_bStopScan) break;
    //    LPMWOBJ pMWObj = m_enumMemObjArray.GetAt (i);
    //    ScanMemoryObject (pMWObj);
    //}
    //for (i = 0; i < m_enumStObjArray.GetSize (); ++i) {
    //    if (m_bStopScan) break;
    //    LPMWOBJ pMWObj = m_enumStObjArray.GetAt (i);
    //    ScanMemoryObject (pMWObj);
    //}
    for (i = 0; i < m_enumMWObjArray.GetSize (); ++i) {
        if (m_bStopScan) break;
        LPMWOBJ pMWObj = m_enumMWObjArray.GetAt (i);
        ScanMemoryObject (pMWObj);
    }

    if (m_wndLvScanResult.GetItemCount () > 0) {
        //GetDlgItem (IDC_CHKSELALL)->EnableWindow (TRUE);
        ((CButton*)GetDlgItem (IDC_CHKSELALL))->SetCheck (1);
        OnChkSelAllClicked ();
        GetDlgItem (IDC_BTNMOVERENFILES)->EnableWindow (TRUE);
        GetDlgItem (IDC_BTNDELFILES)->EnableWindow (TRUE);

        for (i = 0; i < m_wndLvScanResult.GetItemCount (); ++i) {
            LPMWOBJ pMWObj = (LPMWOBJ) m_wndLvScanResult.GetItemData (i);
            if (pMWObj->dwType == MWT_PROCESS)
                KillProcess (pMWObj->dwProcessId);
            else
                StopAndDeleteService (pMWObj->szServiceName, pMWObj->dwProcessId > 0);
        }
        GetDlgItem (IDC_LBLSCANFILE)->SetWindowText (_T(""));
    }
    else
        SetDlgItemText (IDC_LBLSCANFILE, _T("NO VIRUS FOUND"));

    GetDlgItem (IDC_BTNSTOPCLOSE)->SetWindowText (_T("Close"));
    return 0;
}

//BOOL KillProcessPath (LPMWOBJ pMWObj)
//{
//    BOOL bRet = FALSE;
//    HANDLE hProcessSnap, hModuleSnap;
//    PROCESSENTRY32 procEntry32;
//    MODULEENTRY32 moduleEntry32;
//
//    hProcessSnap = ::CreateToolhelp32Snapshot (TH32CS_SNAPPROCESS, 0);
//    if (hProcessSnap == INVALID_HANDLE_VALUE)
//        return 0;
//
//    // Get the application files first
//    procEntry32.dwSize = sizeof(PROCESSENTRY32);
//    if (::Process32First (hProcessSnap, &procEntry32)) {
//        while (::Process32Next (hProcessSnap, &procEntry32)) {
//            hModuleSnap = ::CreateToolhelp32Snapshot (TH32CS_SNAPMODULE, 
//                procEntry32.th32ProcessID);
//            if (hModuleSnap != INVALID_HANDLE_VALUE) {
//                moduleEntry32.dwSize = sizeof(MODULEENTRY32);
//                if (::Module32First (hModuleSnap, &moduleEntry32)) {
//                    if (::lstrcmpi (pMWObj->szFilePathName, moduleEntry32.szExePath) == 0) {
//                        ::CloseHandle (hModuleSnap);
//                        HANDLE hProcess = ::OpenProcess (PROCESS_TERMINATE | PROCESS_QUERY_INFORMATION, 
//                            FALSE, procEntry32.th32ProcessID);
//                        if (hProcess) {
//                            bRet = ::TerminateProcess (hProcess, 0);
//                            if (bRet) {
//                                DWORD dwExitCode;
//                                while (::GetExitCodeProcess (hProcess, &dwExitCode) && 
//                                    dwExitCode == STILL_ACTIVE);
//                            }
//                            ::CloseHandle (hProcess);
//                        }
//                        else
//                            bRet = StopAndDeleteService (pMWObj);
//                    }
//                }
//                ::CloseHandle (hModuleSnap);
//            }
//        }
//    }
//    ::CloseHandle (hProcessSnap);
//    return bRet;
//}

int CScanMemDlg::ScanMemoryObject (LPMWOBJ pMWObj)
{
    int ret = FL_CLEAN;
    UpdateScanUI (pMWObj->szFilePathName);//, FALSE);
    ret = Sca_ScanFile (pMWObj->szFilePathName, m_pFvdBlock);
    if (ret == FL_VIRUS || ret == FL_SUSPECT) {
        pMWObj->nScanResult = ret;
        ++m_uNumInfected;
        SuspendResumeProcess (pMWObj->dwProcessId, FALSE);
        InsertLvScanResultItem (pMWObj->szFilePathName, (LPARAM) pMWObj);
    }
    ++m_uNumScanned;
    UpdateScanUI (pMWObj->szFilePathName);
    return ret;
}

void CScanMemDlg::UpdateScanUI (LPCTSTR szFilePath)//, BOOL bInfected)
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

void CScanMemDlg::InsertLvScanResultItem (LPCTSTR szFilePath, LPARAM lParam)
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

void CScanMemDlg::OnBtnStopCloseClicked()
{
    TCHAR szText[10];
    if (GetDlgItemText (IDC_BTNSTOPCLOSE, szText, sizeof(szText))) {
		if (::lstrcmpi (szText, _T("Stop")) == 0) {
            m_bStopScan = TRUE;
		}
        else {
            m_bStopScan = FALSE;
            EndDialog (0);
        }
    }
}

void CScanMemDlg::OnChkSelAllClicked()
{
    int i;
    BOOL fCheck = ((CButton*)GetDlgItem (IDC_CHKSELALL))->GetCheck ();
    int nCount = m_wndLvScanResult.GetItemCount ();
    for (i = 0; i < nCount; ++i)
        m_wndLvScanResult.SetCheck (i, fCheck);
}

void CScanMemDlg::OnLstScanResultNMClick(NMHDR *pNMHDR, LRESULT *pResult)
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

void CScanMemDlg::OnLstScanResultNMDblClick(NMHDR *pNMHDR, LRESULT *pResult)
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

void CScanMemDlg::OnLstScanResultLvnColumnClick(NMHDR *pNMHDR, LRESULT *pResult)
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

void CScanMemDlg::OnBtnDelFilesClicked()
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
            LPMWOBJ pMWObj = (LPMWOBJ) m_wndLvScanResult.GetItemData (i);
            if (pMWObj->dwType == MWT_PROCESS)
                KillProcess (pMWObj->dwProcessId);
            else
                StopAndDeleteService (pMWObj->szServiceName, pMWObj->dwProcessId > 0);

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

void CScanMemDlg::OnBtnMoveRenFilesClicked()
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
            LPMWOBJ pMWObj = (LPMWOBJ) m_wndLvScanResult.GetItemData (i);
            if (pMWObj->dwType == MWT_PROCESS)
                KillProcess (pMWObj->dwProcessId);
            else
                StopAndDeleteService (pMWObj->szServiceName, pMWObj->dwProcessId > 0);

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

//int CScanMemDlg::MoveRenameMalware (LPCTSTR szFilePathName, _TCHAR* szDestFolderPath, 
//                                    BOOL bRenFile, BOOL bMoveFile, BOOL bMoveRenLockedFiles)
//{
//    // Note about lParam:
//    // LOWORD(lParam) = pfia_info->nArchiveType
//    // HIWORD(lParam) = pfia_info->nArchiveFileNameLen
//    TCHAR szNewPathName[MAX_PATH], szResult[MAX_PATH];
//
//    int ret = FL_SUCCESS;
//    if (_taccess (szFilePathName, 0) == -1)
//        return ret;//FL_ENOFILE;
//
//    ::SetFileAttributes (szFilePathName, FILE_ATTRIBUTE_NORMAL);
//    ::lstrcpy (szResult, szFilePathName);
//
//    BOOL bSuccess = TRUE;
//    if (bRenFile) {
//        ::lstrcpy (szNewPathName, szFilePathName);
//        ::lstrcat (szNewPathName, _T(".vir"));
//        bSuccess = ::MoveFileEx (szResult, szNewPathName, MOVEFILE_REPLACE_EXISTING);
//        if (bSuccess)
//            ::lstrcpy (szResult, szNewPathName);
//    }
//
//    if (bMoveFile && bSuccess) {
//        if (_taccess (szDestFolderPath, 0) == -1 && MakeDir (szDestFolderPath) == -1)
//            return FL_EACCESS;
//        ::lstrcpy (szNewPathName, szDestFolderPath);
//        ::PathAddBackslash (szNewPathName);
//        ::lstrcat (szNewPathName, ::PathFindFileName (szResult));
//        bSuccess = ::MoveFileEx (szResult, szNewPathName, MOVEFILE_REPLACE_EXISTING);
//    }
//
//    if (!bSuccess) {
//        ret = FL_EWRITE;
//        if (bMoveRenLockedFiles) {
//            // The file might be locked, try move/rename it at next system restart.
//            ::MoveFileEx (szResult, szNewPathName, MOVEFILE_DELAY_UNTIL_REBOOT);
//        }
//    }
//
//    return ret;
//}

//void CScanMemDlg::InsertLvScanResultItem (CMemoryObjPtrArray* pMemObjPtrArray)
//{
//    for (int i = 0; i < pMemObjPtrArray->GetSize(); ++i) {
//        InsertLvScanResultItem(pMemObjPtrArray->GetAt(i)->szFilePathName,
//    }
//}

//void CScanMemDlg::FreeInsertedMWObj ()
//{
//    int nItemCount = m_wndLvScanResult.GetItemCount();
//    for (int i = 0; i < nItemCount; ++i) {
//        LPMWOBJ pMWObj = (LPMWOBJ) m_wndLvScanResult.GetItemData(i);
//        delete pMWObj;
//    }
//}

void CScanMemDlg::OnDestroy()
{
    CDialog::OnDestroy();
    //int nItemCount = m_wndLvScanResult.GetItemCount ();
    //for (int i = 0; i < nItemCount; ++i) {
    //    LPMWOBJ pMWObj = (LPMWOBJ) m_wndLvScanResult.GetItemData (i);
    //    if (pMWObj->dwType == MWT_PROCESS)
    //        KillProcess (pMWObj->dwProcessId);
    //    else
    //        StopAndDeleteService (pMWObj->szServiceName, pMWObj->dwProcessId > 0);
    //}
}

BOOL CScanMemDlg::PreTranslateMessage(MSG* pMsg)
{
	if (!m_bStopScan) {
		m_toolTipCtrl.RelayEvent(pMsg);
	}

	return CDialog::PreTranslateMessage(pMsg);
}
