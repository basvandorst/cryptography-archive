/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	GMTree.h - create and fill group manager control
	

	$Id: GMMisc.c,v 1.25 1998/08/11 15:20:03 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

// project header files
#include "pgpgmx.h"

// pgp header files
#include "pgpClientPrefs.h"

// constant definitions
#define DEFAULTCOLWIDTHNAME  240
#define DEFAULTCOLWIDTHVALID  50
#define DEFAULTCOLWIDTHDESC	 200
#define MINWIDTH			  10
#define MAXWIDTH			1000

// typedefs
typedef struct {
	FARPROC			lpfnCallback;
	PGROUPMAN		pGM;
	PGPContextRef	context;
	PGPKeySetRef	keyset;
	INT				icount;
	PGPKeyID*		pkeyidList;
} SERVERSTRUCT, *PSERVERSTRUCT;

typedef struct {
	WORD wColumnField[NUMBERFIELDS];
	WORD wFieldWidth[NUMBERFIELDS];
	LONG lSortField;
} COLUMNPREFSSTRUCT, *PCOLUMNPREFSSTRUCT;

typedef struct {
	PGPKeySetRef	keySet;
	PGPGroupSetRef	set;
	INT				iInvert;
} GMSortInfo;


// external globals
extern HINSTANCE g_hInst;


//	____________________________________
//
//	Private memory allocation routine

VOID* 
gmAlloc (LONG size) 
{
	return malloc (size);
}

//	____________________________________
//
//	Private memory deallocation routine

VOID 
gmFree (VOID* p) 
{
	if (p) 
		free (p);
}

//	____________________________________
//
//	Message box routine
//
//	iCaption and iMessage are string table resource IDs.

LRESULT 
PGPgmMessageBox (
		HWND	hWnd, 
		INT		iCaption, 
		INT		iMessage,
		ULONG	ulFlags) 
{
	CHAR szCaption [128];
	CHAR szMessage [512];

	LoadString (g_hInst, iCaption, szCaption, sizeof(szCaption));
	LoadString (g_hInst, iMessage, szMessage, sizeof(szMessage));

	ulFlags |= MB_SETFOREGROUND;
	return (MessageBox (hWnd, szMessage, szCaption, ulFlags));
}

//	___________________________________________________
//
//	Convert validity from weird PGP values to 0-2 scale

UINT 
GMConvertFromPGPValidity (UINT uPGPValidity) 
{
	switch (uPGPValidity) {
	case kPGPValidity_Unknown :
	case kPGPValidity_Invalid :
		return GM_VALIDITY_INVALID;
	case kPGPValidity_Marginal :
		return GM_VALIDITY_MARGINAL;
	case kPGPValidity_Complete :
		return GM_VALIDITY_COMPLETE;
	}
}

//	___________________________________________
//
//	Get widths of control columns from prefs file

VOID 
GMGetColumnPreferences (PGROUPMAN pGM)
{
	PGPError			err;
	PGPPrefRef			prefref;
	PGPSize				size;
	PCOLUMNPREFSSTRUCT	pcps;
	INT					i;
	WORD				wField, wWidth;

	pGM->wColumnField[GMI_NAME]			= 0;
	pGM->wColumnField[GMI_VALIDITY]		= 1;
	pGM->wColumnField[GMI_DESCRIPTION]	= 2;

	pGM->wFieldWidth[GMI_NAME]			= DEFAULTCOLWIDTHNAME;
	pGM->wFieldWidth[GMI_VALIDITY]		= DEFAULTCOLWIDTHVALID;
	pGM->wFieldWidth[GMI_DESCRIPTION]	= DEFAULTCOLWIDTHDESC;

	pGM->lKeyListSortField				= kPGPUserIDOrdering;

	err = PGPclOpenClientPrefs (PGPGetContextMemoryMgr (pGM->context),
									&prefref);
	if (IsntPGPError (err)) {
		err = PGPGetPrefData (prefref, kPGPPrefPGPkeysWinGroupsData,
							  &size, &pcps);

		if (IsntPGPError (err)) {
			if (size == sizeof(COLUMNPREFSSTRUCT)) {
				for (i=0; i<NUMBERFIELDS; i++) {
					wField = pcps->wColumnField[i];
					if ((wField >= 0) && (wField < NUMBERFIELDS))
						pGM->wColumnField[i] = wField;
					wWidth = pcps->wFieldWidth[i];
					if ((wWidth >= MINWIDTH) && (wWidth <= MAXWIDTH))
						pGM->wFieldWidth[i] = wWidth;
				}
				pGM->lKeyListSortField = pcps->lSortField;
			}
			PGPDisposePrefData (prefref, pcps);
		}
		PGPclCloseClientPrefs (prefref, FALSE);
	}
}

