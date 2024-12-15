/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	KMMisc.c - miscellaneous routines

	$Id: KMMisc.c,v 1.30 1999/04/14 23:23:45 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

// project header files
#include "pgpkmx.h"

// external global variables
extern HINSTANCE g_hInst;


//	___________________________________________________
//
// Private memory allocation routine

VOID* KMAlloc (LONG size) {
	VOID* p;
	p = malloc (size);
	if (p) {
		memset (p, 0, size);
	}
	return p;
}


//	___________________________________________________
//
// Private memory deallocation routine

VOID KMFree (VOID* p) {
	if (p) {
		free (p);
	}
}


//	___________________________________________________
//
// Private memory deallocation routine

VOID KMFindWindowFromPoint (PKEYMAN pKM, POINT* ppt, HWND* phwnd) 
{
	POINT	pt;

	pt.x = ppt->x;
	pt.y = ppt->y;

	(pKM->lpfnHwndListFunc)(NULL, FALSE, &pt, phwnd);
}


//	___________________________________________________
//
//  Get and truncate the name of a userid.

BOOL KMGetUserIDName (PGPUserIDRef UserID, LPSTR sz, UINT uLen) {
	PGPError	err			= kPGPError_NoErr;
	UINT		u;
	PGPBoolean	bAttrib;
	PGPInt32	iType;

	PGPGetUserIDBoolean (UserID, kPGPUserIDPropIsAttribute, &bAttrib);
	if (bAttrib) {
		PGPGetUserIDNumber (UserID, kPGPUserIDPropAttributeType, &iType);
		switch (iType) {
		case kPGPAttribute_Image :
			LoadString (g_hInst, IDS_PHOTOUSERID, sz, uLen);
			break;

		default :
			LoadString (g_hInst, IDS_UNKNOWNUSERID, sz, uLen);
			break;
		}
	}
	else {
		err = PGPGetUserIDStringBuffer (UserID, kPGPUserIDPropName, 
														uLen, sz, &u);
	}

	switch (err) {
	case kPGPError_BufferTooSmall :
	case kPGPError_NoErr :
		return TRUE;

	default :
		PGPclErrorBox (NULL, err);
		return FALSE;
	}
}


//	___________________________________________________
//
//  Get and truncate the name of a primary userid on a key.

BOOL KMGetKeyName (PGPKeyRef Key, LPSTR sz, UINT uLen) {
	UINT u, uErr;

	uErr = PGPGetPrimaryUserIDNameBuffer (Key, uLen, sz, &u);

	switch (uErr) {
	case kPGPError_BufferTooSmall :
	case kPGPError_NoErr :
		return TRUE;

	default :
		PGPclErrorBox (NULL, uErr);
		return FALSE;
	}
}


//	___________________________________________________
//
//  Get the keyid of a key.

BOOL KMGetKeyIDFromKey (PGPKeyRef Key, LPSTR sz, UINT u) {

	PGPKeyID	KeyID;
	CHAR		szID[kPGPMaxKeyIDStringSize];

	if (u < 11) return FALSE;

	PGPGetKeyIDFromKey (Key, &KeyID);
	PGPGetKeyIDString (&KeyID, kPGPKeyIDString_Abbreviated, szID);
	lstrcpyn (sz, szID, u);

	return TRUE;
}


//	___________________________________________________
//
//  Get the keyid of a signing key.

BOOL KMGetKeyIDFromCert (PGPSigRef Cert, LPSTR sz, UINT u) {

	PGPKeyID	KeyID;
	CHAR		szID[kPGPMaxKeyIDStringSize];

	if (u < 11) return FALSE;

	PGPGetKeyIDOfCertifier (Cert, &KeyID);
	PGPGetKeyIDString (&KeyID, kPGPKeyIDString_Abbreviated, szID);
	lstrcpyn (sz, szID, u);

	return TRUE;
}


//	___________________________________________________
//
//  Get the parent key of a userid

PGPKeyRef KMGetKeyFromUserID (PKEYMAN pKM, PGPUserIDRef UserID) {
	TL_TREEITEM tli;

	KMGetUserIDUserVal (pKM, UserID, (long*)&(tli.hItem));
	if (tli.hItem) {
		tli.mask = TLIF_PARENTHANDLE;
		TreeList_GetItem (pKM->hWndTree, &tli);
		if (tli.hItem) {
			tli.mask = TLIF_PARAM;
			TreeList_GetItem (pKM->hWndTree, &tli);
			return ((PGPKeyRef)(tli.lParam));
		}
	}
	return NULL;
}


