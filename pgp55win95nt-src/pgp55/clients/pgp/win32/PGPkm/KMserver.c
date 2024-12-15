/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	KMserver.c - handle communication with keyserver
	

	$Id: KMserver.c,v 1.21 1997/10/08 17:31:55 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

#include "pgpkmx.h"
#include "resource.h"
#include "pgpkeyserver.h"
#include "pgpkeyserverprefs.h"


// external globals
extern HINSTANCE g_hInst;

typedef struct {
	FARPROC			lpfnCallback;
	PKEYMAN			pKM;
	PGPContextRef	context;
	PGPKeySetRef	keyset;
	INT				icount;
	PGPKeyID*		pkeyidList;
} SERVERSTRUCT, *PSERVERSTRUCT;

// local globals


//	_______________________________________________
//
//	routines called as callback functions from the 
//  TreeList control
//

BOOL CALLBACK 
AddKeyToKeySet (
		TL_TREEITEM*	lptli, 
		LPARAM			lParam) 
{
	SERVERSTRUCT*	pss = (SERVERSTRUCT*)lParam;
	PGPKeySetRef	KeySet;
	PGPKeyRef		Key;

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
		Key = (PGPKeyRef)(lptli->lParam);
		break;

	case IDX_RSAUSERID :
	case IDX_DSAUSERID :
		Key = KMGetKeyFromUserID (pss->pKM, (PGPUserIDRef)(lptli->lParam));
		break;

	case IDX_CERT :
	case IDX_REVCERT :
	case IDX_BADCERT :
	case IDX_EXPORTCERT :
	case IDX_METACERT :
	case IDX_EXPORTMETACERT :
		Key = KMGetKeyFromCert (pss->pKM, (PGPSigRef)(lptli->lParam));
		break;

	default :
		return FALSE;
	}

	PGPNewSingletonKeySet (Key, &KeySet);
	PGPcomdlgErrorMessage (PGPAddKeys (KeySet, pss->keyset));
	PGPCommitKeyRingChanges (pss->keyset);
	PGPFreeKeySet (KeySet);

	++(pss->icount);

	return TRUE;
}

BOOL CALLBACK 
CountSelectedObjects (
		TL_TREEITEM*	lptli, 
		LPARAM			lParam) 
{
	SERVERSTRUCT*	pss = (SERVERSTRUCT*)lParam;
	++(pss->icount);

	return TRUE;
}

BOOL CALLBACK 
AddKeyIDToList (
		TL_TREEITEM*	lptli, 
		LPARAM			lParam) 
{
	SERVERSTRUCT*	pss = (SERVERSTRUCT*)lParam;
	PGPSigRef		cert;

	cert = (PGPSigRef)(lptli->lParam);
	PGPGetKeyIDOfCertifier (cert, &pss->pkeyidList[pss->icount]);

	++(pss->icount);

	return TRUE;
}

//	___________________________________________________
//
//	Send selected keys to keyserver

BOOL 
KMSendToServer (PKEYMAN pKM) 
{
	SERVERSTRUCT	ss;
	PGPError		err;

	ss.context = pKM->Context;
	ss.pKM = pKM;
	ss.icount = 0;

	PGPNewKeySet (pKM->Context, &(ss.keyset));
	if (PGPRefIsValid (ss.keyset)) {
		ss.lpfnCallback = AddKeyToKeySet;
		TreeList_IterateSelected (pKM->hWndTree, &ss);
		err = PGPcomdlgSendKeysToServer (pKM->Context, pKM->hWndParent, 
					pKM->szPutKeyserver, pKM->KeySetMain, ss.keyset);
		PGPFreeKeySet (ss.keyset);
	}

	if (err == kPGPError_ServerKeyAlreadyExists) {
		if (ss.icount > 1) {
			KMMessageBox (pKM->hWndParent, IDS_CAPTION, 
						IDS_KEYSALREADYONSERVER, MB_OK|MB_ICONINFORMATION);
		}
		else {
			KMMessageBox (pKM->hWndParent, IDS_CAPTION, 
						IDS_KEYALREADYONSERVER, MB_OK|MB_ICONINFORMATION);
		}
	}
	else if (IsPGPError (err)) {
		PGPcomdlgErrorMessage (err);
	}
	else {
		KMMessageBox (pKM->hWndParent, IDS_CAPTION, IDS_UPLOADOK,
								MB_OK|MB_ICONINFORMATION);
	}

	return TRUE;
}

//	___________________________________________________
//
//	Disable selected keys on keyserver

