//////////////////////////////////////////////////////////////////////////////
// PGPdiskPrefs.cpp
//
// Functions for accessing PGPdisk preferences.
//////////////////////////////////////////////////////////////////////////////

// $Id: PGPdiskPrefs.cpp,v 1.3 1999/02/26 04:09:59 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#if defined(PGPDISK_MFC)

#include "StdAfx.h"

#include "PGPclx.h"
#include "pflPrefs.h"
#include "pflPrefTypes.h"
#include "pgpErrors.h"

#else
#error Define PGPDISK_MFC.
#endif	// PGPDISK_MFC

#include "Required.h"
#include "GlobalPGPContext.h"
#include "PGPdiskPrefs.h"
#include "UtilityFunctions.h"


////////////////////////
// Preferences Functions
////////////////////////

DualErr	
GetPGPdiskWin32Prefs(PGPdiskWin32Prefs&	prefs)
{
	DualErr	derr;
	PGPMemoryMgrRef	memoryMgr	= 
		PGPGetContextMemoryMgr(GetGlobalPGPContext());
	PGPPrefRef		clientPrefsRef;

	derr = PGPclOpenClientPrefs(memoryMgr, &clientPrefsRef);

	if (derr.IsntError())
	{
		PGPdiskWin32Prefs	*gottenPrefs;
		PGPSize				prefsSize;

		derr = PGPGetPrefData(clientPrefsRef, 
			kPGPPrefPGPdiskWin32PrivateData, &prefsSize, 
			(void **) &gottenPrefs);

		if (derr.IsntError())
		{
			prefs = *gottenPrefs;
			PGPDisposePrefData(clientPrefsRef, gottenPrefs);
		}

		PGPclCloseClientPrefs(clientPrefsRef, FALSE);
	}

	return derr;
}

DualErr 
SetPGPdiskWin32Prefs(PGPdiskWin32Prefs& prefs)
{
	DualErr			derr;
	PGPMemoryMgrRef	memoryMgr	= 
		PGPGetContextMemoryMgr(GetGlobalPGPContext());
	PGPPrefRef		clientPrefsRef;

	derr = PGPclOpenClientPrefs(memoryMgr, &clientPrefsRef);

	if (derr.IsntError())
	{
		derr = PGPSetPrefData(clientPrefsRef, 
			kPGPPrefPGPdiskWin32PrivateData, sizeof(prefs), &prefs);

		PGPclCloseClientPrefs(clientPrefsRef, TRUE);
	}	

	return derr;
}

DualErr 
GetPGPdiskUserName(CString& userName)
{
	DualErr			derr;
	PGPMemoryMgrRef	memoryMgr	= 
		PGPGetContextMemoryMgr(GetGlobalPGPContext());
	PGPPrefRef		clientPrefsRef;

	derr = PGPclOpenClientPrefs(memoryMgr, &clientPrefsRef);

	if (derr.IsntError())
	{
		char	strBuf[64];

		derr = PGPGetPrefStringBuffer(clientPrefsRef, 
			kPGPPrefOwnerName, sizeof(strBuf), strBuf);

		if (derr.IsntError())
			userName = strBuf;

		PGPclCloseClientPrefs(clientPrefsRef, FALSE);
	}

	return derr;
}

DualErr 
GetPGPdiskCompanyName(CString& companyName)
{
	DualErr			derr;
	PGPMemoryMgrRef	memoryMgr	= 
		PGPGetContextMemoryMgr(GetGlobalPGPContext());
	PGPPrefRef		clientPrefsRef;

	derr = PGPclOpenClientPrefs(memoryMgr, &clientPrefsRef);

	if (derr.IsntError())
	{
		char	strBuf[64];

		derr = PGPGetPrefStringBuffer(clientPrefsRef, 
			kPGPPrefCompanyName, sizeof(strBuf), strBuf);

		if (derr.IsntError())
			companyName = strBuf;

		PGPclCloseClientPrefs(clientPrefsRef, FALSE);
	}

	return derr;
}

// RecallWindowPos recalls the position of a given window from the given
// registry key.

DualErr 
RecallWindowPos(CWnd *pWnd)
{
	DualErr				derr;
	PGPdiskWin32Prefs	prefs;
	PGPUInt32			coords;

	pgpAssertAddrValid(pWnd, CWnd);

	derr = GetPGPdiskWin32Prefs(prefs);

	if (derr.IsntError())
	{
		coords = prefs.mainCoords;

		// Move the window.
		PGPUInt16 x = GetLowWord(coords);
		PGPUInt16 y = GetHighWord(coords);

		if ((x >= 0) && (y >= 0) &&
			(x < GetSystemMetrics(SM_CXSCREEN)) &&
			(y < GetSystemMetrics(SM_CYSCREEN)))
		{
			pWnd->SetWindowPos(&CWnd::wndTop, x, y, 0, 0, 
				SWP_NOSIZE | SWP_NOZORDER);
		}
	}

	return derr;
}