//	___________________________________________________
//
//  Get the signing key from a cert

PGPKeyRef KMGetKeyFromCert (PKEYMAN pKM, PGPSigRef Cert) {
	TL_TREEITEM tli;

	KMGetCertUserVal (pKM, Cert, (long*)&(tli.hItem));
	if (tli.hItem) {
		tli.mask = TLIF_PARENTHANDLE;
		TreeList_GetItem (pKM->hWndTree, &tli);
		if (tli.hItem) {
			tli.mask = TLIF_PARENTHANDLE;
			TreeList_GetItem (pKM->hWndTree, &tli);
			if (tli.hItem) {
				tli.mask = TLIF_PARAM;
				TreeList_GetItem (pKM->hWndTree, &tli);
				return ((PGPKeyRef)(tli.lParam));
			}
		}
	}
	return NULL;
}


//	___________________________________________________
//
//  Look for secret keys

BOOL KMCheckForSecretKeys (PGPKeySetRef KeySet) {
	PGPKeyListRef KeyList;
	PGPKeyIterRef KeyIter;
	PGPKeyRef Key;
	BOOL bSecretKeys;
	Boolean bSecret;

	PGPOrderKeySet (KeySet, kPGPAnyOrdering, &KeyList);
	PGPNewKeyIter (KeyList, &KeyIter);

	bSecretKeys = FALSE;
	PGPKeyIterNext (KeyIter, &Key);

	while (Key && !bSecretKeys) {
		PGPGetKeyBoolean (Key, kPGPKeyPropIsSecret, &bSecret);
		if (bSecret) {
			bSecretKeys = TRUE;
		}
		PGPKeyIterNext (KeyIter, &Key);
	}

	PGPFreeKeyIter (KeyIter);
	PGPFreeKeyList (KeyList);

	return bSecretKeys;
}


//	___________________________________________________
//
//  Is this the only userID on the key ?

BOOL KMIsThisTheOnlyUserID (PKEYMAN pKM, PGPUserIDRef UID) {
	INT iCount				= 0;
	PGPKeyListRef KeyList;
	PGPKeyIterRef KeyIter;
	PGPKeyRef Key;
	PGPUserIDRef UserID;
	PGPBoolean bAttrib;

	Key = KMGetKeyFromUserID (pKM, UID);
	if (PGPKeyRefIsValid (Key)) {
		PGPOrderKeySet (pKM->KeySetDisp, kPGPAnyOrdering, &KeyList);
		PGPNewKeyIter (KeyList, &KeyIter);
		PGPKeyIterSeek (KeyIter, Key);

		PGPKeyIterNextUserID (KeyIter, &UserID);

		while (UserID) {
			PGPGetUserIDBoolean (UserID, kPGPUserIDPropIsAttribute, &bAttrib);
			if (!bAttrib) iCount++;
			PGPKeyIterNextUserID (KeyIter, &UserID);
		}

		PGPFreeKeyIter (KeyIter);
		PGPFreeKeyList (KeyList);
	}

	return (iCount == 1);
}


//	___________________________________________________
//
//  Is this the primary userID on the key ?

BOOL KMIsThisThePrimaryUserID (PKEYMAN pKM, PGPUserIDRef UID) {
	PGPUserIDRef PrimaryUserID	= kInvalidPGPUserIDRef;
	PGPKeyRef Key;

	Key = KMGetKeyFromUserID (pKM, UID);
	if (PGPKeyRefIsValid (Key))
		PGPGetPrimaryUserID (Key, &PrimaryUserID);

	if (UID == PrimaryUserID) return TRUE;
	else return FALSE;
}


//	___________________________________________________
//
//  Are there existing photoids on the key ?

