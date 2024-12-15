/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	KMFocus.c - routines for tracking the focused/selected items
	

	$Id: KMFocus.c,v 1.9 1997/09/12 17:41:33 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"	

#include "pgpkmx.h"
#include "resource.h"

#define KEYIDCHARS	512

extern HINSTANCE g_hInst; 

typedef struct {
	FARPROC lpfnCallback;
	PGPContextRef Context;
	PGPKeySetRef KeySetSelected;
	CHAR* pKeyIDList;
	INT iKeyIDListLen; 
} SELOBJECTINFO;

typedef struct {
	FARPROC lpfnCallback;
	INT iObjectCount;
} SELOBJECTCOUNT;

//	________________________________________
//
//  Count a single object
//
//	lptli	= pointer to TreeList item to delete

BOOL CALLBACK 
CountSingleObject (
		TL_TREEITEM*	lptli, 
		LPARAM			lParam) 
{
	SELOBJECTCOUNT*	psoc = (SELOBJECTCOUNT*)lParam;
	++(psoc->iObjectCount);
	return TRUE;
}

//	____________________________________________
//
//  Update status bar text on basis of selection

VOID 
UpdateStatusBarText (PKEYMAN pKM) 
{
	CHAR sz1[128];
	CHAR sz2[128];
	INT ids;
	SELOBJECTCOUNT soc;

	if (!pKM->hWndStatusBar) return;

	switch (pKM->uSelectedFlags) {
		case 0 :				ids = 0;					break;
		case PGPKM_KEYFLAG :	ids = IDS_KEYSELECTED;		break;
		case PGPKM_UIDFLAG :	ids = IDS_UIDSELECTED;		break;
		case PGPKM_CERTFLAG :	ids = IDS_CERTSELECTED;		break;
		default :				ids = IDS_MULTISELECTED;	break;
	}

	if (pKM->bMultipleSelected) {
		soc.lpfnCallback = CountSingleObject;
		soc.iObjectCount = 0;
		TreeList_IterateSelected (pKM->hWndTree, &soc);
	}
	else 
		soc.iObjectCount = 1;

	if (ids) {
		LoadString (g_hInst, ids, sz1, sizeof(sz1));
		wsprintf (sz2, sz1, soc.iObjectCount);
	}
	else
		lstrcpy (sz2, "");

	SendMessage (pKM->hWndStatusBar, SB_SETTEXT, 0, (LPARAM)sz2);
}

//	________________________________________
//
//  Set handle of selected item.
//  Type and pointer to object are retrieved from 
//  TreeList as imagelist index.

VOID 
KMSetFocus (
		PKEYMAN		pKM, 
		HTLITEM		hFocused, 
		BOOL		bMultiple) 
{
	TL_TREEITEM tli;

	pKM->hFocusedItem = hFocused;
	pKM->bMultipleSelected = bMultiple;

	if (!bMultiple) pKM->uSelectedFlags = 0;

	if (hFocused) { 
		tli.hItem = hFocused;
		tli.mask = TLIF_IMAGE | TLIF_PARAM;
		if (!TreeList_GetItem (pKM->hWndTree, &tli)) return;
		pKM->iFocusedItemType = tli.iImage;
		pKM->pFocusedObject = (void*)tli.lParam;
		switch (pKM->iFocusedItemType) {
			case IDX_NONE : 
				pKM->iFocusedObjectType = OBJECT_NONE; 
				break;

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
				pKM->iFocusedObjectType = OBJECT_KEY; 
				pKM->uSelectedFlags |= PGPKM_KEYFLAG;
				break;

			case IDX_RSAUSERID :
			case IDX_DSAUSERID : 
				pKM->iFocusedObjectType = OBJECT_USERID; 
				pKM->uSelectedFlags |= PGPKM_UIDFLAG;
				break;

			case IDX_CERT :
			case IDX_REVCERT : 
			case IDX_BADCERT :
			case IDX_EXPORTCERT :
			case IDX_METACERT :
			case IDX_EXPORTMETACERT :
				pKM->iFocusedObjectType = OBJECT_CERT; 
				pKM->uSelectedFlags |= PGPKM_CERTFLAG;
				break;
		}
	}
	else {
		pKM->iFocusedItemType = IDX_NONE;
		pKM->iFocusedObjectType = OBJECT_NONE;
		pKM->pFocusedObject = NULL;
	}

	UpdateStatusBarText (pKM);
}

//	________________________________________
//
//	Get type of focused item

INT 
KMFocusedItemType (PKEYMAN pKM) 
{
	return pKM->iFocusedItemType;
}

//	________________________________________
//
//	Get type of focused object

INT 
KMFocusedObjectType (PKEYMAN pKM) 
{
	return pKM->iFocusedObjectType;
}

//	________________________________________
//
//	Get handle of focused item

HTLITEM 
KMFocusedItem (PKEYMAN pKM) 
{
	return pKM->hFocusedItem;
}

//	________________________________________
//
//	Get pointer to focused object

VOID* KMFocusedObject (PKEYMAN pKM) 
{
	return pKM->pFocusedObject;
}

//	________________________________________
//
//	Get type of focused object

BOOL 
KMMultipleSelected (PKEYMAN pKM) 
{
	return pKM->bMultipleSelected;
}

//	________________________________________
//
//	Get type of focused object

UINT 
KMSelectedFlags (PKEYMAN pKM) 
{
	return pKM->uSelectedFlags;
}

