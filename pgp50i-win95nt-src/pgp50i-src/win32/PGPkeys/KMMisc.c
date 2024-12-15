/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */
//: KMMisc.c - miscellaneous routines
//
//	$Id: KMMisc.c,v 1.5 1997/05/28 20:47:15 pbj Exp $
//

#include <windows.h>
#include "pgpkeys.h"
#include "resource.h"

// external global variables
extern CHAR g_szbuf[G_BUFLEN];
extern HINSTANCE g_hInst;
extern HWND g_hWndTree;
extern PGPKeySet* g_pKeySetMain;
extern LONG g_lKeyListSortField;
extern UINT g_uReloadKeyringMessage;


//----------------------------------------------------|
// Private memory allocation routine

VOID* KMAlloc (LONG size) {
	return HeapAlloc (GetProcessHeap (), HEAP_ZERO_MEMORY, size);
}


//----------------------------------------------------|
// Private memory deallocation routine

VOID KMFree (VOID* p) {
	if (p) {
		FillMemory (p, lstrlen(p), '\0');
		HeapFree (GetProcessHeap (), 0, p);
	}
}


//----------------------------------------------------|
// Get window position information from registry
//
//	return main window size and position from registry

BOOL KMGetWindowPosFromRegistry (DWORD* dwShow, INT* iX, INT* iY,
								 INT* iWidth, INT* iHeight) {
	HKEY hKey;
	LONG lResult;
	DWORD dwValueType, dwValue, dwSize=4;

	*iX = DEFAULTWINDOWX;
	*iY = DEFAULTWINDOWY;
	*iWidth = DEFAULTWINDOWWIDTH;
	*iHeight = DEFAULTWINDOWHEIGHT;

	LoadString (g_hInst, IDS_REGISTRYKEY, g_szbuf, G_BUFLEN);
	lResult = RegOpenKeyEx (HKEY_CURRENT_USER, g_szbuf, 0, KEY_READ, &hKey);
	if (lResult == ERROR_SUCCESS) {
		lResult = RegQueryValueEx (hKey, "WindowShow", 0, &dwValueType,
			(LPBYTE)&dwValue, &dwSize);
		if (lResult == ERROR_SUCCESS) *dwShow = dwValue;
		lResult = RegQueryValueEx (hKey, "WindowPosX", 0, &dwValueType,
			(LPBYTE)&dwValue, &dwSize);
		if (lResult == ERROR_SUCCESS) *iX = dwValue;
		lResult = RegQueryValueEx (hKey, "WindowPosY", 0, &dwValueType,
			(LPBYTE)&dwValue, &dwSize);
		if (lResult == ERROR_SUCCESS) *iY = dwValue;
		lResult = RegQueryValueEx (hKey, "WindowWidth", 0, &dwValueType,
			(LPBYTE)&dwValue, &dwSize);
		if (lResult == ERROR_SUCCESS) *iWidth = dwValue;
		lResult = RegQueryValueEx (hKey, "WindowHeight", 0, &dwValueType,
			(LPBYTE)&dwValue, &dwSize);
		if (lResult == ERROR_SUCCESS) *iHeight = dwValue;
		RegCloseKey (hKey);
		return TRUE;
	}
	return FALSE;
}


//----------------------------------------------------|
// Get sort order information from registry

BOOL KMGetSortOrderFromRegistry (LONG* lField) {
	HKEY hKey;
	LONG lResult;
	DWORD dwValueType, dwValue, dwSize=4;

	*lField = kPGPUserIDOrdering;

	LoadString (g_hInst, IDS_REGISTRYKEY, g_szbuf, G_BUFLEN);
	lResult = RegOpenKeyEx (HKEY_CURRENT_USER, g_szbuf, 0, KEY_READ, &hKey);
	if (lResult == ERROR_SUCCESS) {
		lResult = RegQueryValueEx (hKey, "SortField", 0, &dwValueType,
			(LPBYTE)&dwValue, &dwSize);
		if (lResult == ERROR_SUCCESS) *lField = dwValue;
		RegCloseKey (hKey);
		return TRUE;
	}
	return FALSE;
}


