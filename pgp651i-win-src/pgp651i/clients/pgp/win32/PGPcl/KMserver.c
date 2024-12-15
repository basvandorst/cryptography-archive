/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	KMserver.c - handle communication with keyserver
	

	$Id: KMserver.c,v 1.26 1999/03/29 05:00:22 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

// project header files
#include "pgpkmx.h"

// external globals
extern HINSTANCE g_hInst;

// typedefs
typedef struct {
	FARPROC			lpfnCallback;
	PKEYMAN			pKM;
	PGPContextRef	context;
	PGPKeySetRef	keyset;
	INT				icount;
	PGPKeyID*		pkeyidList;
} SERVERSTRUCT, *PSERVERSTRUCT;


static BOOL CALLBACK 
sAddKeyIDToList (
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
KMSendToServer (PKEYMAN pKM, UINT uServerFlags) 
{
	PGPKeySetRef	keysetSend		= kInvalidPGPKeySetRef;
	PGPError		err				= kPGPError_BadParams;
	INT				iKeyCount;

	if ((uServerFlags == PGPCL_SPECIFIEDSERVER) && 
		(pKM->keyserver.structSize == 0))
	{
		uServerFlags = PGPCL_USERIDBASEDSERVER;
	}

	KMGetSelectedKeys (pKM, &keysetSend, &iKeyCount);
	if (PGPRefIsValid (keysetSend)) {
		err = PGPclSendKeysToServer (pKM->Context, pKM->tlsContext, 
					pKM->hWndParent, uServerFlags, &pKM->keyserver, 
					pKM->KeySetMain, keysetSend);
		PGPFreeKeySet (keysetSend);
	}

	if (err == kPGPError_ServerKeyAlreadyExists) {
		if (iKeyCount > 1) {
			KMMessageBox (pKM->hWndParent, IDS_CAPTION, 
						IDS_KEYSALREADYONSERVER, MB_OK|MB_ICONINFORMATION);
		}
		else {
			KMMessageBox (pKM->hWndParent, IDS_CAPTION, 
						IDS_KEYALREADYONSERVER, MB_OK|MB_ICONINFORMATION);
		}
	}
	else if (IsPGPError (err)) {
		PGPclErrorBox (NULL, err);
	}
	else {
		KMMessageBox (pKM->hWndParent, IDS_PGP, IDS_UPLOADOK,
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
	PGPKeySetRef			keysetDisable	= kInvalidPGPKeySetRef;
	PGPError				err				= kPGPError_BadParams;
	PGPKeyServerKeySpace	space;

	if (pKM->ulOptionFlags & KMF_PENDINGBUCKET)
		space = kPGPKeyServerKeySpace_Normal;
	else
		space = kPGPKeyServerKeySpace_Default;

	KMGetSelectedKeys (pKM, &keysetDisable, NULL);
	if (PGPRefIsValid (keysetDisable)) {
		err = PGPclDisableKeysOnServer (pKM->Context, pKM->tlsContext,
					pKM->hWndParent, &pKM->keyserver, 
					space, pKM->KeySetMain, keysetDisable);
		PGPFreeKeySet (keysetDisable);
	}

	if (IsPGPError (err)) PGPclErrorBox (NULL, err);
	else {
		KMMessageBox (pKM->hWndParent, IDS_PGP, IDS_DISABLEONSERVEROK,
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
	PGPKeySetRef			keysetDelete	= kInvalidPGPKeySetRef;
	PGPError				err				= kPGPError_BadParams;
	PGPKeyServerKeySpace	space;

	if (pKM->ulOptionFlags & KMF_PENDINGBUCKET)
		space = kPGPKeyServerKeySpace_Pending;
	else
		space = kPGPKeyServerKeySpace_Default;

	KMGetSelectedKeys (pKM, &keysetDelete, NULL);
	if (PGPKeySetRefIsValid (keysetDelete)) {
		err = PGPclDeleteKeysFromServer (pKM->Context, pKM->tlsContext,
					pKM->hWndParent, &pKM->keyserver, 
					space, pKM->KeySetMain, keysetDelete);
		PGPFreeKeySet (keysetDelete);
	}

	if (IsPGPError (err)) PGPclErrorBox (NULL, err);
	else {
		KMMessageBox (pKM->hWndParent, IDS_PGP, IDS_DELETEFROMSERVEROK,
								MB_OK|MB_ICONINFORMATION);
	}

	return TRUE;
}

//	_______________________________________________
//
//  Update selected keys from server

BOOL 
KMGetFromServerInternal (
		PKEYMAN		pKM, 
		BOOL		bQueryAdd, 
		BOOL		bWarn,
		BOOL		bGetSigners) 
{
	SERVERSTRUCT	ss;
	PGPKeySetRef	keysetGet		= kInvalidPGPKeySetRef;
	PGPKeySetRef	keysetFound		= kInvalidPGPKeySetRef;
	PGPError		err				= kPGPError_NoErr;

	ss.context = pKM->Context;
	ss.pKM = pKM;
	ss.icount = 0;

	// get selected keys
	if ((KMFocusedObjectType (pKM) == OBJECT_KEY) ||
		(KMFocusedObjectType (pKM) == OBJECT_USERID) ||
		(!bGetSigners)) 
	{
		KMGetSelectedKeys (pKM, &keysetGet, NULL);
		if (PGPRefIsValid (keysetGet)) {
			err = PGPclUpdateKeySetFromServer (pKM->Context, 
										pKM->tlsContext,
										pKM->hWndParent, 
										keysetGet,
										PGPCL_USERIDBASEDSERVER,
										pKM->KeySetMain,
										&keysetFound);
			PGPFreeKeySet (keysetGet);
		}
	}

	// get signing keys
	else {
		KMGetSelectedKeys (pKM, NULL, &ss.icount);
		if (ss.icount) {
			ss.pkeyidList = 
				(PGPKeyID*)malloc ((ss.icount) * sizeof(PGPKeyID));
			ss.icount = 0;
			ss.lpfnCallback = sAddKeyIDToList;
			TreeList_IterateSelected (pKM->hWndTree, &ss);
			err = PGPclSearchServerForKeyIDs (pKM->Context,
								pKM->tlsContext,
								pKM->hWndParent, 
								ss.pkeyidList,
								ss.icount, 
								PGPCL_DEFAULTSERVER,
								pKM->KeySetMain,
								&keysetFound);
			free (ss.pkeyidList);
		}
	}

	if (err == kPGPError_UserAbort) return FALSE;

	if (IsPGPError (err)) {
		if (bWarn) PGPclErrorBox (NULL, err);
	}
	else {
		if (PGPRefIsValid (keysetFound)) {
			PGPUInt32 u;
			PGPCountKeys (keysetFound, &u);
			if (u > 0) {
				if (bQueryAdd) {
					PGPclQueryAddKeys (pKM->Context, pKM->tlsContext,
							pKM->hWndParent, keysetFound, pKM->KeySetDisp);
				}
				else {
					if (pKM->ulOptionFlags & KMF_ENABLECOMMITS) {
						PGPAddKeys (keysetFound, pKM->KeySetDisp);
						PGPCommitKeyRingChanges (pKM->KeySetDisp);
					}
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
	return KMGetFromServerInternal (pKM, TRUE, TRUE, TRUE);
}


//	_______________________________________________
//
//  Request CA for certificate for key

BOOL 
KMAddCertificate (PKEYMAN pKM) 
{
	PGPKeySetRef	keysetKey		= kInvalidPGPKeySetRef;
	PGPKeyRef		key				= kInvalidPGPKeyRef;
	PGPUserIDRef	userid			= kInvalidPGPUserIDRef;
	PGPError		err;

	if (KMSelectedFlags (pKM) == OBJECT_USERID)
	{
		userid = (PGPUserIDRef)KMFocusedObject (pKM);
		key = KMGetKeyFromUserID (pKM, userid);
	}
	else
	{
		key = (PGPKeyRef)KMFocusedObject (pKM);
		PGPGetPrimaryUserID (key, &userid);
	}

	PGPNewSingletonKeySet (key, &keysetKey);
	if (PGPRefIsValid (keysetKey)) 
	{
		err = PGPclSendCertificateRequestToServer (
									pKM->Context, 
									pKM->tlsContext,
									pKM->hWndParent, 
									pKM->KeySetMain,
									userid,
									keysetKey);
		PGPFreeKeySet (keysetKey);
	}

	if (err == kPGPError_InvalidProperty)
	{
		if (KMIsThisTheOnlyUserID (pKM, userid))
			KMMessageBox (pKM->hWndParent, IDS_CAPTION, 
					IDS_CANTDERIVEATTRIBUTESONLY, MB_OK|MB_ICONEXCLAMATION);
		else
			KMMessageBox (pKM->hWndParent, IDS_CAPTION, 
					IDS_CANTDERIVEATTRIBUTES, MB_OK|MB_ICONEXCLAMATION);
	}
	else if (IsPGPError (err))
	{
		PGPclErrorBox (pKM->hWndParent, err);
	}
	else
	{
		KMMessageBox (pKM->hWndParent, IDS_PGP, 
				IDS_CERTIFICATEREQUESTED, MB_OK|MB_ICONINFORMATION);
	}

	return TRUE;
}


//	_______________________________________________
//
//  Retrieve a previously-requested certificate

BOOL 
KMRetrieveCertificate (PKEYMAN pKM) 
{
	PGPKeySetRef	keysetKey		= kInvalidPGPKeySetRef;
	PGPKeySetRef	keysetReturned	= kInvalidPGPKeySetRef;
	PGPUserIDRef	userid			= kInvalidPGPUserIDRef;
	PGPKeyRef		key				= kInvalidPGPKeyRef;
	PGPError		err;

	if (KMSelectedFlags (pKM) == OBJECT_USERID)
	{
		userid = (PGPUserIDRef)KMFocusedObject (pKM);
		key = KMGetKeyFromUserID (pKM, userid);
	}
	else
	{
		key = (PGPKeyRef)KMFocusedObject (pKM);
		PGPGetPrimaryUserID (key, &userid);
	}

	PGPNewSingletonKeySet (key, &keysetKey);
	if (PGPRefIsValid (keysetKey)) 
	{
		err = PGPclRetrieveCertificateFromServer (
									pKM->Context, 
									pKM->tlsContext,
									pKM->hWndParent, 
									pKM->KeySetMain,
									keysetKey,
									userid,
									&keysetReturned);
		PGPFreeKeySet (keysetKey);
	}
	if (err == kPGPError_UserAbort) 
		return FALSE;

	if (IsPGPError (err)) 
	{
		PGPclErrorBox (pKM->hWndParent, err);
	}
	else 
	{
		if (PGPRefIsValid (keysetReturned)) 
		{
			PGPUInt32 u;

			PGPCountKeys (keysetReturned, &u);
			if (u > 0) 
			{
#if 0	// <- set to "1" to enable selective import dialog
				PGPclQueryAddKeys (pKM->Context, pKM->tlsContext,
							pKM->hWndParent, keysetReturned, pKM->KeySetDisp);
#else
				if (pKM->ulOptionFlags & KMF_ENABLECOMMITS) 
				{
					PGPAddKeys (keysetReturned, pKM->KeySetDisp);
					KMCommitKeyRingChanges (pKM);
					KMLoadKeyRingIntoTree (pKM, FALSE, TRUE, FALSE);
					InvalidateRect (pKM->hWndTree, NULL, TRUE);

					KMMessageBox (pKM->hWndParent, IDS_PGP,
							IDS_CERTRETRIEVALSUCCESS, 
							MB_OK|MB_ICONINFORMATION);
				}
#endif
			}
			else 
			{
				KMMessageBox (pKM->hWndParent, IDS_CAPTION,
						IDS_CERTRETRIEVALFAIL, MB_OK|MB_ICONEXCLAMATION);
			}
			PGPFreeKeySet (keysetReturned);
		}
	}
	return TRUE;
}
