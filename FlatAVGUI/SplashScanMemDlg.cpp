/****************************************************************************\
*
*	Splash Scan Memory Dialog
*
*   COPYRIGHT:  2008 Suyandi Wu
*
\****************************************************************************/

// SplashDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FlatAVGUI.h"
#include "SplashScanMemDlg.h"
#include "SplashScanResDlg.h"

#include "MemoryOp.h"
#include "FileOp.h"


#define WM_USER_DESTROY (WM_USER+1)

///////////////////////////
//
// CSplashScanMemDlg dialog
//

IMPLEMENT_DYNAMIC(CSplashScanMemDlg, CDialog)
CSplashScanMemDlg::CSplashScanMemDlg(LPFVD_BLOCK pFvdBlock, LPSCANOPT pScanOpt, CWnd* pParent /*=NULL*/)
	: CDialog(CSplashScanMemDlg::IDD, pParent)
{
	m_pFvdBlock = pFvdBlock;
	m_pScanOpt = pScanOpt;

    m_szScanFileName[0] = '\0';
    m_bSplashActive = FALSE;
    m_bStopScan = FALSE;
    m_bInfected = FALSE;
	m_bActive = FALSE;
	m_bFailLaunch = FALSE;
    //m_bFinishScan = FALSE;
}

void CSplashScanMemDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSplashScanMemDlg, CDialog)
    ON_WM_PAINT()
    ON_WM_MOUSEMOVE()
    //ON_MESSAGE(WM_USER_DESTROY, OnUserDestroy)
    ON_WM_DESTROY()
    ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()


// CSplashScanMemDlg message handlers

//BOOL CSplashScanMemDlg::Create(UINT uBitmapID)
//{
//	m_uBitmapID = uBitmapID;
//	CDialog::Create(IDD);
//
//	return TRUE;
//}

BOOL CSplashScanMemDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    if (m_bSplashActive) {
		m_bStopScan = TRUE;
		EndDialog(0);
		theApp.g_event.SetEvent();
        return FALSE;
    }
    m_bSplashActive = TRUE;

    //m_splashWnd.CenterWindow();
    //m_splashWnd.UpdateWindow();

	if (!m_bitmap.LoadBitmap(IDB_SPLASH)) {
		m_bStopScan = TRUE;
		EndDialog(0);
		theApp.g_event.SetEvent();
	    return FALSE;
	}
	m_bitmap.GetBitmap(&m_bm);

    m_displayTextRect.left = 25;
    m_displayTextRect.top = m_bm.bmHeight / 2;
    m_displayTextRect.right = m_bm.bmWidth - 5;
    m_displayTextRect.bottom = m_bm.bmHeight - 5;

	if (m_pFvdBlock == NULL) {
		m_bStopScan = TRUE;
		EndDialog(0);
		theApp.g_event.SetEvent();
        return FALSE;
	}

    //m_enumMemObjArray.EnumObjects ();
    //if (m_enumMemObjArray.GetSize() == 0)
    //    return FALSE;
    //m_enumStObjArray.EnumObjects ();

    //scanMemDlg.Create(IDD_SCANMEMDLG);

 //   CSplashScanMemDlg* pclsParam = this;
	//m_pThread = ::AfxBeginThread (ThreadSplashFunc, this);

    m_enumMWObjArray.EnumObjects(ET_ALL);
	if (m_enumMWObjArray.GetSize() == 0) {
		m_bStopScan = TRUE;
		EndDialog(0);
		theApp.g_event.SetEvent();
        return FALSE;
	}

	m_bActive = TRUE;
	//AfxMessageBox(_T("hei"));
	theApp.g_event.SetEvent();
	// EXCEPTION: OCX Property Pages should return FALSE
    return TRUE;

}

