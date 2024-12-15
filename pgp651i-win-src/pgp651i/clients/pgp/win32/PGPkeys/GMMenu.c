/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	GMMenu.c - handle menu enabling/disabling chores
	

	$Id: GMMenu.c,v 1.13 1998/08/11 15:20:02 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

// project header files
#include "pgpgmx.h"

// typedefs
typedef struct {
	FARPROC lpfnCallback;
	HWND hWndTree;
} EXPANDCOLLAPSESTRUCT;

// External globals
extern HINSTANCE g_hInst;

//	___________________________________________________
//
//  Expand a single item
//	routine called as a
//	callback function from the TreeList control to 
//	expand a single item.
//
//	lptli	= pointer to TreeList item to expand

static BOOL CALLBACK 
sExpandSingleItem (TL_TREEITEM* lptli, LPARAM lParam) 
{
	EXPANDCOLLAPSESTRUCT* pecs = (EXPANDCOLLAPSESTRUCT*)lParam;
	TreeList_Expand (pecs->hWndTree, lptli, TLE_EXPANDALL);
	return TRUE;
}


//	___________________________________________________
//
//  Expand the selected items

BOOL 
GMExpandSelected (PGROUPMAN pGM) 
{
	TL_TREEITEM tli;
	EXPANDCOLLAPSESTRUCT ecs;

	if (GMMultipleSelected (pGM)) {
		ecs.lpfnCallback = sExpandSingleItem;
		ecs.hWndTree = pGM->hWndTree;
		TreeList_IterateSelected (pGM->hWndTree, &ecs);
		InvalidateRect (pGM->hWndTree, NULL, TRUE);
		return TRUE;
	}
	else {
		tli.hItem = GMFocusedItem (pGM);
		TreeList_Expand (pGM->hWndTree, &tli, TLE_EXPANDALL);
		InvalidateRect (pGM->hWndTree, NULL, TRUE);
		return TRUE;
	}
	return FALSE;
}


//	___________________________________________________
//
//  Collapse a single item
//	routine called as a
//	callback function from the TreeList control to 
//	collapse a single item.
// 
//	lptli	= pointer to TreeList item to collapse

static BOOL CALLBACK 
sCollapseSingleItem (TL_TREEITEM* lptli, LPARAM lParam) 
{
	EXPANDCOLLAPSESTRUCT* pecs = (EXPANDCOLLAPSESTRUCT*)lParam;
	TreeList_Expand (pecs->hWndTree, lptli, TLE_COLLAPSEALL);
	return TRUE;
}


//	___________________________________________________
//
//  Collapse the selected items

BOOL GMCollapseSelected (PGROUPMAN pGM) {
	TL_TREEITEM tli;
	EXPANDCOLLAPSESTRUCT ecs;

	if (GMMultipleSelected (pGM)) {
		ecs.lpfnCallback = sCollapseSingleItem;
		ecs.hWndTree = pGM->hWndTree;
		TreeList_IterateSelected (pGM->hWndTree, &ecs);
		InvalidateRect (pGM->hWndTree, NULL, TRUE);
		return TRUE;
	}
	else {
		tli.hItem = GMFocusedItem (pGM);
		TreeList_Expand (pGM->hWndTree, &tli, TLE_COLLAPSEALL);
		InvalidateRect (pGM->hWndTree, NULL, TRUE);
		return TRUE;
	}
	return FALSE;
}


//	___________________________________________
//
//  Determine if there is text in the clipboard

BOOL 
GMDataToPaste (VOID) 
{
	if (IsClipboardFormatAvailable (CF_TEXT)) return TRUE;
	return FALSE;
}

//	___________________________________________________
//
//	determine if each action is enabled or not

//	paste 
static BOOL sIsPasteEnabled (PGROUPMAN pGM) {
	if (GMMultipleSelected (pGM)) return FALSE;
	if (!GMDataToPaste ()) return FALSE;
	return TRUE;
}

//	delete 
static BOOL sIsDeleteEnabled (PGROUPMAN pGM) {
	if (GMFocusedItemType (pGM) == IDX_NONE) return FALSE;
	return TRUE;
}

//	select all 
static BOOL sIsSelectAllEnabled (PGROUPMAN pGM) {
	return TRUE;
}

//	collapse all 
static BOOL sIsCollapseAllEnabled (PGROUPMAN pGM) {
	if (GMFocusedItemType (pGM) == IDX_NONE) return TRUE;
	return FALSE;
}

//	expand all 
static BOOL sIsExpandAllEnabled (PGROUPMAN pGM) {
	if (GMFocusedItemType (pGM) == IDX_NONE) return TRUE;
	return FALSE;
}

//	collapse selected 
static BOOL sIsCollapseSelEnabled (PGROUPMAN pGM) {
	if (GMFocusedItemType (pGM) == IDX_NONE) return FALSE;
	return TRUE;
}

//	expand selected 
static BOOL sIsExpandSelEnabled (PGROUPMAN pGM) {
	if (GMFocusedItemType (pGM) == IDX_NONE) return FALSE;
	return TRUE;
}

//	locate keys
static BOOL sIsLocateKeysEnabled (PGROUPMAN pGM) {
	if (GMFocusedObjectType (pGM) != OBJECT_KEY) return FALSE;
	return TRUE;
}

