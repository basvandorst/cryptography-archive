//////////////////////////////////////////////////////////////////////////////
// CHiddenWindow.cpp
//
// Implementation of class CHiddenWindow.
//////////////////////////////////////////////////////////////////////////////

// $Id: CHiddenWindow.cpp,v 1.8 1999/05/26 00:19:45 heller Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"
#include <Pbt.h>

#include "Required.h"
#include "CommonStrings.h"
#include "DriverComm.h"
#include "DualErr.h"
#include "PGPdiskPrefs.h"
#include "PGPdiskResidentDefines.h"
#include "StringAssociation.h"
#include "UtilityFunctions.h"
#include "WindowsVersion.h"

#include "CHiddenWindow.h"
#include "CPGPdiskResidentApp.h"
#include "Globals.h"


////////////
// Constants
////////////

PGPUInt16 kPGPdiskResAppHotKeyId = 0;


///////////////////////////
// MFC specific definitions
///////////////////////////

// MFC message map

BEGIN_MESSAGE_MAP(CHiddenWindow, CWnd)
	//{{AFX_MSG_MAP(CHiddenWindow)
	ON_MESSAGE(WM_ENDSESSION, OnEndSession)
	ON_MESSAGE(WM_HOTKEY, OnHotKey)
	ON_MESSAGE(WM_PGPDISKRES_NEWPREFS, OnNewPrefs)
	ON_MESSAGE(WM_POWERBROADCAST, OnPowerBroadcast)
	ON_WM_CLOSE()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////
// CHiddenWindow public custom functions and non-default message handlers
/////////////////////////////////////////////////////////////////////////

// Default constructor for CHiddenWindow.

CHiddenWindow::CHiddenWindow()
{
	mIsHotKeyRegistered = FALSE;
	mTellUserAboutFail = FALSE;
}

// Default destructor for CHiddenWindow.

CHiddenWindow::~CHiddenWindow()
{
}


////////////////////////////////////////////////////////////////////////////
// CHiddenWindow protected custom functions and non-default message handlers
////////////////////////////////////////////////////////////////////////////

// OnEndSession is called when the system is shutting down or the user is
// logging off. Notify the driver in the latter case.

void 
CHiddenWindow::OnEndSession(WPARAM wParam, LPARAM lParam)
{
	DualErr derr;

	if (((BOOL) wParam) && (lParam = ENDSESSION_LOGOFF))
		NotifyUserLogoff();
}

// OnHotKey is called when our unmount hot key is pressed.

void 
CHiddenWindow::OnHotKey(WPARAM wParam, LPARAM lParam)
{
	DualErr derr;

	if (wParam != kPGPdiskResAppHotKeyId)
		return;

	derr = UnmountAllPGPdisks();

	if (derr.IsError())
	{
		ReportError(kPGDMajorError_PGPdiskUnmountAllFailed, derr);
	}
}

// OnNewPrefs is called when the application tells us it has updated the
// PGPdisk preferences.

void 
CHiddenWindow::OnNewPrefs(WPARAM wParam, LPARAM lParam)
{
	if (wParam != kPGPdiskMessageMagic)		// prevent collisions
		return;

	// Update our preferences.
	UpdatePrefs();

	// Update the hot key;
	UpdateHotKey();
}

// We handle OnPowerBroadcast to see sleep events.

int 
CHiddenWindow::OnPowerBroadcast(WPARAM wParam, LPARAM lParam)
{
	DualErr				derr;
	PGPBoolean			denyThisMessage			= FALSE;
	static PGPBoolean	sawASleepRequest		= FALSE;
	static PGPBoolean	failAllSleepRequests	= FALSE;

	// Windows will send us multiple sleep requests, but we will only process
	// the first one.

	switch (wParam)
	{
	case PBT_APMQUERYSUSPEND:
		if (sawASleepRequest)
		{
			denyThisMessage = failAllSleepRequests;
		}
		else
		{
			PGPBoolean canWarnUser = (lParam & 1 > 0 ? TRUE : FALSE);

			sawASleepRequest = TRUE;

			if (mUnmountOnSleep)
			{
				derr = UnmountAllPGPdisks();

				// Fail the sleep if error and the preference was set.
				if (derr.IsError() && mNoSleepIfUnmountFail)
				{
					denyThisMessage			= TRUE;
					failAllSleepRequests	= TRUE;

					if (canWarnUser)
						mTellUserAboutFail = TRUE;
				}
			}
		}
		break;

	case PBT_APMQUERYSUSPENDFAILED:
		if (mTellUserAboutFail)
		{
			mTellUserAboutFail = FALSE;
			ReportError(kPGDMajorError_NoSleepOnUnmountFailure);
		}

		sawASleepRequest		= FALSE;
		failAllSleepRequests	= FALSE;
		break;

	case PBT_APMSUSPEND:
		sawASleepRequest		= FALSE;
		failAllSleepRequests	= FALSE;
		break;
	}	

	if (denyThisMessage)
		return BROADCAST_QUERY_DENY;
	else
		return TRUE;
}

