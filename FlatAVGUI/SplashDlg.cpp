// SplashDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FlatAVGUI.h"
#include "SplashDlg.h"

#include "MemoryOp.h"
#include "FileOp.h"
#include ".\splashdlg.h"

#define WM_USER_DESTROY (WM_USER+1)

static UINT ThreadSplashFunc (LPVOID pParam)
{
    CSplashWnd* splashWnd = (CSplashWnd*) pParam;
    return splashWnd->ScanMemory ();
}

BEGIN_MESSAGE_MAP(CSplashWnd, CWnd)
    ON_WM_PAINT()
    ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

BOOL CSplashWnd::Create (UINT uBitmapID, LPFVD_BLOCK pFvdBlock, LPSCANOPT pScanOpt, HWND hwndParent)
{
    if (!m_bitmap.LoadBitmap(uBitmapID))
	    return FALSE;

    BITMAP bm;
    m_bitmap.GetBitmap(&bm);

    CString strWndClass = AfxRegisterWndClass(0, AfxGetApp()->LoadStandardCursor(IDC_ARROW));

    if (!CreateEx (0, strWndClass, NULL, WS_CHILD | WS_VISIBLE,
        0, 0, bm.bmWidth, bm.bmHeight, hwndParent, NULL))
    {
        return FALSE;
    }

    m_pFvdBlock = pFvdBlock;
    m_pScanOpt = pScanOpt;
    m_hwndParent = hwndParent;

    if (m_pFvdBlock == NULL)
        return FALSE;

    m_memObjArray.EnumMemoryObjects ();
    if (m_memObjArray.GetSize() == 0)
        return FALSE;

    CSplashWnd* pclsParam = this;
    m_pThread = ::AfxBeginThread (ThreadSplashFunc, pclsParam);
    return TRUE;
}

void CSplashWnd::OnPaint()
{
    CPaintDC dc(this); // device context for painting

	CDC dcImage;
	if (dcImage.CreateCompatibleDC (&dc))
	{
		BITMAP bm;
		m_bitmap.GetBitmap (&bm);

		// Paint the image
		CBitmap* pOldBitmap = dcImage.SelectObject (&m_bitmap);
		dc.BitBlt(0, 0, bm.bmWidth, bm.bmHeight, &dcImage, 0, 0, SRCCOPY);
		dcImage.SelectObject (pOldBitmap);

        // Draw the text
        //dc.SetTextAlign (TA_CENTER
        TCHAR szDisplayText[MAX_PATH];
        int nLength;

        if (!m_bFinishScan) {
            
            ::wsprintf (szDisplayText, _T("Scanning memory and startup objects: %s"), m_szScanFileName);
            nLength = ::lstrlen(szDisplayText);
        }
        else
            ::lstrcpy (szDisplayText, _T("Memory scan complete. Program will continue..."));
        //dc.TextOut (25 , bm.bmHeight / 2, szDisplayText, nLength);

        RECT rect;

        rect.left = 401;
        rect.top = 243;
        rect.right = 500;
        rect.bottom = 265;

        POINT pt;
        ::GetCursorPos (&pt);
        ::ScreenToClient (m_hWnd, &pt);
        if (::PtInRect (&rect, pt)) {
            dc.Rectangle (&rect);

            ::InflateRect (&rect, -1, -1);
            CBrush brush(RGB(180, 188, 224));
            dc.FillRect (&rect, &brush);
            dc.SetBkColor (RGB(180, 188, 224));
        }

        CFont font;
        font.CreatePointFont (88, _T("Arial"));
        CFont* pOldFont = dc.SelectObject (&font);

        rect.left = 406;
        rect.top = 248;
        rect.right = 495;
        rect.bottom = 260;
        TCHAR szStopMemTest[] = _T("Stop memory test");
        //dc.DrawText (szStopMemTest, sizeof(szStopMemTest)/sizeof(TCHAR), &rect, 0);
        dc.ExtTextOut (406, 248, ETO_CLIPPED, NULL, szStopMemTest, ::lstrlen (szStopMemTest), NULL);
        
        dc.SetBkColor (RGB(255, 255, 255));

        rect.left = 25;
        rect.top = bm.bmHeight / 2;
        rect.right = bm.bmWidth - 5;
        rect.bottom = bm.bmHeight - 5;

        dc.SelectObject (pOldFont);

        //dc.ExtTextOut (25, bm.bmHeight / 2, ETO_CLIPPED, &rect, szDisplayText, nLength, NULL);
        dc.DrawText (szDisplayText, nLength, &rect, DT_END_ELLIPSIS);
	}
}