void CSplashScanMemDlg::OnPaint()
{
    CPaintDC dc(this); // device context for painting

	//CDC dcImage;
	if (!m_dcImage.GetSafeHdc()) {
		m_dcImage.CreateCompatibleDC(&dc);
		m_dcImage.SelectObject(&m_bitmap);
	}

	//if (m_bitmap.m_hObject && dcImage.CreateCompatibleDC(&dc)) {
		//BITMAP bm;
		//m_bitmap.GetBitmap(&bm);

		// Paint the image
		//CBitmap* pOldBitmap = dcImage.SelectObject(&m_bitmap);
		dc.BitBlt(0, 0, m_bm.bmWidth, m_bm.bmHeight, &m_dcImage, 0, 0, SRCCOPY);
		//dcImage.SelectObject(pOldBitmap);

        // Draw the text
        //dc.SetTextAlign(TA_CENTER);
        TCHAR szDisplayText[MAX_PATH];

        if (!m_bStopScan) {
            ::wsprintf (szDisplayText, _T("Scanning memory and startup objects: %s"), m_szScanFileName);
        }
        else {
            ::lstrcpy (szDisplayText, _T("Memory scan complete. Program will continue..."));
        }
        int nLength = ::lstrlen(szDisplayText);
        //dc.TextOut (25 , bm.bmHeight / 2, szDisplayText, nLength);

        //RECT rect;

        //POINT pt;
        //::GetCursorPos (&pt);
        //::ScreenToClient (m_hWnd, &pt);
        //if (::PtInRect (&rect, pt)) {
        //    dc.Rectangle (&rect);
        //    ::InflateRect (&rect, -1, -1);
        //    CBrush brush(RGB(180, 188, 224));
        //    dc.FillRect (&rect, &brush);
        //    dc.SetBkColor (RGB(180, 188, 224));
        //}

        //CFont font;
        //font.CreatePointFont (88, _T("Arial"));
        //CFont* pOldFont = dc.SelectObject (&font);

        //rect.left = 406;
        //rect.top = 248;
        //rect.right = 495;
        //rect.bottom = 260;
        //TCHAR szStopMemTest[] = _T("Stop memory test");
        //dc.ExtTextOut (406, 248, ETO_CLIPPED, NULL, szStopMemTest, ::lstrlen (szStopMemTest), NULL);
        
        dc.SetBkColor(RGB(255, 255, 255));

        //dc.SelectObject (pOldFont);

        //rect.left = 25;
        //rect.top = bm.bmHeight / 2;
        //rect.right = bm.bmWidth - 5;
        //rect.bottom = bm.bmHeight - 5;

        //dc.ExtTextOut (25, bm.bmHeight / 2, ETO_CLIPPED, &rect, szDisplayText, nLength, NULL);
        dc.DrawText (szDisplayText, nLength, &m_displayTextRect, DT_END_ELLIPSIS);

        if (m_bInfected) {
			TCHAR szMemInfected[64];
            ::lstrcpy(szMemInfected, _T("Memory is infected!"));
            m_displayTextRect.top += 26;
            dc.SetTextColor(RGB(255, 0, 0));
            dc.DrawText(szMemInfected, ::lstrlen(szMemInfected), &m_displayTextRect, DT_END_ELLIPSIS);
			m_displayTextRect.top -= 26;
        }
	//}
}

//INT_PTR CSplashScanMemDlg::ShowSplash()
//{
//    //return DoModal ();
//	return ShowWindow(SW_SHOW);
//}

//BOOL CSplashScanMemDlg::CloseSplash()
//{
//    //return DoModal ();
//	ShowWindow(SW_HIDE);
//	return DestroyWindow();
//
//}
//LONG CSplashScanMemDlg::OnUserDestroy (UINT wParam, LONG lParam)
//{
//    EndDialog (lParam);
//    return 0;
//}

//void CSplashScanMemDlg::OnMouseMove(UINT nFlags, CPoint point)
//{
//    CWnd::OnMouseMove(nFlags, point);
//}

