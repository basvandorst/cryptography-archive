/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PGPkm.c - PGP key manager DLL
	

	$Id: PGPkm.c,v 1.10 1998/08/11 14:43:44 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

// project header files
#include "pgpkmx.h"

// external global variables
extern HINSTANCE g_hInst;

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
			else {
				CHAR	sz[MAX_PATH];

				PGPclGetPGPPath (pKM->szHelpFile, sizeof(pKM->szHelpFile));
				LoadString (g_hInst, IDS_HELPFILENAME, sz, sizeof(sz));
				lstrcat (pKM->szHelpFile, sz);
			}
		}

		// set keyserver
		if (pKMConfig->ulMask & PGPKM_KEYSERVER) {
			if (pKMConfig->keyserver.structSize) 
				CopyMemory (&pKM->keyserver, &pKMConfig->keyserver,
							sizeof(PGPKeyServerEntry));
			else
				ZeroMemory (&pKM->keyserver, sizeof(PGPKeyServerEntry));
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
		
		// set the procedure address for adding/removing

		// enable or disable the key properties windows
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
PGPkmSelectColumns (HKEYMAN hKeyMan, ULONG ulColumnFlags, BOOL bRedraw) 
{
	PKEYMAN pKM = (PKEYMAN)hKeyMan;

	KMSelectColumns (pKM, ulColumnFlags);

	if (bRedraw) {
		KMDeleteAllUserValues (pKM);
		KMSetFocus (pKM, NULL, FALSE);
		TreeList_DeleteTree (pKM->hWndTree, TRUE);
		TreeList_DeleteAllColumns (pKM->hWndTree);

		KMAddColumns (pKM);
		KMLoadKeyRingIntoTree (pKM, FALSE, FALSE, TRUE);
		InvalidateRect (pKM->hWndTree, NULL, TRUE);
		UpdateWindow (pKM->hWndTree);
	}

	KMSetColumnPreferences (pKM);

	return kPGPError_NoErr;
}


//	___________________________________________________
//
//  Get selected columns

PGPError PGPkmExport 
PGPkmGetSelectedColumns (HKEYMAN hKeyMan, ULONG* pulColumnFlags) 
{
	PKEYMAN pKM = (PKEYMAN)hKeyMan;

	KMGetSelectedColumns (pKM, pulColumnFlags);

	return kPGPError_NoErr;
}


//	___________________________________________________
//
//  process message from prop sheet

VOID PGPkmExport 
PGPkmProcessKeyPropMessage (
		HKEYMAN hKeyMan, 
		WPARAM	wParam,
		LPARAM	lParam)
{
	PKEYMAN pKM = (PKEYMAN)hKeyMan;

	switch (wParam) {
	case KM_PROPACTION_UPDATEKEY :
		KMUpdateKeyInTree (pKM, (PGPKeyRef)lParam, FALSE);
		KMUpdateAllValidities (pKM);
		InvalidateRect (pKM->hWndTree, NULL, TRUE);
		break;


	case KM_PROPACTION_SPLITKEY :
		KMSplitKey (pKM, (PGPKeyRef)lParam);
		break;
	}
}


//	___________________________________________________
//
//  check if OK to close key manager

BOOL PGPkmExport 
PGPkmOKToClose (HKEYMAN hKeyMan, PUINT puReason)
{
	PKEYMAN pKM = (PKEYMAN)hKeyMan;

	if (KMExistSplitKeyDialog (pKM)) {
		*puReason = KMR_EXISTINGSPLITKEYDLGS;
		return FALSE;
	}

	return TRUE;
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