// UnmountAllPGPdisks unmounts all PGPdisks.

DualErr	
CHiddenWindow::UnmountAllPGPdisks()
{
	DualErr	derr;
	DualErr	storedDerr;

	PGPUInt8	i;
	PGPUInt32	drives;

	drives = GetLogicalDrives();

	// For every drive...
	for (i = 0; i < kMaxDrives; i++)
	{
		if (drives & (1 << i))
		{
			PGPBoolean isVolumeAPGPdisk = FALSE;

			//... ask if it's a mounted PGPdisk.
			IsVolumeAPGPdisk(i, &isVolumeAPGPdisk);

			// If so, unmount it.
			if (isVolumeAPGPdisk)
			{
				AD_Unmount UNMNT;

				UNMNT.drive = i;
				UNMNT.isThisEmergency = FALSE;

				derr = SendUnmountRequest(&UNMNT);

				if (derr.IsError() && storedDerr.IsntError())
					storedDerr = derr;
			}
		}

		if (derr.IsError())
			break;
	}

	derr = storedDerr;
	return derr;
}

// UpdateHotKey registeres or deregisters the hotkey as necessary.

void 
CHiddenWindow::UpdateHotKey()
{
	DualErr derr;

	// First deregister the current hot key.
	if (mIsHotKeyRegistered)
	{
		if (!UnregisterHotKey(m_hWnd, kPGPdiskResAppHotKeyId))
			derr = DualErr(kPGDMinorError_UnregisterHotKeyFailed);

		mIsHotKeyRegistered = FALSE;
	}

	// Now re-enable the hotkey with the new information if we should.
	if (derr.IsntError() && mHotKeyEnabled)
	{
		PGPUInt8	primaryVKey, modKeyState;
		PGPUInt32	hotKeyModKeyState;

		primaryVKey = GetLowByte(mHotKeyCode);
		modKeyState = GetHighByte(mHotKeyCode);

		hotKeyModKeyState = NULL;

		if (modKeyState & kSHK_Alt)
			hotKeyModKeyState |= MOD_ALT;

		if (modKeyState & kSHK_Control)
			hotKeyModKeyState |= MOD_CONTROL;

		if (modKeyState & kSHK_Shift)
			hotKeyModKeyState |= MOD_SHIFT;

		if (!RegisterHotKey(m_hWnd, kPGPdiskResAppHotKeyId, 
			hotKeyModKeyState, primaryVKey))
		{
			derr = DualErr(kPGDMinorError_RegisterHotKeyFailed);
		}

		mIsHotKeyRegistered = derr.IsntError();
	}

	if (derr.IsError())
		ReportError(kPGDMajorError_PGPdiskResHotKeyOpFailed, derr);
}

// UpdatePrefs updates our prefs from the registry.

void 
CHiddenWindow::UpdatePrefs()
{
	PGPdiskWin32Prefs	prefs;

	if (GetPGPdiskWin32Prefs(prefs).IsntError())
	{

		mAutoUnmount			= prefs.autoUnmount;
		mHotKeyCode				= prefs.hotKeyCode;
		mHotKeyEnabled			= prefs.hotKeyEnabled;
		mUnmountOnSleep			= prefs.unmountOnSleep;
		mNoSleepIfUnmountFail	= prefs.noSleepIfFail;

		// Previous two options not available on NT4.
		if (IsWinNT4CompatibleMachine() && !IsWinNT5CompatibleMachine())
		{
			mUnmountOnSleep = mNoSleepIfUnmountFail = FALSE;
		}

		mUnmountTimeout = prefs.unmountTimeout;

		if (mUnmountTimeout > kDefaultUnmountTimeout)
			mUnmountTimeout = kDefaultUnmountTimeout;
	}
}


///////////////////////////////////////////////////
// CHiddenWindow protected default message handlers
///////////////////////////////////////////////////

// OnClose is called when the user is trying to kill us in Windows 95. Warn
// him.

void 
CHiddenWindow::OnClose() 
{
	UserResponse button;

	button = DisplayMessage(kPGPdiskResConfirmPGPdiskResQuit, 
		kPMBS_YesNo, kPMBF_NoButton);

	if (button == kUR_Yes)
		CWnd::OnClose();
}

// OnCreate is called when we're being created.

int 
CHiddenWindow::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// Update our preferences.
	UpdatePrefs();

	// Update the hot key;
	UpdateHotKey();

	return 0;
}
