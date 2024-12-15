//////////////////////////////////////////////////////////////////////////////
// CMainDialog.h
//
// Declaration of class CMainDialog.
//////////////////////////////////////////////////////////////////////////////

// $Id: CMainDialog.h,v 1.1.2.10.2.1 1998/10/22 22:27:43 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CMainDialog_h	// [
#define Included_CMainDialog_h

#include "DualErr.h"

#include "Resource.h"


////////////
// Constants
////////////

const PGPUInt16 IDM_STAYONTOP	= 4243; 		// extra menu item
const PGPUInt16 IDM_REMOVEALL	= 4242; 		// extra menu item


////////////////////
// Class CMainDialog
////////////////////

// Class CMainDialog is the class that displays and handles the main dialog
// box for the dialog-box based Umbra application.

class CMainDialog : public CDialog
{
public:
	DualErr mInitErr;

	CMainDialog(CWnd *pParent = NULL);
	~CMainDialog();

	//{{AFX_DATA(CMainDialog)
	enum { IDD = IDD_MAIN_DLG };
	CButton mUnmountButton;
	CButton mMountButton;
	CButton mPrefsButton;
	CButton mNewButton;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CMainDialog)
	public:
	virtual BOOL PreTranslateMessage(MSG *pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange *pDX);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

protected:
	CMenu		mMainMenu;				// main menu

	#if PGP_DEBUG
	#if PGPDISK_ENABLEDEBUGMENU

	CMenu		mDebugMenu; 			// special debug menu

	#endif	// PGPDISK_ENABLEDEBUGMENU
	#endif	// PGP_DEBUG

	CPalette	mMainLogoPalette;		// palette for main logo

	HACCEL		mAccelTable;			// accelerator table
	HICON		mHIcon; 				// an icon

	HICON		mMountBigIcon;			// big mount button icon
	HICON		mNewBigIcon;			// big new button icon
	HICON		mPrefsBigIcon;			// big prefs button icon
	HICON		mUnmountBigIcon;		// big unmount button icon

	HICON		mMountSmallIcon;		// small mount button icon
	HICON		mNewSmallIcon;			// small new button icon
	HICON		mPrefsSmallIcon;		// small prefs button icon
	HICON		mUnmountSmallIcon;		// small unmount button icon

	PGPBoolean	mAreWeInLargeMode;		// large main window?
	PGPBoolean	mArePastInitDialog; 	// received WM_INITDIALOG yet?

	inline PGPBoolean	HasFocus(CButton *button)
		{return (button->GetState() & 0x0008 ? TRUE : FALSE);};

	DualErr 			ShowMainMenu();
	void				HideMainMenu(); 

	void				ConvertMainDialogToLarge();
	void				ConvertMainDialogToSmall();

	void				RecallMainWndOnTop();
	void				SaveMainWndOnTop();

	//{{AFX_MSG(CMainDialog)
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnAboutBox();
	afx_msg void OnAddPassphrase();
	afx_msg void OnChangePassphrase();
	afx_msg void OnDropFiles(HDROP hDrop);
	afx_msg void OnRemoveAlternates();
	afx_msg void OnRemovePassphrase();
	afx_msg void OnExit();
	afx_msg void OnMountPGPdisk();
	afx_msg void OnUnmountPGPdisk();
	afx_msg void OnNewPGPdisk();
	afx_msg void OnPreferences();
	afx_msg void OnAddRemovePublicKeys();
	afx_msg void OnGlobalConvert();
	afx_msg void OnHelpContents();
	afx_msg void OnClose();
	afx_msg void OnInitMenu(CMenu *pMenu);
	afx_msg BOOL OnHelpInfo(HELPINFO *pHelpInfo);
	afx_msg void OnSysCommand(UINT nId, LONG lParam);
	virtual BOOL OnInitDialog();
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif // ] Included_CMainDialog_h
