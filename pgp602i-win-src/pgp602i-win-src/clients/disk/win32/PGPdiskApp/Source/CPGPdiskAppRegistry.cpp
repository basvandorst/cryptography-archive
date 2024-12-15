//////////////////////////////////////////////////////////////////////////////
// CPGPdiskAppRegistry.cpp
//
// Registry functions specific to the app.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPGPdiskAppRegistry.cpp,v 1.1.2.22.2.1 1998/10/05 18:28:22 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
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

// GetPersonalizationInfo retrieves the user nam and org from the registry, 
// and if any of this info is missing, the user is asked to re-enter it.

DualErr 
CPGPdiskApp::GetPersonalizationInfo()
{
	DualErr derr;

	try
	{
#if PGPDISK_PUBLIC_KEY

		// Get the licensing name.
		PGPdiskGetPrefString(kRegistryPGP60LicenseUserKey, kNoInfoString, 
			&mUserText, kPRL_PGP60CurrentUser);

		// Get and set the licensing org.
		PGPdiskGetPrefString(kRegistryPGP60LicenseOrgKey, kNoInfoString, 
			&mOrgText, kPRL_PGP60CurrentUser);

#else	// !PGPDISK_PUBLIC_KEY

		// Get the licensing name.
		PGPdiskGetPrefString(kRegistryLicenseUserKey, kNoInfoString, 
			&mUserText);

		// Get and set the licensing org.
		PGPdiskGetPrefString(kRegistryLicenseOrgKey, kNoInfoString, 
			&mOrgText);

#endif	// PGPDISK_PUBLIC_KEY

		// If any of these keys have been lost, ask the user to re-enter all
		// of the information.

		if ((mUserText == kNoInfoString) || 
			(mOrgText == kNoInfoString))
		{
			CGetUserInfoDialog	getUserInfoDialog;
			DualErr				dialogError;

			dialogError = getUserInfoDialog.AskForUserInfo();

			// The user is only allowed to hit OK if he entered something.
			if (dialogError.IsntError())
			{
				mUserText	= getUserInfoDialog.mUserText;
				mOrgText	= getUserInfoDialog.mOrgText;

				derr = SetPersonalizationInfo();
			}

			// If he cancelled or the set failed, show 'None' to the user in
			// all fields, not just the one that had the missing key. Don't
			// set them in the registry, however, since we want PGPdisk to
			// ask him again next time it starts.

			if (dialogError.IsError() || derr.IsError())
			{
				mUserText	= kNoInfoString;
				mOrgText	= kNoInfoString;
			}
		}
	}
	catch (CMemoryException *ex)
	{
		derr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}

	return derr;
}

// SetPersonalizationInfo updates the registry with new registration
// information.

DualErr 
CPGPdiskApp::SetPersonalizationInfo()
{
	DualErr derr;

#if PGPDISK_PUBLIC_KEY

	// Set the licensing name.
	derr = PGPdiskSetPrefString(kRegistryPGP60LicenseUserKey, mUserText, 
		kPRL_PGP60CurrentUser);

	// Set the organization name.
	if (derr.IsntError())
	{
		derr = PGPdiskSetPrefString(kRegistryPGP60LicenseOrgKey, mOrgText, 
			kPRL_PGP60CurrentUser);
	}

#else	// !PGPDISK_PUBLIC_KEY

	// Set the licensing name.
	derr = PGPdiskSetPrefString(kRegistryLicenseUserKey, mUserText);

	// Set the organization name.
	if (derr.IsntError())
	{
		derr = PGPdiskSetPrefString(kRegistryLicenseOrgKey, mOrgText);
	}

#endif	// PGPDISK_PUBLIC_KEY

	return derr;
}

// GetRegistryPrefs retrieves the PGPdisk preferences from the registry,
// validates them, and writes them out again.

