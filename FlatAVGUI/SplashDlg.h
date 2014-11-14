#pragma once

#include "..\libflatav\flatav.h"
#include "MemoryObjectArray.h"

class CSplashWnd : public CWnd
{
public:
    CSplashWnd()
    {
        m_bStopScan = FALSE;
        m_bFinishScan = FALSE;
        *m_szScanFileName = '\0';
        //m_hwndParent = NULL;
    }
    BOOL Create (UINT uBitmapID, LPFVD_BLOCK pFvdBlock, LPSCANOPT pScanOpt, HWND hwndParent = NULL);

    int ScanMemory ();
protected:
    CMemoryObjectArray m_memObjArray;
    LPFVD_BLOCK m_pFvdBlock;
    LPSCANOPT m_pScanOpt;

    TCHAR m_szScanFileName[MAX_PATH];
    CBitmap m_bitmap;
    CWinThread* m_pThread;
    BOOL m_bStopScan;
    BOOL m_bFinishScan;

    HWND m_hwndParent;

    int ScanMemoryObject (LPMEMORYOBJ pMemoryObj);
    void UpdateScanUI (LPCTSTR szFilePath, BOOL bInfected);

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnPaint();
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};

// CSplashDlg dialog

class CSplashDlg : protected CDialog
{
	DECLARE_DYNAMIC(CSplashDlg)

public:
	CSplashDlg(UINT uBitmapID, LPFVD_BLOCK pFvdBlock, LPSCANOPT pScanOpt, CWnd* pParent = NULL);
	virtual ~CSplashDlg();

// Dialog Data
	enum { IDD = IDD_SPLASHDLG };

    INT_PTR ShowSplash ();
    //void HideSplash();

protected:
    //CMemoryObjectArray m_memObjArray;
    LPFVD_BLOCK m_pFvdBlock;
    LPSCANOPT m_pScanOpt;

    //BOOL m_bStopScan;
    //CWinThread* m_pThread;

    BOOL m_bSplashActive;
    //TCHAR m_szScanFileName[MAX_PATH];

    UINT m_uBitmapId;
    CSplashWnd m_splashWnd;
protected:
    afx_msg LONG OnUserDestroy (UINT wParam, LONG lParam);
    //int ScanMemoryObject (LPMEMORYOBJ pMemoryObj);
    //void UpdateScanUI (LPCTSTR szFilePath, BOOL bInfected);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

//private:
//    void Create() {};
//    void CreateEx() {};
//    void CreateIndirect() {};
//    void InitModalIndirect() {};
//    virtual int DoModal() { return 0; };
public:
    virtual BOOL OnInitDialog();
    //afx_msg void OnPaint();
};
