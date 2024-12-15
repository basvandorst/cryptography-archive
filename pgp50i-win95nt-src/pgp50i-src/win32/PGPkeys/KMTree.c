/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */
//:KMTree.c - handle creating and filling TreeList control
//
//	$Id: KMTree.c,v 1.6 1997/05/27 22:07:07 pbj Exp $
//

#include <windows.h>
#include <process.h>
#include <commctrl.h>
#include "pgpkeys.h"
#include "resource.h"
#include "..\include\treelist.h"

#define BITMAP_WIDTH 16
#define BITMAP_HEIGHT 16

#define LOADINGTIMEOUT	1000

// external globals
extern HINSTANCE g_hInst;
extern HWND g_hWndMain;
extern HWND g_hWndTree;
extern PGPKeySet* g_pKeySetMain;
extern LONG g_lKeyListSortField;
extern CHAR g_szbuf[G_BUFLEN];

// local globals
static BOOL bCancel;
static BOOL bOpeningKeyring;
static HWND hWndLoading;
static HWND hWndLoadingParent;
static HIMAGELIST hIml;
static LONG lCurrentKeyCount, lKeyCountThreshold;


//----------------------------------------------------|
//  Create TreeList Window

HWND KMCreateTreeList (HWND hWndParent) {
	HWND hWndTree;     // handle to tree view window
	RECT rcl;          // rectangle for setting size of window
	HBITMAP hBmp;      // handle to a bitmap
	HBITMAP hBmpMsk;   // handle to a bitmap

	GetClientRect (hWndParent, &rcl);

// Create the tree view window.
	hWndTree = CreateWindowEx (WS_EX_CLIENTEDGE, WC_TREELIST, "",
			WS_VISIBLE | WS_CHILD | WS_BORDER | TLS_HASBUTTONS |
			TLS_HASLINES | TLS_AUTOSCROLL | TLS_PROMISCUOUS,
			0, LOGO_HEIGHT, rcl.right - rcl.left,
			rcl.bottom - rcl.top -LOGO_HEIGHT,
			hWndParent, (HMENU)IDC_TREELIST, g_hInst, NULL);

	if (hWndTree == NULL) return NULL;

// Initialize the tree view window.
// First create the image list you will need.
	hIml = ImageList_Create (BITMAP_WIDTH, BITMAP_HEIGHT,
								ILC_COLOR | ILC_MASK, NUM_BITMAPS, 0);

// Load the bitmaps and add them to the image list, order is important
	// RSA public key
	hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_RSAPUB));
	hBmpMsk = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_RSAPUBMSK));
	ImageList_Add (hIml, hBmp, hBmpMsk);
	DeleteObject (hBmpMsk);
	DeleteObject (hBmp);

	// RSA public disabled key
	hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_RSAPUBDIS));
	hBmpMsk = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_RSAPUBDISMSK));
	ImageList_Add (hIml, hBmp, hBmpMsk);
	DeleteObject (hBmpMsk);
	DeleteObject (hBmp);

	// RSA public revoked key
	hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_RSAPUBREV));
	hBmpMsk = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_RSAPUBREVMSK));
	ImageList_Add (hIml, hBmp, hBmpMsk);
	DeleteObject (hBmpMsk);
	DeleteObject (hBmp);

	// RSA public expired key
	hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_RSAPUBEXP));
	hBmpMsk = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_RSAPUBEXPMSK));
	ImageList_Add (hIml, hBmp, hBmpMsk);
	DeleteObject (hBmpMsk);
	DeleteObject (hBmp);

	// RSA secret key
	hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_RSASEC));
	hBmpMsk = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_RSASECMSK));
	ImageList_Add (hIml, hBmp, hBmpMsk);
	DeleteObject (hBmpMsk);
	DeleteObject (hBmp);

	// RSA secret disabled key
	hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_RSASECDIS));
	hBmpMsk = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_RSASECDISMSK));
	ImageList_Add (hIml, hBmp, hBmpMsk);
	DeleteObject (hBmpMsk);
	DeleteObject (hBmp);

	// RSA secret revoked key
	hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_RSASECREV));
	hBmpMsk = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_RSASECREVMSK));
	ImageList_Add (hIml, hBmp, hBmpMsk);
	DeleteObject (hBmpMsk);
	DeleteObject (hBmp);

	// RSA secret expired key
	hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_RSASECEXP));
	hBmpMsk = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_RSASECEXPMSK));
	ImageList_Add (hIml, hBmp, hBmpMsk);
	DeleteObject (hBmpMsk);
	DeleteObject (hBmp);

	// DSA public key
	hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_DSAPUB));
	hBmpMsk = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_DSAPUBMSK));
	ImageList_Add (hIml, hBmp, hBmpMsk);
	DeleteObject (hBmpMsk);
	DeleteObject (hBmp);

	// DSA public disabled key
	hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_DSAPUBDIS));
	hBmpMsk = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_DSAPUBDISMSK));
	ImageList_Add (hIml, hBmp, hBmpMsk);
	DeleteObject (hBmpMsk);
	DeleteObject (hBmp);

	// DSA public revoked key
	hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_DSAPUBREV));
	hBmpMsk = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_DSAPUBREVMSK));
	ImageList_Add (hIml, hBmp, hBmpMsk);
	DeleteObject (hBmpMsk);
	DeleteObject (hBmp);

	// DSA public expired key
	hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_DSAPUBEXP));
	hBmpMsk = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_DSAPUBEXPMSK));
	ImageList_Add (hIml, hBmp, hBmpMsk);
	DeleteObject (hBmpMsk);
	DeleteObject (hBmp);

	// DSA secret key
	hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_DSASEC));
	hBmpMsk = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_DSASECMSK));
	ImageList_Add (hIml, hBmp, hBmpMsk);
	DeleteObject (hBmpMsk);
	DeleteObject (hBmp);

	// DSA secret disabled key
	hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_DSASECDIS));
	hBmpMsk = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_DSASECDISMSK));
	ImageList_Add (hIml, hBmp, hBmpMsk);
	DeleteObject (hBmpMsk);
	DeleteObject (hBmp);

	// DSA secret revoked key
	hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_DSASECREV));
	hBmpMsk = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_DSASECREVMSK));
	ImageList_Add (hIml, hBmp, hBmpMsk);
	DeleteObject (hBmpMsk);
	DeleteObject (hBmp);

	// DSA secret expired key
	hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_DSASECEXP));
	hBmpMsk = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_DSASECEXPMSK));
	ImageList_Add (hIml, hBmp, hBmpMsk);
	DeleteObject (hBmpMsk);
	DeleteObject (hBmp);

	// RSA user id
	hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_RSAUID));
	hBmpMsk = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_UIDMSK));
	ImageList_Add (hIml, hBmp, hBmpMsk);
	DeleteObject (hBmpMsk);
	DeleteObject (hBmp);

	// DSA user id
	hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_DSAUID));
	hBmpMsk = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_UIDMSK));
	ImageList_Add (hIml, hBmp, hBmpMsk);
	DeleteObject (hBmpMsk);
	DeleteObject (hBmp);

	// certification
	hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_CERT));
	hBmpMsk = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_CERTMSK));
	ImageList_Add (hIml, hBmp, hBmpMsk);
	DeleteObject (hBmpMsk);
	DeleteObject (hBmp);

	// revoked certification
	hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_REVCERT));
	hBmpMsk = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_REVCERTMSK));
	ImageList_Add (hIml, hBmp, hBmpMsk);
	DeleteObject (hBmpMsk);
	DeleteObject (hBmp);

	// bad certification
	hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_BADCERT));
	hBmpMsk = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_BADCERTMSK));
	ImageList_Add (hIml, hBmp, hBmpMsk);
	DeleteObject (hBmpMsk);
	DeleteObject (hBmp);

