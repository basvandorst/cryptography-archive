/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	PGPkeysMisc.c - miscellaneous routines
	

	$Id: PKMisc.c,v 1.17.4.2 1997/11/20 20:55:26 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"	/* or pgpConfig.h in the CDK */

#include "pgpkeysx.h"
#include "resource.h"

typedef struct {
	PGPUInt32	uWindowShow;
	PGPInt32	iWindowPosX;
	PGPInt32	iWindowPosY;
	PGPInt32	iWindowWidth;
	PGPInt32	iWindowHeight;
	PGPBoolean	bGroupShow;
	PGPInt32	iGroupPercent;
} KEYSWINDOWSTRUCT, *PKEYSWINDOWSTRUCT;


// external global variables
extern HINSTANCE		g_hInst;
extern HWND				g_hWndMain;
extern UINT				g_uReloadKeyringMessage;
extern PGPContextRef	g_ContextRef;
extern PGPBoolean		g_bExpertMode;
extern PGPBoolean		g_bMarginalAsInvalid;

//	____________________________________
//
//	Private memory allocation routine

VOID* 
pkAlloc (LONG size) 
{
	return HeapAlloc (GetProcessHeap (), HEAP_ZERO_MEMORY, size);
}

//	____________________________________
//
//	Private memory deallocation routine

VOID 
pkFree (VOID* p) 
{
	if (p) {
		FillMemory (p, lstrlen(p), '\0');
		HeapFree (GetProcessHeap (), 0, p);
	}
}

//	____________________________________
//
//	Get window position information from registry

BOOL 
PGPkeysGetWindowPosFromRegistry (
			DWORD*	dwShow, 
			INT*	iX, 
			INT*	iY, 
			INT*	iWidth, 
			INT*	iHeight,
			BOOL*	bGroupShow,
			INT*	iGroupPercent) 
{
	PKEYSWINDOWSTRUCT		pkws;
	PGPPrefRef				prefref;
	PGPError				err;
	PGPSize					size;

	*dwShow			= SW_SHOWNORMAL;
	*iX				= DEFAULTWINDOWX;
	*iY				= DEFAULTWINDOWY;
	*iWidth			= DEFAULTWINDOWWIDTH; 
	*iHeight		= DEFAULTWINDOWHEIGHT; 
	*bGroupShow		= DEFAULTGROUPSHOW;
	*iGroupPercent	= DEFAULTGROUPPERCENT;

	err = PGPcomdlgOpenClientPrefs (&prefref);
	if (IsntPGPError (err)) {
		size = 0;
		err = PGPGetPrefData (prefref, kPGPPrefPGPkeysWinMainWinPos,
						&size, &pkws);
		PGPcomdlgCloseClientPrefs (prefref, FALSE);

		if (IsntPGPError (err)) {
			if (size == sizeof(KEYSWINDOWSTRUCT)) {
				*dwShow			= pkws->uWindowShow;
				*iX				= pkws->iWindowPosX;
				*iY				= pkws->iWindowPosY;
				*iWidth			= pkws->iWindowWidth;
				*iHeight		= pkws->iWindowHeight;
				*bGroupShow		= pkws->bGroupShow;
				*iGroupPercent	= pkws->iGroupPercent;
			}
			PGPDisposePrefData (prefref, pkws);
		}
	}

	return FALSE;
}

//	____________________________________
//
//	 Put window position information in registry

VOID 
PGPkeysSetWindowRegistryData (
		HWND	hWnd, 
		BOOL	bGroupShow,
		INT		iGroupPercent) 
	{
	WINDOWPLACEMENT		wp;
	KEYSWINDOWSTRUCT	kws;
	PGPPrefRef			prefref;
	PGPError			err;

	wp.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement (hWnd, &wp);

	err = PGPcomdlgOpenClientPrefs (&prefref);
	if (IsntPGPError (err)) {
		kws.uWindowShow = wp.showCmd;
		kws.iWindowPosX = wp.rcNormalPosition.left;
		kws.iWindowPosY = wp.rcNormalPosition.top;
		kws.iWindowWidth = 
			wp.rcNormalPosition.right - wp.rcNormalPosition.left;
		kws.iWindowHeight = 
			wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
		kws.bGroupShow = bGroupShow;
		kws.iGroupPercent = iGroupPercent;
		PGPSetPrefData (prefref, kPGPPrefPGPkeysWinMainWinPos, 
						sizeof(kws), &kws);

		PGPcomdlgCloseClientPrefs (prefref, TRUE);
	}
}

//	____________________________________
//
//	Put PGPkeys application path into registry

VOID 
PGPkeysSetPathRegistryData (void) 
{
	HKEY	hKey;
	LONG	lResult;
	CHAR	szPath[MAX_PATH];
	DWORD	dw;
	CHAR	sz[256];

	LoadString (g_hInst, IDS_REGISTRYKEY, sz, sizeof(sz));
	lResult = RegCreateKeyEx (HKEY_CURRENT_USER, sz, 0, NULL,
				REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dw);

	if (lResult == ERROR_SUCCESS) {
		GetModuleFileName (NULL, szPath, sizeof(szPath));
		RegSetValueEx (hKey, "ExePath", 0, REG_SZ, (LPBYTE)szPath, 
			lstrlen(szPath)+1);
		RegCloseKey (hKey);
	}
}

//	____________________________________
//
//  Put up preferences property sheet