BOOL 
KMDisableOnServer (PKEYMAN pKM) 
{
	SERVERSTRUCT			ss;
	PGPError				err;
	PGPKeyServerKeySpace	space;

	ss.context = pKM->Context;
	ss.pKM = pKM;
	ss.icount = 0;

	if (pKM->ulOptionFlags & KMF_PENDINGBUCKET)
		space = kPGPKSKeySpacePending;
	else
		space = kPGPKSKeySpaceDefault;

	PGPNewKeySet (pKM->Context, &(ss.keyset));
	if (PGPRefIsValid (ss.keyset)) {
		ss.lpfnCallback = AddKeyToKeySet;
		TreeList_IterateSelected (pKM->hWndTree, &ss);
		err = PGPcomdlgDisableKeysOnServer (pKM->Context, pKM->hWndParent, 
					pKM->szPutKeyserver, space, pKM->KeySetMain, ss.keyset);
		PGPFreeKeySet (ss.keyset);
	}

	if (IsPGPError (err)) PGPcomdlgErrorMessage (err);
	else {
		KMMessageBox (pKM->hWndParent, IDS_CAPTION, IDS_DISABLEONSERVEROK,
								MB_OK|MB_ICONINFORMATION);
	}

	return TRUE;
}

//	___________________________________________________
//
//	Delete selected keys from keyserver

BOOL 
KMDeleteFromServer (PKEYMAN pKM) 
{
	SERVERSTRUCT			ss;
	PGPError				err;
	PGPKeyServerKeySpace	space;

	ss.context = pKM->Context;
	ss.pKM = pKM;
	ss.icount = 0;

	if (pKM->ulOptionFlags & KMF_PENDINGBUCKET)
		space = kPGPKSKeySpacePending;
	else
		space = kPGPKSKeySpaceDefault;

	PGPNewKeySet (pKM->Context, &(ss.keyset));
	if (PGPRefIsValid (ss.keyset)) {
		ss.lpfnCallback = AddKeyToKeySet;
		TreeList_IterateSelected (pKM->hWndTree, &ss);
		err = PGPcomdlgDeleteKeysFromServer (pKM->Context, pKM->hWndParent, 
					pKM->szPutKeyserver, space, pKM->KeySetMain, ss.keyset);
		PGPFreeKeySet (ss.keyset);
	}

	if (IsPGPError (err)) PGPcomdlgErrorMessage (err);
	else {
		KMMessageBox (pKM->hWndParent, IDS_CAPTION, IDS_DELETEFROMSERVEROK,
								MB_OK|MB_ICONINFORMATION);
	}

	return TRUE;
}

//	_______________________________________________
//
//  Update selected keys from server

BOOL 
KMGetFromServerInternal (PKEYMAN pKM, BOOL bQueryAdd, BOOL bWarn) 
{
	SERVERSTRUCT	ss;
	PGPKeySetRef	keysetFound	= kPGPInvalidRef;
	PGPError		err;

	ss.context = pKM->Context;
	ss.pKM = pKM;
	ss.icount = 0;

	// get selected keys
	if (KMFocusedObjectType (pKM) == OBJECT_KEY) {
		PGPNewKeySet (pKM->Context, &(ss.keyset));
		if (PGPRefIsValid (ss.keyset)) {
			ss.lpfnCallback = AddKeyToKeySet;
			TreeList_IterateSelected (pKM->hWndTree, &ss);
			err = PGPcomdlgUpdateKeySetFromServer (pKM->Context, 
										pKM->hWndParent, 
										ss.keyset, 
										&keysetFound);
			PGPFreeKeySet (ss.keyset);
		}
	}

	// get signing keys
	else {
		ss.icount = 0;
		ss.lpfnCallback = CountSelectedObjects;
		TreeList_IterateSelected (pKM->hWndTree, &ss);
		if (ss.icount) {
			ss.pkeyidList = 
				(PGPKeyID*)HeapAlloc (GetProcessHeap (), 0,
					(ss.icount) * sizeof(PGPKeyID));
			ss.icount = 0;
			ss.lpfnCallback = AddKeyIDToList;
			TreeList_IterateSelected (pKM->hWndTree, &ss);
			err = PGPcomdlgSearchServerForKeyIDs (pKM->Context, 
								pKM->hWndParent, ss.pkeyidList,
								ss.icount, &keysetFound);
			HeapFree (GetProcessHeap (), 0, ss.pkeyidList);
		}
	}

	if (err == kPGPError_UserAbort) return FALSE;

	if (IsPGPError (err)) {
		if (bWarn) PGPcomdlgErrorMessage (err);
	}
	else {
		if (PGPRefIsValid (keysetFound)) {
			PGPUInt32 u;
			PGPCountKeys (keysetFound, &u);
			if (u > 0) {
				if (bQueryAdd) {
					PGPkmQueryAddKeys (pKM->Context, pKM->hWndParent,
										keysetFound, pKM->KeySetDisp);
				}
				else {
					PGPAddKeys (keysetFound, pKM->KeySetDisp);
					PGPCommitKeyRingChanges (pKM->KeySetDisp);
				}
			}
			else {
				KMMessageBox (pKM->hWndParent, IDS_CAPTION,
						IDS_SERVERSEARCHFAIL, MB_OK|MB_ICONEXCLAMATION);
			}
			PGPFreeKeySet (keysetFound);
		}
	}

	return TRUE;
}

//	_______________________________________________
//
//  Update selected keys from server

BOOL 
KMGetFromServer (PKEYMAN pKM) 
{
	return KMGetFromServerInternal (pKM, TRUE, TRUE);
}