// Be sure that all the bitmaps were added.
	if (ImageList_GetImageCount (hIml) < NUM_BITMAPS) return FALSE;

// Associate the image list with the tree view control.
	TreeList_SetImageList (hWndTree, hIml);

 	return hWndTree;
}


//----------------------------------------------------|
//  Insert column information into control

BOOL KMAddColumns (HWND hWndTree) {
	TL_COLUMN tlc;
	CHAR sz64[64];
	WORD wWidth[NUMBERCOLUMNS];

	KMGetColWidthsFromRegistry (&wWidth[0], NUMBERCOLUMNS);

	tlc.mask = TLCF_FMT | TLCF_WIDTH | TLCF_TEXT |
				TLCF_SUBITEM | TLCF_DATATYPE | TLCF_DATAMAX;
	tlc.pszText = sz64;

	tlc.iSubItem = 0;
	tlc.fmt = TLCFMT_LEFT;
	tlc.iDataType = TLC_DATASTRING;
	tlc.cx = wWidth[0];
	LoadString (g_hInst, IDS_KEYCOLTITLE, sz64, 64);
	TreeList_InsertColumn (hWndTree, 0, &tlc);

	tlc.iSubItem = 1;
	tlc.fmt = TLCFMT_LINBAR;
	tlc.iDataType = TLC_DATALONG;
	tlc.cchTextMax = KMConvertFromPGPValidity (PGP_VALIDITY_COMPLETE);
	tlc.cx = wWidth[1];
	LoadString (g_hInst, IDS_VALCOLTITLE, sz64, 64);
	TreeList_InsertColumn (hWndTree, 1, &tlc);

	tlc.iSubItem = 2;
	tlc.fmt = TLCFMT_LINBAR;
	tlc.iDataType = TLC_DATALONG;
	tlc.cchTextMax = KMConvertFromPGPTrust (PGP_KEYTRUST_COMPLETE);
	tlc.cx = wWidth[2];
	LoadString (g_hInst, IDS_TRUSTCOLTITLE, sz64, 64);
	TreeList_InsertColumn (hWndTree, 2, &tlc);

	tlc.iSubItem = 3;
	tlc.fmt = TLCFMT_LEFT;
	tlc.iDataType = TLC_DATASTRING;
	tlc.cx = wWidth[3];
	LoadString (g_hInst, IDS_DATECOLTITLE, sz64, 64);
	TreeList_InsertColumn (hWndTree, 3, &tlc);

	tlc.iSubItem = 4;
	tlc.fmt = TLCFMT_LEFT;
	tlc.iDataType = TLC_DATASTRING;
	tlc.cx = wWidth[4];
	LoadString (g_hInst, IDS_BITSCOLTITLE, sz64, 64);
	TreeList_InsertColumn (hWndTree, 4, &tlc);

	return TRUE;
}


