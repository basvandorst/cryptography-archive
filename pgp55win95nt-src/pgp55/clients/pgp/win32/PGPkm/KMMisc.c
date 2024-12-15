//: KMMisc.c - miscellaneous routines
//
//	$Id: KMMisc.c,v 1.20.4.1 1997/11/18 18:27:00 pbj Exp $
//

#include "pgpkmx.h"
#include "resource.h"

// external global variables
extern HINSTANCE g_hInst;


//----------------------------------------------------|
// Private memory allocation routine

VOID* KMAlloc (LONG size) {
	return HeapAlloc (GetProcessHeap (), HEAP_ZERO_MEMORY, size);
}


//----------------------------------------------------|
// Private memory deallocation routine

VOID KMFree (VOID* p) {
	if (p) {
		FillMemory (p, lstrlen(p), '\0');
		HeapFree (GetProcessHeap (), 0, p);
	}
}


//----------------------------------------------------|
//  Get and truncate the name of a userid.

BOOL KMGetUserIDName (PGPUserIDRef UserID, LPSTR sz, UINT uLen) {
	UINT u, uErr;

	uErr = PGPGetUserIDStringBuffer (UserID, kPGPUserIDPropName, 
														uLen, sz, &u);

	switch (uErr) {
	case kPGPError_BufferTooSmall :
	case kPGPError_NoErr :
		return TRUE;

	default :
		PGPcomdlgErrorMessage (uErr);
		return FALSE;
	}
}


//----------------------------------------------------|
//  Get and truncate the name of a primary userid on a key.

BOOL KMGetKeyName (PGPKeyRef Key, LPSTR sz, UINT uLen) {
	UINT u, uErr;

	uErr = PGPGetPrimaryUserIDNameBuffer (Key, uLen, sz, &u);

	switch (uErr) {
	case kPGPError_BufferTooSmall :
	case kPGPError_NoErr :
		return TRUE;

	default :
		PGPcomdlgErrorMessage (uErr);
		return FALSE;
	}
}


//----------------------------------------------------|
//  Get the keyid of a key.

BOOL KMGetKeyIDFromKey (PGPKeyRef Key, LPSTR sz, UINT u) {

	PGPKeyID	KeyID;
	CHAR		szID[kPGPMaxKeyIDStringSize];

	if (u < 11) return FALSE;

	PGPGetKeyIDFromKey (Key, &KeyID);
	PGPGetKeyIDString (&KeyID, kPGPKeyIDString_Abbreviated, szID);
	lstrcpy (sz, "0x");
	lstrcat (sz, &szID[2]);

	return TRUE;
}


//----------------------------------------------------|
//  Get the keyid of a signing key.

BOOL KMGetKeyIDFromCert (PGPSigRef Cert, LPSTR sz, UINT u) {

	PGPKeyID	KeyID;
	CHAR		szID[kPGPMaxKeyIDStringSize];

	if (u < 11) return FALSE;

	PGPGetKeyIDOfCertifier (Cert, &KeyID);
	PGPGetKeyIDString (&KeyID, kPGPKeyIDString_Abbreviated, szID);
	lstrcpy (sz, "0x");
	lstrcat (sz, &szID[2]);

	return TRUE;
}


//----------------------------------------------------|
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


//----------------------------------------------------|
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


//----------------------------------------------------|
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


//----------------------------------------------------|
//  Is this the only userID on the key ?

BOOL KMIsThisTheOnlyUserID (PKEYMAN pKM, PGPUserIDRef UID) {
	PGPKeyListRef KeyList;
	PGPKeyIterRef KeyIter;
	PGPKeyRef Key;
	PGPUserIDRef UserID;
	INT iCount;

	Key = KMGetKeyFromUserID (pKM, UID);
	PGPOrderKeySet (pKM->KeySetDisp, kPGPAnyOrdering, &KeyList);
	PGPNewKeyIter (KeyList, &KeyIter);
	PGPKeyIterSeek (KeyIter, Key);

	PGPKeyIterNextUserID (KeyIter, &UserID);

	iCount = 0;
	while (UserID) {
		iCount++;
		PGPKeyIterNextUserID (KeyIter, &UserID);
	}

	PGPFreeKeyIter (KeyIter);
	PGPFreeKeyList (KeyList);

	return (iCount == 1);
}


//----------------------------------------------------|
//  Is this the primary userID on the key ?

BOOL KMIsThisThePrimaryUserID (PKEYMAN pKM, PGPUserIDRef UID) {
	PGPKeyRef Key;
	PGPUserIDRef PrimaryUserID;

	Key = KMGetKeyFromUserID (pKM, UID);
	PGPGetPrimaryUserID (Key, &PrimaryUserID);

	if (UID == PrimaryUserID) return TRUE;
	else return FALSE;
}


//----------------------------------------------------|
//  Put up preferences property sheet

