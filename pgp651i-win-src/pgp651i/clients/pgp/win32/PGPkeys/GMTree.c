/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	GMTree.h - create and fill group manager control
	

	$Id: GMTree.c,v 1.35 1999/04/01 03:48:19 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

// project header files
#include "pgpgmx.h"

// constant definitions
#define BITMAP_WIDTH 16
#define BITMAP_HEIGHT 16

// external globals
extern HINSTANCE g_hInst;

//	___________________________________________________
//
//  Return handle of group manager window

HWND  
PGPgmGetManagerWindow (HGROUPMAN hGM) 
{
	if (!hGM) return NULL;
	return (((PGROUPMAN)hGM)->hWndTree);
}


//	___________________________________________________
//
//  Insert column information into control

BOOL 
GMAddColumns (PGROUPMAN pGM) 
{
	TL_COLUMN tlc;
	CHAR sz[64];
	INT iField; 
	INT iCol, ids;

	TreeList_DeleteAllColumns (pGM->hWndTree);

	tlc.mask = TLCF_FMT | TLCF_WIDTH | TLCF_TEXT | 
				TLCF_SUBITEM | TLCF_DATATYPE | TLCF_DATAMAX |
				TLCF_MOUSENOTIFY;
	tlc.pszText = sz;

	tlc.iSubItem = 0;
	tlc.fmt = TLCFMT_LEFT;
	tlc.iDataType = TLC_DATASTRING;
	tlc.cx = pGM->wFieldWidth[0];
	tlc.bMouseNotify = FALSE;
	LoadString (g_hInst, IDS_GROUPNAMEFIELD, sz, sizeof(sz));
	TreeList_InsertColumn (pGM->hWndTree, 0, &tlc);

	for (iCol=1; iCol<NUMBERFIELDS; iCol++) {
		iField = pGM->wColumnField[iCol];
		if (iField) {
			switch (iField) {
				case GMI_VALIDITY :
					ids = IDS_VALIDITYFIELD;
					if (pGM->ulOptionFlags & GMF_NOVICEMODE) 
						tlc.fmt = TLCFMT_IMAGE;
					else 
						tlc.fmt = TLCFMT_LINBAR;
					tlc.cchTextMax = 
							GMConvertFromPGPValidity (kPGPValidity_Complete);
					tlc.iDataType = TLC_DATALONG;
					tlc.bMouseNotify = FALSE;
					break;

				case GMI_DESCRIPTION :
					ids = IDS_DESCRIPTIONFIELD;
					tlc.fmt = TLCFMT_LEFT;
					tlc.iDataType = TLC_DATASTRING;
					tlc.bMouseNotify = FALSE;
					break;
			}
			LoadString (g_hInst, ids, sz, sizeof(sz));
			tlc.cx = pGM->wFieldWidth[iField];
			TreeList_InsertColumn (pGM->hWndTree, iCol, &tlc);
		}
	}

	return TRUE;
}

//	___________________________________________________
//
//  Create TreeList Window