void CSplashScanMemDlg::OnCancel()
{
    if (!m_bStopScan) {
        m_bStopScan = TRUE;
        RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
        GetDlgItem(IDCANCEL)->EnableWindow (FALSE);
    }
    //::TerminateThread (m_pThread->m_hThread, 0);
    //CDialog::OnCancel();
	//if (::WaitForSingleObject(m_pThread->m_hThread, INFINITE) == WAIT_OBJECT_0) {
	//	EndDialog(0);
	//}
}

//void CSplashScanMemDlg::OnDestroy()
//{
//    CDialog::OnDestroy();
//    //AfxGetMainWnd()->UpdateWindow();
//}

void CSplashScanMemDlg::UpdateScanUI (LPCTSTR szFilePath)
{
    ::lstrcpy(m_szScanFileName, szFilePath);
    ::PathStripPath(m_szScanFileName);
    InvalidateRect(&m_displayTextRect, FALSE);
}

void CSplashScanMemDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
    ::ReleaseCapture ();
    ::SendMessage (m_hWnd, WM_SYSCOMMAND, 0xf012, 0);
	
    CDialog::OnLButtonDown(nFlags, point);
}

int CSplashScanMemDlg::ScanMemory()
{
    int i;
    m_uNumScanned = 0;

    for (i = 0; i < m_enumMWObjArray.GetSize(); ++i) {
        if (m_bStopScan) break;
        LPMWOBJ pMWObj = m_enumMWObjArray.GetAt (i);
        ScanMemoryObject(pMWObj);
    }
    //m_bFinishScan = TRUE;
    m_bStopScan = TRUE;
    
	//DBG_MSGBOX(_T("size = %x - %d"), m_MWObjPtrArray, m_MWObjPtrArray.GetSize());
    for (i = 0; i < m_MWObjPtrArray.GetSize(); ++i) {
        LPMWOBJ pMWObj = (LPMWOBJ) m_MWObjPtrArray.GetAt(i);
		if (pMWObj->dwType == MWT_PROCESS) {
            KillProcess(pMWObj->dwProcessId);
		}
		else {
            StopAndDeleteService(pMWObj->szServiceName, pMWObj->dwProcessId > 0);
		}
    }
    //if (m_hwndParent)
    //    ::SendMessage (m_hwndParent, WM_USER_DESTROY, 0, 0);

    if (m_bInfected == TRUE) {
        ShowWindow(SW_HIDE);
        CSplashScanResDlg splashScanResDlg(&m_MWObjPtrArray, m_uNumScanned);
        splashScanResDlg.DoModal();
    }
	//EndDialog (0);
    //CDialog::OnCancel();
    RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
    GetDlgItem(IDCANCEL)->EnableWindow (FALSE);

	//m_bFinishedScan = TRUE;
    return 0;
}

int CSplashScanMemDlg::ScanMemoryObject(LPMWOBJ pMWObj)
{
    int ret = FL_CLEAN;
    UpdateScanUI(pMWObj->szFilePathName);//, FALSE);
    ret = Sca_ScanFile (pMWObj->szFilePathName, m_pFvdBlock);
    if (ret == FL_VIRUS || ret == FL_SUSPECT) {
        pMWObj->nScanResult = ret;
        m_bInfected = TRUE;
        SuspendResumeProcess(pMWObj->dwProcessId, FALSE);
        //InsertLvScanResultItem (pMWObj->szFilePathName, ret, (LPARAM) pMWObj);
        //scanMemDlg.InsertLvScanResultItem (pMWObj->szFilePathName, ret, (LPARAM) pMWObj);
        ::lstrcpyA(pMWObj->szMalwareName, m_pFvdBlock->szMalwareName);
        pMWObj->nMalwareType = m_pFvdBlock->bMalwareType;
        m_MWObjPtrArray.Add(pMWObj);
    }
    ++m_uNumScanned;
	if (!m_bStopScan) {
		UpdateScanUI(pMWObj->szFilePathName);
	}
    return ret;
}