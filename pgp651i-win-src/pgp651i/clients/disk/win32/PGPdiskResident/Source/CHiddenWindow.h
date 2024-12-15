//////////////////////////////////////////////////////////////////////////////
// CHiddenWindow.h
//
// Declaration of class CHiddenWindow..
//////////////////////////////////////////////////////////////////////////////

// $Id: CHiddenWindow.h,v 1.6 1999/05/26 00:19:46 heller Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CHiddenWindow_h // [
#define Included_CHiddenWindow_h

#include "DualErr.h"


//////////////////////
// Class CHiddenWindow
//////////////////////

class CHiddenWindow : public CWnd
{
public:
			CHiddenWindow();
	virtual	~CHiddenWindow();

	//{{AFX_VIRTUAL(CHiddenWindow)
	//}}AFX_VIRTUAL

protected:
	PGPBoolean	mAutoUnmount;				// auto-unmount enabled?
	PGPBoolean	mHotKeyEnabled;				// unmount hotkey enabled?
	PGPUInt16	mHotKeyCode;				// hot key codes
	PGPBoolean	mUnmountOnSleep;			// unmount on sleep?
	PGPBoolean	mNoSleepIfUnmountFail;		// no sleep unmount fails?
	PGPUInt32	mUnmountTimeout;			// mins b4 auto-unmount
	PGPBoolean	mTellUserAboutFail;

	PGPBoolean	mIsHotKeyRegistered;		// hot key currently registered?

	//{{AFX_MSG(CHiddenWindow)
	afx_msg void OnEndSession(WPARAM wParam, LPARAM lParam);
	afx_msg void OnNewPrefs(WPARAM wParam, LPARAM lParam);
	afx_msg void OnHotKey(WPARAM wParam, LPARAM lParam);
	afx_msg int OnPowerBroadcast(WPARAM wParam, LPARAM lParam);
	afx_msg void OnClose();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG

	DualErr	UnmountAllPGPdisks();
	void	UpdateHotKey();
	void	UpdatePrefs();

	DECLARE_MESSAGE_MAP()
};

#endif // Included_CHiddenWindow_h