//	___________________________________________________
//
//	Return TRUE if more than one type of object selected

BOOL 
KMPromiscuousSelected (PKEYMAN pKM) 
{
	if (!pKM->bMultipleSelected) return FALSE;

	switch (pKM->uSelectedFlags) {
	case 0 :
	case PGPKM_KEYFLAG :
	case PGPKM_UIDFLAG :
	case PGPKM_CERTFLAG :
		return FALSE;

	default :
		return TRUE;
	}
}

//	________________________________________
//
//	Return TRUE if signing operations allowed

BOOL 
KMSigningAllowed (PKEYMAN pKM) 
{
	if ((!pKM->uSelectedFlags) || 
		(pKM->uSelectedFlags & PGPKM_CERTFLAG)) return FALSE;
	else return TRUE;
}

//	________________________________________
//
//  Get a single key
//	routine called either from KMGetSelectedKeys or as a
//	callback function from the TreeList control to 
//	get the KeyID of a single key.
//
//	lptli	= pointer to TreeList item to delete

BOOL CALLBACK 
GetSingleKey (
		TL_TREEITEM*	lptli, 
		LPARAM			lParam) 
{
	PGPKeyRef		Key;
	PGPSigRef		Cert;
	PGPKeySetRef	KeySet;
	CHAR			szKeyID[20];
	UINT			u;
	SELOBJECTINFO*	psoi = (SELOBJECTINFO*)lParam;

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
		if (psoi->KeySetSelected) {
			Key = (PGPKeyRef)(lptli->lParam);
			if (Key) {
				PGPNewSingletonKeySet (Key, &KeySet);
				PGPcomdlgErrorMessage (PGPAddKeys (
								KeySet, psoi->KeySetSelected));
				PGPCommitKeyRingChanges (psoi->KeySetSelected);
				PGPFreeKeySet (KeySet);
			}
		}
		return TRUE;

	case IDX_CERT :
	case IDX_REVCERT :
	case IDX_BADCERT :
	case IDX_EXPORTCERT :
	case IDX_METACERT :
	case IDX_EXPORTMETACERT :
		if (psoi->pKeyIDList) {
			Cert = (PGPSigRef)(lptli->lParam);
			KMGetKeyIDFromCert (Cert, szKeyID, sizeof(szKeyID));
			u = 10;
			lstrcpy (psoi->pKeyIDList, szKeyID);
			if (lstrlen (psoi->pKeyIDList) > 0) {
				lstrcat (psoi->pKeyIDList, "\n");
				u++;
			}
			psoi->pKeyIDList = &psoi->pKeyIDList[u];
		}
		else {
			psoi->iKeyIDListLen += 20;
		}
		return TRUE;

	default :
		return FALSE;
	}

}

//	___________________________
//
//	Get keyset of selected keys

VOID 
KMGetSelectedObjects (
		PKEYMAN			pKM, 
		PGPKeySetRef*	pKeySet, 
		LPSTR*			pszKeyIDs) 
{
	PGPKeyRef		Key;
	PGPKeySetRef	KeySet;
	SELOBJECTINFO	soi;

	soi.lpfnCallback = GetSingleKey;
	soi.Context = pKM->Context;

	switch (pKM->iFocusedObjectType) {
	case OBJECT_KEY :
		*pszKeyIDs = NULL;
		PGPNewKeySet (pKM->Context, &(soi.KeySetSelected));
		if (soi.KeySetSelected) {
			TreeList_IterateSelected (pKM->hWndTree, &soi);
			*pKeySet = soi.KeySetSelected;
		}
		else *pKeySet = NULL;
		break;

	case OBJECT_CERT :
		*pKeySet = NULL;
		soi.pKeyIDList = NULL;
		soi.iKeyIDListLen = 0;
		TreeList_IterateSelected (pKM->hWndTree, &soi);
		*pszKeyIDs = KMAlloc (soi.iKeyIDListLen*sizeof(CHAR));
		if (*pszKeyIDs) {
			soi.pKeyIDList = *pszKeyIDs;
			(*pszKeyIDs)[0] = '\0';
			TreeList_IterateSelected (pKM->hWndTree, &soi);
		}
		break;

	case OBJECT_USERID :
		*pszKeyIDs = NULL;
		PGPNewKeySet (pKM->Context, &(soi.KeySetSelected));
		if (soi.KeySetSelected) {
			Key = KMGetKeyFromUserID (pKM,(PGPUserIDRef)KMFocusedObject(pKM));
			if (Key) {
				PGPNewSingletonKeySet (Key, &KeySet);
				PGPcomdlgErrorMessage (PGPAddKeys (
								KeySet, soi.KeySetSelected));
				PGPCommitKeyRingChanges (soi.KeySetSelected);
				PGPFreeKeySet (KeySet);
			}
			*pKeySet = soi.KeySetSelected;
		}
		else *pKeySet = NULL;
		break;

	default :
		*pszKeyIDs = NULL;
		*pKeySet = NULL;
		break;
	}
}

//	_______________________________
//
//	Free previously allocated keyset

VOID 
KMFreeSelectedObjects (
		PGPKeySetRef	KeySet, 
		LPSTR			szKeyIDs) 
{
	if (KeySet) 
		PGPFreeKeySet (KeySet);
	if (szKeyIDs) 
		KMFree (szKeyIDs);
}