//----------------------------------------------------|
// Get widths of control columns from registry

BOOL KMGetColWidthsFromRegistry (WORD* pwWidths, INT iWidths) {
	HKEY hKey;
	LONG lResult;
	DWORD dwValueType, dwSize;

	LoadString (g_hInst, IDS_REGISTRYKEY, g_szbuf, G_BUFLEN);
	lResult = RegOpenKeyEx (HKEY_CURRENT_USER, g_szbuf, 0, KEY_READ, &hKey);
	if (lResult == ERROR_SUCCESS) {
		dwSize = iWidths * sizeof (WORD);
		lResult = RegQueryValueEx (hKey, "ColumnWidths", 0, &dwValueType,
			(LPBYTE)&pwWidths[0], &dwSize);
		RegCloseKey (hKey);
	}
	if (lResult != ERROR_SUCCESS) {
		pwWidths[0] = DEFAULTCOLWIDTHNAME;
		pwWidths[1] = DEFAULTCOLWIDTHVALID;
		pwWidths[2] = DEFAULTCOLWIDTHTRUST;
		pwWidths[3] = DEFAULTCOLWIDTHDATE;
		pwWidths[4] = DEFAULTCOLWIDTHSIZE;
	}
	return TRUE;
}


//----------------------------------------------------|
// Put window position information in registry
//
//	store window size and position in registry.

void KMSetWindowRegistryData (HWND hWnd) {
	WINDOWPLACEMENT wp;
	HKEY hKey;
	LONG lResult;
	INT i;
	WORD w[NUMBERCOLUMNS];
	DWORD dwValue, dw;

	wp.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement (hWnd, &wp);

	LoadString (g_hInst, IDS_REGISTRYKEY, g_szbuf, G_BUFLEN);
	lResult = RegCreateKeyEx (HKEY_CURRENT_USER, g_szbuf, 0, NULL,
				REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dw);

	if (lResult == ERROR_SUCCESS) {
		dwValue = wp.showCmd;
		RegSetValueEx (hKey, "WindowShow", 0, REG_DWORD, (LPBYTE)&dwValue,
			sizeof(DWORD));
		dwValue = wp.rcNormalPosition.left;
		RegSetValueEx (hKey, "WindowPosX", 0, REG_DWORD, (LPBYTE)&dwValue,
			sizeof(DWORD));
		dwValue = wp.rcNormalPosition.top;
		RegSetValueEx (hKey, "WindowPosY", 0, REG_DWORD, (LPBYTE)&dwValue,
			sizeof(DWORD));
		dwValue = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
		RegSetValueEx (hKey, "WindowWidth", 0, REG_DWORD, (LPBYTE)&dwValue,
			sizeof(DWORD));
		dwValue = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
		RegSetValueEx (hKey, "WindowHeight", 0, REG_DWORD, (LPBYTE)&dwValue,
			sizeof(DWORD));
		dwValue = g_lKeyListSortField;
		RegSetValueEx (hKey, "SortField", 0, REG_DWORD, (LPBYTE)&dwValue,
			sizeof(DWORD));
		for (i=0; i<NUMBERCOLUMNS; i++)
			w[i] = LOWORD (TreeList_GetColumnWidth (g_hWndTree, i));
		RegSetValueEx (hKey, "ColumnWidths", 0, REG_BINARY, (LPBYTE)&w[0],
			NUMBERCOLUMNS*sizeof(WORD));
		RegCloseKey (hKey);
	}
}


//----------------------------------------------------|
// Put PGPkeys application path into registry

void KMSetPathRegistryData (void) {
	HKEY hKey;
	LONG lResult;
	CHAR szPath[MAX_PATH];
	DWORD dw;

	LoadString (g_hInst, IDS_REGISTRYKEY, g_szbuf, G_BUFLEN);
	lResult = RegCreateKeyEx (HKEY_CURRENT_USER, g_szbuf, 0, NULL,
				REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dw);

	if (lResult == ERROR_SUCCESS) {
		GetModuleFileName (NULL, szPath, sizeof(szPath));
		RegSetValueEx (hKey, "ExePath", 0, REG_SZ, (LPBYTE)szPath,
			lstrlen(szPath)+1);
		RegCloseKey (hKey);
	}
}


