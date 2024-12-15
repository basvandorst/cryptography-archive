/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */
//: KMFocus.c - miscellaneous conversion routines
//
//	$Id: KMFocus.c,v 1.3 1997/05/09 19:29:13 pbj Exp $
//

#include <windows.h>
#include "pgpkeys.h"

#define KEYIDCHARS	512
#define KEYFLAG		0x01
#define UIDFLAG		0x02
#define CERTFLAG	0x04

// external globals
extern HWND g_hWndTree;
extern CHAR g_szbuf[G_BUFLEN];

// local globals
static BOOL bMultipleSelected;
static UINT uSelectedFlags;
static INT iFocusedItemType;
static INT iFocusedObjectType;
static HTLITEM hFocusedItem;
static VOID* pFocusedObject;

static PGPKeySet* pKeySetSelected;
static CHAR* pKeyIDList;
static INT iKeyIDListLen;


//----------------------------------------------------|
//  Set handle of selected item.
//  Type and pointer to object are retrieved from
//  TreeList as imagelist index.

void KMSetFocus (HTLITEM hFocused, BOOL bMultiple) {
	TL_TREEITEM tli;

	hFocusedItem = hFocused;
	bMultipleSelected = bMultiple;

	if (!bMultipleSelected) uSelectedFlags = 0;

	if (hFocusedItem) {
		tli.hItem = hFocusedItem;
		tli.mask = TLIF_IMAGE | TLIF_PARAM;
		if (!TreeList_GetItem (g_hWndTree, &tli)) return;
		iFocusedItemType = tli.iImage;
		pFocusedObject = (void*)tli.lParam;
		switch (iFocusedItemType) {
			case IDX_NONE : iFocusedObjectType = OBJECT_NONE; break;
			case IDX_RSAPUBKEY :
			case IDX_RSAPUBDISKEY :
			case IDX_RSAPUBREVKEY :
			case IDX_RSAPUBEXPKEY :
			case IDX_RSASECKEY :
			case IDX_RSASECDISKEY :
			case IDX_RSASECREVKEY :
			case IDX_RSASECEXPKEY :
			case IDX_DSAPUBKEY :
			case IDX_DSAPUBDISKEY :
			case IDX_DSAPUBREVKEY :
			case IDX_DSAPUBEXPKEY :
			case IDX_DSASECKEY :
			case IDX_DSASECDISKEY :
			case IDX_DSASECREVKEY :
			case IDX_DSASECEXPKEY :
				iFocusedObjectType = OBJECT_KEY;
				uSelectedFlags |= KEYFLAG;
				break;

			case IDX_RSAUSERID :
			case IDX_DSAUSERID :
				iFocusedObjectType = OBJECT_USERID;
				uSelectedFlags |= UIDFLAG;
				break;

			case IDX_CERT :
			case IDX_REVCERT :
				iFocusedObjectType = OBJECT_CERT;
				uSelectedFlags |= CERTFLAG;
				break;
		}
	}
	else {
		iFocusedItemType = IDX_NONE;
		iFocusedObjectType = OBJECT_NONE;
		pFocusedObject = NULL;
	}
}


//----------------------------------------------------|
// Get type of focused item

int KMFocusedItemType (void) {
	return iFocusedItemType;
}


//----------------------------------------------------|
// Get type of focused object

int KMFocusedObjectType (void) {
	return iFocusedObjectType;
}


//----------------------------------------------------|
// Get handle of focused item

HTLITEM KMFocusedItem (void) {
	return hFocusedItem;
}


//----------------------------------------------------|
// Get pointer to focused object

void* KMFocusedObject (void) {
	return pFocusedObject;
}


//----------------------------------------------------|
// Get type of focused object

BOOL KMMultipleSelected (void) {
	return bMultipleSelected;
}


//----------------------------------------------------|
// Return TRUE if more than one type of object selected

BOOL KMPromiscuousSelected (void) {

	if (!bMultipleSelected) return FALSE;

	switch (uSelectedFlags) {
	case 0 :
	case KEYFLAG :
	case UIDFLAG :
	case CERTFLAG :
		return FALSE;

	default :
		return TRUE;
	}
}


//----------------------------------------------------|
// Return TRUE if signing operations allowed

BOOL KMSigningAllowed (void) {
	if ((!uSelectedFlags) || (uSelectedFlags & CERTFLAG)) return FALSE;
	else return TRUE;
}


