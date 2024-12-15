/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	GMTree.h - create and fill group manager control
	

	$Id: GMMisc.c,v 1.14 1997/10/20 15:23:52 pbj Exp $
____________________________________________________________________________*/

#include "pgpgmx.h"
#include <process.h>

#include "resource.h"
#include "pgpClientPrefs.h"

#define DEFAULTCOLWIDTHNAME  240
#define DEFAULTCOLWIDTHVALID  50
#define DEFAULTCOLWIDTHDESC	 200
#define MINWIDTH			  10
#define MAXWIDTH			1000

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
	return HeapAlloc (GetProcessHeap (), HEAP_ZERO_MEMORY, size);
}

//	____________________________________
//
//	Private memory deallocation routine

VOID 
gmFree (VOID* p) 
{
	if (p) {
		FillMemory (p, lstrlen(p), '\0');
		HeapFree (GetProcessHeap (), 0, p);
	}
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
ConvertFromPGPValidity (UINT uPGPValidity) 
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
GetColumnPreferences (PGROUPMAN pGM)
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

	err = PGPcomdlgOpenClientPrefs (&prefref);
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
		PGPcomdlgCloseClientPrefs (prefref, FALSE);
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

	err = PGPcomdlgOpenClientPrefs (&prefref);
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

		PGPcomdlgCloseClientPrefs (prefref, TRUE);
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
	PGPKeyRef		key	= kPGPInvalidRef;
	PGPKeyID		keyID;
	
	context	= PGPGetKeySetContext( keySet );
	
	/* get the key ID into a PGPsdk form */
	err	= PGPImportKeyID (item->u.key.exportedKeyID, &keyID);
	if ( IsntPGPError( err ) )
	{
		err	= PGPGetKeyByKeyID( keySet, &keyID,
				(PGPPublicKeyAlgorithm)item->u.key.algorithm, &key );
	}
	
	*outKey	= key;
	
	return( err );
}
	
//	___________________________________________
//
//	Sort on name callback routine

static int
SortOnName (
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
		err	= PGPSortGroupSet (pGM->groupsetMain, SortOnName, &info);
		break;

	case kPGPReverseUserIDOrdering :
		info.iInvert = 1;
		err	= PGPSortGroupSet (pGM->groupsetMain, SortOnName, &info);
		break;

	case kPGPValidityOrdering :
		info.iInvert = 1;
		err	= PGPSortGroupSet (pGM->groupsetMain, SortOnName, &info);
		break;

	case kPGPReverseValidityOrdering :
		info.iInvert = 1;
		err	= PGPSortGroupSet (pGM->groupsetMain, SortOnName, &info);
		break;
	}
	
	return (err);
}


//	_______________________________________________
//
//	recursively add keys to list
//
VOID 
AddKeysInGroup (PSERVERSTRUCT pss, PGPGroupID groupid) 
{
	INT				i, iNumKeys, iNumTotal;
	PGPGroupItem	groupitem;
	PGPError		err;

	PGPCountGroupItems (pss->pGM->groupsetMain, groupid, FALSE,
				&iNumKeys, &iNumTotal);

	for (i=0; i<iNumTotal; i++) {
		PGPGetIndGroupItem (pss->pGM->groupsetMain, groupid, i, &groupitem);

		if (groupitem.type == kPGPGroupItem_KeyID) {
			err = PGPImportKeyID ( 
							groupitem.u.key.exportedKeyID, 
							&pss->pkeyidList[pss->icount]);

			if (IsntPGPError (err))
				++(pss->icount);
		}

		// otherwise it's a group
		else {
			AddKeysInGroup (pss, groupitem.u.group.id);
		}
	}
}

//	_______________________________________________
//
//	routines called as callback functions from the 
//  TreeList control

BOOL CALLBACK 
CountSelectedObjects (
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

BOOL CALLBACK 
AddKeyIDToList (
		TL_TREEITEM*	lptli, 
		LPARAM			lParam) 
{
	SERVERSTRUCT*	pss = (SERVERSTRUCT*)lParam;
	PGPGroupID		id;
	PGPUInt32		index;
	PGPGroupItem	groupitem;
	PGPError		err;

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
			err = PGPImportKeyID (
							groupitem.u.key.exportedKeyID, 
							&pss->pkeyidList[pss->icount]);

			if (IsntPGPError (err))
				++(pss->icount);
		}

		// not a key, it's a non-root group
		else id = groupitem.u.group.id;
	}

	// it's a group
	if (id) {
		AddKeysInGroup (pss, id);
	}

	return TRUE;
}

//	_______________________________________________
//
//  Update selected keys from server

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
	ss.lpfnCallback = CountSelectedObjects;
	TreeList_IterateSelected (pGM->hWndTree, &ss);
	if (ss.icount) {
		ss.pkeyidList = 
			(PGPKeyID*)HeapAlloc (GetProcessHeap (), 0,
				(ss.icount) * sizeof(PGPKeyID));
		ss.icount = 0;
		ss.lpfnCallback = AddKeyIDToList;
		TreeList_IterateSelected (pGM->hWndTree, &ss);
		err = PGPcomdlgSearchServerForKeyIDs (pGM->context, 
							pGM->hWndParent, ss.pkeyidList,
							ss.icount, &keysetFound);
		HeapFree (GetProcessHeap (), 0, ss.pkeyidList);
	}
	else {
		PGPgmMessageBox (pGM->hWndParent, IDS_CAPTION,
					IDS_EMPTYGROUP, MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}
	
	if (err == kPGPError_UserAbort) return FALSE;

	if (IsPGPError (err)) {
		PGPcomdlgErrorMessage (err);
	}
	else {
		if (PGPRefIsValid (keysetFound)) {
			PGPUInt32 u;
			PGPCountKeys (keysetFound, &u);
			if (u > 0) {
				PGPkmQueryAddKeys (pGM->context, pGM->hWndParent,
										keysetFound, pGM->keysetMain);
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

//	___________________________________________________
//
//  commit groups changes and broadcast reload message

PGPError GMCommitGroupChanges (PGROUPMAN pGM, BOOL bBroadcast) {

	PGPError		err			= kPGPError_NoErr;
	HCURSOR			hCursorOld;
	UINT			uReloadMessage;

	if (PGPGroupSetNeedsCommit (pGM->groupsetMain)) {

		hCursorOld = SetCursor (LoadCursor (NULL, IDC_WAIT));
		err = PGPcomdlgSaveGroupFile (pGM->pGroupFile);
		SetCursor (hCursorOld);

		if (!(PGPcomdlgErrorMessage (err)) && bBroadcast) {
			uReloadMessage = RegisterWindowMessage (RELOADKEYRINGMSG);
			PostMessage (HWND_BROADCAST, uReloadMessage, 
				MAKEWPARAM (LOWORD (pGM->hWndParent), FALSE), 
				GetCurrentProcessId ());
		}
	}
	return err;
}