//----------------------------------------------------|
//  Get and truncate the name of a userid.

BOOL KMGetUserIDName (PGPUserID* pUserID, LPSTR sz, UINT uLen) {
	UINT u, uErr;

	u = uLen-1;
	uErr = pgpGetUserIDString (pUserID, kPGPUserIDPropName, sz, &u);

	switch (uErr) {
	case PGPERR_KEYDB_BUFFERTOOSHORT :
		sz[uLen-1] = '\0';
		return TRUE;

	case PGPERR_OK :
		u = min (u, uLen-1);
		sz[u] = '\0';
		return TRUE;

	default :
		PGPcomdlgErrorMessage (uErr);
		sz[0] = '\0';
		return FALSE;
	}
}


//----------------------------------------------------|
//  Get and truncate the name of a primary userid on a key.

BOOL KMGetKeyName (PGPKey* pKey, LPSTR sz, UINT uLen) {
	UINT u, uErr;

	uErr = pgpGetPrimaryUserIDName (pKey, NULL, &u);

	u = uLen-1;
	uErr = pgpGetPrimaryUserIDName (pKey, sz, &u);

	switch (uErr) {
	case PGPERR_KEYDB_BUFFERTOOSHORT :
		sz[uLen-1] = '\0';
		return TRUE;

	case PGPERR_OK :
		u = min (u, uLen-1);
		sz[u] = '\0';
		return TRUE;

	default :
		PGPcomdlgErrorMessage (uErr);
		sz[0] = '\0';
		return FALSE;
	}
}


//----------------------------------------------------|
//  Get the parent key of a userid

PGPKey* KMGetKeyFromUserID (HWND hWndTree, PGPUserID* pUserID) {
	TL_TREEITEM tli;

	pgpGetUserIDUserVal (pUserID, (long*)&(tli.hItem));
	if (tli.hItem) {
		tli.mask = TLIF_PARENTHANDLE;
		TreeList_GetItem (hWndTree, &tli);
		if (tli.hItem) {
			tli.mask = TLIF_PARAM;
			TreeList_GetItem (hWndTree, &tli);
			return ((PGPKey*)(tli.lParam));
		}
	}
	return NULL;
}


//----------------------------------------------------|
//  Look for secret keys

BOOL KMCheckForSecretKeys (PGPKeySet* pKeySet) {
	PGPKeyList* pKeyList;
	PGPKeyIter* pKeyIter;
	PGPKey* pKey;
	BOOL bSecretKeys;
	Boolean bSecret;

	pKeyList = pgpOrderKeySet (pKeySet, g_lKeyListSortField);
	pKeyIter = pgpNewKeyIter (pKeyList);

	bSecretKeys = FALSE;
	pKey = pgpKeyIterNext (pKeyIter);

	while (pKey && !bSecretKeys) {
		pgpGetKeyBoolean (pKey, kPGPKeyPropIsSecret, &bSecret);
		if (bSecret) {
			bSecretKeys = TRUE;
		}
		pKey = pgpKeyIterNext (pKeyIter);
	}

	pgpFreeKeyIter (pKeyIter);
	pgpFreeKeyList (pKeyList);

	return bSecretKeys;
}


//----------------------------------------------------|
//  Is this the only userID on the key ?

BOOL KMIsThisTheOnlyUserID (HWND hWndTree, PGPUserID* pUID) {
	PGPKeyList* pKeyList;
	PGPKeyIter* pKeyIter;
	PGPKey* pKey;
	PGPUserID* pUserID;
	INT iCount;

	pKey = KMGetKeyFromUserID (hWndTree, pUID);
	pKeyList = pgpOrderKeySet (g_pKeySetMain, kPGPAnyOrdering);
	pKeyIter = pgpNewKeyIter (pKeyList);
	pgpKeyIterSet (pKeyIter, pKey);

	pUserID = pgpKeyIterNextUserID (pKeyIter);

	iCount = 0;
	while (pUserID) {
		iCount++;
		pUserID = pgpKeyIterNextUserID (pKeyIter);
	}

	pgpFreeKeyIter (pKeyIter);
	pgpFreeKeyList (pKeyList);

	return (iCount == 1);
}