HGROUPMAN 
PGPgmCreateGroupManager (
		PGPContextRef		Context, 
		PGPtlsContextRef	tlsContext,
		HWND				hWndParent, 
		INT					iID, 
		INT					x, 
		INT					y,
		INT					nWidth, 
		INT					nHeight) 
{

	HBITMAP		hBmp;      // handle to a bitmap
	PGROUPMAN	pGM;
	HDC			hDC;
	INT			iNumBits;

	// Ensure that the common control DLL is loaded.
	InitCommonControls ();

	// Ensure that the custom control DLL is loaded.
	InitTreeListControl ();

	pGM = gmAlloc (sizeof (_GROUPMAN));
	if (!pGM) return NULL;
	memset (pGM, 0x00, sizeof (_GROUPMAN));

	pGM->hWndParent = hWndParent;
	pGM->hWndTree = NULL;
	pGM->groupsetMain = NULL;
	pGM->iID = iID;
	pGM->hIml = NULL;
	pGM->pDropTarget = NULL;			//pointer to DropTarget object
	lstrcpy (pGM->szHelpFile, "");		//string containing name of help file

	pGM->context = Context;			//PGP context
	pGM->tlsContext = tlsContext;
	pGM->ulOptionFlags = 0;
	pGM->ulDisableActions = 0;

	pGM->bMultipleSelected = FALSE;
	pGM->bLocatingKeys = FALSE;
	pGM->uSelectedFlags = 0;
	pGM->iFocusedItemType = 0;
	pGM->iFocusedObjectType = 0;
	pGM->hFocusedItem = NULL;
	pGM->pFocusedObject = NULL;

	pGM->iValidityThreshold = GM_VALIDITY_MARGINAL;

	GMGetColumnPreferences (pGM);

	// Create the tree view window.
	pGM->hWndTree = CreateWindowEx (WS_EX_CLIENTEDGE, WC_TREELIST, "",
			WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP | WS_GROUP |
			TLS_HASBUTTONS | TLS_HASLINES | TLS_AUTOSCROLL | 
			TLS_PROMISCUOUS | TLS_INTERNALDRAG | TLS_DRAGABLEHEADERS,
			x, y, nWidth, nHeight,
			hWndParent, (HMENU)iID, g_hInst, NULL);

	if (pGM->hWndTree == NULL) return NULL;

	// Initialize the tree view window.
	// First create imagelist and load the appropriate bitmaps 
	// based on current display capabilities.
	
	hDC = GetDC (NULL);		// DC for desktop
	iNumBits = GetDeviceCaps (hDC, BITSPIXEL) * GetDeviceCaps (hDC, PLANES);
	ReleaseDC (NULL, hDC);

	if (iNumBits <= 8) {
		pGM->hIml =	ImageList_Create (16, 16, ILC_COLOR|ILC_MASK, 
										NUM_BITMAPS, 0); 
		hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_IMAGES4BIT));
		ImageList_AddMasked (pGM->hIml, hBmp, RGB(255, 0, 255));
		DeleteObject (hBmp);
	}
	else {
		pGM->hIml =	ImageList_Create (16, 16, ILC_COLOR24|ILC_MASK, 
										NUM_BITMAPS, 0); 
		hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_IMAGES24BIT));
		ImageList_AddMasked (pGM->hIml, hBmp, RGB(255, 0, 255));
		DeleteObject (hBmp);
	}

	// Associate the image list with the tree view control.
	TreeList_SetImageList (pGM->hWndTree, pGM->hIml);

	GMSetFocus (pGM, NULL, FALSE);
	pGM->pDropTarget = GMCreateDropTarget (pGM->hWndParent, (VOID*)pGM); 
	CoLockObjectExternal ((IUnknown*)pGM->pDropTarget, TRUE, TRUE);
	RegisterDragDrop (pGM->hWndTree, pGM->pDropTarget);
	GMEnableDropTarget (pGM->pDropTarget, FALSE);

 	return (HGROUPMAN)pGM;
}


//	___________________________________________________
//
//  Set (or add) a tree item to the tree

static HTLITEM 
sSetOneGroupItem (
		PGROUPMAN	pGM, 
		BOOL		bReInsert, 
		HTLITEM		hItem, 
		HTLITEM		hParent, 
		LPSTR		szText, 
		HTLITEM		hInsAfter, 
		INT			iImage, 
		UINT		uState, 
		LPARAM		lParam) 
{
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
		return (TreeList_InsertItem (pGM->hWndTree, &tlIns));
	}
	else {
		TreeList_SetItem (pGM->hWndTree, &tlI);
		return hItem;
	}
}


//	___________________________________________________
//
//  Set list data for a key

static HTLITEM 
sSetGroupData (
		PGROUPMAN	pGM, 
		HTLITEM		hItem, 
		PGPGroupID	groupid, 
		LPSTR		pszDesc) 
{ 
	TL_LISTITEM tlL;
	INT iField, iCol;

	INT iValue, iNumNotFound;

	tlL.hItem = hItem;
	tlL.stateMask = TLIS_VISIBLE;

	for (iCol=1; iCol<NUMBERFIELDS; iCol++) {
		iField = pGM->wColumnField[iCol];
		if (iField) {
			switch (iField) {
			case GMI_VALIDITY :
				tlL.pszText = NULL;

				PGPGetGroupLowestValidity (pGM->groupsetMain, groupid, 
							pGM->keysetMain, &iValue, &iNumNotFound);

				if (iNumNotFound > 0) iValue = 0;
				else iValue = GMConvertFromPGPValidity (iValue);

				tlL.state = TLIS_VISIBLE;
				tlL.mask = TLIF_DATAVALUE | TLIF_STATE;
				if (pGM->ulOptionFlags & GMF_NOVICEMODE) {
					if (iValue > GM_VALIDITY_COMPLETE)
						tlL.lDataValue = IDX_AXIOMATIC;
					else if (iValue >= pGM->iValidityThreshold) 
						tlL.lDataValue = IDX_VALID;
					else tlL.lDataValue = IDX_INVALID;
				}
				else tlL.lDataValue = iValue;
				break;

			case GMI_DESCRIPTION :
				tlL.state = TLIS_VISIBLE;
				tlL.mask = TLIF_TEXT | TLIF_STATE;
				tlL.pszText = pszDesc;
				break;

			}
			tlL.iSubItem = iCol;
			hItem = (HTLITEM) TreeList_SetListItem (pGM->hWndTree, 
													&tlL, FALSE);
		}
	}

	return (hItem);
}