BOOL KMExistingPhotoID (PKEYMAN pKM, PGPKeyRef key) {

	BOOL			bExistingPhotoID		= FALSE;

	PGPKeyListRef	keylist;
	PGPKeyIterRef	keyiter;
	PGPUserIDRef	userid;
	PGPBoolean		bAttrib;
	PGPInt32		iType;

	PGPOrderKeySet (pKM->KeySetDisp, kPGPAnyOrdering, &keylist);
	PGPNewKeyIter (keylist, &keyiter);
	PGPKeyIterSeek (keyiter, key);

	PGPKeyIterNextUserID (keyiter, &userid);

	while (userid) {

		PGPGetUserIDBoolean (userid, kPGPUserIDPropIsAttribute, &bAttrib);
		if (bAttrib) {
			PGPGetUserIDNumber (userid, kPGPUserIDPropAttributeType, &iType);
			if (iType == kPGPAttribute_Image) {
				bExistingPhotoID = TRUE;
				break;
			}
		}

		PGPKeyIterNextUserID (keyiter, &userid);
	}

	PGPFreeKeyIter (keyiter);
	PGPFreeKeyList (keylist);

	return bExistingPhotoID;
}


//	___________________________________________________
//
//  Put up preferences property sheet

INT KMCommitKeyRingChanges (PKEYMAN pKM) {
	INT iError = 0;
	HCURSOR hCursorOld;
	UINT uReloadMessage;

	if (pKM->ulOptionFlags & KMF_ENABLECOMMITS) {
		hCursorOld = SetCursor (LoadCursor (NULL, IDC_WAIT));
		iError = PGPCommitKeyRingChanges (pKM->KeySetMain);
		SetCursor (hCursorOld);
		if (IsntPGPError (PGPclErrorBox (NULL, iError))) {
			KMUpdateKeyProperties (pKM);
		}
	}
	if (pKM->ulOptionFlags & KMF_ENABLERELOADS) {
		if (!iError) {
			uReloadMessage = RegisterWindowMessage (RELOADKEYRINGMSG);
			PostMessage (HWND_BROADCAST, uReloadMessage, 
				MAKEWPARAM (LOWORD (pKM->hWndParent), FALSE), 
				GetCurrentProcessId ());
		}
	}
	return iError;
}


//	___________________________________________________
//
//  Determine the appropriate icon for a userid, based on
//	key properties

INT KMDetermineUserIDIcon (
		PGPKeyRef		Key,
		PGPUserIDRef	UserID,
		BOOL*			pbItalics) 
{
	PGPInt32	iAlg;
	PGPInt32	iType;
	PGPBoolean	bAttrib;

	if (pbItalics) *pbItalics = FALSE;

	if (UserID) {
		PGPGetUserIDBoolean (UserID, kPGPUserIDPropIsAttribute, &bAttrib);
		if (bAttrib) {
			PGPGetUserIDNumber (UserID, kPGPUserIDPropAttributeType, &iType);
			switch (iType) {
			case kPGPAttribute_Image :
				return IDX_PHOTOUSERID;

			default :
				if (pbItalics) 
					*pbItalics = TRUE;
				return IDX_INVALIDUSERID;
			}
		}
	}

	PGPGetKeyNumber (Key, kPGPKeyPropAlgID, &iAlg);
	if (iAlg == kPGPPublicKeyAlgorithm_RSA) return (IDX_RSAUSERID);
	else return (IDX_DSAUSERID);
}


//	___________________________________________________
//
//  Determine the appropriate icon for a cert, based on
//	its properties

INT 
KMDetermineCertIcon (
		PGPSigRef	cert, 
		BOOL*		pbItalics,
		BOOL*		pbX509) 
{
	PGPBoolean	bRevoked, bVerified, bTried, bExpired;
	PGPBoolean	bNotCorrupt, bExportable, bX509;
	PGPUInt32	uTrustLevel;
	INT			idx;

	PGPGetSigBoolean (cert, kPGPSigPropIsRevoked, &bRevoked);
	PGPGetSigBoolean (cert, kPGPSigPropIsExpired, &bExpired);
	PGPGetSigBoolean (cert, kPGPSigPropIsVerified, &bVerified);
	PGPGetSigBoolean (cert, kPGPSigPropIsTried, &bTried);
	PGPGetSigBoolean (cert, kPGPSigPropIsNotCorrupt, &bNotCorrupt);
	PGPGetSigBoolean (cert, kPGPSigPropIsExportable, &bExportable);
	PGPGetSigBoolean (cert, kPGPSigPropIsX509, &bX509);
	PGPGetSigNumber  (cert, kPGPSigPropTrustLevel, &uTrustLevel);

	if (pbX509)
		*pbX509 = bX509;

	if (bX509) {
		if (bRevoked)
			idx = IDX_X509REVCERT;
		else if (bExpired)
			idx = IDX_X509EXPCERT;
		else
			idx = IDX_X509CERT;
	}
	else if (bRevoked) 
		idx = IDX_REVCERT;
	else if (bExpired) 
		idx = IDX_EXPCERT;
	else if (bVerified) {
		if (bExportable) {
			if (uTrustLevel == 1) 
				idx = IDX_TRUSTEDCERT;
			else 
				idx = IDX_EXPORTCERT;
		}
		else {
			if (uTrustLevel == 2) 
				idx = IDX_METACERT;
			else 
				idx = IDX_CERT;
		}
	}
	else if (bTried) 
		idx = IDX_BADCERT;
	else if (bNotCorrupt) {
		if (bExportable) 
			idx = IDX_EXPORTCERT;
		else 
			idx = IDX_CERT;
	}
	else 
		idx = IDX_BADCERT;

	if ((idx == IDX_BADCERT) || (idx == IDX_REVCERT) || (idx == IDX_EXPCERT)) 
	{
		if (pbItalics)
			*pbItalics = TRUE;
	}

	return idx;
}