//----------------------------------------------------|
//  Set (or add) a tree item to the tree

HTLITEM SetOneItem (BOOL bReInsert, HTLITEM hItem, HTLITEM hParent,
					LPSTR szText, HTLITEM hInsAfter, INT iImage,
					UINT uState, LPARAM lParam,
					HWND hWndTree) {
	TL_TREEITEM tlI;
	TL_INSERTSTRUCT tlIns;

	tlI.hItem = hItem;
	tlI.mask = TLIF_TEXT | TLIF_IMAGE | TLIF_STATE | TLIF_PARAM;
	tlI.stateMask = TLIS_BOLD | TLIS_ITALICS;
	tlI.stateMask |= uState;
	tlI.state = uState;
	tlI.pszText = szText;
	tlI.cchTextMax = lstrlen (szText);
	tlI.iImage = iImage;
	tlI.iSelectedImage = iImage;
	tlI.lParam = lParam;

// Insert the data into the tree.
	if (bReInsert || !hItem) {
		tlIns.item = tlI;
		tlIns.hInsertAfter = hInsAfter;
		tlIns.hParent = hParent;
		return (TreeList_InsertItem (hWndTree, &tlIns));
	}
	else {
		TreeList_SetItem (hWndTree, &tlI);
		return hItem;
	}
}

//----------------------------------------------------|
//  Set list data for a key

HTLITEM SetKeyData (HTLITEM hItem, HWND hWndTree, INT iVal,
					INT iTrust, LPSTR szDate, LPSTR szBits) {
	TL_LISTITEM tlL;
	CHAR szText [128];

	tlL.pszText = szText;
	tlL.hItem = hItem;
	tlL.stateMask = TLIS_VISIBLE;

	tlL.iSubItem = 1;
	tlL.state = TLIS_VISIBLE;
	tlL.mask = TLIF_DATAVALUE | TLIF_STATE;
	tlL.lDataValue = iVal;
	hItem = (HTLITEM) TreeList_SetListItem (hWndTree, &tlL);

	tlL.iSubItem = 2;
	tlL.state = TLIS_VISIBLE;
	tlL.mask = TLIF_DATAVALUE | TLIF_STATE;
	tlL.lDataValue = iTrust;
	hItem = (HTLITEM) TreeList_SetListItem (hWndTree, &tlL);

	tlL.iSubItem = 3;
	tlL.state = TLIS_VISIBLE;
	tlL.mask = TLIF_TEXT | TLIF_STATE;
	lstrcpy (szText, szDate);
	hItem = (HTLITEM) TreeList_SetListItem (hWndTree, &tlL);

	tlL.iSubItem = 4;
	tlL.state = TLIS_VISIBLE;
	tlL.mask = TLIF_TEXT | TLIF_STATE;
	lstrcpy (szText, szBits);
	hItem = (HTLITEM) TreeList_SetListItem (hWndTree, &tlL);

	return (hItem);
}


//----------------------------------------------------|
//  Set treelist list data for a userID