//	___________________________________________________
//
//  Set list data for a key

static HTLITEM 
sSetKeyData (
		PGROUPMAN	pGM, 
		HTLITEM		hItem, 
		PGPKeyRef	key, 
		LPSTR		pszDesc) 
{ 
	TL_LISTITEM	tlL;
	INT			iField, iCol;
	PGPBoolean	bAxiomatic;

	INT iValue;

	tlL.hItem = hItem;
	tlL.stateMask = TLIS_VISIBLE;

	for (iCol=1; iCol<NUMBERFIELDS; iCol++) {
		iField = pGM->wColumnField[iCol];
		if (iField) {
			switch (iField) {
			case GMI_VALIDITY :
				PGPGetPrimaryUserIDValidity (key, &iValue);
				iValue = GMConvertFromPGPValidity (iValue);
				PGPGetKeyBoolean (key, kPGPKeyPropIsAxiomatic, &bAxiomatic);
				if (bAxiomatic) iValue = 
						GMConvertFromPGPValidity (kPGPValidity_Complete)+1;
				tlL.state = TLIS_VISIBLE;
				tlL.mask = TLIF_DATAVALUE | TLIF_STATE;
				if (pGM->ulOptionFlags & GMF_NOVICEMODE) {
					if (iValue > GM_VALIDITY_COMPLETE)
						tlL.lDataValue = IDX_AXIOMATIC;
					else if (iValue >= pGM->iValidityThreshold) 
						tlL.lDataValue = IDX_VALID;
					else tlL.lDataValue = IDX_INVALID;
				}
				else tlL.lDataValue = iValue;
				break;

			case GMI_DESCRIPTION :
				tlL.state = TLIS_VISIBLE;
				tlL.mask = TLIF_TEXT | TLIF_STATE;
				tlL.pszText = pszDesc;
				break;

			}
			tlL.iSubItem = iCol;
			hItem = (HTLITEM) TreeList_SetListItem (pGM->hWndTree, 
													&tlL, FALSE);
		}
	}

	return (hItem);
}


//	___________________________________________________
//
//  Determine the appropriate icon for a key, based on
//	its properties