BOOL 
PGPkeysPGPPreferences (
		PGPKEYSSTRUCT* ppks, 
		HWND hWnd, 
		INT iPage) 
{
	PGPError	Retval;
	PGPPrefRef	prefRef;
	PGPBoolean	b, bUpdate;

	EnableWindow (hWnd, FALSE);
	Retval = PGPcomdlgPreferences (g_ContextRef, hWnd, iPage);
	if (Retval != kPGPError_Win32_AlreadyOpen) SetForegroundWindow (hWnd);
	EnableWindow (hWnd, TRUE);
	SetFocus (ppks->hWndTreeList);

	bUpdate = FALSE;
	PGPcomdlgOpenClientPrefs (&prefRef);

	// has the validity dot vs. bar pref changed?
	PGPGetPrefBoolean (prefRef, kPGPPrefDisplayMarginalValidity, &b);
	if (b != g_bExpertMode) {
		g_bExpertMode = b;
		bUpdate = TRUE;
		if (!g_bExpertMode) {
			ppks->kmConfig.ulOptionFlags |= KMF_NOVICEMODE;
			ppks->gmConfig.ulOptionFlags |= GMF_NOVICEMODE;
		}
		else {
			ppks->kmConfig.ulOptionFlags &= ~KMF_NOVICEMODE;
			ppks->gmConfig.ulOptionFlags &= ~GMF_NOVICEMODE;
		}
	}

	// has the display marginal validity as invalid pref changed?
	PGPGetPrefBoolean (prefRef, kPGPPrefMarginalIsInvalid, &b);
	if (b != g_bMarginalAsInvalid) {
		g_bMarginalAsInvalid = b;
		bUpdate = TRUE;
		if (g_bMarginalAsInvalid) {
			ppks->kmConfig.ulOptionFlags |= KMF_MARGASINVALID;
			ppks->gmConfig.ulOptionFlags |= GMF_MARGASINVALID;
		}
		else {
			ppks->kmConfig.ulOptionFlags &= ~KMF_MARGASINVALID;
			ppks->gmConfig.ulOptionFlags &= ~GMF_MARGASINVALID;
		}
	}

	PGPcomdlgCloseClientPrefs (prefRef, FALSE);

	// if prefs have changed => redraw the windows
	if (bUpdate) {
		ppks->kmConfig.ulMask = PGPKM_OPTIONS;
		PGPkmConfigure (ppks->hKM, &(ppks->kmConfig));
		PGPkmLoadKeySet (ppks->hKM, ppks->KeySetMain, ppks->KeySetMain);
		PGPgmConfigure (ppks->hGM, &(ppks->gmConfig));
		PGPgmLoadGroups (ppks->hGM);
	}

	if (Retval == kPGPError_NoErr) return TRUE;
	else return FALSE;
}

//	___________________________________________________
//
//  commit keyring changes and broadcast reload message

VOID 
PGPkeysRegister (VOID) 
{
	CHAR		szURL[256];
#if PGP_FREEWARE
	LoadString (g_hInst, IDS_UPGRADEURL, szURL, sizeof(szURL));
#else
	PGPError	err;
	PGPPrefRef	PrefRef;
	CHAR		sz[64];

	sz[0] = '\0';

	// try getting site license number
#if PGP_BUSINESS_SECURITY
	if (PGPcomdlgOpenAdminPrefs (&PrefRef, FALSE) != kPGPError_NoErr)
		return;
	err = PGPGetPrefStringBuffer (PrefRef, kPGPPrefSiteLicenseNumber, 
										sizeof(sz), sz);
	if (IsPGPError (err)) sz[0] = '\0';
	PGPcomdlgCloseAdminPrefs (PrefRef, FALSE);
#endif //PGP_BUSINESS_SECURITY

	// that didn't work -- try getting license number
	if (sz[0] == '\0') {
		if (PGPcomdlgOpenClientPrefs (&PrefRef) != kPGPError_NoErr)
			return;
		err = PGPGetPrefStringBuffer (PrefRef, kPGPPrefLicenseNumber, 
										sizeof(sz), sz);
		if (IsPGPError (err)) sz[0] = '\0';
		PGPcomdlgCloseClientPrefs (PrefRef, FALSE);
	}

	if (sz[0] == '\0') 
		LoadString (g_hInst, IDS_REGISTERURL, szURL, sizeof(szURL));
	else 
		LoadString (g_hInst, IDS_REGISTERURLCGI, szURL, sizeof(szURL));

	lstrcat (szURL, sz);
#endif
	PGPcomdlgWebBrowse (szURL);
}


//	___________________________________________________
//
//  commit keyring changes and broadcast reload message

INT 
PGPkeysCommitKeyRingChanges (PGPKeySetRef KeySet, BOOL bBroadcast) 
{
	INT iError;
	HCURSOR hCursorOld;

	hCursorOld = SetCursor (LoadCursor (NULL, IDC_WAIT));
	iError = PGPCommitKeyRingChanges (KeySet);
	if (!PGPcomdlgErrorMessage (iError) && bBroadcast) {
		PostMessage (HWND_BROADCAST, g_uReloadKeyringMessage,
			MAKEWPARAM (LOWORD (g_hWndMain), FALSE),
			(LPARAM)GetCurrentProcessId ());
	}
	SetCursor (hCursorOld);
	return iError;
}


