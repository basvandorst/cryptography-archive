//////////////////////////////////////////////////////////////////////////////
// CPGPdiskAppRegistry.cpp
//
// Registry functions specific to the app.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPGPdiskAppRegistry.cpp,v 1.16 1998/12/15 01:25:03 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "PGPdiskPrefs.h"
#include "PGPdiskRegistry.h"
#include "UtilityFunctions.h"

#include "CMainDialog.h"
#include "CPGPdiskApp.h"
#include "CPGPdiskAppRegistry.h"
#include "CGetUserInfoDialog.h"


////////////
// Constants
////////////

static LPCSTR kNoInfoString = "None";

// Places in the registry where the application path needs to go.

const HKEY kRegistryAppPathRoot = HKEY_LOCAL_MACHINE;

static LPCSTR kRegistryAppPathSection = 
	"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\PGPdisk.exe";

static LPCSTR	kRegistryAppPathCommandKey		= "";
static LPCSTR	kRegistryAppPathCommandValue	= "%s";

const HKEY		kRegistryShellNewRoot			= HKEY_CLASSES_ROOT;
static LPCSTR	kRegistryShellNewSection		= ".pgd\\ShellNew";
static LPCSTR	kRegistryShellNewCommandKey		= "Command";
static LPCSTR	kRegistryShellNewCommandValue	= "%s create \"%%2\"";

const HKEY		kRegistryIconRoot		= HKEY_CLASSES_ROOT;
static LPCSTR	kRegistryIconSection	= "PGPdisk Volume\\DefaultIcon";
static LPCSTR	kRegistryIconValue		= "%s,5";

const HKEY kRegistryShellOpenRoot = HKEY_CLASSES_ROOT;

static LPCSTR kRegistryShellOpenSection = 
	"PGPdisk Volume\\shell\\open\\command";

static LPCSTR kRegistryShellOpenValue = "%s open \"%%1\"";


////////////
// Functions
////////////

// GetPersonalizationInfo retrieves the user name and company name.

DualErr 
CPGPdiskApp::GetPersonalizationInfo()
{
	DualErr derr;

	derr = GetPGPdiskUserName(mUserText);

	if (derr.IsntError())
		derr = GetPGPdiskCompanyName(mOrgText);

	return derr;
}

// GetRegistryPrefs retrieves the PGPdisk preferences, validates them, and
// writes them out again.

DualErr 
CPGPdiskApp::GetRegistryPrefs()
{
	DualErr				derr;
	PGPdiskWin32Prefs	prefs;

	derr = GetPGPdiskWin32Prefs(prefs);

	// If prefs not found, write out default pref values.
	if (derr.IsError())
	{
		pgpClearMemory((void *) &prefs, sizeof(prefs));

		prefs.autoUnmount		= kDefaultAutoUnmount;
		prefs.hotKeyEnabled		= kDefaultHotKeyEnabled;
		prefs.unmountOnSleep	= kDefaultUnmountOnSleep;
		prefs.noSleepIfFail		= kDefaultNoSleepIfUnmountFail;
		prefs.unmountTimeout	= kDefaultUnmountTimeout;

		derr = SetPGPdiskWin32Prefs(prefs);
	}

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

	if ((mUnmountTimeout == 0) || (mUnmountTimeout > kDefaultUnmountTimeout))
		mUnmountTimeout = kDefaultUnmountTimeout;

	derr = SetPGPdiskWin32Prefs(prefs);

	return derr;
}

// SetRegistryPrefs updates the registry with the value of the app's user-
// defined preferences.

DualErr 
CPGPdiskApp::SetRegistryPrefs()
{
	DualErr				derr;
	PGPdiskWin32Prefs	prefs;

	derr = GetPGPdiskWin32Prefs(prefs);

	if (derr.IsntError())
	{
		prefs.autoUnmount		= mAutoUnmount;
		prefs.hotKeyCode		= mHotKeyCode;
		prefs.hotKeyEnabled		= mHotKeyEnabled;
		prefs.unmountOnSleep	= mUnmountOnSleep;
		prefs.noSleepIfFail		= mNoSleepIfUnmountFail;

		derr = SetPGPdiskWin32Prefs(prefs);
	}

	return derr;
}

// SetRegistryPaths updates all the places in the registry where the path to
// the PGPdisk application is stored.

DualErr 
CPGPdiskApp::SetRegistryPaths()
{
	CString	appPath, newValue;
	DualErr	derr;

	try
	{
		// Get the pathname of the application.
		if (!::GetModuleFileName(NULL, appPath.GetBuffer(kMaxStringSize), 
			kMaxStringSize))
		{
			derr = DualErr(kPGDMinorError_FindAppPathFailed, GetLastError());
		}

		appPath.ReleaseBuffer();

		if (derr.IsntError())
		{
			CString temp = appPath;

			// Must change the path to "short" form.
			GetShortPathName(temp, appPath.GetBuffer(kMaxStringSize), 
				kMaxStringSize);

			appPath.ReleaseBuffer();

			// Update the application path key.
			newValue.Format(kRegistryAppPathCommandValue, appPath);

			derr = PGPdiskCreateAndSetRawKey(kRegistryAppPathRoot, 
				kRegistryAppPathSection, kRegistryAppPathCommandKey, 
				(const PGPUInt8 *) (LPCSTR) newValue, 
				newValue.GetLength() + 1, REG_SZ);
		}

		// Update the "ShellNew" key.
		if (derr.IsntError())
		{
			newValue.Format(kRegistryShellNewCommandValue, appPath);

			derr = PGPdiskCreateAndSetRawKey(kRegistryShellNewRoot, 
				kRegistryShellNewSection, kRegistryShellNewCommandKey, 
				(const PGPUInt8 *) (LPCSTR) newValue, 
				newValue.GetLength() + 1, REG_SZ);
		}
		
		// Update the "IconSection" key.
		if (derr.IsntError())
		{
			newValue.Format(kRegistryIconValue, appPath);

			derr = PGPdiskCreateAndSetRawKey(kRegistryIconRoot, 
				kRegistryIconSection, NULL, 
				(const PGPUInt8 *) (LPCSTR) newValue, 
				newValue.GetLength() + 1, REG_SZ);
		}

		// Update the "ShellOpen" key.
		if (derr.IsntError())
		{
			newValue.Format(kRegistryShellOpenValue, appPath);

			derr = PGPdiskCreateAndSetRawKey(kRegistryShellOpenRoot, 
				kRegistryShellOpenSection, NULL, 
				(const PGPUInt8 *) (LPCSTR) newValue, 
				newValue.GetLength() + 1, REG_SZ);
		}
	}
	catch (CMemoryException *ex)
	{
		derr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}

	return derr;
}