int CSplashWnd::ScanMemory ()
{
    int i;
    for (i = 0; i < m_memObjArray.GetSize (); ++i) {
        if (m_bStopScan)
            break;
        LPMEMORYOBJ pMemoryObj = m_memObjArray.GetAt (i);
        ScanMemoryObject (pMemoryObj);
        //PumpMessage ();
    }
    m_bFinishScan = TRUE;
    
    //for (i = 0; i < m_wndLvScanResult.GetItemCount (); ++i) {
    //    LPMEMORYOBJ pMemoryObj = (LPMEMORYOBJ) m_wndLvScanResult.GetItemData (i);
    //    if (pMemoryObj->dwType = MEM_PROCESS)
    //        KillProcess (pMemoryObj->dwProcessId);
    //    else
    //        StopAndDeleteService (pMemoryObj->szServiceName, pMemoryObj->dwProcessId > 0);
    //}

    if (m_hwndParent)
        ::SendMessage (m_hwndParent, WM_USER_DESTROY, 0, 0);
    DestroyWindow ();
    //EndDialog (0);
    return 0;
}

int CSplashWnd::ScanMemoryObject (LPMEMORYOBJ pMemoryObj)
{
    int ret = FL_CLEAN;
    UpdateScanUI (pMemoryObj->szFilePathName, FALSE);
    ret = Sca_ScanFile (pMemoryObj->szFilePathName, m_pFvdBlock);
    if (ret == FL_VIRUS || ret == FL_SUSPECT) {
        SuspendResumeProcess (pMemoryObj->dwProcessId, FALSE);
        UpdateScanUI (pMemoryObj->szFilePathName, ret);
        //InsertLvScanResultItem (pMemoryObj->szFilePathName, ret, (LPARAM) pMemoryObj);
    }
    return ret;
}

void CSplashWnd::UpdateScanUI (LPCTSTR szFilePath, BOOL bInfected)
{
    ::lstrcpy (m_szScanFileName, szFilePath);
    ::PathStripPath (m_szScanFileName);
    //int n = GetDlgItemInt (IDC_LBLSCANNUM, NULL, FALSE) + 1;
    //SetDlgItemInt (IDC_LBLSCANNUM, n, FALSE);

    //SetDlgItemText (IDC_LBLSCANFILE, szFilePath);

    if (bInfected) {
        //n = GetDlgItemInt (IDC_LBLINFFILES, NULL, FALSE) + 1;
        //SetDlgItemInt (IDC_LBLINFFILES, n, FALSE);
    }
    Invalidate ();
}

///////////////////////////
//
// CSplashDlg dialog
//

IMPLEMENT_DYNAMIC(CSplashDlg, CDialog)
CSplashDlg::CSplashDlg(UINT uBitmapID, LPFVD_BLOCK pFvdBlock, LPSCANOPT pScanOpt, CWnd* pParent /*=NULL*/)
	: CDialog(CSplashDlg::IDD, pParent)
{
    m_pFvdBlock = pFvdBlock;
    m_pScanOpt = pScanOpt;
    //m_szScanFileName[0] = '\0';
    m_bSplashActive = FALSE;
    m_uBitmapId = uBitmapID;
}

CSplashDlg::~CSplashDlg()
{
}

void CSplashDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSplashDlg, CDialog)
    ON_MESSAGE(WM_USER_DESTROY, OnUserDestroy)
END_MESSAGE_MAP()


// CSplashDlg message handlers

BOOL CSplashDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    if (m_bSplashActive) {
        EndDialog (0);
        return FALSE;
    }
    m_bSplashActive = TRUE;

    m_splashWnd.Create (m_uBitmapId, m_pFvdBlock, m_pScanOpt, m_hWnd);
    //m_splashWnd.SubclassWindow (m_hWnd);

    m_splashWnd.CenterWindow();
    m_splashWnd.UpdateWindow();

    return TRUE;
    // EXCEPTION: OCX Property Pages should return FALSE
}

//void CSplashDlg::OnPaint()
//{
//    CPaintDC dc(this); // device context for painting
//}

INT_PTR CSplashDlg::ShowSplash ()
{
    return CDialog::DoModal ();
}

LONG CSplashDlg::OnUserDestroy (UINT wParam, LONG lParam)
{
    EndDialog (lParam);
    return 0;
}

void CSplashWnd::OnMouseMove(UINT nFlags, CPoint point)
{
    static BOOL bInButton = FALSE;
    RECT rect;
    rect.left = 401;
    rect.top = 243;
    rect.right = 500;
    rect.bottom = 265;
    if (::PtInRect (&rect, point)) {
    //if (point.x > 397 && point.x < 494 && point.y > 238 && point.y < 265) {
    //    RECT rect;
    //    rect.left = 397;
    //    rect.top = 238;
    //    rect.right = 494;
    //    rect.bottom = 265;

    //    CClientDC clientDC(this);
    //    clientDC.Rectangle (&rect);

    //    ::InflateRect (&rect, -1, -1);

    //    CBrush brush(RGB(125, 167, 217));
    //    clientDC.FillRect (&rect, &brush);

    //    InvalidateRect (&rect);
        
        if (!bInButton)
            RedrawWindow ();
        bInButton = TRUE;
    }
    else {
        if (bInButton)
            RedrawWindow ();
        bInButton = FALSE;
    }

    //CWnd::OnMouseMove(nFlags, point);
}
