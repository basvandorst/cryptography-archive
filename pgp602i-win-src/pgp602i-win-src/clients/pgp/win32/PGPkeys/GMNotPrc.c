/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	GMNotProc - notification processing and associated routines
	

	$Id: GMNotPrc.c,v 1.17 1998/08/11 15:20:04 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

// project header files
#include "pgpgmx.h"

// constant definitions
#define STATUS_MSG_PANE 1

// External globals
extern HINSTANCE g_hInst;

// typedefs
typedef struct {
	FARPROC lpfnCallback;
	INT iObjectCount;
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
sUpdateStatusBarText (PGROUPMAN pGM) 
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
		soc.lpfnCallback = sCountSingleObject;
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

	SendMessage (pGM->hWndStatusBar, SB_SETTEXT, 
						STATUS_MSG_PANE, (LPARAM)sz2);
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

	sUpdateStatusBarText (pGM);
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
//  create drag image

static VOID 
sBeginDrag (PGROUPMAN pGM) 
{
	HIMAGELIST			hIml;
	TL_DRAGBITMAP		tldb;
	NONCLIENTMETRICS	ncm;

	// get offset number to convert client to window coordinates
	ncm.cbSize = sizeof(NONCLIENTMETRICS);
	SystemParametersInfo (SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS),
							&ncm, 0);
	pGM->iXCursorOffset = ncm.iBorderWidth;
	pGM->iYCursorOffset = ncm.iBorderWidth + ncm.iCaptionHeight;
	if (GetMenu (pGM->hWndParent))
		pGM->iYCursorOffset += ncm.iMenuHeight;

	GetCursorPos (&tldb.ptCursorPos);
	ScreenToClient (pGM->hWndTree, &tldb.ptCursorPos);
	tldb.ulFlags = 0;
	TreeList_RenderDragBitmap (pGM->hWndTree, &tldb);
	hIml = ImageList_Create (tldb.sizeDrag.cx, tldb.sizeDrag.cy, 
							ILC_MASK|ILC_COLORDDB, 1, 1);
	ImageList_AddMasked (hIml, tldb.hbmDrag, GetSysColor (COLOR_WINDOW));
	DeleteObject (tldb.hbmDrag);
 
	ImageList_BeginDrag (hIml, 0, tldb.ptHotSpot.x, tldb.ptHotSpot.y);
	ImageList_Destroy (hIml);

	ImageList_DragEnter (pGM->hWndParent, tldb.ptCursorPos.x, 
								tldb.ptCursorPos.y);
}

static VOID
sEndDrag (PGROUPMAN pGM) {
	ImageList_DragLeave (pGM->hWndParent);
	ImageList_EndDrag ();
}

//	___________________________________________________
//
//  reorder columns according to new header order

static VOID  
sReorderColumns (PGROUPMAN pGM)
{
	INT		iOrderArray[NUMBERFIELDS];
	WORD	wColumnField[NUMBERFIELDS];
	INT		i, iNumCols;

	// save order
	GMSetColumnPreferences (pGM);
	iNumCols = 1;
	for (i=0; i<NUMBERFIELDS; i++) {
		wColumnField[i] = pGM->wColumnField[i];
		if (wColumnField[i]) iNumCols++;
	}

	TreeList_GetOrderArray (pGM->hWndTree, iNumCols, &iOrderArray);

	for (i=0; i<iNumCols; i++) {
		pGM->wColumnField[i] = wColumnField[iOrderArray[i]];
	}

	TreeList_DeleteTree (pGM->hWndTree, TRUE);
	TreeList_DeleteAllColumns (pGM->hWndTree);
	InvalidateRect (pGM->hWndTree, NULL, TRUE);
	UpdateWindow (pGM->hWndTree);
	GMAddColumns (pGM);
	GMLoadGroupsIntoTree (pGM, FALSE, FALSE, TRUE);
	InvalidateRect (pGM->hWndTree, NULL, TRUE);

	GMSetColumnPreferences (pGM);
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
	POINT		pt;

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
		if (GMFocusedItem (pGM) != hFocused) {
			GMSetFocus (pGM, hFocused, FALSE);
			((LPNM_TREELIST)lParam)->flags = GMSelectedFlags (pGM);

			ImageList_DragLeave (pGM->hWndParent);
			InvalidateRect (pGM->hWndTree, NULL, TRUE);
			UpdateWindow (pGM->hWndTree);
			ImageList_DragEnter (pGM->hWndParent, 0, 0);
		}
		pt = ((LPNM_TREELIST)lParam)->ptDrag;
		MapWindowPoints (pGM->hWndTree, pGM->hWndParent, &pt, 1);
		ImageList_DragMove (pt.x + pGM->iXCursorOffset, 
							pt.y + pGM->iYCursorOffset);
		break;

	case TLN_AUTOSCROLLED :
		ImageList_DragLeave (pGM->hWndParent);
		InvalidateRect (pGM->hWndTree, NULL, TRUE);
		UpdateWindow (pGM->hWndTree);
		ImageList_DragEnter (pGM->hWndParent, 0, 0);
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

	case TLN_HEADERREORDERED :
		sReorderColumns (pGM);
		break;

	case TLN_BEGINDRAG :
		pGM->hFocusedItemDragging = GMFocusedItem (pGM);
		GMEnableDropTarget (pGM->pDropTarget, FALSE);
		sBeginDrag (pGM);
		break;

	case TLN_DROP :
		if (((LPNM_TREELIST)lParam)->index) {
			GMAddSelectedToFocused (pGM);
		}
		sEndDrag (pGM);
		GMEnableDropTarget (pGM->pDropTarget, TRUE);
		GMSetFocus (pGM, pGM->hFocusedItemDragging, FALSE);
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