// SaveWindowPos saves the position of a given window in the given registry
// key.

DualErr 
SaveWindowPos(CWnd *pWnd)
{
	DualErr				derr;
	PGPdiskWin32Prefs	prefs;
	PGPUInt32			coords;
	RECT				rect;

	pgpAssertAddrValid(pWnd, Wnd);

	derr = GetPGPdiskWin32Prefs(prefs);

	if (derr.IsntError())
	{
		pWnd->GetWindowRect(&rect);
		coords = MakeLong((PGPUInt16) rect.left, (PGPUInt16) rect.top);

		prefs.mainCoords = coords;

		derr = SetPGPdiskWin32Prefs(prefs);
	}

	return derr;
}

#if PGPDISK_BETAVERSION

// HasBetaTimedOut returns TRUE if this beta version of PGPdisk has timed
// out, FALSE otherwise.

PGPBoolean 
HasBetaTimedOut()
{
	DualErr				derr;
	PGPdiskWin32Prefs	prefs;
	PGPUInt64			currentTime, timeoutTime;

	derr = GetPGPdiskWin32Prefs(prefs);

	if (derr.IsntError())
	{
		// Get the current time.
		currentTime = GetSecondsSince1970();

		// Get the timeout time.
		timeoutTime = prefs.betaTimeout;

		// If we've exceeded the timeout data, return TRUE.
		return (currentTime >= timeoutTime);
	}

	return FALSE;
}

// SetPGPdiskBetaTimeout burns a beta timeout date into the registry, but
// only if no date is there already.

DualErr 
SetPGPdiskBetaTimeout()
{
	DualErr				derr;
	PGPdiskWin32Prefs	prefs;
	PGPUInt64			timeoutTime;

	derr = GetPGPdiskWin32Prefs(prefs);

	if (derr.IsntError())
	{
		timeoutTime = prefs.betaTimeout;

		// If no demo timeout has been set, set it now.
		if (timeoutTime == 0)
		{
			FILETIME	fileTimeoutTime;
			SYSTEMTIME	sysTimeoutTime;

			// Prepare data structure with target timeout date.
			pgpClearMemory(&sysTimeoutTime, sizeof(sysTimeoutTime));

			sysTimeoutTime.wYear	= kPGPdiskBetaTimeoutYear;
			sysTimeoutTime.wMonth	= kPGPdiskBetaTimeoutMonth;
			sysTimeoutTime.wDay		= kPGPdiskBetaTimeoutDay;

			// Convert it to a 64-bit value.
			SystemTimeToFileTime(&sysTimeoutTime, &fileTimeoutTime);

			// Convert it to standard form.
			timeoutTime = ConvertFileTimeToSecondsSince1970(&fileTimeoutTime);

			// Set the timeout time.
			prefs.betaTimeout = timeoutTime;
			derr = SetPGPdiskWin32Prefs(prefs);
		}
	}

	return derr;
}

#elif PGPDISK_DEMOVERSION

// HasDemoTimedOut returns TRUE if this demo version of PGPdisk has timed
// out, FALSE otherwise.

PGPBoolean 
HasDemoTimedOut()
{
	DualErr				derr;
	PGPdiskWin32Prefs	prefs;
	PGPUInt64			currentTime, timeoutTime;

	derr = GetPGPdiskWin32Prefs(prefs);

	if (derr.IsntError())
	{
		// Get the current time.
		currentTime = GetSecondsSince1970();

		// Get the timeout time.
		timeoutTime = prefs.demoTimeout;

		// If we've exceeded the timeout data, return TRUE.
		return (currentTime >= timeoutTime);
	}

	return FALSE;
}

// SetPGPdiskDemoTimeout burns a demo timeout date into the registry, but
// only if no date is there already.

DualErr 
SetPGPdiskDemoTimeout()
{
	DualErr				derr;
	PGPdiskWin32Prefs	prefs;
	PGPUInt64			timeoutTime;

	derr = GetPGPdiskWin32Prefs(prefs);

	if (derr.IsntError())
	{
		timeoutTime = prefs.demoTimeout;

		// If no demo timeout has been set, set it now.
		if (timeoutTime == 0)
		{
			// Determine the timeout time.
			timeoutTime = GetSecondsSince1970() + 
				kDemoTimeoutDays*kSecondsInADay;

			// Set the timeout time.
			prefs.demoTimeout = timeoutTime;
			derr = SetPGPdiskWin32Prefs(prefs);
		}
	}

	return derr;
}

#endif // PGPDISK_BETAVERSION
