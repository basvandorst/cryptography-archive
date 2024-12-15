//////////////////////////////////////////////////////////////////////////////
// PGPdiskRegistry.h
//
// Declarations for PGPdiskRegistry.cpp.
//////////////////////////////////////////////////////////////////////////////

// $Id: PGPdiskRegistry.h,v 1.1.2.7.2.1 1998/10/05 18:28:27 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_PGPdiskRegistry_h	// [
#define Included_PGPdiskRegistry_h

#include "DualErr.h"


////////////
// Constants
////////////

const PGPUInt32	kInvalidRegDWordValue	= 0xFFFFFFFF;
const PGPUInt64	kInvalidRegQWordValue	= 0xFFFFFFFFFFFFFFFF;

// Under "HKEY_CURRENT_USER\Software\Network Associates\PGP60".

static LPCSTR	kRegistryPGP60LicenseUserKey	= "User";
static LPCSTR	kRegistryPGP60LicenseOrgKey		= "Company Name";

// Under "HKEY_CURRENT_USER\Software\Network Associates\PGPdisk".

static LPCSTR	kRegistryLicenseUserKey		= "User";
static LPCSTR	kRegistryLicenseOrgKey		= "Company Name";
static LPCSTR	kRegistryLastOpenDirKey		= "LastOpenDir";
static LPCSTR	kRegistryLastSaveDirKey		= "LastSaveDir";
static LPCSTR	kRegistryLastCreateSizeKey	= "LastCreateSize";
static LPCSTR	kRegistryLastCreateScaleKey	= "LastCreateScale";
static LPCSTR	kRegistryLastDateSplashKey	= "LastDateSplashed";
static LPCSTR	kRegistryWasDialogSmallKey	= "WasMainDialogSmall";
static LPCSTR	kRegistryMainCoordsKey		= "MainWindowCoords";
static LPCSTR	kRegistryMainStayOnTopKey	= "MainWndStayOnTop";
static LPCSTR	kRegistryAutoUnmountKey		= "AutoUnmount";
static LPCSTR	kRegistryHotKeyCodeKey		= "HotKeyCode";
static LPCSTR	kRegistryHotKeyEnabledKey	= "HotKeyEnabled";
static LPCSTR	kRegistryUnmountOnSleepKey	= "UnmountOnSleep";
static LPCSTR	kRegistryNoSleepIfFailKey	= "NoSleepIfUnmountFail";
static LPCSTR	kRegistryUnmountTimeoutKey	= "UnmountTimeout";
static LPCSTR	kRegistryPGPdiskVolsKey		= "PGPdiskVols";

#if PGP_DEBUG
static LPCSTR	kRegistryUseAsyncIOKey		= "UseAsyncIO";
#endif	// PGP_DEBUG

#if PGPDISK_BETAVERSION
static LPCSTR	kRegistryBetaTimeoutKey		= "BetaTimeout";
#elif PGPDISK_DEMOVERSION
static LPCSTR	kRegistryDemoTimeoutKey		= "DemoTimeout";
#endif // PGPDISK_BETAVERSION

// Shift state flags for the hotkey.
const PGPUInt8	kSHK_Control	= 0x01;
const PGPUInt8	kSHK_Alt		= 0x02;
const PGPUInt8	kSHK_Shift		= 0x04;
const PGPUInt8	kSHK_Extended	= 0x08;


////////
// Types
////////

// PGPdiskRegLocation is used to specify where to look for the requested
// PGPdisk preference.

enum PGPdiskRegLocation {kPRL_CurrentUser, kPRL_LocalMachine, 
	kPRL_PGP60CurrentUser};

// PGPdiskCreateScale enumerates the scale of the last create size.

enum PGPdiskCreateScale {kLCS_KbScale, kLCS_MbScale, kLCS_GbScale, 
	kLCS_InvalidScale};


/////////////////////
// Exported Functions
/////////////////////

DualErr		PGPdiskEnumSubkeys(HKEY root, LPCSTR section, 
				PGPUInt32 index, LPSTR keyName, 
				unsigned long valueSize, PGPBoolean *isIndexValid);
DualErr		PGPdiskEnumValues(HKEY root, LPCSTR section, 
				PGPUInt32 index, LPSTR valueName, 
				unsigned long valueSize, unsigned long *valueType, 
				PGPBoolean *isIndexValid);

DualErr		PGPdiskGetRawKey(HKEY root, LPCSTR section, 
				LPCSTR keyName, PGPUInt8 *buf, unsigned long bufSize);

DualErr		PGPdiskCreateAndSetRawKey(HKEY root, LPCSTR section, 
				LPCSTR keyName, const PGPUInt8 *value, 
				PGPUInt32 valueSize, PGPUInt32 dataType);

DualErr		PGPdiskDeleteSubkey(HKEY root, LPCSTR section, 
				LPCSTR keyName);
DualErr		PGPdiskDeleteValue(HKEY root, LPCSTR section, 
				LPCSTR valueName);

PGPUInt32	PGPdiskGetPrefDWord(LPCSTR regKey, PGPUInt32 defaultDWord, 
				PGPdiskRegLocation regLoc = kPRL_CurrentUser);
DualErr		PGPdiskSetPrefDWord(LPCSTR regKey, PGPUInt32 newDWord, 
				PGPdiskRegLocation regLoc = kPRL_CurrentUser);

PGPUInt64	PGPdiskGetPrefQWord(LPCSTR regKey, 
				PGPUInt64 defaultQWord, 
				PGPdiskRegLocation regLoc = kPRL_CurrentUser);
DualErr		PGPdiskSetPrefQWord(LPCSTR regKey, PGPUInt64 newQWord, 
				PGPdiskRegLocation regLoc = kPRL_CurrentUser);

void		PGPdiskGetPrefString(LPCSTR regKey, LPCSTR defaultString, 
				CString *outputString, 
				PGPdiskRegLocation regLoc = kPRL_CurrentUser);
DualErr		PGPdiskSetPrefString(LPCSTR regKey, LPCSTR newString, 
				PGPdiskRegLocation regLoc = kPRL_CurrentUser);

void		RecallWindowPos(CWnd *pWnd, LPCSTR wndCoordsKey);
void		SaveWindowPos(CWnd *pWnd, LPCSTR wndCoordsKey);

#if PGPDISK_BETAVERSION

DualErr		SetPGPdiskBetaTimeout();

#elif PGPDISK_DEMOVERSION

DualErr		SetPGPdiskDemoTimeout();

#endif // PGPDISK_BETAVERSION

#endif	// ] Included_PGPdiskRegistry_h
