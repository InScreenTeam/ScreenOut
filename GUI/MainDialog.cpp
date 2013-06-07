#include "stdafx.h"
#include "afxdialogex.h"
#include "GUI.h"
#include "MainDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

MainDialog::MainDialog(CWnd* pParent /*=NULL*/): CDialogEx(MainDialog::IDD, pParent)
{
	trayIconData.hWnd				= m_hWnd;
	m_hIcon							= AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	trayIconData.cbSize			= sizeof(NOTIFYICONDATA);
	trayIconData.uID				= 1;
	trayIconData.uCallbackMessage	= WM_TRAY_ICON_NOTIFY_MESSAGE;
	trayIconData.hIcon				= 0;
	trayIconData.szTip[0]			= 0;	
	trayIconData.uFlags			=  NIF_ICON | NIF_MESSAGE | NIF_TIP;
	
	isTrayIconVisible				= FALSE;
	trayMenuDefaultItem				= 0;
	m_bMinimizeToTray				= TRUE;
}

void MainDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(MainDialog, CDialogEx)	
	
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_RECORD_BUTTON, &MainDialog::OnBnClickedRecordButton)
	ON_WM_TIMER()
	
	ON_MESSAGE(WM_TRAY_ICON_NOTIFY_MESSAGE, OnTrayNotify)
	ON_BN_CLICKED(IDC_HOTKEY_BUTTON, &MainDialog::OnBnClickedHotkeyButton)
	ON_MESSAGE(WM_HOTKEY, &MainDialog::OnHotKey)
	ON_NOTIFY(NM_OUTOFMEMORY, IDC_RECORD_HOTKEY, &MainDialog::OnNMOutofmemoryRecordHotkey)
END_MESSAGE_MAP()

#pragma region Generated
BOOL MainDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	TraySetIcon(IDR_MAINFRAME);
	TraySetToolTip(L"ToolTip for tray icon");
	TraySetMenu(IDR_TRAY_MENU);

	processingLabel = (CStatic*) this->GetDlgItem(IDC_PROCESSING_LABEL);
	recordButton = (CButton*) this->GetDlgItem(IDC_RECORD_BUTTON);
	
	recordHotkey = (CHotKeyCtrl*) this->GetDlgItem(IDC_RECORD_HOTKEY);	
	recordHotkey->SetHotKey(0x52, HOTKEYF_CONTROL | HOTKEYF_ALT);
	minimizeHotkey = (CHotKeyCtrl*) this->GetDlgItem(IDC_MINIMIZE_HOTKEY);
	minimizeHotkey->SetHotKey(0x52, HOTKEYF_SHIFT | HOTKEYF_ALT);	
	recordHotkeyId = 21;
	minimizeHotkeyId = 13;	

	OnBnClickedHotkeyButton();

	doneTimer = -1;
	isRecording = false;
	isHidden = false;
	

	return TRUE;
}

void MainDialog::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

