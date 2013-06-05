#include "stdafx.h"
#include "afxdialogex.h"
#include "GUI.h"
#include "MainDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
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


//
// MainDialog dialog
//
MainDialog::MainDialog(CWnd* pParent /*=NULL*/): CDialogEx(MainDialog::IDD, pParent)
{
	m_nidIconData.hWnd				= m_hWnd;
	m_hIcon							= AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_nidIconData.cbSize			= sizeof(NOTIFYICONDATA);
	m_nidIconData.uID				= 1;
	m_nidIconData.uCallbackMessage	= WM_TRAY_ICON_NOTIFY_MESSAGE;
	m_nidIconData.hIcon				= 0;
	m_nidIconData.szTip[0]			= 0;	
	m_nidIconData.uFlags			=  NIF_ICON | NIF_MESSAGE | NIF_TIP;
	
	m_bTrayIconVisible				= FALSE;
	m_nDefaultMenuItem				= 0;
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


// MainDialog message handlers

BOOL MainDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

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
	

	return TRUE;  // return TRUE  unless you set the focus to a control
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void MainDialog::OnPaint()
{	
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR MainDialog::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT MainDialog::OnTrayNotify(WPARAM wParam,LPARAM lParam)
{ 
	UINT uID; 
	UINT uMsg; 

	uID = (UINT) wParam; 
	uMsg = (UINT)lParam; 
			
	if (uID != 1)
		return uID;

	CPoint pt;	

	switch (uMsg ) 
	{ 
	case WM_MOUSEMOVE:
		GetCursorPos(&pt);
		ClientToScreen(&pt);
		OnTrayMouseMove(pt);
		break;
	case WM_LBUTTONDOWN:
		GetCursorPos(&pt);
		ClientToScreen(&pt);
		OnTrayLButtonDown(pt);
		break;
	case WM_LBUTTONDBLCLK:
		GetCursorPos(&pt);
		ClientToScreen(&pt);
		OnTrayLButtonDblClk(pt);
		break;

	case WM_RBUTTONDOWN:
	case WM_CONTEXTMENU:
		GetCursorPos(&pt);
		ClientToScreen(&pt);
		OnTrayRButtonDown(pt);
		break;
	case WM_RBUTTONDBLCLK:
		GetCursorPos(&pt);
		ClientToScreen(&pt);
		OnTrayRButtonDblClk(pt);
		break;
	} 
	TrayUpdate();
	return 1;
	
} 




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
	return RegisterHotKey(m_hWnd, id, *keyModStorage, *keyStorage);
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

void MainDialog::OnTrayLButtonDown( CPoint pt )
{
	if(m_bMinimizeToTray)
		if(TrayHide())
			this->ShowWindow(SW_SHOW);
}

void MainDialog::OnTrayLButtonDblClk( CPoint pt )
{
	if(m_bMinimizeToTray)
		if(TrayHide())
			this->ShowWindow(SW_SHOW);
}

void MainDialog::OnTrayRButtonDown(CPoint pt)
{	
	BOOL x = m_mnuTrayMenu.TrackPopupMenu(TPM_RIGHTALIGN |TPM_LEFTBUTTON | TPM_BOTTOMALIGN, 
		pt.x - 300, pt.y-100, this);	
	
}

void MainDialog::OnTrayRButtonDblClk( CPoint pt )
{
	

}

void MainDialog::OnTrayMouseMove( CPoint pt )
{

}




BOOL MainDialog::TraySetMenu( UINT nResourceID,UINT nDefaultPo)
{
	BOOL bSuccess;
	bSuccess = m_mnuTrayMenu.LoadMenu(nResourceID);	
	return bSuccess;

}

BOOL MainDialog::TraySetMenu( HMENU hMenu,UINT nDefaultPos )
{

	m_mnuTrayMenu.Attach(hMenu);
	return TRUE;
}

BOOL MainDialog::TraySetMenu( LPCTSTR lpszMenuName,UINT nDefaultPos )
{
	BOOL bSuccess;
	bSuccess = m_mnuTrayMenu.LoadMenu(lpszMenuName);
	return bSuccess;

}

void MainDialog::TraySetIcon( UINT nResourceID )
{	
	ASSERT(nResourceID>0);
	HICON hIcon = 0;
	hIcon = AfxGetApp()->LoadIcon(nResourceID);
	if(hIcon)
	{
		m_nidIconData.hIcon = hIcon;
		//m_nidIconData.uFlags = NIF_ICON;
	}
	else
	{
		TRACE0("FAILED TO LOAD ICON\n");
	}
}

void MainDialog::TraySetToolTip( LPCTSTR lpszToolTip )
{
	//ASSERT(strlen(lpszToolTip) > 0 && strlen(lpszToolTip) < 64);
	
	StrCpyW(m_nidIconData.szTip,lpszToolTip);
	m_nidIconData.uFlags |=NIF_MESSAGE | NIF_ICON | NIF_TIP;
}

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
		m_nidIconData.hWnd = m_hWnd;// it is necessary!
		TrayShow();
		ShowWindow(SW_HIDE);
		isHidden = true;
	}
}

BOOL MainDialog::TrayHide()
{
	BOOL bSuccess = FALSE;
	if(m_bTrayIconVisible)
	{
		bSuccess = Shell_NotifyIcon(NIM_DELETE,&m_nidIconData);
		if(bSuccess)
			m_bTrayIconVisible= FALSE;
	}
	else
	{
		TRACE0("ICON ALREADY HIDDEN");
	}
	return bSuccess;
}

BOOL MainDialog::TrayShow()
{
	BOOL bSuccess = FALSE;
	if(!m_bTrayIconVisible)
	{
		bSuccess = Shell_NotifyIcon(NIM_ADD,&m_nidIconData);
		if(bSuccess)
			m_bTrayIconVisible= TRUE;
	}
	else
	{
		TRACE0("ICON ALREADY VISIBLE");
	}
	return bSuccess;
}

BOOL MainDialog::TrayUpdate()
{
	BOOL bSuccess = FALSE;
	if(m_bTrayIconVisible)
	{
		bSuccess = Shell_NotifyIcon(NIM_MODIFY,&m_nidIconData);
	}
	else
	{
		TRACE0("ICON NOT VISIBLE");
	}
	return bSuccess;

}

void MainDialog::TraySetMinimizeToTray( BOOL bMinimizeToTray /*= TRUE*/ )
{

}

BOOL MainDialog::TrayIsVisible()
{
	return 1;
}





void MainDialog::OnNMOutofmemoryRecordHotkey(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;
}