//----------------------------------------------------|
//  Get a single key
//	routine called either from KMGetSelectedKeys or as a
//	callback function from the TreeList control to
//	get the KeyID of a single key.
//
//	lptli	= pointer to TreeList item to delete

BOOL GetSingleKey (TL_TREEITEM* lptli) {
	PGPKey* pKey;
	PGPCert* pCert;
	PGPKeySet* pKeySet;
	CHAR szKeyID[20];
	UINT u;

	switch (lptli->iImage) {
	case IDX_RSAPUBKEY :
	case IDX_RSAPUBDISKEY :
	case IDX_RSASECKEY :
	case IDX_RSASECDISKEY :
	case IDX_DSAPUBKEY :
	case IDX_DSAPUBDISKEY :
	case IDX_DSASECKEY :
	case IDX_DSASECDISKEY :
	case IDX_RSAPUBREVKEY :
	case IDX_RSAPUBEXPKEY :
	case IDX_RSASECREVKEY :
	case IDX_RSASECEXPKEY :
	case IDX_DSAPUBREVKEY :
	case IDX_DSAPUBEXPKEY :
	case IDX_DSASECREVKEY :
	case IDX_DSASECEXPKEY :
		if (pKeySetSelected) {
			pKey = (PGPKey*)(lptli->lParam);
			if (pKey) {
				pKeySet = pgpNewSingletonKeySet (pKey);
				PGPcomdlgErrorMessage (pgpAddKeys (pKeySetSelected, pKeySet));
				pgpFreeKeySet (pKeySet);
			}
		}
		return TRUE;

	case IDX_CERT :
	case IDX_REVCERT :
	case IDX_BADCERT :
		if (pKeyIDList) {
			pCert = (PGPCert*)(lptli->lParam);
			u = sizeof(szKeyID);
			pgpGetCertString (pCert, kPGPCertPropKeyID, szKeyID, &u);
			KMConvertStringKeyID (szKeyID);
			u = 10;
			lstrcpy (pKeyIDList, szKeyID);
			if (lstrlen (pKeyIDList) > 0) {
				lstrcat (pKeyIDList, "\n");
				u++;
			}
			pKeyIDList = &pKeyIDList[u];
		}
		else {
			iKeyIDListLen += 20;
		}
		return TRUE;

	default :
		return FALSE;
	}

}


//----------------------------------------------------|
// Get keyset of selected keys

VOID KMGetSelectedObjects (PGPKeySet** ppKeySet, LPSTR* pszKeyIDs) {
	PGPKey* pKey;
	PGPKeySet* pKeySet;

	switch (iFocusedObjectType) {
	case OBJECT_KEY :
		*pszKeyIDs = NULL;
		pKeySetSelected = pgpNewKeySet ();
		if (pKeySetSelected) {
			TreeList_IterateSelected (g_hWndTree, GetSingleKey);
			*ppKeySet = pKeySetSelected;
		}
		else *ppKeySet = NULL;
		break;

	case OBJECT_CERT :
		pKeyIDList = NULL;
		*ppKeySet = NULL;
		iKeyIDListLen = 0;
		TreeList_IterateSelected (g_hWndTree, GetSingleKey);
		*pszKeyIDs = KMAlloc (iKeyIDListLen*sizeof(CHAR));
		if (*pszKeyIDs) {
			pKeyIDList = *pszKeyIDs;
			(*pszKeyIDs)[0] = '\0';
			TreeList_IterateSelected (g_hWndTree, GetSingleKey);
		}
		break;

	case OBJECT_USERID :
		*pszKeyIDs = NULL;
		pKeySetSelected = pgpNewKeySet ();
		if (pKeySetSelected) {
			pKey = KMGetKeyFromUserID (g_hWndTree,
									    (PGPUserID*)KMFocusedObject ());
			if (pKey) {
				pKeySet = pgpNewSingletonKeySet (pKey);
				PGPcomdlgErrorMessage (pgpAddKeys (pKeySetSelected, pKeySet));
				pgpFreeKeySet (pKeySet);
			}
			*ppKeySet = pKeySetSelected;
		}
		else *ppKeySet = NULL;
		break;

	default :
		*pszKeyIDs = NULL;
		*ppKeySet = NULL;
		break;
	}
}


//----------------------------------------------------|
// Free previously allocated keyset

VOID KMFreeSelectedObjects (PGPKeySet* pKeySet, LPSTR szKeyIDs) {
	if (pKeySet)
		pgpFreeKeySet (pKeySet);
	if (szKeyIDs)
		KMFree (szKeyIDs);
}
