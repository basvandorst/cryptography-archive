//////////////////////////////////////////////////////////////////////////////
// PGPdiskPrefs.h
//
// Declarations for PGPdiskPrefs.cpp.
//////////////////////////////////////////////////////////////////////////////

// $Id: PGPdiskPrefs.h,v 1.3 1999/01/17 22:59:34 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_PGPdiskPrefs_h	// [
#define Included_PGPdiskPrefs_h

#include "DualErr.h"
#include "SystemConstants.h"


////////////
// Constants
////////////

// Shift state flags for the hotkey.
const PGPUInt8	kSHK_Control	= 0x01;
const PGPUInt8	kSHK_Alt		= 0x02;
const PGPUInt8	kSHK_Shift		= 0x04;
const PGPUInt8	kSHK_Extended	= 0x08;


////////
// Types
////////

// PGPdiskCreateScale enumerates the scale of the last create size.

enum PGPdiskCreateScale {kLCS_KbScale, kLCS_MbScale, kLCS_GbScale, 
	kLCS_InvalidScale};

struct PGPdiskWin32Prefs
{
	char	lastOpenDir[kMaxPathSize];
	char	lastSaveDir[kMaxPathSize];

	PGPUInt32			lastCreateSize;
	PGPdiskCreateScale	lastCreateScale;

	PGPBoolean	wasDialogSmall;
	PGPUInt32	mainCoords;
	PGPBoolean	mainStayOnTop;
	PGPBoolean	autoUnmount;
	PGPUInt32	hotKeyCode;
	PGPBoolean	hotKeyEnabled;
	PGPBoolean	unmountOnSleep;
	PGPBoolean	noSleepIfFail;
	PGPUInt32	unmountTimeout;
	PGPBoolean	useAsyncIo;
	PGPUInt32	betaTimeout;
	PGPUInt32	demoTimeout;

	PGPUInt8	padding[512];
};


/////////////////////
// Exported Functions
/////////////////////

DualErr	GetPGPdiskWin32Prefs(PGPdiskWin32Prefs&	prefs);
DualErr	SetPGPdiskWin32Prefs(PGPdiskWin32Prefs& prefs);

DualErr	GetPGPdiskUserName(CString& userName);
DualErr	GetPGPdiskCompanyName(CString& companyName);

DualErr	RecallWindowPos(CWnd *pWnd);
DualErr	SaveWindowPos(CWnd *pWnd);

#if PGPDISK_BETAVERSION
DualErr		SetPGPdiskBetaTimeout();
#elif PGPDISK_DEMOVERSION
DualErr		SetPGPdiskDemoTimeout();
#endif // PGPDISK_BETAVERSION

#endif	// ] Included_PGPdiskPrefs_h