HTLITEM SetIDData (HTLITEM hItem, HWND hWndTree, INT iValidity) {
	TL_LISTITEM tlL;
	CHAR szText [128];

	tlL.pszText = szText;
	tlL.hItem = hItem;
	tlL.stateMask = TLIS_VISIBLE;

	tlL.iSubItem = 1;
	tlL.state = TLIS_VISIBLE;
	tlL.mask = TLIF_DATAVALUE | TLIF_STATE;
	tlL.lDataValue = iValidity;
	hItem = (HTLITEM) TreeList_SetListItem (hWndTree, &tlL);

	return (hItem);
}


//----------------------------------------------------|
//  Set treelist list data for a certification

HTLITEM SetCertData (HTLITEM hItem, HWND hWndTree, LPSTR szDate) {
	TL_LISTITEM tlL;
	char szText [128];

	tlL.pszText = szText;
	tlL.hItem = hItem;
	tlL.stateMask = TLIS_VISIBLE;

	tlL.iSubItem = 3;
	tlL.state = TLIS_VISIBLE;
	tlL.mask = TLIF_TEXT | TLIF_STATE;
	lstrcpy (szText, szDate);
	hItem = (HTLITEM) TreeList_SetListItem (hWndTree, &tlL);

	return (hItem);
}


//----------------------------------------------------|
//  Construct string representation of number of key bits

VOID GetKeyBitsString (PGPKey* pKey, LPSTR sz, UINT u) {
	UINT uAlg;
	UINT uKeyBits, uSubKeyBits;
	PGPSubKey* pSubKey;
	PGPKeyList* pKeyList;
	PGPKeyIter* pKeyIter;
	INT iError;

	pgpGetKeyNumber (pKey, kPGPKeyPropAlgId, &uAlg);
	switch (uAlg) {
	case PGP_PKALG_RSA :
		pgpGetKeyNumber (pKey, kPGPKeyPropBits, &uKeyBits);
		wsprintf (g_szbuf, "%i", uKeyBits);
		lstrcpyn (sz, g_szbuf, u);
		break;

	case PGP_PKALG_DSA :
		iError = pgpGetKeyNumber (pKey, kPGPKeyPropBits, &uKeyBits);
		pKeyList = pgpOrderKeySet (g_pKeySetMain, kPGPAnyOrdering);
		pKeyIter = pgpNewKeyIter (pKeyList);
		pgpKeyIterSet (pKeyIter, pKey);
		pSubKey = pgpKeyIterNextSubKey (pKeyIter);
		if (pSubKey) {
			pgpGetSubKeyNumber (pSubKey, kPGPKeyPropBits, &uSubKeyBits);
			wsprintf (g_szbuf, "%i/%i", uKeyBits, uSubKeyBits);
		}
		else wsprintf (g_szbuf, "%i", uKeyBits);
		pgpFreeKeyIter (pKeyIter);
		pgpFreeKeyList (pKeyList);
		lstrcpyn (sz, g_szbuf, u);
		break;

	default :
		LoadString (g_hInst, IDS_UNKNOWN, sz, u);
		break;
	}
}


//----------------------------------------------------|
//  Reload a single key