void MainDialog::OnPaint()
{	
	if (IsIconic())
	{
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);
		
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;
		
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR MainDialog::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
#pragma endregion Dialog init

#pragma region Record

void MainDialog::OnTimer(UINT nIDEvent)
{
	if((recorder->IsDone()))
	{
		delete recorder;
		processingLabel->ShowWindow(SW_HIDE);	
		KillTimer(doneTimer);	
	}
	CDialogEx::OnTimer(nIDEvent);
}

void MainDialog::OnBnClickedRecordButton()
{
	Record();	
}

void MainDialog::Record()
{
	if(isRecording)
	{
		
		recorder->Stop();	
		processingLabel->ShowWindow(SW_SHOW);			
		isRecording = false;
		recordButton->SetWindowTextW(L"Start recording");
		doneTimer = SetTimer(1, 50, NULL);
		
	}
	else
	{	
		recorder = new Recorder();
		isRecording = true;
		recordButton->SetWindowTextW(L"Stop recording");
		recorder->Start();			
		Minimize();
			
	}

}

LRESULT MainDialog::OnHotKey(WPARAM wParam, LPARAM lParam)
{	
	if(LOWORD(lParam) == recordKeyMod && HIWORD(lParam) == recordKey)
		Record();
	else if(LOWORD(lParam) == minimizeKeyMod && HIWORD(lParam) == minimizeKey)
		Minimize();		
	return 0;
}

void MainDialog::OnBnClickedHotkeyButton()
{	
	RegisterHotKeyFromControl(recordHotkey, recordHotkeyId, &recordKey, &recordKeyMod);
	RegisterHotKeyFromControl(minimizeHotkey, minimizeHotkeyId, &minimizeKey, &minimizeKeyMod);
}

BOOL MainDialog::RegisterHotKeyFromControl(CHotKeyCtrl* control, UINT id, WORD* keyStorage, WORD* keyModStorage)
{
	UnregisterHotKey(m_hWnd, id);
	control->GetHotKey(*keyStorage, *keyModStorage);
	*keyModStorage = GetKeyModifiers(*keyModStorage);		
	return RegisterHotKey(m_hWnd, id, *keyModStorage | MOD_NOREPEAT, *keyStorage);
}

WORD MainDialog::GetKeyModifiers(WORD flags)
{
	UINT modifiers = 0;
	if(flags & HOTKEYF_ALT)
		modifiers |= MOD_ALT;
	if(flags & HOTKEYF_SHIFT)
		modifiers |= MOD_SHIFT;
	if(flags & HOTKEYF_CONTROL)
		modifiers |= MOD_CONTROL;
	return modifiers;
}
#pragma endregion Recording infrastructure

#pragma region Tray

#pragma region Tray init
BOOL MainDialog::TraySetMenu(UINT nResourceID, UINT nDefaultPo)
{	
	return trayMenu.LoadMenu(nResourceID);	
}

void MainDialog::TraySetIcon(UINT nResourceID)
{	
	ASSERT(nResourceID > 0);
	HICON hIcon = 0;
	hIcon = AfxGetApp()->LoadIcon(nResourceID);
	if(hIcon)
	{
		trayIconData.hIcon = hIcon;
		//trayIconData.uFlags = NIF_ICON;
	}
}

void MainDialog::TraySetToolTip(LPCTSTR lpszToolTip)
{
	StrCpyW(trayIconData.szTip, lpszToolTip);
	trayIconData.uFlags |= NIF_MESSAGE | NIF_ICON | NIF_TIP;
}
#pragma endregion

#pragma region Tray events
LRESULT MainDialog::OnTrayNotify(WPARAM wParam,LPARAM lParam)
{ 
	UINT uID = (UINT) wParam; 
	UINT uMsg = (UINT)lParam; 
			
	if (uID != 1)
		return uID;

	CPoint cursorPosition;	
	switch (uMsg) 
	{ 
	case WM_MOUSEMOVE:
		GetCursorPos(&cursorPosition);		
		OnTrayMouseMove(cursorPosition);
		break;
	case WM_LBUTTONDOWN:
		GetCursorPos(&cursorPosition);		
		OnTrayLButtonDown(cursorPosition);
		break;
	case WM_LBUTTONDBLCLK:
		GetCursorPos(&cursorPosition);		
		OnTrayLButtonDblClk(cursorPosition);
		break;
	case WM_RBUTTONDOWN:
	case WM_CONTEXTMENU:
		GetCursorPos(&cursorPosition);	
		OnTrayRButtonDown(cursorPosition);
		break;
	} 
	TrayUpdate();
	return 1;
	
}

void MainDialog::OnTrayLButtonDown( CPoint cursorPosition )
{
	if(m_bMinimizeToTray)
		if(TrayHide())
			this->ShowWindow(SW_SHOW);
}

void MainDialog::OnTrayLButtonDblClk( CPoint cursorPosition )
{
	if(m_bMinimizeToTray)
		if(TrayHide())
			this->ShowWindow(SW_SHOW);
}

void MainDialog::OnTrayRButtonDown(CPoint cursorPosition)
{	
	UINT flags = TPM_RIGHTALIGN 
		|TPM_LEFTBUTTON 
		| TPM_BOTTOMALIGN 
		| TPM_RETURNCMD;
	trayMenu.GetSubMenu(0)->TrackPopupMenu(flags, 
		cursorPosition.x, cursorPosition.y, this);	
	
}

void MainDialog::OnTrayMouseMove( CPoint cursorPosition )
{

}
#pragma endregion

void MainDialog::Minimize()
{
	if(isHidden)
	{
		TrayHide();
		ShowWindow(SW_SHOW);		
		isHidden = false;
	}
	else
	{
		trayIconData.hWnd = m_hWnd;// it is necessary!
		TrayShow();
		ShowWindow(SW_HIDE);
		isHidden = true;
	}
}

BOOL MainDialog::TrayHide()
{
	BOOL bSuccess = FALSE;
	if(isTrayIconVisible)
	{
		bSuccess = Shell_NotifyIcon(NIM_DELETE,&trayIconData);
		if(bSuccess)
			isTrayIconVisible= FALSE;
	}
	return bSuccess;
}

BOOL MainDialog::TrayShow()
{
	BOOL bSuccess = FALSE;
	if(!isTrayIconVisible)
	{
		bSuccess = Shell_NotifyIcon(NIM_ADD,&trayIconData);
		if(bSuccess)
			isTrayIconVisible= TRUE;
	}
	return bSuccess;
}

BOOL MainDialog::TrayUpdate()
{
	BOOL bSuccess = FALSE;
	if(isTrayIconVisible)
	{
		bSuccess = Shell_NotifyIcon(NIM_MODIFY,&trayIconData);
	}
	return bSuccess;

}
#pragma endregion Minimize to tray infrastructure

void MainDialog::OnNMOutofmemoryRecordHotkey(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;
}
