#if !defined(AFX_TRAYDIALOG_H__24C2AEA9_B40D_11D4_BFA7_00C0DF034AED__INCLUDED_)
#define AFX_TRAYDIALOG_H__24C2AEA9_B40D_11D4_BFA7_00C0DF034AED__INCLUDED_

// MainDialog.h : header file
//

#pragma once

#include "..\Recorder.h"

using namespace ScreenOut;

#define WM_TRAY_ICON_NOTIFY_MESSAGE (WM_USER + 1)
// MainDialog dialog
class MainDialog : public CDialogEx
{
// Construction
public:
	MainDialog(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_GUI_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	Recorder *recorder;
	bool isRecording;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT_PTR nIDEvent);	
	afx_msg LRESULT OnTrayNotify(WPARAM, LPARAM);
protected:
	DECLARE_MESSAGE_MAP()	

	void Record();
	void Minimize();

	CStatic* processingLabel;
	CButton* recordButton;
	CHotKeyCtrl* recordHotkey;
	WORD recordKey;
	WORD recordKeyMod;
	CHotKeyCtrl* minimizeHotkey;
	WORD minimizeKey;
	WORD minimizeKeyMod;
	UINT recordHotkeyId, minimizeHotkeyId;
	UINT doneTimer;
	bool isHidden;
public:
	afx_msg void OnBnClickedRecordButton();
	afx_msg void OnBnClickedHotkeyButton();
	afx_msg LRESULT OnHotKey(WPARAM, LPARAM);
private:
	BOOL			m_bMinimizeToTray;
	BOOL			m_bTrayIconVisible;
	NOTIFYICONDATA	m_nidIconData;
	CMenu			m_mnuTrayMenu;
	UINT			m_nDefaultMenuItem;

	void TraySetMinimizeToTray(BOOL bMinimizeToTray = TRUE);
	BOOL TraySetMenu(UINT nResourceID,UINT nDefaultPos=0);	
	BOOL TraySetMenu(HMENU hMenu,UINT nDefaultPos=0);	
	BOOL TraySetMenu(LPCTSTR lpszMenuName,UINT nDefaultPos=0);	
	BOOL TrayUpdate();
	BOOL TrayShow();
	BOOL TrayHide();
	BOOL TrayIsVisible();
	void TraySetToolTip(LPCTSTR lpszToolTip);
	void TraySetIcon(HICON hIcon);
	void TraySetIcon(UINT nResourceID);
	void TraySetIcon(LPCTSTR lpszResourceName);
private:
	BOOL RegisterHotKeyFromControl(CHotKeyCtrl* control, UINT id, WORD* keyStorage, WORD* keyModStorage);
	WORD MainDialog::GetKeyModifiers(WORD flags);
protected:
	virtual void OnTrayLButtonDown(CPoint pt);
	virtual void OnTrayLButtonDblClk(CPoint pt);

	virtual void OnTrayRButtonDown(CPoint pt);
	virtual void OnTrayRButtonDblClk(CPoint pt);

	virtual void OnTrayMouseMove(CPoint pt);
public:
	afx_msg void OnNMOutofmemoryRecordHotkey(NMHDR *pNMHDR, LRESULT *pResult);
};
#endif