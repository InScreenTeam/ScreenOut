#if !defined(AFX_TRAYDIALOG_H__24C2AEA9_B40D_11D4_BFA7_00C0DF034AED__INCLUDED_)
#define AFX_TRAYDIALOG_H__24C2AEA9_B40D_11D4_BFA7_00C0DF034AED__INCLUDED_

#pragma once

#include "..\Recorder.h"
#include "..\Configurator\Configurator.h"

using namespace ScreenOut;

#define WM_TRAY_ICON_NOTIFY_MESSAGE (WM_USER + 1)

class MainDialog : public CDialogEx
{
public:
	MainDialog(CWnd* pParent = NULL);

	enum { IDD = IDD_GUI_DIALOG };
	Configurator conf;
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	
protected:
	HICON m_hIcon;

	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnTrayNotify(WPARAM, LPARAM);
protected:
	DECLARE_MESSAGE_MAP()

	CStatic* processingLabel;
	CButton* recordButton;
	UINT doneTimer;
	Recorder *recorder;
	bool isRecording;
protected:
	afx_msg void OnBnClickedRecordButton();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
private:
	void Record();
	
private:
	CHotKeyCtrl* recordHotkey;
	WORD recordKey;
	WORD recordKeyMod;
	CHotKeyCtrl* minimizeHotkey;
	WORD minimizeKey;
	WORD minimizeKeyMod;
	UINT recordHotkeyId, minimizeHotkeyId;
private:
	BOOL RegisterHotKeyFromControl(CHotKeyCtrl* control, UINT id, WORD* keyStorage, WORD* keyModStorage);
	WORD MainDialog::GetKeyModifiers(WORD flags);
protected:
	afx_msg LRESULT OnHotKey(WPARAM, LPARAM);
	afx_msg void OnBnClickedHotkeyButton();

private:
	bool isHidden;
	BOOL isTrayIconVisible;
	NOTIFYICONDATA trayIconData;
	CMenu *trayMenu;
	UINT trayMenuDefaultItem;
private:
	BOOL TraySetMenu(UINT nResourceID,UINT nDefaultPos=0);
	BOOL TrayUpdate();
	BOOL TrayShow();
	BOOL TrayHide();
	BOOL TrayIsVisible();
	void TraySetToolTip(LPCTSTR lpszToolTip);
	void TraySetIcon(HICON hIcon);
	void TraySetIcon(UINT nResourceID);
	void Minimize();
protected:
	virtual void OnTrayLButtonDown(CPoint cursorPosition);
	virtual void OnTrayLButtonDblClk(CPoint cursorPosition);
	virtual void OnTrayRButtonDown(CPoint cursorPosition);
	virtual void OnTrayMouseMove(CPoint cursorPosition);

	afx_msg void OnNMOutofmemoryRecordHotkey(NMHDR *pNMHDR, LRESULT *pResult);
};
#endif