//	___________________________________________________
//
//  Determine the appropriate icon for a key, based on
//	its properties

INT KMDetermineKeyIcon (
		PKEYMAN		pKM, 
		PGPKeyRef	Key, 
		BOOL*		pbItalics) 
{

	PGPBoolean bRevoked, bSecret, bDisabled, bExpired, bSplit;
	PGPBoolean bCanSign, bCanEncrypt, bCantDoAnything;
	PGPUInt32 iIdx, iAlg;

	PGPGetKeyBoolean (Key, kPGPKeyPropIsRevoked, &bRevoked);
	PGPGetKeyBoolean (Key, kPGPKeyPropIsSecret, &bSecret);
	PGPGetKeyBoolean (Key, kPGPKeyPropIsDisabled, &bDisabled);
	PGPGetKeyBoolean (Key, kPGPKeyPropIsExpired, &bExpired);
	PGPGetKeyBoolean (Key, kPGPKeyPropIsSecretShared, &bSplit);
	PGPGetKeyBoolean (Key, kPGPKeyPropCanSign, &bCanSign);
	PGPGetKeyBoolean (Key, kPGPKeyPropCanEncrypt, &bCanEncrypt);
	PGPGetKeyNumber (Key, kPGPKeyPropAlgID, &iAlg);

	bCantDoAnything = FALSE;

	// RSA
	if (iAlg == kPGPPublicKeyAlgorithm_RSA) 
	{
		if (!bCanSign && !bCanEncrypt) bCantDoAnything = TRUE;

		if (bSecret) {
			if (bRevoked) iIdx = IDX_RSASECREVKEY;
			else if (bExpired) iIdx = IDX_RSASECEXPKEY;
			else if (bDisabled) iIdx = IDX_RSASECDISKEY;
			else if (bSplit) iIdx = IDX_RSASECSHRKEY;
			else iIdx = IDX_RSASECKEY;
		}
		else {
			if (bRevoked) iIdx = IDX_RSAPUBREVKEY;
			else if (bExpired) iIdx = IDX_RSAPUBEXPKEY;
			else if (bDisabled) iIdx = IDX_RSAPUBDISKEY;
			else iIdx = IDX_RSAPUBKEY;
		}
	}
	// DSA/ElGamal
	else if (iAlg == kPGPPublicKeyAlgorithm_DSA) 
	{
		if (bSecret) {
			if (bRevoked) iIdx = IDX_DSASECREVKEY;
			else if (bExpired) iIdx = IDX_DSASECEXPKEY;
			else if (bDisabled) iIdx = IDX_DSASECDISKEY;
			else if (bSplit) iIdx = IDX_DSASECSHRKEY;
			else iIdx = IDX_DSASECKEY;
		}
		else {
			if (bRevoked) iIdx = IDX_DSAPUBREVKEY;
			else if (bExpired) iIdx = IDX_DSAPUBEXPKEY;
			else if (bDisabled) iIdx = IDX_DSAPUBDISKEY;
			else iIdx = IDX_DSAPUBKEY;
		}
	}
	// unknown 
	else
	{
		iIdx = IDX_UNKNOWNKEY;
		bCantDoAnything = TRUE;
	}

	if (pbItalics) 
		*pbItalics = bRevoked || bExpired || bDisabled || bCantDoAnything;
	return iIdx;
}