HTLITEM ReloadKey (HWND hWndTree, PGPKeyIter* pKeyIter, PGPKey* pKey,
				      BOOL bReInsert, BOOL bForceNewAlloc, BOOL bExpandNew,
					 BOOL bFirstCall, HTLITEM hTPrev) {

	HTLITEM hTKey, hTUID, hTCert;
	TL_TREEITEM tli;
	INT idx, iAlg;
	CHAR sz[256];
	CHAR szBits[16];
	CHAR szID[32];
	UINT u, uTrust, uValidity, uState;
	PGPTime time;
	BOOL bItalics, bNew;
	Boolean bRevoked, bAxiomatic;
	Boolean bVerified, bTried, bNotCorrupt;
	PGPUserID* pUserID;
	PGPCert* pCert;
	PGPKey* pCertKey;
	static BOOL bNewKeyExpanded;

	uState = 0;
	bNew = FALSE;
	if (bFirstCall) bNewKeyExpanded = FALSE;

	pgpGetKeyUserVal (pKey, (long*)&hTKey);
	if (!hTKey) {
		pgpIncKeyRefCount (pKey);
		bNew = TRUE;
	}

	// determine icon and italics
	idx = KMDetermineKeyIcon (pKey, &bItalics);
	if (bItalics) uState |= TLIS_ITALICS;

	// get primary userid name string
	KMGetKeyName (pKey, sz, sizeof(sz));

	// insert key item into tree and save pointer to tree item
	if (!hTKey && bExpandNew) uState |= TLIS_SELECTED;
	if (bForceNewAlloc) hTKey = NULL;
	hTKey = SetOneItem (bReInsert, hTKey, NULL, sz, hTPrev, idx,
						uState, (LPARAM)pKey, hWndTree);
	pgpSetKeyUserVal (pKey, (long)hTKey);

	// if a reinsertion, then we're done
	if (bReInsert) return hTKey;

	// get and set treelist list data for this key
	if (PGPcomdlgErrorMessage (pgpGetPrimaryUserIDValidity (pKey,&uValidity)))
		uValidity = 0;
	else
		uValidity = KMConvertFromPGPValidity (uValidity);
	GetKeyBitsString (pKey, szBits, sizeof(szBits));
	pgpGetKeyNumber (pKey, kPGPKeyPropTrust, &uTrust);
	uTrust = KMConvertFromPGPTrust (uTrust);
	pgpGetKeyTime (pKey, kPGPKeyPropCreation, &time);

	KMConvertTimeToString (&time, sz, sizeof(sz));
	// if axiomatic set trust and validity to out-of-range values
	//  in order to flag different graphical representation
	pgpGetKeyBoolean (pKey, kPGPKeyPropIsAxiomatic, &bAxiomatic);
	if (bAxiomatic) {
		uTrust = KMConvertFromPGPTrust (PGP_KEYTRUST_ULTIMATE) + 1;		
		uValidity = KMConvertFromPGPValidity (PGP_VALIDITY_COMPLETE) + 1;
	}
	SetKeyData (hTKey, hWndTree, uValidity, uTrust, sz, szBits);

	// iterate through userids
	pUserID = pgpKeyIterNextUserID (pKeyIter);
	while (pUserID) {
		uState = 0;
		pgpGetUserIDUserVal (pUserID, (long*)&hTUID);
		if (!hTUID) bNew = TRUE;

		// get and set treelist tree data for this userid
		KMGetUserIDName (pUserID, sz, sizeof(sz));
		if (bForceNewAlloc) hTUID = NULL;
		pgpGetKeyNumber (pKey, kPGPKeyPropAlgId, &iAlg);
		if (iAlg == PGP_PKALG_RSA) idx = IDX_RSAUSERID;
		else idx = IDX_DSAUSERID;
		hTUID = SetOneItem (bReInsert, hTUID, hTKey, sz, (HTLITEM)TLI_LAST,
							idx, uState, (LPARAM)pUserID, hWndTree);
		pgpSetUserIDUserVal (pUserID, (long)hTUID);

		// get and set treelist list data for this userid
		pgpGetUserIDNumber (pUserID, kPGPUserIDPropValidity, &u);
		u = KMConvertFromPGPValidity (u);
		SetIDData (hTUID, hWndTree, u);
		
		// iterate through certifications
		pCert = pgpKeyIterNextUIDCert (pKeyIter);
		while (pCert) {
			pgpGetCertUserVal (pCert, (long*)&hTCert);
			if (!hTCert) bNew = TRUE;

			// get and set treelist tree data for this cert
			PGPcomdlgErrorMessage (pgpGetCertifier (pCert, g_pKeySetMain,
													&pCertKey));
			if (pCertKey) {
				KMGetKeyName (pCertKey, sz, sizeof(sz));
			}
			else {
				u = sizeof(szID);
				pgpGetCertString (pCert, kPGPCertPropKeyID, szID, &u);
				KMConvertStringKeyID (szID);
				LoadString (g_hInst, IDS_UNAVAILABLECERT,
									g_szbuf, sizeof(g_szbuf));
				wsprintf (sz, g_szbuf, szID);
			}
			pgpGetCertBoolean (pCert, kPGPCertPropIsRevoked, &bRevoked);
			pgpGetCertBoolean (pCert, kPGPCertPropIsVerified, &bVerified);
			pgpGetCertBoolean (pCert, kPGPCertPropIsTried, &bTried);
			pgpGetCertBoolean (pCert, kPGPCertPropIsNotCorrupt, &bNotCorrupt);

			if (bRevoked) idx = IDX_REVCERT;
			else if (bVerified) idx = IDX_CERT;
			else if (bTried) idx = IDX_BADCERT;
			else if (bNotCorrupt) idx = IDX_CERT;
			else idx = IDX_BADCERT;

			if ((idx != IDX_CERT) || !bVerified) uState = TLIS_ITALICS;
			else uState = 0;

			if (bForceNewAlloc) hTCert = NULL;
			hTCert = SetOneItem (bReInsert, hTCert, hTUID, sz,
								(HTLITEM)TLI_LAST, idx,
								uState, (LPARAM)pCert, hWndTree);
			pgpSetCertUserVal (pCert, (long)hTCert);

			// get and set treelist list data for this cert
			pgpGetCertTime (pCert, kPGPCertPropCreation, &time);
			KMConvertTimeToString (&time, sz, sizeof(sz));
			SetCertData (hTCert, hWndTree, sz);

			pCert = pgpKeyIterNextUIDCert (pKeyIter);
		}
		pUserID = pgpKeyIterNextUserID (pKeyIter);
	}

	// select and expand key, if appropriate
	if (bExpandNew && bNew) {
		tli.hItem = hTKey;
		if (!bNewKeyExpanded) {
			TreeList_Select (hWndTree, &tli, TRUE);
			TreeList_Expand (hWndTree, &tli, TLE_EXPANDALL);
			bNewKeyExpanded = TRUE;
		}
		else {
			KMSetFocus (KMFocusedItem (), TRUE);
			TreeList_Expand (hWndTree, &tli, TLE_EXPANDALL);
		}
	}

	return hTKey;
}