//	___________________________________________
// 
//	Put window position information in registry

VOID 
GMSetColumnPreferences (PGROUPMAN pGM) 
{
	PGPError			err;
	PGPPrefRef			prefref;
	COLUMNPREFSSTRUCT	cps;
	INT					i, iField;

	err = PGPclOpenClientPrefs (PGPGetContextMemoryMgr (pGM->context),
									&prefref);
	if (IsntPGPError (err)) {
		for (i=0; i<NUMBERFIELDS; i++) {
			cps.wColumnField[i] = pGM->wColumnField[i];

			iField = pGM->wColumnField[i];
			if ((i == 0) || (iField != 0)) pGM->wFieldWidth[iField] = 
					LOWORD (TreeList_GetColumnWidth (pGM->hWndTree, i));
			cps.wFieldWidth[i] = pGM->wFieldWidth[i];
		}

		cps.lSortField = pGM->lKeyListSortField;

		PGPSetPrefData (prefref, kPGPPrefPGPkeysWinGroupsData,
							  sizeof(cps), &cps);

		PGPclCloseClientPrefs (prefref, TRUE);
	}
}

//	___________________________________________
//
//	Get key from groupitem struct

static PGPError
sGroupItemToKey(
	PGPGroupItem const *	item,
	PGPKeySetRef			keySet,
	PGPKeyRef *				outKey )
{
	PGPError		err	= kPGPError_NoErr;
	PGPContextRef	context;
	
	context	= PGPGetKeySetContext( keySet );
	
	err	= PGPGetKeyByKeyID( keySet, &item->u.key.keyID,
				item->u.key.algorithm, outKey );
	
	return( err );
}
	
//	___________________________________________
//
//	Sort on name callback routine

static INT
sSortOnName (
	PGPGroupItem *	item1,
	PGPGroupItem *	item2,
	void *			userValue )
{
	PGPKeyRef		key1;
	PGPKeyRef		key2;
	PGPError		err1;
	PGPError		err2;
	GMSortInfo const *	info	= (GMSortInfo const *)userValue;
	PGPKeySetRef	keySet	= info->keySet;
	int				result;
	int				iInvert;
			
	keySet	= info->keySet;
	iInvert = info->iInvert;
	
	if ( item1->type == kPGPGroupItem_Group ||
			item2->type == kPGPGroupItem_Group )
	{
		PGPGroupInfo	groupInfo1;
		PGPGroupInfo	groupInfo2;
		
		/* for now, always make groups come before keys */
		if (item1->type == kPGPGroupItem_Group &&
			item2->type == kPGPGroupItem_KeyID)
		{
			return (-1 * iInvert);
		}
		else if (item1->type == kPGPGroupItem_KeyID &&
			item2->type == kPGPGroupItem_Group)
		{
			return ( 1 * iInvert);
		}
		/* both are groups */
		PGPGetGroupInfo (info->set, item1->u.group.id, &groupInfo1);
		PGPGetGroupInfo (info->set, item2->u.group.id, &groupInfo2);
		return (lstrcmpi (groupInfo1.name, groupInfo2.name) * iInvert);
	}
	
	err1	= sGroupItemToKey (item1, keySet, &key1);
	err2	= sGroupItemToKey (item2, keySet, &key2);
	
	/*
		Rules:
		errors on both: consider the same
		error on one: it's always less than
	*/
	if (IsPGPError (err1) && IsPGPError (err2))
	{
		/* if we got errors on both, consider them the same */
		return (0);
	}
	else if (IsPGPError (err1))
	{
		return (-1 * iInvert);
	}
	else if (IsPGPError (err2))
	{
		return ( 1 * iInvert);
	}
	
	result	= PGPCompareKeys (key1, key2, kPGPUserIDOrdering);
	
	return (result * iInvert);
}

//	___________________________________________
//
//	Sort groupset based on selected criteria

