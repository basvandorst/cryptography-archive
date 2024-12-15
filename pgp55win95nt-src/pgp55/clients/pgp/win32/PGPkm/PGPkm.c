/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	PGPkm.c - PGP key manager DLL
	

	$Id: PGPkm.c,v 1.18 1997/10/20 15:23:59 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

#include "pgpkmx.h"
#include "resource.h"

#define IMPORTDLGCLASS	("PGPDLGCLASS")

// DLL global variables
HINSTANCE g_hInst;

//	___________________________________________________
//
//	Register window class for selective import dialog
//	This allows setting the icon to the PGPkeys icon

VOID
RegisterSelectiveImportClass (VOID)
{
	WNDCLASSEX	wc;

	// class 0x8002 is the "hardwired" class of standard dialogs
	wc.cbSize = sizeof (WNDCLASSEX);
	GetClassInfoEx (NULL, MAKEINTRESOURCE(0x8002), &wc);

	wc.lpszClassName = IMPORTDLGCLASS;
	wc.hIcon = LoadIcon (g_hInst, MAKEINTRESOURCE(IDI_KEYICON));
	wc.hInstance = g_hInst;

	RegisterClassEx (&wc);
}

//	___________________________________________________
//
//	DLL entry/exit point

BOOL WINAPI 
DllMain (
		HINSTANCE	hInstDll, 
		DWORD		dwReason, 
		LPVOID		lpvReserved) 
{
	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		g_hInst = hInstDll;
		RegisterSelectiveImportClass ();
		break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
		break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

//	___________________________________________________
//
//  Return handle of keymanager window

HWND PGPkmExport 
PGPkmGetManagerWindow (HKEYMAN hKM) 
{
	if (!hKM) return NULL;
	return (((PKEYMAN)hKM)->hWndTree);

}

//	___________________________________________________
//
//	get PGPkeys path from registry and substitute Help file name 

VOID 
GetHelpFilePath (PKEYMAN pKM) 
{
	CHAR	sz[MAX_PATH];

	PGPcomdlgGetPGPPath (pKM->szHelpFile, sizeof(pKM->szHelpFile));
	LoadString (g_hInst, IDS_HELPFILENAME, sz, sizeof(sz));
	lstrcat (pKM->szHelpFile, sz);
}

//	___________________________________________________
//
//  Set configuration

PGPError PGPkmExport 
PGPkmConfigure (
		HKEYMAN		hKeyMan, 
		LPKMCONFIG	pKMConfig) 
{
	PKEYMAN pKM = (PKEYMAN)hKeyMan;
	if (!pKM) return kPGPError_BadParams;

	if (pKM) {
		// set help file
		if (pKMConfig->ulMask & PGPKM_HELPFILE) {
			if (pKMConfig->lpszHelpFile) {
				lstrcpyn (pKM->szHelpFile, 
								pKMConfig->lpszHelpFile, 
								sizeof(pKM->szHelpFile));
			}
			else 
				GetHelpFilePath (pKM);
		}

		// set keyserver
		if (pKMConfig->ulMask & PGPKM_KEYSERVER) {
			if (pKMConfig->lpszPutKeyserver) {
				if (pKMConfig->lpszPutKeyserver[0]) {
					lstrcpyn (pKM->szPutKeyserver, 
								pKMConfig->lpszPutKeyserver, 
								sizeof(pKM->szPutKeyserver));
				}
			}
			else
				pKM->szPutKeyserver[0] = '\0';
		}

		// set disabled actions
		if (pKMConfig->ulMask & PGPKM_DISABLEFLAGS) {
			pKM->ulDisableActions = pKMConfig->ulDisableActions;
		}

		// set option flags
		if (pKMConfig->ulMask & PGPKM_OPTIONS) {
			pKM->ulOptionFlags = pKMConfig->ulOptionFlags;
		}

		// set column flags
		if (pKMConfig->ulMask & PGPKM_COLUMNFLAGS) {
			pKM->ulShowColumns = pKMConfig->ulShowColumns;
			pKM->ulHideColumns = pKMConfig->ulHideColumns;
		}

		// set statusbar hwnd
		if (pKMConfig->ulMask & PGPKM_STATUSBAR) {
			pKM->hWndStatusBar = pKMConfig->hWndStatusBar;
		}

		if (pKM->ulOptionFlags & KMF_DISABLEKEYPROPS) 
			KMEnableAllKeyProperties (pKM, FALSE);
		else
			KMEnableAllKeyProperties (pKM, TRUE);
	
		// determine validity threshold for green dot
		if (pKM->ulOptionFlags & KMF_MARGASINVALID) 
			pKM->iValidityThreshold = KM_VALIDITY_COMPLETE;
		else
			pKM->iValidityThreshold = KM_VALIDITY_MARGINAL;
			
		return kPGPError_NoErr;
	}

	else return kPGPError_BadParams;

}

//	___________________________________________________
//
//  Select columns

PGPError PGPkmExport 
PGPkmSelectColumns (HKEYMAN hKeyMan, BOOL bDialog) 
{
	PKEYMAN pKM = (PKEYMAN)hKeyMan;

	KMSelectColumns (pKM, bDialog);

	return kPGPError_NoErr;
}


//	___________________________________________________
//
//  Synchronize thread access to SDK by using mutexs

VOID PGPkmExport 
PGPkmSynchronizeThreadAccessToSDK (HKEYMAN	hKeyMan)
{
	PKEYMAN pKM = (PKEYMAN)hKeyMan;

	ReleaseMutex (pKM->hAccessMutex);
	WaitForSingleObject (pKM->hRequestMutex, INFINITE);
	WaitForSingleObject (pKM->hAccessMutex, INFINITE);
	ReleaseMutex (pKM->hRequestMutex);
}


//	___________________________________________________
//
//  Synchronize thread access to SDK by using mutexs

VOID PGPkmExport 
PGPkmUpdateKeyInTree (HKEYMAN hKeyMan, PGPKeyRef key)
{
	PKEYMAN pKM = (PKEYMAN)hKeyMan;

	KMUpdateKeyInTree (pKM, key);
	KMUpdateAllValidities (pKM);
}


//	___________________________________________________
//
//  request thread access to SDK (called internally)

VOID
KMRequestSDKAccess (PKEYMAN pKM)
{
	WaitForSingleObject (pKM->hRequestMutex, INFINITE);
	PostMessage (pKM->hWndParent, KM_M_REQUESTSDKACCESS, 0, 0);
	WaitForSingleObject (pKM->hAccessMutex, INFINITE);
}


//	___________________________________________________
//
//  release thread access to SDK (called internally)

VOID
KMReleaseSDKAccess (PKEYMAN pKM)
{
	ReleaseMutex (pKM->hAccessMutex);
	ReleaseMutex (pKM->hRequestMutex);
}