//----------------------------------------------------|
//  Is this the primary userID on the key ?

BOOL KMIsThisThePrimaryUserID (HWND hWndTree, PGPUserID* pUID) {
	PGPKey* pKey;
	PGPUserID* pPrimaryUserID;

	pKey = KMGetKeyFromUserID (hWndTree, pUID);
	pPrimaryUserID = pgpGetPrimaryUserID (pKey);

	if (pUID == pPrimaryUserID) return TRUE;
	else return FALSE;
}


//----------------------------------------------------|
//  Put up preferences property sheet

BOOL KMPGPPreferences (HWND hWnd, INT iPage) {
	UINT uRetval;

	EnableWindow (hWnd, FALSE);
	uRetval = PGPcomdlgPreferences (hWnd, iPage);
	if (uRetval != PGPCOMDLG_ALREADYOPEN) SetForegroundWindow (hWnd);
	EnableWindow (hWnd, TRUE);

	if (uRetval == PGPCOMDLG_OK) return TRUE;
	else return FALSE;
}


//----------------------------------------------------|
//  Put up preferences property sheet

INT KMCommitKeyRingChanges (PGPKeySet* pKeySet) {
	INT iError;
	HCURSOR hCursorOld;

	hCursorOld = SetCursor (LoadCursor (NULL, IDC_WAIT));
	iError = pgpCommitKeyRingChanges (pKeySet);
	if (!PGPcomdlgErrorMessage (iError)) {
		KMUpdateKeyProperties ();
		PostMessage (HWND_BROADCAST, g_uReloadKeyringMessage, 0,
			GetCurrentProcessId ());
	}
	SetCursor (hCursorOld);
	return iError;
}


//----------------------------------------------------|
//  Determine the appropriate icon for a key, based on
//	its properties

INT KMDetermineKeyIcon (PGPKey* pKey, BOOL* lpbItalics) {

	Boolean bRevoked, bSecret, bDisabled, bExpired;
	UINT iIdx, iAlg;

	pgpGetKeyBoolean (pKey, kPGPKeyPropIsRevoked, &bRevoked);
	pgpGetKeyBoolean (pKey, kPGPKeyPropIsSecret, &bSecret);
	pgpGetKeyBoolean (pKey, kPGPKeyPropIsDisabled, &bDisabled);
	pgpGetKeyBoolean (pKey, kPGPKeyPropIsExpired, &bExpired);
	pgpGetKeyNumber (pKey, kPGPKeyPropAlgId, &iAlg);

	if (iAlg == PGP_PKALG_RSA) {
		if (bSecret) {
			if (bRevoked) iIdx = IDX_RSASECREVKEY;
			else if (bExpired) iIdx = IDX_RSASECEXPKEY;
			else if (bDisabled) iIdx = IDX_RSASECDISKEY;
			else iIdx = IDX_RSASECKEY;
		}
		else {
			if (bRevoked) iIdx = IDX_RSAPUBREVKEY;
			else if (bExpired) iIdx = IDX_RSAPUBEXPKEY;
			else if (bDisabled) iIdx = IDX_RSAPUBDISKEY;
			else iIdx = IDX_RSAPUBKEY;
		}
	}
	// DSA/ElGamal
	else {
		if (bSecret) {
			if (bRevoked) iIdx = IDX_DSASECREVKEY;
			else if (bExpired) iIdx = IDX_DSASECEXPKEY;
			else if (bDisabled) iIdx = IDX_DSASECDISKEY;
			else iIdx = IDX_DSASECKEY;
		}
		else {
			if (bRevoked) iIdx = IDX_DSAPUBREVKEY;
			else if (bExpired) iIdx = IDX_DSAPUBEXPKEY;
			else if (bDisabled) iIdx = IDX_DSAPUBDISKEY;
			else iIdx = IDX_DSAPUBKEY;
		}
	}

	if (lpbItalics) *lpbItalics = bRevoked || bExpired || bDisabled;
	return iIdx;
}