BOOL KMPGPPreferences (PKEYMAN pKM, INT iPage) {
	UINT uRetval;

	EnableWindow (pKM->hWndParent, FALSE);
	uRetval = PGPcomdlgPreferences (pKM->Context, pKM->hWndParent, iPage);
	if (uRetval != kPGPError_Win32_AlreadyOpen) 
		SetForegroundWindow (pKM->hWndParent);
	EnableWindow (pKM->hWndParent, TRUE);

	if (uRetval == kPGPError_NoErr) return TRUE;
	else return FALSE;
}


//----------------------------------------------------|
//  Put up preferences property sheet

INT KMCommitKeyRingChanges (PKEYMAN pKM) {
	INT iError = 0;
	HCURSOR hCursorOld;
	UINT uReloadMessage;

	if (pKM->ulOptionFlags & KMF_ENABLECOMMITS) {
		hCursorOld = SetCursor (LoadCursor (NULL, IDC_WAIT));
		iError = PGPCommitKeyRingChanges (pKM->KeySetMain);
		SetCursor (hCursorOld);
		if (!PGPcomdlgErrorMessage (iError)) {
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


//----------------------------------------------------|
//  Determine the appropriate icon for a userid, based on
//	key properties

INT KMDetermineUserIDIcon (PGPKeyRef Key) {
	INT iAlg;
	PGPGetKeyNumber (Key, kPGPKeyPropAlgID, &iAlg);

	if (iAlg == kPGPPublicKeyAlgorithm_RSA) return (IDX_RSAUSERID);
	else return (IDX_DSAUSERID);
}


//----------------------------------------------------|
//  Determine the appropriate icon for a cert, based on
//	its properties

INT 
KMDetermineCertIcon (
		PGPSigRef cert, 
		BOOL* pbItalics) 
{
	PGPBoolean	bRevoked, bVerified, bTried;
	PGPBoolean	bNotCorrupt, bExportable;
	PGPUInt32	uTrustLevel;
	INT			idx;

	PGPGetSigBoolean (cert, kPGPSigPropIsRevoked, &bRevoked);
	PGPGetSigBoolean (cert, kPGPSigPropIsVerified, &bVerified);
	PGPGetSigBoolean (cert, kPGPSigPropIsTried, &bTried);
	PGPGetSigBoolean (cert, kPGPSigPropIsNotCorrupt, &bNotCorrupt);
	PGPGetSigBoolean (cert, kPGPSigPropIsExportable, &bExportable);
	PGPGetSigNumber  (cert, kPGPSigPropTrustLevel, &uTrustLevel);

	if (bRevoked) idx = IDX_REVCERT;
	else if (bVerified) {
		if (bExportable) {
			if (uTrustLevel == 1) idx = IDX_EXPORTMETACERT;
			else idx = IDX_EXPORTCERT;
		}
		else {
			if (uTrustLevel == 2) idx = IDX_METACERT;
			else idx = IDX_CERT;
		}
	}
	else if (bTried) idx = IDX_BADCERT;
	else if (bNotCorrupt) idx = IDX_EXPORTCERT;
	else idx = IDX_BADCERT;

	if ((idx == IDX_BADCERT) || (idx == IDX_REVCERT)) 
		*pbItalics = TRUE;

	return idx;
}


//----------------------------------------------------|
//  Determine the appropriate icon for a key, based on
//	its properties

INT KMDetermineKeyIcon (PKEYMAN pKM, PGPKeyRef Key, BOOL* lpbItalics) {

	PGPBoolean bRevoked, bSecret, bDisabled, bExpired, bNoRSA;
	PGPUInt32 iIdx, iAlg;

	bNoRSA = FALSE;
	PGPGetKeyBoolean (Key, kPGPKeyPropIsRevoked, &bRevoked);
	PGPGetKeyBoolean (Key, kPGPKeyPropIsSecret, &bSecret);
	PGPGetKeyBoolean (Key, kPGPKeyPropIsDisabled, &bDisabled);
	PGPGetKeyBoolean (Key, kPGPKeyPropIsExpired, &bExpired);
	PGPGetKeyNumber (Key, kPGPKeyPropAlgID, &iAlg);

	if (iAlg == kPGPPublicKeyAlgorithm_RSA) {
		if (pKM->ulOptionFlags & KMF_DISABLERSAOPS) bNoRSA = TRUE;
		if (bSecret) {
			if (bRevoked) iIdx = IDX_RSASECREVKEY;
			else if (bExpired) iIdx = IDX_RSASECEXPKEY;
			else if (bDisabled) iIdx = IDX_RSASECDISKEY;
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
	else {
		if (bSecret) {
			if (bRevoked) iIdx = IDX_DSASECREVKEY;
			else if (bExpired) iIdx = IDX_DSASECEXPKEY;
			else if (bDisabled) iIdx = IDX_DSASECDISKEY;
			else iIdx = IDX_DSASECKEY;
		}
		else {
			if (bRevoked) iIdx = IDX_DSAPUBREVKEY;
			else if (bExpired) iIdx = IDX_DSAPUBEXPKEY;
			else if (bDisabled) iIdx = IDX_DSAPUBDISKEY;
			else iIdx = IDX_DSAPUBKEY;
		}
	}

	if (lpbItalics) 
		*lpbItalics = bRevoked || bExpired || bDisabled || bNoRSA;
	return iIdx;
}