//----------------------------------------------------|
// "Working..." dialog message procedure

BOOL CALLBACK LoadingDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam,
							LPARAM lParam) {

	static CHAR sz64[64];
	static HCURSOR hCursorWait;

	switch (uMsg) {

	case WM_INITDIALOG:
		if (bCancel) EndDialog (hDlg, 0);
		else {
			hWndLoading = hDlg;
			SetForegroundWindow (hDlg);
			hCursorWait = LoadCursor (NULL, IDC_WAIT);
		}
		return FALSE;

    case WM_DESTROY:
		hWndLoading = NULL;
        break;

	case WM_SETCURSOR :
		if (hCursorWait) {
			SetCursor (hCursorWait);
			return TRUE;
		}
		break;

	case WM_USER :
		hCursorWait = NULL;
		EnableWindow (GetDlgItem (hDlg, IDCANCEL), TRUE);
		if (wParam) SetDlgItemText (hDlg, IDC_KEYSTRING, (LPSTR)lParam);
		else EndDialog (hDlg, 0);
		return TRUE;

	case WM_COMMAND:
		switch(LOWORD (wParam)) {
		case IDCANCEL:
			bCancel = TRUE;
			EndDialog (hDlg, 0);
			break;
		}
		return TRUE;
	}
	return FALSE;
}


//----------------------------------------------------|
//

VOID LoadingThread (PVOID pvoid) {
	Sleep (LOADINGTIMEOUT);
	if (lCurrentKeyCount < lKeyCountThreshold) {
		DialogBox (g_hInst, MAKEINTRESOURCE (IDD_LOADING),
			hWndLoadingParent, LoadingDlgProc);
	}
}


//----------------------------------------------------|
//  Scan entire keyring loading in any new data