//	get keys from server
static BOOL sIsGetFromServerEnabled (PGROUPMAN pGM) {
	if (GMFocusedObjectType (pGM) == OBJECT_NONE) return FALSE;
	return TRUE;
}

//	properties
static BOOL sIsPropertiesEnabled (PGROUPMAN pGM) {
	if (GMMultipleSelected (pGM)) return FALSE;
	if (GMFocusedObjectType (pGM) == OBJECT_NONE) return FALSE;
	return TRUE;
}

//	import groups from file
static BOOL sIsImportGroupsEnabled (PGROUPMAN pGM) {
	return TRUE;
}

//	unselect all 
static BOOL sIsUnselectAllEnabled (PGROUPMAN pGM) {
	if (pGM->bLocatingKeys) return FALSE;
	return TRUE;
}

//	update all groups from keyserver
static BOOL sIsUpdateAllGroupsEnabled (PGROUPMAN pGM) {
	return TRUE;
}

//	send all groups to keyserver
static BOOL sIsSendAllGroupsEnabled (PGROUPMAN pGM) {
	return TRUE;
}

//	___________________________________________________
//
//	determine if action is enabled or not, based on
//	flags passed in and current selection

BOOL PGPgmExport 
PGPgmIsActionEnabled (
		HGROUPMAN hGM, 
		ULONG	ulAction) 
{

	PGROUPMAN pGM = (PGROUPMAN)hGM;

	if (!hGM) return FALSE;
	if (ulAction & pGM->ulDisableActions) return FALSE;

	switch (ulAction) {

	case GM_PASTE :				return sIsPasteEnabled (pGM);
	case GM_DELETE :			return sIsDeleteEnabled (pGM);
	case GM_SELECTALL :			return sIsSelectAllEnabled (pGM);
	case GM_COLLAPSEALL :		return sIsCollapseAllEnabled (pGM);
	case GM_COLLAPSESEL :		return sIsCollapseSelEnabled (pGM);
	case GM_EXPANDALL :			return sIsExpandAllEnabled (pGM);
	case GM_EXPANDSEL :			return sIsExpandSelEnabled (pGM);
	case GM_PROPERTIES :		return sIsPropertiesEnabled (pGM);
	case GM_UNSELECTALL:		return sIsUnselectAllEnabled (pGM);
	case GM_LOCATEKEYS :		return sIsLocateKeysEnabled (pGM);
	case GM_GETFROMSERVER :		return sIsGetFromServerEnabled (pGM);
	case GM_IMPORTGROUPS :		return sIsImportGroupsEnabled (pGM);
	case GM_UPDATEALLGROUPS :	return sIsUpdateAllGroupsEnabled (pGM);
	case GM_SENDALLGROUPS :		return sIsSendAllGroupsEnabled (pGM);
	default :					return FALSE;
	}
}

//	___________________________________________________
//
//	perform the specified action

PGPError PGPgmExport 
PGPgmPerformAction (
		HGROUPMAN hGM, 
		ULONG	ulAction) 
{
	PGROUPMAN			pGM = (PGROUPMAN)hGM;

	HTLITEM			hFirst;
	TL_TREEITEM		tlI;

	if (!hGM) return kPGPError_BadParams;
	if (!PGPgmIsActionEnabled (hGM, ulAction)) 
		return kPGPError_UnknownRequest;

	switch (ulAction) {

	case GM_PASTE :
		GMPasteKeys (pGM);
		break;

	case GM_DELETE :
		GMDeleteObject (pGM);
		break;

	case GM_SELECTALL :
		// get first item in list
		TreeList_SelectChildren (pGM->hWndTree, NULL);
		hFirst = TreeList_GetFirstItem (pGM->hWndTree);
		// try to get second item
		tlI.hItem = hFirst;
		if (tlI.hItem) {
			tlI.mask = TLIF_NEXTHANDLE;
			TreeList_GetItem (pGM->hWndTree, &tlI);
		}
		// if second item exists, then multiple select
		if (tlI.hItem)
			GMSetFocus (pGM, hFirst, TRUE);
		else 
			GMSetFocus (pGM, hFirst, FALSE);
		SetFocus (pGM->hWndTree);
		break;

	case GM_COLLAPSEALL :
		GMCollapseSelected (pGM);
		break;

	case GM_COLLAPSESEL :
		GMCollapseSelected (pGM);
		break;

	case GM_EXPANDALL :
		GMExpandSelected (pGM);
		break;

	case GM_EXPANDSEL :
		GMExpandSelected (pGM);
		break;

	case GM_PROPERTIES :
		GMGroupProperties (pGM);
		break;

	case GM_LOCATEKEYS :
		GMLocateKeys (pGM);
		break;

	case GM_GETFROMSERVER :
		GMGetFromServer (pGM);
		break;

	case GM_IMPORTGROUPS :
		GMImportGroups (pGM, NULL);
		break;

	case GM_UNSELECTALL :
		TreeList_Select (pGM->hWndTree, NULL, TRUE);
		GMSetFocus (pGM, NULL, FALSE);
		break;

	case GM_UPDATEALLGROUPS :
		GMGetGroupsFromServer (pGM);
		break;
		
	case GM_SENDALLGROUPS :
		GMSendGroupsToServer (pGM);
		break;
		
	}

	return kPGPError_NoErr;
}

