/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	KMFocus.c - routines for tracking the focused/selected items
	

	$Id: KMFocus.c,v 1.10 1998/08/11 14:43:24 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"	

// project header files
#include "pgpkmx.h"

// constant definitions
#define KEYIDCHARS	512

// external globals
extern HINSTANCE g_hInst; 

// typedefs
typedef struct {
	FARPROC			lpfnCallback;
	PKEYMAN			pKM;
	PGPKeySetRef	keysetSelected;
	INT				icount;
} SELOBJECTINFO;

typedef struct {
	FARPROC			lpfnCallback;
	INT				iObjectCount;
} SELOBJECTCOUNT;

//	________________________________________
//
//  Count a single object
//
//	lptli	= pointer to TreeList item to delete

static BOOL CALLBACK 
sCountSingleObject (
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

static VOID 
sUpdateStatusBarText (PKEYMAN pKM) 
{
	CHAR sz1[128];
	CHAR sz2[128];
	INT ids;
	SELOBJECTCOUNT soc;

	if (!pKM->hWndStatusBar) return;
	if (pKM->ulOptionFlags & KMF_DISABLESTATUSBAR) return;

	switch (pKM->uSelectedFlags) {
		case 0 :				ids = 0;					break;
		case PGPKM_KEYFLAG :	ids = IDS_KEYSELECTED;		break;
		case PGPKM_UIDFLAG :	ids = IDS_UIDSELECTED;		break;
		case PGPKM_CERTFLAG :	ids = IDS_CERTSELECTED;		break;
		default :				ids = IDS_MULTISELECTED;	break;
	}

	if (pKM->bMultipleSelected) {
		soc.lpfnCallback = sCountSingleObject;
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

	SendMessage (pKM->hWndStatusBar, SB_SETTEXT, 1, (LPARAM)sz2);
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
			case IDX_RSASECSHRKEY :
			case IDX_DSAPUBKEY :
			case IDX_DSAPUBDISKEY :
			case IDX_DSAPUBREVKEY :
			case IDX_DSAPUBEXPKEY :
			case IDX_DSASECKEY :
			case IDX_DSASECDISKEY :
			case IDX_DSASECREVKEY :
			case IDX_DSASECEXPKEY :
			case IDX_DSASECSHRKEY :
				pKM->iFocusedObjectType = OBJECT_KEY; 
				pKM->uSelectedFlags |= PGPKM_KEYFLAG;
				break;

			case IDX_RSAUSERID :
			case IDX_DSAUSERID : 
			case IDX_PHOTOUSERID :
			case IDX_INVALIDUSERID :
				pKM->iFocusedObjectType = OBJECT_USERID; 
				pKM->uSelectedFlags |= PGPKM_UIDFLAG;
				break;

			case IDX_CERT :
			case IDX_REVCERT : 
			case IDX_BADCERT :
			case IDX_EXPCERT :
			case IDX_EXPORTCERT :
			case IDX_TRUSTEDCERT :
			case IDX_METACERT :
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

	sUpdateStatusBarText (pKM);
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

//	_______________________________________________
//
//	routines called as callback functions from the 
//  TreeList control
//

static BOOL CALLBACK 
sAddKeyToKeySet (
		TL_TREEITEM*	lptli, 
		LPARAM			lParam) 
{
	SELOBJECTINFO*	psoi = (SELOBJECTINFO*)lParam;
	PGPKeyRef		key;
	PGPKeySetRef	newkeyset;
	PGPKeySetRef	oldkeyset;
	PGPError		err;

	switch (lptli->iImage) {
	case IDX_RSAPUBKEY :
	case IDX_RSAPUBDISKEY :
	case IDX_RSASECKEY :
	case IDX_RSASECDISKEY :
	case IDX_DSAPUBKEY :
	case IDX_DSAPUBDISKEY :
	case IDX_DSASECKEY :
	case IDX_DSASECDISKEY :
	case IDX_DSASECSHRKEY :
	case IDX_RSAPUBREVKEY :
	case IDX_RSAPUBEXPKEY :
	case IDX_RSASECREVKEY :
	case IDX_RSASECEXPKEY :
	case IDX_RSASECSHRKEY :
	case IDX_DSAPUBREVKEY :
	case IDX_DSAPUBEXPKEY :
	case IDX_DSASECREVKEY :
	case IDX_DSASECEXPKEY :
		key = (PGPKeyRef)(lptli->lParam);
		break;

	case IDX_RSAUSERID :
	case IDX_DSAUSERID :
	case IDX_PHOTOUSERID :
	case IDX_INVALIDUSERID :
		key = KMGetKeyFromUserID (psoi->pKM, (PGPUserIDRef)(lptli->lParam));
		break;

	case IDX_CERT :
	case IDX_REVCERT :
	case IDX_BADCERT :
	case IDX_EXPORTCERT :
	case IDX_TRUSTEDCERT :
	case IDX_METACERT :
		key = KMGetKeyFromCert (psoi->pKM, (PGPSigRef)(lptli->lParam));
		break;

	default :
		return FALSE;
	}

	if (PGPKeyRefIsValid (key)) {
		if (PGPKeySetRefIsValid (psoi->keysetSelected)) {
			err = PGPNewSingletonKeySet (key, &newkeyset);
			if (IsntPGPError (err)) {
				oldkeyset = psoi->keysetSelected;
				err = PGPUnionKeySets (oldkeyset, 
								newkeyset, &psoi->keysetSelected);
				if (IsntPGPError (err))
					PGPFreeKeySet (oldkeyset);
				PGPFreeKeySet (newkeyset);
			}
		}

		++(psoi->icount);
	}

	return TRUE;
}

//	___________________________
//
//	Get keyset of selected keys

PGPError 
KMGetSelectedKeys (
		PKEYMAN			pKM, 
		PGPKeySetRef*	pkeyset,
		INT*			piCount) 
{
	PGPError			err		= kPGPError_NoErr;
	SELOBJECTINFO		soi;

	soi.lpfnCallback	= sAddKeyToKeySet;
	soi.pKM				= pKM;
	soi.icount			= 0;

	if (pkeyset)
		*pkeyset = kInvalidPGPKeySetRef;
	if (piCount)
		*piCount = 0;

	if (pkeyset)
		err = PGPNewEmptyKeySet (pKM->KeySetDisp, &(soi.keysetSelected));
	else
		soi.keysetSelected = NULL;

	if (IsntPGPError (err)) {
		TreeList_IterateSelected (pKM->hWndTree, &soi);
		if (pkeyset)
			*pkeyset = soi.keysetSelected;
		if (piCount)
			*piCount = soi.icount;
	}

	return err;
}