PGPError
GMSortGroupSet (PGROUPMAN pGM)
{
	PGPError	err	= kPGPError_NoErr;
	GMSortInfo	info;
	
	info.set	= pGM->groupsetMain;
	info.keySet	= pGM->keysetMain;
	
	switch (pGM->lKeyListSortField) {
	case kPGPUserIDOrdering :
		info.iInvert = 1;
		err	= PGPSortGroupSet (pGM->groupsetMain, sSortOnName, &info);
		break;

	case kPGPReverseUserIDOrdering :
		info.iInvert = 1;
		err	= PGPSortGroupSet (pGM->groupsetMain, sSortOnName, &info);
		break;

	case kPGPValidityOrdering :
		info.iInvert = 1;
		err	= PGPSortGroupSet (pGM->groupsetMain, sSortOnName, &info);
		break;

	case kPGPReverseValidityOrdering :
		info.iInvert = 1;
		err	= PGPSortGroupSet (pGM->groupsetMain, sSortOnName, &info);
		break;
	}
	
	return (err);
}


//	_______________________________________________
//
//	recursively add keys to list
//
static VOID 
sAddKeysInGroup (PSERVERSTRUCT pss, PGPGroupID groupid) 
{
	INT				i, iNumKeys, iNumTotal;
	PGPGroupItem	groupitem;

	PGPCountGroupItems (pss->pGM->groupsetMain, groupid, FALSE,
				&iNumKeys, &iNumTotal);

	for (i=0; i<iNumTotal; i++) {
		PGPGetIndGroupItem (pss->pGM->groupsetMain, groupid, i, &groupitem);

		if (groupitem.type == kPGPGroupItem_KeyID) {
			pss->pkeyidList[pss->icount] = 
				groupitem.u.key.keyID;
			++(pss->icount);
		}

		// otherwise it's a group
		else {
			sAddKeysInGroup (pss, groupitem.u.group.id);
		}
	}
}

//	_______________________________________________
//
//	routines called as callback functions from the 
//  TreeList control

static BOOL CALLBACK 
sCountSelectedObjects (
		TL_TREEITEM*	lptli, 
		LPARAM			lParam) 
{
	SERVERSTRUCT*	pss			= (SERVERSTRUCT*)lParam;
	PGPGroupID		groupid;
	PGPGroupItem	groupitem;
	INT				iNumKeys, iNumTotal;
	INT				iIndex;

	// is the current item a group ?
	if (lptli->iImage == IDX_GROUP) {
		groupid = HIWORD(lptli->lParam);
		if (groupid) {
			iIndex = LOWORD(lptli->lParam);
			PGPGetIndGroupItem (pss->pGM->groupsetMain, groupid, iIndex,
									&groupitem);
			groupid = groupitem.u.group.id; 
		}
		else groupid = LOWORD(lptli->lParam);

		PGPCountGroupItems (pss->pGM->groupsetMain, groupid, TRUE,
				&iNumKeys, &iNumTotal);
		pss->icount += iNumKeys;
	}

	// otherwise, it's a key; count it
	else { 
		++(pss->icount);
	}

	return TRUE;
}

static BOOL CALLBACK 
sAddKeyIDToList (
		TL_TREEITEM*	lptli, 
		LPARAM			lParam) 
{
	SERVERSTRUCT*	pss = (SERVERSTRUCT*)lParam;
	PGPGroupID		id;
	PGPUInt32		index;
	PGPGroupItem	groupitem;

	id = HIWORD(lptli->lParam);
	index = LOWORD(lptli->lParam);

	// is it a root group?
	if (id == 0) id = index;

	// not a root group
	else {
		PGPGetIndGroupItem (pss->pGM->groupsetMain, id, index, &groupitem);

		// is it a key ?
		if (groupitem.type == kPGPGroupItem_KeyID) {
			id = 0;
			pss->pkeyidList[pss->icount] =
				groupitem.u.key.keyID;

			++(pss->icount);
		}

		// not a key, it's a non-root group
		else id = groupitem.u.group.id;
	}

	// it's a group
	if (id) {
		sAddKeysInGroup (pss, id);
	}

	return TRUE;
}

//	_______________________________________________
//
//  Update selected group from server

