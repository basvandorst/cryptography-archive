/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	GMMenu.c - handle menu enabling/disabling chores
	

	$Id: GMMenu.c,v 1.8 1997/09/18 16:13:08 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

#include "pgpgmx.h"
#include "resource.h"

#include "pgpBuildFlags.h"

typedef struct {
	FARPROC lpfnCallback;
	HWND hWndTree;
} EXPANDCOLLAPSESTRUCT;

// External globals
extern HINSTANCE g_hInst;


//----------------------------------------------------|
//  Expand a single item
//	routine called as a
//	callback function from the TreeList control to 
//	expand a single item.
//
//	lptli	= pointer to TreeList item to expand

BOOL CALLBACK ExpandSingleItem (TL_TREEITEM* lptli, LPARAM lParam) {
	EXPANDCOLLAPSESTRUCT* pecs = (EXPANDCOLLAPSESTRUCT*)lParam;
	TreeList_Expand (pecs->hWndTree, lptli, TLE_EXPANDALL);
	return TRUE;
}


//----------------------------------------------------|
//  Expand the selected items

BOOL GMExpandSelected (PGROUPMAN pGM) {
	TL_TREEITEM tli;
	EXPANDCOLLAPSESTRUCT ecs;

	if (GMMultipleSelected (pGM)) {
		ecs.lpfnCallback = ExpandSingleItem;
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


//----------------------------------------------------|
//  Collapse a single item
//	routine called as a
//	callback function from the TreeList control to 
//	collapse a single item.
// 
//	lptli	= pointer to TreeList item to collapse

BOOL CALLBACK CollapseSingleItem (TL_TREEITEM* lptli, LPARAM lParam) {
	EXPANDCOLLAPSESTRUCT* pecs = (EXPANDCOLLAPSESTRUCT*)lParam;
	TreeList_Expand (pecs->hWndTree, lptli, TLE_COLLAPSEALL);
	return TRUE;
}


//----------------------------------------------------|
//  Collapse the selected items

BOOL GMCollapseSelected (PGROUPMAN pGM) {
	TL_TREEITEM tli;
	EXPANDCOLLAPSESTRUCT ecs;

	if (GMMultipleSelected (pGM)) {
		ecs.lpfnCallback = CollapseSingleItem;
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
BOOL IsPasteEnabled (PGROUPMAN pGM) {
	if (GMMultipleSelected (pGM)) return FALSE;
	if (!GMDataToPaste ()) return FALSE;
	return TRUE;
}

//	delete 
BOOL IsDeleteEnabled (PGROUPMAN pGM) {
	if (GMFocusedItemType (pGM) == IDX_NONE) return FALSE;
	return TRUE;
}

//	select all 
BOOL IsSelectAllEnabled (PGROUPMAN pGM) {
	return TRUE;
}

//	collapse all 
BOOL IsCollapseAllEnabled (PGROUPMAN pGM) {
	if (GMFocusedItemType (pGM) == IDX_NONE) return TRUE;
	return FALSE;
}

//	expand all 
BOOL IsExpandAllEnabled (PGROUPMAN pGM) {
	if (GMFocusedItemType (pGM) == IDX_NONE) return TRUE;
	return FALSE;
}

//	collapse selected 
BOOL IsCollapseSelEnabled (PGROUPMAN pGM) {
	if (GMFocusedItemType (pGM) == IDX_NONE) return FALSE;
	return TRUE;
}

//	expand selected 
BOOL IsExpandSelEnabled (PGROUPMAN pGM) {
	if (GMFocusedItemType (pGM) == IDX_NONE) return FALSE;
	return TRUE;
}

//	locate keys
BOOL IsLocateKeysEnabled (PGROUPMAN pGM) {
	if (GMFocusedObjectType (pGM) != OBJECT_KEY) return FALSE;
	return TRUE;
}

//	get keys from server
BOOL IsGetFromServerEnabled (PGROUPMAN pGM) {
	if (GMFocusedObjectType (pGM) == OBJECT_NONE) return FALSE;
	return TRUE;
}

//	properties
BOOL IsPropertiesEnabled (PGROUPMAN pGM) {
	if (GMMultipleSelected (pGM)) return FALSE;
	if (GMFocusedObjectType (pGM) == OBJECT_NONE) return FALSE;
	return TRUE;
}

//	import groups from file
BOOL IsImportGroupsEnabled (PGROUPMAN pGM) {
	return TRUE;
}

//	unselect all 
BOOL IsUnselectAllEnabled (PGROUPMAN pGM) {
	if (pGM->bLocatingKeys) return FALSE;
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

	case GM_PASTE :				return IsPasteEnabled (pGM);
	case GM_DELETE :			return IsDeleteEnabled (pGM);
	case GM_SELECTALL :			return IsSelectAllEnabled (pGM);
	case GM_COLLAPSEALL :		return IsCollapseAllEnabled (pGM);
	case GM_COLLAPSESEL :		return IsCollapseSelEnabled (pGM);
	case GM_EXPANDALL :			return IsExpandAllEnabled (pGM);
	case GM_EXPANDSEL :			return IsExpandSelEnabled (pGM);
	case GM_PROPERTIES :		return IsPropertiesEnabled (pGM);
	case GM_UNSELECTALL:		return IsUnselectAllEnabled (pGM);
	case GM_LOCATEKEYS :		return IsLocateKeysEnabled (pGM);
	case GM_GETFROMSERVER :		return IsGetFromServerEnabled (pGM);
	case GM_IMPORTGROUPS :		return IsImportGroupsEnabled (pGM);
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
		
	}

	return kPGPError_NoErr;
}

