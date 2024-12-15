/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	GMNotProc - notification processing and associated routines
	

	$Id: GMNotPrc.c,v 1.7 1997/09/22 23:09:35 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

#include "pgpgmx.h"
#include <stdio.h>
#include "resource.h"


// External globals
extern HINSTANCE g_hInst;

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
UpdateStatusBarText (PGROUPMAN pGM) 
{
	CHAR sz1[128];
	CHAR sz2[128];
	INT ids;
	SELOBJECTCOUNT soc;

	if (!pGM->hWndStatusBar) return;

	switch (pGM->uSelectedFlags) {
		case 0 :				ids = 0;					break;
		case PGPGM_GROUPFLAG :	ids = IDS_GROUPSELECTED;	break;
		case PGPGM_KEYFLAG :	ids = IDS_KEYSELECTED;		break;
		default :				ids = IDS_MULTISELECTED;	break;
	}

	if (pGM->bMultipleSelected) {
		soc.lpfnCallback = CountSingleObject;
		soc.iObjectCount = 0;
		TreeList_IterateSelected (pGM->hWndTree, &soc);
	}
	else 
		soc.iObjectCount = 1;

	if (ids) {
		LoadString (g_hInst, ids, sz1, sizeof(sz1));
		wsprintf (sz2, sz1, soc.iObjectCount);
	}
	else
		lstrcpy (sz2, "");

	SendMessage (pGM->hWndStatusBar, SB_SETTEXT, 0, (LPARAM)sz2);
}

//	________________________________________
//
//  Set handle of selected item.
//  Type and pointer to object are retrieved from 
//  TreeList as imagelist index.

VOID 
GMSetFocus (
		PGROUPMAN	pGM, 
		HTLITEM		hFocused, 
		BOOL		bMultiple) 
{
	TL_TREEITEM tli;

	pGM->hFocusedItem = hFocused;
	pGM->bMultipleSelected = bMultiple;

	if (!bMultiple) pGM->uSelectedFlags = 0;

	if (hFocused) { 
		tli.hItem = hFocused;
		tli.mask = TLIF_IMAGE | TLIF_PARAM;
		if (!TreeList_GetItem (pGM->hWndTree, &tli)) return;
		pGM->iFocusedItemType = tli.iImage;
		pGM->pFocusedObject = (void*)tli.lParam;
		switch (pGM->iFocusedItemType) {
			case IDX_NONE : 
				pGM->iFocusedObjectType = OBJECT_NONE; 
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
				pGM->iFocusedObjectType = OBJECT_KEY; 
				pGM->uSelectedFlags |= PGPGM_KEYFLAG;
				break;

			case IDX_GROUP :
				pGM->iFocusedObjectType = OBJECT_GROUP; 
				pGM->uSelectedFlags |= PGPGM_GROUPFLAG;
				break;

		}
	}
	else {
		pGM->iFocusedItemType = IDX_NONE;
		pGM->iFocusedObjectType = OBJECT_NONE;
		pGM->pFocusedObject = NULL;
	}

	UpdateStatusBarText (pGM);
}

//	________________________________________
//
//	Get type of focused item

INT 
GMFocusedItemType (PGROUPMAN pGM) 
{
	return pGM->iFocusedItemType;
}

//	________________________________________
//
//	Get type of focused object

INT 
GMFocusedObjectType (PGROUPMAN pGM) 
{
	return pGM->iFocusedObjectType;
}

//	________________________________________
//
//	Get handle of focused item

HTLITEM 
GMFocusedItem (PGROUPMAN pGM) 
{
	return pGM->hFocusedItem;
}

//	________________________________________
//
//	Get pointer to focused object

VOID* GMFocusedObject (PGROUPMAN pGM) 
{
	return pGM->pFocusedObject;
}

//	________________________________________
//
//	Get type of focused object

BOOL 
GMMultipleSelected (PGROUPMAN pGM) 
{
	return pGM->bMultipleSelected;
}

//	________________________________________
//
//	Get type of focused object

UINT 
GMSelectedFlags (PGROUPMAN pGM) 
{
	return pGM->uSelectedFlags;
}

//	___________________________________________________
//
//	Return TRUE if more than one type of object selected

BOOL 
GMPromiscuousSelected (PGROUPMAN pGM) 
{
	if (!pGM->bMultipleSelected) return FALSE;

	switch (pGM->uSelectedFlags) {
	case 0 :
	case PGPGM_KEYFLAG :
	case PGPGM_GROUPFLAG :
		return FALSE;

	default :
		return TRUE;
	}
}

//	___________________________________________________
//
//  Group manager notification processing procedure

PGPError PGPgmExport 
PGPgmDefaultNotificationProc (
		HGROUPMAN	hGM, 
		LPARAM		lParam) 
{
	PGROUPMAN	pGM = (PGROUPMAN)hGM;

	HTLITEM		hFocused;
	BOOL		bMultiple;
	INT			i, iField;

	if (!hGM) return kPGPError_BadParams;

	switch (((LPNM_TREELIST)lParam)->hdr.code) {

	case TLN_SELCHANGED :
		bMultiple = (((LPNM_TREELIST)lParam)->flags & TLC_MULTIPLE);
		hFocused = ((LPNM_TREELIST)lParam)->itemNew.hItem;
		GMSetFocus (pGM, hFocused, bMultiple);
		((LPNM_TREELIST)lParam)->flags = GMSelectedFlags (pGM);
		break;

	case TLN_DRAGGEDOVER :
		hFocused = ((LPNM_TREELIST)lParam)->itemNew.hItem;
		GMSetFocus (pGM, hFocused, FALSE);
		((LPNM_TREELIST)lParam)->flags = GMSelectedFlags (pGM);
		break;

	case TLN_KEYDOWN :
		switch (((TL_KEYDOWN*)lParam)->wVKey) {
		case VK_DELETE :
			GMDeleteObject (pGM);
			break;
		}
		break;

	case TLN_CONTEXTMENU :
		((LPNM_TREELIST)lParam)->flags = GMSelectedFlags (pGM);
		break;

	case TLN_BEGINDRAG :
		GMEnableDropTarget (pGM->pDropTarget, FALSE);
		break;

	case TLN_DROP :
		if (((LPNM_TREELIST)lParam)->index) {
			GMAddSelectedToFocused (pGM);
		}
		GMEnableDropTarget (pGM->pDropTarget, TRUE);
		break;

	case TLN_HEADERCLICKED :
		i = pGM->lKeyListSortField;
		iField = pGM->wColumnField[((LPNM_TREELIST)lParam)->index];
		switch (iField) {
			case GMI_NAME : 
				if (pGM->lKeyListSortField == kPGPUserIDOrdering)
					i = kPGPReverseUserIDOrdering; 
				else 
					i = kPGPUserIDOrdering; 
				break;

			case GMI_VALIDITY :
				if (pGM->lKeyListSortField == kPGPValidityOrdering)
					i = kPGPReverseValidityOrdering;
				else
					i = kPGPValidityOrdering;
				break;

			default : break;
		}
		if (i != pGM->lKeyListSortField) {
			pGM->lKeyListSortField = i;
			TreeList_DeleteTree (pGM->hWndTree, FALSE);
			InvalidateRect (pGM->hWndTree, NULL, TRUE);
			UpdateWindow (pGM->hWndTree);
			GMSortGroupSet (pGM);
			GMLoadGroupsIntoTree (pGM, TRUE, FALSE, FALSE);
			InvalidateRect (pGM->hWndTree, NULL, TRUE);
		}
		break;
	}
	return 0;
}