BOOL 
GMGetFromServer (PGROUPMAN pGM) 
{
	SERVERSTRUCT	ss;
	PGPKeySetRef	keysetFound	= kPGPInvalidRef;
	PGPError		err;

	ss.context = pGM->context;
	ss.pGM = pGM;

	// get selected keys
	ss.icount = 0;
	ss.lpfnCallback = sCountSelectedObjects;
	TreeList_IterateSelected (pGM->hWndTree, &ss);
	if (ss.icount) {
		ss.pkeyidList = 
			(PGPKeyID*)gmAlloc(ss.icount * sizeof(PGPKeyID));
		ss.icount = 0;
		ss.lpfnCallback = sAddKeyIDToList;
		TreeList_IterateSelected (pGM->hWndTree, &ss);
		err = PGPclSearchServerForKeyIDs (pGM->context, pGM->tlsContext,
							pGM->hWndParent, ss.pkeyidList,
							ss.icount, PGPCL_DEFAULTSERVER,
							pGM->keysetMain, &keysetFound);
		gmFree (ss.pkeyidList);
	}
	else {
		PGPgmMessageBox (pGM->hWndParent, IDS_CAPTION,
					IDS_EMPTYGROUP, MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}
	
	if (err == kPGPError_UserAbort) return FALSE;

	if (IsPGPError (err)) {
		PGPclErrorBox (NULL, err);
	}
	else {
		if (PGPRefIsValid (keysetFound)) {
			PGPUInt32 u;
			PGPCountKeys (keysetFound, &u);
			if (u > 0) {
				PGPclQueryAddKeys (pGM->context, pGM->tlsContext,
						pGM->hWndParent, keysetFound, pGM->keysetMain);
			}
			else {
				PGPgmMessageBox (pGM->hWndParent, IDS_CAPTION,
						IDS_SERVERSEARCHFAIL, MB_OK|MB_ICONEXCLAMATION);
			}
			PGPFreeKeySet (keysetFound);
		}
	}

	return TRUE;
}

//	_______________________________________________
//
//  Update selected group from server

VOID 
GMGetGroupsFromServer (PGROUPMAN pGM) 
{
	PGPError		err			= kPGPError_NoErr;
	PGPGroupSetRef	groupset	= kInvalidPGPGroupSetRef;

	err = PGPclGetGroupsFromRootServer (
				pGM->context,
				pGM->tlsContext,
				pGM->hWndParent,
				pGM->keysetMain,
				&groupset);

	if (IsntPGPError (err) && PGPGroupSetRefIsValid (groupset))
	{
		err = PGPMergeGroupSets (groupset, pGM->groupsetMain);
		if (IsntPGPError (err)) 
		{
			GMSortGroupSet (pGM);
			GMCommitGroupChanges (pGM, TRUE);
			GMLoadGroupsIntoTree (pGM, FALSE, TRUE, FALSE);
			InvalidateRect (pGM->hWndTree, NULL, TRUE);
			UpdateWindow (pGM->hWndTree);
		}
		PGPFreeGroupSet (groupset);
	}

	PGPclErrorBox (pGM->hWndParent, err);
}

//	_______________________________________________
//
//  Send groups to root server

VOID 
GMSendGroupsToServer (PGROUPMAN pGM) 
{
	PGPError		err			= kPGPError_NoErr;
	PGPGroupSetRef	groupset	= kInvalidPGPGroupSetRef;

	err = PGPclSendGroupsToRootServer (
				pGM->context,
				pGM->tlsContext,
				pGM->hWndParent,
				pGM->keysetMain,
				pGM->groupsetMain);

	PGPclErrorBox (pGM->hWndParent, err);
}

//	___________________________________________________
//
//  commit groups changes and broadcast reload message

PGPError 
GMCommitGroupChanges (PGROUPMAN pGM, BOOL bBroadcast) 
{

	PGPError		err			= kPGPError_NoErr;
	HCURSOR			hCursorOld;
	UINT			uReloadMessage;

	if (PGPGroupSetNeedsCommit (pGM->groupsetMain)) {

		hCursorOld = SetCursor (LoadCursor (NULL, IDC_WAIT));
		err = PGPclSaveGroupFile (pGM->pGroupFile);
		SetCursor (hCursorOld);

		if (IsntPGPError (PGPclErrorBox (NULL, err)) && bBroadcast) {
			uReloadMessage = RegisterWindowMessage (RELOADKEYRINGMSG);
			PostMessage (HWND_BROADCAST, uReloadMessage, 
				MAKEWPARAM (LOWORD (pGM->hWndParent), FALSE), 
				GetCurrentProcessId ());
		}
	}
	return err;
}