static INT 
sDetermineKeyIcon (PGPKeyRef Key, BOOL* lpbItalics) 
{

	PGPBoolean bRevoked, bSecret, bDisabled, bExpired, bSplit;
	PGPUInt32 iIdx, iAlg;

	PGPGetKeyBoolean (Key, kPGPKeyPropIsRevoked, &bRevoked);
	PGPGetKeyBoolean (Key, kPGPKeyPropIsSecret, &bSecret);
	PGPGetKeyBoolean (Key, kPGPKeyPropIsDisabled, &bDisabled);
	PGPGetKeyBoolean (Key, kPGPKeyPropIsExpired, &bExpired);
	PGPGetKeyBoolean (Key, kPGPKeyPropIsSecretShared, &bSplit);
	PGPGetKeyNumber (Key, kPGPKeyPropAlgID, &iAlg);

	if (iAlg == kPGPPublicKeyAlgorithm_RSA) {
		if (bSecret) {
			if (bRevoked) iIdx = IDX_RSASECREVKEY;
			else if (bExpired) iIdx = IDX_RSASECEXPKEY;
			else if (bDisabled) iIdx = IDX_RSASECDISKEY;
			else if (bSplit) iIdx = IDX_RSASECSHRKEY;
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
			else if (bSplit) iIdx = IDX_DSASECSHRKEY;
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


//	___________________________________________________
//
//  Reload a single group

static HTLITEM 
sReloadGroup (
		PGROUPMAN	pGM, 
		PGPGroupID	groupid, 
		INT			iIndex,
		BOOL		bReInsert, 
		BOOL		bForceNewAlloc, 
		BOOL		bExpandNew,
		BOOL		bFirstCall, 
		HTLITEM		hTParent, 
		HTLITEM		hTPrev) 
{

	HTLITEM				hTGroup, hTItem, hTItemPrev;
	TL_TREEITEM			tli;
	INT					i, iNumKeys, iNumTotal, idx;
	UINT				u, uState, uVal;
	BOOL				bNew, bItalics;
	PGPGroupInfo		groupinfo;
	PGPGroupItem		groupitem;
	PGPGroupID			groupidThis, groupidParent;
	PGPKeyRef			key;
	PGPError			err;
	CHAR				sz[kPGPMaxUserIDSize];
	CHAR				szID[kPGPMaxKeyIDStringSize];
	static BOOL			bNewKeyExpanded;

	uState = 0;
	bNew = FALSE;
	hTItem = NULL;
	hTItemPrev = TLI_FIRST;

	if (bFirstCall) bNewKeyExpanded = FALSE;

	if (hTParent) {
		groupidParent = groupid;
		PGPGetIndGroupItem (pGM->groupsetMain, groupidParent, 
												iIndex, &groupitem);
		groupidThis = groupitem.u.group.id;
		PGPGetGroupInfo (pGM->groupsetMain, groupidThis, &groupinfo);
		hTGroup = groupitem.userValue;
	}
	else {
		groupidThis = groupid;
		PGPGetGroupInfo (pGM->groupsetMain, groupidThis, &groupinfo);
		hTGroup = groupinfo.userValue;
	}
	if (!hTGroup) bNew = TRUE;

	// determine icon and italics
	idx = IDX_GROUP;

	// insert group item into tree and save pointer to tree item
	if (bNew && bExpandNew) uState |= TLIS_SELECTED;
	if (bForceNewAlloc) hTGroup = NULL;
	if (hTParent) {
		uVal = MAKELONG (iIndex, groupidParent);
		hTGroup = sSetOneGroupItem (pGM, bReInsert, hTGroup, hTParent, 
					groupinfo.name, hTPrev, idx, uState, uVal);
		PGPSetIndGroupItemUserValue (pGM->groupsetMain, groupidParent, 
					iIndex, (PGPUserValue)hTGroup);
	}
	else {
		uVal = MAKELONG (groupidThis, 0);
		hTGroup = sSetOneGroupItem (pGM, bReInsert, hTGroup, hTParent, 
					groupinfo.name, hTPrev, idx, uState, uVal);
		PGPSetGroupUserValue (pGM->groupsetMain, groupidThis, 
											(PGPUserValue)hTGroup);
	}

	// if a reinsertion, then we're done
	if (bReInsert) return hTGroup;

	sSetGroupData (pGM, hTGroup, groupidThis, groupinfo.description);

	// if not at root, then we don't draw items in group
	if (hTParent) return hTGroup;

	// iterate through items in group
	PGPCountGroupItems (pGM->groupsetMain, groupidThis, FALSE, &iNumKeys,
							&iNumTotal);

	for (i=0; i<iNumTotal; i++) {
		PGPGetIndGroupItem (pGM->groupsetMain, groupidThis, i, &groupitem);

		if (groupitem.type == kPGPGroupItem_KeyID) {

			err = PGPGetKeyByKeyID (pGM->keysetMain,
						&groupitem.u.key.keyID, 
						groupitem.u.key.algorithm, &key);

			if (IsntPGPError (err) && PGPRefIsValid (key)) {
				uState = 0;
				hTItem = (HTLITEM)groupitem.userValue;
				if (!hTItem) bNew = TRUE;
	
				// get and set treelist tree data for this key
				PGPGetPrimaryUserIDNameBuffer (key, sizeof(sz), sz, &u);
				if (bForceNewAlloc) hTItem = NULL;
				idx = sDetermineKeyIcon (key, &bItalics);
				if (bItalics) uState |= TLIS_ITALICS;

				uVal = MAKELONG (i, groupidThis);
				hTItem = sSetOneGroupItem (pGM, bReInsert, hTItem, hTGroup, 
								sz, hTItemPrev, idx, uState, uVal);

				PGPSetIndGroupItemUserValue (pGM->groupsetMain,
									groupidThis, i, (PGPUserValue)hTItem);

				sSetKeyData (pGM, hTItem, key, "");
			}
			else {
				hTItem = (HTLITEM)groupitem.userValue;
				if (!hTItem) bNew = TRUE;
	
				// get and set treelist tree data for this key
				if (bForceNewAlloc) hTItem = NULL;
				if (groupitem.u.key.algorithm == kPGPPublicKeyAlgorithm_RSA)
					idx = IDX_RSAPUBDISKEY;
				else
					idx = IDX_DSAPUBDISKEY;

				uState = TLIS_ITALICS;

				LoadString (g_hInst, IDS_UNAVAILABLE, sz, sizeof(sz));
				PGPGetKeyIDString (&groupitem.u.key.keyID, 
									kPGPKeyIDString_Abbreviated, szID);
				lstrcat (sz, szID);

				uVal = MAKELONG (i, groupidThis);
				hTItem = sSetOneGroupItem (pGM, bReInsert, hTItem, hTGroup, 
									sz, hTItemPrev, idx, uState, uVal);

				PGPSetIndGroupItemUserValue (pGM->groupsetMain,
									groupidThis, i, (PGPUserValue)hTItem);

			}
		}

		// this is a group within a group
		else {
			hTItem = sReloadGroup (pGM, groupidThis, i, bReInsert, 
					bForceNewAlloc, bExpandNew, FALSE, hTGroup, hTItemPrev);
		}

		hTItemPrev = hTItem;
	}

	// select and expand key, if appropriate 
	if (bExpandNew && bNew) {
		tli.hItem = hTGroup;
		if (!bNewKeyExpanded) {
			TreeList_Select (pGM->hWndTree, &tli, TRUE);
			TreeList_Expand (pGM->hWndTree, &tli, TLE_EXPANDALL);
			bNewKeyExpanded = TRUE;
		}
		else {
			GMSetFocus (pGM, GMFocusedItem (pGM), TRUE);
			TreeList_Expand (pGM->hWndTree, &tli, TLE_EXPANDALL);
		}
	}

	return hTGroup;
}


//	___________________________________________________
//
//  Scan entire groupset loading in any new data

BOOL 
GMLoadGroupsIntoTree (
		PGROUPMAN	pGM, 
		BOOL		bReInsert, 
		BOOL		bExpandNew, 
		BOOL		bForceRealloc) 
{

	HCURSOR			hCursorOld;
	TL_TREEITEM		tli;
	HTLITEM			hTPrevKey;
	BOOL			bFirst;
	PGPGroupID		groupid;
	INT				i, iNumGroups;

	hTPrevKey = (HTLITEM)TLI_FIRST;
	bFirst = TRUE;

	if (pGM->groupsetMain) {
		hCursorOld = SetCursor (LoadCursor (NULL, IDC_WAIT));
		PGPCountGroupsInSet (pGM->groupsetMain, &iNumGroups);

		for (i=0; i<iNumGroups; i++) {
			PGPGetIndGroupID (pGM->groupsetMain, i, &groupid);
			hTPrevKey = sReloadGroup (pGM, groupid, 0, bReInsert, 
						bForceRealloc, bExpandNew, bFirst, NULL, hTPrevKey);
			bFirst = FALSE;
		}

		SetCursor (hCursorOld);
		if (bExpandNew) {
			tli.hItem = GMFocusedItem (pGM);
			if (tli.hItem) TreeList_EnsureVisible (pGM->hWndTree, &tli);
		}
		return TRUE;
	}

	return FALSE;
}


//	___________________________________________________
//
//  Cleanup group manager

PGPError 
PGPgmDestroyGroupManager (HGROUPMAN hGM) 
{

	PGROUPMAN		pGM					= (PGROUPMAN)hGM;

	if (!hGM) return kPGPError_BadParams;

	RevokeDragDrop (pGM->hWndTree);
	GMReleaseDropTarget (pGM->pDropTarget);  
	CoLockObjectExternal ((IUnknown*)pGM->pDropTarget, FALSE, TRUE);

	PGPclCloseGroupFile (pGM->pGroupFile);

	GMSetColumnPreferences (pGM);

	SendMessage (pGM->hWndTree, WM_CLOSE, 0, 0);
	ImageList_Destroy (pGM->hIml);

	return kPGPError_NoErr;
}

//	___________________________________________________
//
//  Map dragover screen coordinates to window coords

BOOL
GMSelectGroup (PGROUPMAN pGM, POINTL ptl)
{
	POINT	pt;

	pt.x = ptl.x;
	pt.y = ptl.y;
	MapWindowPoints (NULL, pGM->hWndTree, &pt, 1);

	return (TreeList_DragOver (pGM->hWndTree, MAKELONG (pt.x, pt.y)));
}

//	___________________________________________________
//
//  Load keyset into treelist

PGPError 
PGPgmLoadGroups (HGROUPMAN hGroupMan)
{
	PGROUPMAN		pGM			= (PGROUPMAN)hGroupMan;

	if (!pGM) return kPGPError_BadParams;

	if (pGM->groupsetMain) {
		GMSetFocus (pGM, NULL, FALSE);
		TreeList_DeleteTree (pGM->hWndTree, TRUE);
		PGPclCloseGroupFile (pGM->pGroupFile);
		pGM->groupsetMain = NULL;
	}

	if (PGPRefIsValid (pGM->keysetMain)) {
		PGPclOpenGroupFile (pGM->context, &(pGM->pGroupFile));
		pGM->groupsetMain = pGM->pGroupFile->groupset;

		GMAddColumns (pGM);

		GMSortGroupSet (pGM);
		GMLoadGroupsIntoTree (pGM, FALSE, FALSE, TRUE);
		GMEnableDropTarget (pGM->pDropTarget, TRUE);
	}

	InvalidateRect (pGM->hWndTree, NULL, TRUE);
	UpdateWindow (pGM->hWndTree);

	return kPGPError_NoErr;
}


//	___________________________________________________
//
//  Load keyset into treelist

PGPError PGPgmExport 
PGPgmReLoadGroups (HGROUPMAN hGroupMan) 
{
	PGROUPMAN pGM = (PGROUPMAN)hGroupMan;
	if (!pGM) return kPGPError_BadParams;
	if (!pGM->groupsetMain) return kPGPError_BadParams;

	GMSortGroupSet (pGM);
	GMLoadGroupsIntoTree (pGM, FALSE, TRUE, FALSE);

	InvalidateRect (pGM->hWndTree, NULL, TRUE);
	UpdateWindow (pGM->hWndTree);

	return kPGPError_NoErr;
}

//	___________________________________________________
//
//	get PGPkeys path from registry and substitute Help file name 

VOID 
GMGetHelpFilePath (PGROUPMAN pGM) 
{
	CHAR	sz[MAX_PATH];

	PGPclGetPGPPath (pGM->szHelpFile, sizeof(pGM->szHelpFile));
	LoadString (g_hInst, IDS_HELPFILENAME, sz, sizeof(sz));
	lstrcat (pGM->szHelpFile, sz);
}

//	___________________________________________________
//
//  Set configuration

PGPError PGPgmExport 
PGPgmConfigure (
		HGROUPMAN	hGroupMan, 
		LPGMCONFIG	pGMConfig) 
{
	PGROUPMAN pGM = (PGROUPMAN)hGroupMan;
	if (!pGM) return kPGPError_BadParams;

	if (pGM) {
		if (pGMConfig->lpszHelpFile)
			lstrcpy (pGM->szHelpFile, pGMConfig->lpszHelpFile);
		else 
			GMGetHelpFilePath (pGM);

		pGM->hKM = pGMConfig->hKM;
		pGM->keysetMain = pGMConfig->keysetMain;
		pGM->ulOptionFlags = pGMConfig->ulOptionFlags;
		pGM->hWndStatusBar = pGMConfig->hWndStatusBar;

		if (pGM->ulOptionFlags & GMF_MARGASINVALID) 
			pGM->iValidityThreshold = GM_VALIDITY_COMPLETE;
		else
			pGM->iValidityThreshold = GM_VALIDITY_MARGINAL;

		return kPGPError_NoErr;
	}

	else return kPGPError_BadParams;

}

