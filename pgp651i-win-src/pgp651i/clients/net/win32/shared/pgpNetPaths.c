/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	pgpNetPaths.c - PGPnet get module paths
	

	$Id: pgpNetPaths.c,v 1.9 1999/04/28 19:42:06 pbj Exp $
____________________________________________________________________________*/

#include <windows.h>

#include "pgpErrors.h"
#include "pgpMemoryMgr.h"
#include "pgpFileSpec.h"
#include "pgpUtilities.h"
#include "pgpSDKPrefs.h"
#include "pgpNetPaths.h"
#include "pgpNetIPC.h"

// macro definitions
#define CKERR		if (IsPGPError (err)) goto done

//	___________________________________________________
//
//	get path of PGPnet installation from registry key 
//	note: includes trailing '\'
//	The PGPnet application copies its path to this key
//	every time it runs 

PGPError
PGPnetGetPGPnetFullPath (
		LPSTR	szPath, 
		UINT	uLen,
		UINT*	puLen) 
{
	PGPError	err				= kPGPError_FileNotFound;
	HKEY		hKey;
	LONG		lResult;
	DWORD		dwValueType;
	DWORD		dwSize;
	CHAR		szKey[128];

	lstrcpy (szKey, PGP_REGISTRYKEY);
	lResult = RegOpenKeyEx (HKEY_LOCAL_MACHINE, szKey, 0, KEY_READ, &hKey);

	if (lResult == ERROR_SUCCESS) 
	{
		err = kPGPError_OutputBufferTooSmall;
		dwSize = uLen;
		lResult = RegQueryValueEx (hKey, PGP_INSTALLPATHVALUE, 0, 
						&dwValueType, (LPBYTE)szPath, &dwSize);
		RegCloseKey (hKey);
		if (lResult == ERROR_SUCCESS) 
		{
			*puLen = dwSize;
			err = kPGPError_NoErr;
		}
		else
			*puLen = 0;
	}

	return err;
}

//	______________________________________________
//
//  get preferences file name

PGPError
PGPnetGetLogFileFullPath (
		LPSTR	pszFileName,
		UINT	uLen)
{
	PGPError	err;
	UINT		u;

	err = PGPnetGetPGPnetFullPath (pszFileName, uLen, &u);

	if (IsntPGPError (err)) 
	{
		if ((u + lstrlen (PGPNET_LOGFILE) +1) <= uLen)
			lstrcat (pszFileName, PGPNET_LOGFILE);
		else
			err = kPGPError_OutputBufferTooSmall;
	}

	return err;
}

//	______________________________________________
//
//  get preferences file name

PGPError
PGPnetGetPrefsFullPath (
		LPSTR	pszFileName,
		UINT	uLen)
{
	PGPError	err;
	UINT		u;

	err = PGPnetGetPGPnetFullPath (pszFileName, uLen, &u);

	if (IsntPGPError (err)) 
	{
		if ((u + lstrlen (PGPNET_PREFSFILE) +1) <= uLen)
			lstrcat (pszFileName, PGPNET_PREFSFILE);
		else
			err = kPGPError_OutputBufferTooSmall;
	}

	return err;
}

//	______________________________________________
//
//  get preferences file name

PGPError
PGPnetGetAppFullPath (
		LPSTR	pszFileName,
		UINT	uLen)
{
	PGPError	err;
	UINT		u;

	err = PGPnetGetPGPnetFullPath (pszFileName, uLen, &u);

	if (IsntPGPError (err)) 
	{
		if ((u + lstrlen (PGPNET_NETAPP) +1) <= uLen)
			lstrcat (pszFileName, PGPNET_NETAPP);
		else
			err = kPGPError_OutputBufferTooSmall;
	}

	return err;
}

//	______________________________________________
//
//  get help file name

PGPError
PGPnetGetHelpFullPath (
		LPSTR	pszFileName,
		UINT	uLen)
{
	PGPError	err;
	UINT		u;

	err = PGPnetGetPGPnetFullPath (pszFileName, uLen, &u);

	if (IsntPGPError (err)) 
	{
		if ((u + lstrlen (PGPNET_NETAPP) +1) <= uLen)
			lstrcat (pszFileName, PGPNET_HELPFILE);
		else
			err = kPGPError_OutputBufferTooSmall;
	}

	return err;
}

//	______________________________________________
//
//  create a filespec for the PGPnet sdk prefs file

static PGPError
sGetNetPrefsSpec (
		PGPMemoryMgrRef		memoryMgr,
		PFLFileSpecRef*		pRef)
{
	PGPError	err					= kPGPError_NoErr;
	CHAR		szPath[MAX_PATH];
	UINT		u;

	*pRef		= NULL;

	err = PGPnetGetPGPnetFullPath (szPath, sizeof(szPath), &u);

	if (IsntPGPError (err)) 
	{
		if ((u + lstrlen (PGPNET_NETAPP) +1) <= sizeof(szPath))
			lstrcat (szPath, PGPNET_SDKPREFSFILE);
		else
			err = kPGPError_OutputBufferTooSmall;
	}

	err = PFLNewFileSpecFromFullPath (memoryMgr, szPath, pRef);

	return err;
}

//	______________________________________________
//
//  load the PGPnet-specific SDK prefs file

PGPError
PGPnetLoadSDKPrefs (
		PGPContextRef	context)
{
	PGPError		err			= kPGPError_NoErr;
	PFLFileSpecRef	prefsSpec	= NULL;
	OSVERSIONINFO	osid;

	osid.dwOSVersionInfoSize = sizeof (osid);
	GetVersionEx (&osid);

	switch (osid.dwPlatformId) {
	// Windows NT or 9x, use prefs stored in PGPnet sdk prefs file
	case VER_PLATFORM_WIN32_NT :
	case VER_PLATFORM_WIN32_WINDOWS :
		err	= sGetNetPrefsSpec (PGPGetContextMemoryMgr (context), &prefsSpec);
		if (IsntPGPError (err))
		{
			err	= PGPsdkLoadPrefs (context, (PGPFileSpecRef)prefsSpec);
			PFLFreeFileSpec (prefsSpec);
		}
		break;

	// otherwise just use SDK defaults
	default :
		err = PGPsdkLoadDefaultPrefs (context);
		break;
	}

	return err;
}