DualErr 
CPGPdiskApp::GetRegistryPrefs()
{
	DualErr		derr;
	PGPUInt32	temp;

	// Get the value for the 'Auto-Unmount' key.
	temp = PGPdiskGetPrefDWord(kRegistryAutoUnmountKey, 
		kInvalidRegDWordValue);

	if (temp == FALSE)
		mAutoUnmount = FALSE;
	else if (temp == TRUE)
		mAutoUnmount = TRUE;
	else
		mAutoUnmount = kDefaultAutoUnmount;

	// Get the value for the 'HotKeyCode' key.
	mHotKeyCode = (PGPUInt16) PGPdiskGetPrefDWord(kRegistryHotKeyCodeKey, 
		kInvalidRegDWordValue);

	// Get the value for the 'HotKeyEnabled' key.
	temp = PGPdiskGetPrefDWord(kRegistryHotKeyEnabledKey, 
		kInvalidRegDWordValue);
	
	if (temp == FALSE)
		mHotKeyEnabled = FALSE;
	else if (temp == TRUE)
		mHotKeyEnabled = TRUE;
	else
		mHotKeyEnabled = kDefaultHotKeyEnabled;

	// Get the value for the 'UnmountOnSleep' key.
	temp = PGPdiskGetPrefDWord(kRegistryUnmountOnSleepKey, 
		kInvalidRegDWordValue);
	
	if (temp == FALSE)
		mUnmountOnSleep = FALSE;
	else if (temp == TRUE)
		mUnmountOnSleep = TRUE;
	else
		mUnmountOnSleep = kDefaultUnmountOnSleep;

	// Get the value for the 'NoSleepIfUnmountFail' key.
	temp = PGPdiskGetPrefDWord(kRegistryNoSleepIfFailKey, 
		kInvalidRegDWordValue);
	
	if (temp == FALSE)
		mNoSleepIfUnmountFail = FALSE;
	else if (temp == TRUE)
		mNoSleepIfUnmountFail = TRUE;
	else
		mNoSleepIfUnmountFail = kDefaultNoSleepIfUnmountFail;

	// Previous two options not available on NT4.
	if (IsWinNT4CompatibleMachine() && !IsWinNT5CompatibleMachine())
	{
		mUnmountOnSleep = mNoSleepIfUnmountFail = FALSE;
	}

	// Get the value for the 'UnmountTimeout' key.
	temp = PGPdiskGetPrefDWord(kRegistryUnmountTimeoutKey, 
		kMaxUnmountTimeout + 1);
	
	if ((temp == FALSE) || (temp > kMaxUnmountTimeout))
		mUnmountTimeout = kDefaultUnmountTimeout;
	else
		mUnmountTimeout = temp;

	// Now write out the preferences to the registry.
	derr = SetRegistryPrefs();

	return derr;
}

// SetRegistryPrefs updates the registry with the value of the app's user-
// defined preferences.

DualErr 
CPGPdiskApp::SetRegistryPrefs()
{
	DualErr	derr;

	derr = PGPdiskSetPrefDWord(kRegistryAutoUnmountKey, mAutoUnmount);

	if (derr.IsntError())
	{
		derr = PGPdiskSetPrefDWord(kRegistryHotKeyCodeKey, mHotKeyCode);
	}

	if (derr.IsntError())
	{
		derr = PGPdiskSetPrefDWord(kRegistryHotKeyEnabledKey, mHotKeyEnabled);
	}

	if (derr.IsntError())
	{
		derr = PGPdiskSetPrefDWord(kRegistryUnmountOnSleepKey, 
			mUnmountOnSleep);
	}

	if (derr.IsntError())
	{
		derr = PGPdiskSetPrefDWord(kRegistryNoSleepIfFailKey, 
			mNoSleepIfUnmountFail);
	}

	if (derr.IsntError())
	{
		derr = PGPdiskSetPrefDWord(kRegistryUnmountTimeoutKey, 
			mUnmountTimeout);
	}

#if	PGP_DEBUG
#if	PGPDISK_ENABLEDEBUGMENU
	if (derr.IsntError())
	{
		derr = PGPdiskSetPrefDWord(kRegistryUseAsyncIOKey, mUseAsyncIO);
	}
#endif	// PGPDISK_ENABLEDEBUGMENU
#endif	// PGP_DEBUG

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