BOOL KMLoadKeyRingIntoTree (HWND hParent, HWND hTree,
							BOOL bReInsert, BOOL bExpandNew) {

	HCURSOR hCursorOld;
	PGPKeyList* pKeyList;
	PGPKeyIter* pKeyIter;
	PGPKey* pKey;
	PGPKey* pDefaultKey;
	TL_TREEITEM tli;
	CHAR szKeyString[264];
	CHAR sz1[256];
	UINT u;
	HTLITEM hTPrevKey;
	BOOL bFirst;

	lCurrentKeyCount = 0;
	hTPrevKey = (HTLITEM)TLI_FIRST;
	bFirst = TRUE;
	lKeyCountThreshold = 1;
	bCancel = FALSE;

	hWndLoadingParent = hParent;
	hWndLoading = NULL;
	_beginthreadex (NULL, 0, (LPTHREAD_START_ROUTINE)LoadingThread,
								0, 0, &u);

	lKeyCountThreshold = pgpCountKeys (g_pKeySetMain);
	lKeyCountThreshold >>= 2;

	if (g_pKeySetMain) {
		hCursorOld = SetCursor (LoadCursor (NULL, IDC_WAIT));
		pKeyList = pgpOrderKeySet (g_pKeySetMain, g_lKeyListSortField);
		pKeyIter = pgpNewKeyIter (pKeyList);

		pKey = pgpKeyIterNext (pKeyIter);

		while (pKey) {
			lCurrentKeyCount++;

			if (bCancel) break;

			KMGetKeyName (pKey, sz1, sizeof(sz1));
			wsprintf (szKeyString, "%i: %s", lCurrentKeyCount, sz1);
			if (hWndLoading) SendMessage (hWndLoading, WM_USER,
											TRUE, (LPARAM)szKeyString);

			hTPrevKey = ReloadKey (hTree, pKeyIter, pKey, bReInsert, FALSE,
									bExpandNew, bFirst, hTPrevKey);
			pKey = pgpKeyIterNext (pKeyIter);
			bFirst = FALSE;
		}
		pgpFreeKeyIter (pKeyIter);
		pgpFreeKeyList (pKeyList);

		pDefaultKey = pgpGetDefaultPrivateKey (g_pKeySetMain);
		if (pDefaultKey) {
			pgpGetKeyUserVal (pDefaultKey, (long*)&(tli.hItem));
			if (tli.hItem) {
				tli.state = TLIS_BOLD;
				tli.stateMask = TLIS_BOLD;
				tli.mask = TLIF_STATE;
				TreeList_SetItem (hTree, &tli);
			}
		}

		if (bCancel) {
			pgpFreeKeySet (g_pKeySetMain);
			g_pKeySetMain = NULL;
			TreeList_DeleteTree (hTree, TRUE);
		}
		else {
			bCancel = TRUE;
			if (hWndLoading) SendMessage (hWndLoading, WM_USER, FALSE, 0);
		}
		SetCursor (hCursorOld);
		if (bExpandNew) {
			tli.hItem = KMFocusedItem ();
			if (tli.hItem) TreeList_EnsureVisible (hTree, &tli);
		}
		return TRUE;
	}

	// error on open key rings
	else {
		bCancel = TRUE;
		if (hWndLoading) SendMessage (hWndLoading, WM_USER, FALSE, 0);
		KMMessageBox (g_hWndMain, IDS_CAPTION, IDS_CANNOTOPENRINGS,
								MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}
}


//----------------------------------------------------|
//  Insert single key into treelist

BOOL KMUpdateKeyInTree (HWND hTree, PGPKey* pKey) {

	PGPKeyList* pKeyList;
	PGPKeyIter* pKeyIter;
	PGPKey* pPrevKey;
	PGPKey* pDefaultKey;
	TL_TREEITEM tli;
	HTLITEM hTPrevKey;

	if (g_pKeySetMain) {	
		if (!PGPcomdlgErrorMessage (pgpGetKeyUserVal (pKey,
					(long*)&(tli.hItem)))) {
			if (tli.hItem) {
				TreeList_DeleteItem (hTree, &tli);
				pgpSetKeyUserVal (pKey, 0L);
				pgpFreeKey (pKey);
			}

			pKeyList = pgpOrderKeySet (g_pKeySetMain, g_lKeyListSortField);
			pKeyIter = pgpNewKeyIter (pKeyList);
			pgpKeyIterSet (pKeyIter, pKey);
			pPrevKey = pgpKeyIterPrev (pKeyIter);
			pgpKeyIterSet (pKeyIter, pKey);

			hTPrevKey = (HTLITEM)TLI_FIRST;
			if (pPrevKey) {
				if (!PGPcomdlgErrorMessage (pgpGetKeyUserVal (pPrevKey,
						(long*)&(tli.hItem)))) {
					hTPrevKey = tli.hItem;
				}
			}	

			ReloadKey (hTree, pKeyIter, pKey, FALSE, TRUE, FALSE,
						TRUE, hTPrevKey);

			pgpFreeKeyIter (pKeyIter);
			pgpFreeKeyList (pKeyList);

			pDefaultKey = pgpGetDefaultPrivateKey (g_pKeySetMain);
			if (pDefaultKey == pKey) {
				pgpGetKeyUserVal (pDefaultKey, (long*)&(tli.hItem));
				if (tli.hItem) {
					tli.state = TLIS_BOLD;
					tli.stateMask = TLIS_BOLD;
					tli.mask = TLIF_STATE;
					TreeList_SetItem (hTree, &tli);
				}
			}

			pgpGetKeyUserVal (pKey, (long*)&(tli.hItem));
			TreeList_Select (hTree, &tli, TRUE);

			return TRUE;
		}
	}
	return FALSE;
}


//----------------------------------------------------|
//  Set validity for a treelist item

HTLITEM SetItemValidity (HTLITEM hItem, HWND hWndTree, int iValidity) {
	TL_LISTITEM tlL;

	tlL.hItem = hItem;

	tlL.iSubItem = 1;
	tlL.mask = TLIF_DATAVALUE;
	tlL.lDataValue = iValidity;
	hItem = (HTLITEM) TreeList_SetListItem (hWndTree, &tlL);

	return (hItem);
}


//----------------------------------------------------|
//  Scan entire keyring loading in any new data

BOOL KMUpdateAllValidities (void) {

	HCURSOR hCursorOld;
	HTLITEM hTKey, hTUID;
	PGPKeyList* pKeyList;
	PGPKeyIter* pKeyIter;
	PGPKey* pKey;
	PGPUserID* pUserID;
	Boolean bAxiomatic;
	UINT u;

	if (g_pKeySetMain) {
		hCursorOld = SetCursor (LoadCursor (NULL, IDC_WAIT));
		pKeyList = pgpOrderKeySet (g_pKeySetMain, g_lKeyListSortField);
		pKeyIter = pgpNewKeyIter (pKeyList);

		pKey = pgpKeyIterNext (pKeyIter);

		// iterate through keys
		while (pKey) {
			pgpGetKeyUserVal (pKey, (long*)&hTKey);
			if (hTKey) {
				// if axiomatic set trust and validity to out-of-range values
				//  in order to flag different graphical representation
				pgpGetKeyBoolean (pKey, kPGPKeyPropIsAxiomatic, &bAxiomatic);
				if (bAxiomatic)
					u = KMConvertFromPGPValidity (PGP_VALIDITY_COMPLETE) + 1;
				else {
					if (PGPcomdlgErrorMessage (pgpGetPrimaryUserIDValidity (
						pKey, &u))) u = 0;
					else
						u = KMConvertFromPGPValidity (u);
				}
				SetItemValidity (hTKey, g_hWndTree, u);
			}

			// iterate through userids
			pUserID = pgpKeyIterNextUserID (pKeyIter);
			while (pUserID) {
				pgpGetUserIDUserVal (pUserID, (long*)&hTUID);
				if (hTUID) {
					pgpGetUserIDNumber (pUserID, kPGPUserIDPropValidity, &u);
					u = KMConvertFromPGPValidity (u);
					SetItemValidity (hTUID, g_hWndTree, u);
				}
				pUserID = pgpKeyIterNextUserID (pKeyIter);
			}
			pKey = pgpKeyIterNext (pKeyIter);
		}
		pgpFreeKeyIter (pKeyIter);
		pgpFreeKeyList (pKeyList);
		SetCursor (hCursorOld);

		return TRUE;
	}

	// error on open key rings
	else return FALSE;
}


//----------------------------------------------------|
//  Expand a single item
//	routine called as a
//	callback function from the TreeList control to
//	expand a single item.
//
//	lptli	= pointer to TreeList item to expand

BOOL ExpandSingleItem (TL_TREEITEM* lptli) {
	TreeList_Expand (g_hWndTree, lptli, TLE_EXPANDALL);
	return TRUE;
}


//----------------------------------------------------|
//  Expand the selected items

BOOL KMExpandSelected (HWND hTree) {
	TL_TREEITEM tli;

	if (KMMultipleSelected ()) {
		TreeList_IterateSelected (hTree, ExpandSingleItem);
		InvalidateRect (hTree, NULL, TRUE);
		return TRUE;
	}
	else {
		tli.hItem = KMFocusedItem ();
		TreeList_Expand (hTree, &tli, TLE_EXPANDALL);
		InvalidateRect (hTree, NULL, TRUE);
		return TRUE;
	}
	return FALSE;
}


//----------------------------------------------------|
//  Collapse a single item
//	routine called as a
//	callback function from the TreeList control to
//	collapse a single item.
//
//	lptli	= pointer to TreeList item to collapse

BOOL CollapseSingleItem (TL_TREEITEM* lptli) {
	TreeList_Expand (g_hWndTree, lptli, TLE_COLLAPSEALL);
	return TRUE;
}


//----------------------------------------------------|
//  Collapse the selected items

BOOL KMCollapseSelected (HWND hTree) {
	TL_TREEITEM tli;

	if (KMMultipleSelected ()) {
		TreeList_IterateSelected (hTree, CollapseSingleItem);
		InvalidateRect (hTree, NULL, TRUE);
		return TRUE;
	}
	else {
		tli.hItem = KMFocusedItem ();
		TreeList_Expand (hTree, &tli, TLE_COLLAPSEALL);
		InvalidateRect (hTree, NULL, TRUE);
		return TRUE;
	}
	return FALSE;
}


//----------------------------------------------------|
//  Cleanup treelist

BOOL KMDestroyTree (void) {
	SendMessage (g_hWndTree, WM_CLOSE, 0, 0);
	ImageList_Destroy (hIml);
	return TRUE;
}
