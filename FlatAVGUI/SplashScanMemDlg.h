/****************************************************************************\
*
*	Splash Scan Memory Dialog
*
*   COPYRIGHT:  2008 Suyandi Wu
*
\****************************************************************************/

#pragma once

#include "..\libflatav\flatav.h"
#include "EnumMWObjectArray.h"
//#include "EnumMemoryObjectArray.h"
//#include "EnumStartupObjectArray.h"
//#include "ScanMemDlg.h"
/*
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
    CEnumMemoryObjectArray m_enumMemObjArray;
    LPFVD_BLOCK m_pFvdBlock;
    LPSCANOPT m_pScanOpt;

    TCHAR m_szScanFileName[MAX_PATH];
    CBitmap m_bitmap;
    CWinThread* m_pThread;
    BOOL m_bStopScan;
    BOOL m_bFinishScan;

    HWND m_hwndParent;

    int ScanMemoryObject (LPMWOBJ pMemoryObj);
    void UpdateScanUI (LPCTSTR szFilePath, BOOL bInfected);

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnPaint();
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};
*/
// CSplashDlg dialog

typedef struct SplashDlgContainer_t {
	UINT bitmapID;
	LPFVD_BLOCK pFvdBlock;
	LPSCANOPT pScanOpt;
	HWND hWnd;
} SplashDlgContainer;

class CSplashScanMemDlg : public CDialog
{
	DECLARE_DYNAMIC(CSplashScanMemDlg)

public:
	CSplashScanMemDlg(LPFVD_BLOCK pFvdBlock, LPSCANOPT pScanOpt, CWnd* pParent = NULL);

	enum { IDD = IDD_SPLASHDLG };

	// fields:
    BOOL m_bSplashActive;
	BOOL m_bActive;
	BOOL m_bStopScan;
	//BOOL m_bFinishedScan;
    int ScanMemory();

	// methods:
    //INT_PTR ShowSplash();
	//BOOL CloseSplash();

	//BOOL Create(UINT uBitmapID);

protected:
	// fields:
    //UINT m_uBitmapID;
    LPFVD_BLOCK m_pFvdBlock;
    LPSCANOPT m_pScanOpt;

	TCHAR m_szScanFileName[MAX_PATH];
	
    BOOL m_bInfected;
	BOOL m_bFailLaunch;

	UINT m_uNumScanned;

    CBitmap m_bitmap;
	BITMAP m_bm;
	RECT m_displayTextRect;
	CDC m_dcImage;

    CMWObjPtrArray m_MWObjPtrArray;
	CEnumMWObjectArray m_enumMWObjArray;

	// methods:
	int ScanMemoryObject (LPMWOBJ pMemoryObj);
	void UpdateScanUI (LPCTSTR szFilePath);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

//private:
//    void Create() {};
//    void CreateEx() {};
//    void CreateIndirect() {};
//    void InitModalIndirect() {};
//    virtual int DoModal() { return 0; };
public:
	//virtual INT_PTR DoModal();
    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();
    //afx_msg void OnMouseMove(UINT nFlags, CPoint point);
protected:
    virtual void OnCancel();
public:
    //afx_msg void OnDestroy();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};
