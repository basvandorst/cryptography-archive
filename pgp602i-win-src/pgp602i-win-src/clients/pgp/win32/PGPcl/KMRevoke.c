/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	KMKeyOps.c - implements various operations performed on keys. 
	

	$Id: KMRevoke.c,v 1.22 1998/08/12 18:31:23 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

// project header files
#include "pgpkmx.h"

// pgp header files
#include "pgpclientprefs.h"

// constant definitions
#define BITMAP_WIDTH	16
#define BITMAP_HEIGHT	16

#define INITIAL_SIGN_COLUMNWIDTH	210

#define SIG_NONEXPORTABLE	0
#define SIG_EXPORTABLE		1
#define SIG_TRUST			2
#define SIG_META			3

// external globals  
extern HINSTANCE g_hInst;
extern CHAR g_szHelpFile[MAX_PATH];

// typedefs
typedef struct {
	PKEYMAN			pKM;
	PGPKeyRef		key;
	PGPBoolean		bSyncWithServer;
} REVOKECERTSTRUCT, *PREVOKECERTSTRUCT;


//	___________________________________________________
//
//  revoke split key and all subkeys

static PGPError 
sRevokeKeySplit (
		PGPContextRef	context,
		PGPKeySetRef	keyset,
		PGPKeyRef		key,
		PGPByte*		passkey,
		PGPSize			sizePasskey)
{
	UINT			u;
	PGPKeyListRef	keylist;
	PGPKeyIterRef	keyiter;
	PGPSubKeyRef	subkey;
	PGPError		err;

	err = PGPRevokeKey (key, 
				PGPOPasskeyBuffer (context, passkey, sizePasskey),
				PGPOLastOption (context));
	if (IsPGPError (err)) return err;

	PGPGetKeyNumber (key, kPGPKeyPropAlgID, &u);
	switch (u) {
	case kPGPPublicKeyAlgorithm_RSA :
		break;

	case kPGPPublicKeyAlgorithm_DSA :
		PGPOrderKeySet (keyset, kPGPAnyOrdering, &keylist);
		PGPNewKeyIter (keylist, &keyiter);
		PGPKeyIterSeek (keyiter, key);
		PGPKeyIterNextSubKey (keyiter, &subkey);
		while (subkey) {
			err = PGPRevokeSubKey (subkey, 
					PGPOPasskeyBuffer (context, passkey, sizePasskey),
					PGPOLastOption (context));
			PGPKeyIterNextSubKey (keyiter, &subkey);
		}
		PGPFreeKeyIter (keyiter);
		PGPFreeKeyList (keylist);
		break;

	default :
		break;
	}

	return err;
}

//	___________________________________________________
//
//  revoke normal key and all subkeys

static PGPError 
sRevokeKeyNormal (
		PGPContextRef	context,
		PGPKeySetRef	keyset,
		PGPKeyRef		key, 
		LPSTR			pszPhrase)
{
	UINT			u;
	PGPKeyListRef	keylist;
	PGPKeyIterRef	keyiter;
	PGPSubKeyRef	subkey;
	PGPError		err;

	err = PGPRevokeKey (key, 
				PGPOPassphrase (context, pszPhrase),
				PGPOLastOption (context));
	if (IsPGPError (err)) return err;

	PGPGetKeyNumber (key, kPGPKeyPropAlgID, &u);
	switch (u) {
	case kPGPPublicKeyAlgorithm_RSA :
		break;

	case kPGPPublicKeyAlgorithm_DSA :
		PGPOrderKeySet (keyset, kPGPAnyOrdering, &keylist);
		PGPNewKeyIter (keylist, &keyiter);
		PGPKeyIterSeek (keyiter, key);
		PGPKeyIterNextSubKey (keyiter, &subkey);
		while (subkey) {
			err = PGPRevokeSubKey (subkey, 
					PGPOPassphrase (context, pszPhrase),
					PGPOLastOption (context));
			PGPKeyIterNextSubKey (keyiter, &subkey);
		}
		PGPFreeKeyIter (keyiter);
		PGPFreeKeyList (keylist);
		break;

	default :
		break;
	}

	return err;
}

//	___________________________________________________
//
//  Revoke selected key

BOOL 
KMRevokeKey (PKEYMAN pKM) 
{
	BOOL			bRetVal				= TRUE;
	PGPError		err					= kPGPError_NoErr;
	LPSTR			pszPhrase			= NULL;
	PGPByte*		pPasskey			= NULL;
	PGPKeySetRef	keysetRevokers		= kInvalidPGPKeySetRef;
	PGPKeyRef		keyRevoker			= kInvalidPGPKeyRef;

	PGPBoolean		bSecret;
	PGPBoolean		bSplit;
	PGPBoolean		bSyncWithServer;
	PGPSize			sizePasskey;
	PGPKeyRef		key;
	PGPKeyRef		keyToRevoke;
	PGPKeyRef		keyDef;
	CHAR			sz128[128];
	PGPPrefRef		prefref;
	PGPUInt32		u, uNumRevokers;

	keyToRevoke = (PGPKeyRef) KMFocusedObject (pKM);
	PGPGetDefaultPrivateKey (pKM->KeySetMain, &keyDef);

	PGPclOpenClientPrefs (PGPGetContextMemoryMgr (pKM->Context), &prefref);
	PGPGetPrefBoolean (prefref, kPGPPrefKeyServerSyncOnRevocation, 
						&bSyncWithServer);
	PGPclCloseClientPrefs (prefref, FALSE);

	if (keyToRevoke == keyDef) {
		if (KMMessageBox (pKM->hWndParent, IDS_CAPTION, IDS_REVCONFDEFKEY,
			MB_YESNO|MB_TASKMODAL|MB_DEFBUTTON2|MB_ICONWARNING)==IDNO) 
			return FALSE;
	}
	else {
		if (KMMessageBox (pKM->hWndParent, IDS_CAPTION, IDS_REVOKECONFIRM, 
						MB_YESNO|MB_ICONEXCLAMATION) == IDNO) 
			return FALSE;
	}

	err = PGPGetKeyBoolean (keyToRevoke, 
				kPGPKeyPropIsSecret, &bSecret); CKERR;

	if (bSecret) {
		keyRevoker = keyToRevoke;
	}
	else {
		err = PGPCountRevocationKeys (keyToRevoke, &uNumRevokers);  CKERR;
		for (u = 0; u < uNumRevokers; u++) {
			err = PGPGetIndexedRevocationKey (keyToRevoke, pKM->KeySetMain,
					u, &key, NULL); CKERR;
			if (PGPKeyRefIsValid (key)) {
				err = PGPGetKeyBoolean (key, 
						kPGPKeyPropIsSecret, &bSecret); CKERR;
				err = PGPGetKeyBoolean (key, 
						kPGPKeyPropIsSecretShared, &bSplit); CKERR;
				if (bSecret) {
					keyRevoker = key;
					if (!bSplit) 
						break;
				}
			}
		}
	}

	if (!PGPKeyRefIsValid (keyRevoker))
		goto done;

	err = PGPGetKeyBoolean (keyToRevoke, 
						kPGPKeyPropIsSecretShared, &bSplit); CKERR;

	// get valid passphrase
	LoadString (g_hInst, IDS_SELKEYPASSPHRASE, sz128, 128); 
	err = KMGetKeyPhrase (pKM->Context, pKM->tlsContext,
						pKM->hWndParent, sz128,
						pKM->KeySetMain, keyRevoker,
						&pszPhrase, &pPasskey, &sizePasskey);
	PGPclErrorBox (NULL, err);

	// now we have a valid passphrase, if required
	if (IsntPGPError (err)) {

		// update from server
		if (bSyncWithServer) {
			if (!KMGetFromServerInternal (pKM, FALSE, FALSE, FALSE)) {
				if (KMMessageBox (pKM->hWndParent, IDS_CAPTION, 
									IDS_QUERYCONTINUEREVOKINGKEY, 
									MB_YESNO|MB_ICONEXCLAMATION) == IDNO) {
					bRetVal = FALSE;
				}
			}
		}
		
		if (bRetVal) {

			// make sure we have enough entropy
			PGPclRandom (pKM->Context, pKM->hWndParent, 0);

			if (bSplit) {
				sRevokeKeySplit (pKM->Context, pKM->KeySetMain,
									keyToRevoke, 
									pPasskey, sizePasskey);
			}
			else {
				sRevokeKeyNormal (pKM->Context, pKM->KeySetMain,
									keyToRevoke, 
									pszPhrase);
			}
						
			if (IsntPGPError (PGPclErrorBox (NULL, err))) {
				KMCommitKeyRingChanges (pKM);
				KMUpdateKeyInTree (pKM, keyToRevoke, FALSE);
				KMUpdateAllValidities (pKM);
				InvalidateRect (pKM->hWndTree, NULL, TRUE);

				// send to server
				if (bSyncWithServer) {
					KMSendToServer (pKM, PGPCL_DEFAULTSERVER);
				}
			}
			else bRetVal = FALSE;
		}
	}
	else bRetVal = FALSE;

done :
	if (IsntNull (pszPhrase))
		KMFreePhrase (pszPhrase);
	if (IsntNull (pPasskey)) 
		KMFreePasskey (pPasskey, sizePasskey);
	if (PGPKeySetRefIsValid (keysetRevokers))
		PGPFreeKeySet (keysetRevokers);

	PGPclErrorBox (pKM->hWndParent, err);

	return bRetVal;
}


//	___________________________________________________
//
//	Revoke signature dialog message procedure

static BOOL CALLBACK 
sRevokeCertDlgProc (HWND hDlg, 
				   UINT uMsg, 								
				   WPARAM wParam, 
				   LPARAM lParam) 
{
	PREVOKECERTSTRUCT prcs;

	switch (uMsg) {

	case WM_INITDIALOG :
		{
			CHAR sz[kPGPMaxUserIDSize +1];
			SetWindowLong (hDlg, GWL_USERDATA, lParam);
			prcs = (PREVOKECERTSTRUCT)lParam;
			KMGetKeyIDFromKey (prcs->key, sz, sizeof (sz));
			SetDlgItemText (hDlg, IDC_KEYID, sz);
			KMGetKeyName (prcs->key, sz, sizeof (sz));
			SetDlgItemText (hDlg, IDC_NAME, sz);
		}
		break;

	case WM_COMMAND:

		switch (LOWORD(wParam)) {
		case IDCANCEL:
			prcs = (PREVOKECERTSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
			prcs->bSyncWithServer = FALSE;
			EndDialog (hDlg, 1);
			break;

		case IDOK:
			prcs = (PREVOKECERTSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
			EndDialog (hDlg, 0);
			break;

		case IDHELP :
			prcs = (PREVOKECERTSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
			WinHelp (hDlg, prcs->pKM->szHelpFile, HELP_CONTEXT, 
						IDH_PGPKM_REVOKECERTDIALOG); 
			break;
		}
		return TRUE;
	}
	return FALSE;
}


//	___________________________________________________
//
//  Revoke selected signature

BOOL 
KMRevokeCert (PKEYMAN pKM) 
{
	BOOL				bRetVal		= TRUE;
	PGPByte*			pPasskey	= NULL;

	PGPSize				sizePasskey;
	PGPSigRef			cert;
	PGPKeyRef			keySigning, keyParent;
	PGPError			err;
	CHAR				sz128[128];
	REVOKECERTSTRUCT	rcs;
	PGPPrefRef			prefref;

	cert = (PGPSigRef) KMFocusedObject (pKM);

	err = PGPGetSigCertifierKey (cert, pKM->KeySetMain, &keySigning);
	if (err == kPGPError_ItemNotFound) {
		keySigning = NULL;
		err = kPGPError_NoErr;
	}

	if (IsntPGPError (PGPclErrorBox (NULL, err))) {
		if (!keySigning) {
			KMMessageBox (pKM->hWndParent, IDS_CAPTION, IDS_CERTKEYNOTONRING, 
							MB_OK|MB_ICONEXCLAMATION);
			return FALSE;
		}

		rcs.pKM = pKM;
		rcs.key = KMGetKeyFromCert (pKM, cert);

		PGPclOpenClientPrefs (PGPGetContextMemoryMgr (pKM->Context), 
						&prefref);
		PGPGetPrefBoolean (prefref, kPGPPrefKeyServerSyncOnRevocation, 
						&(rcs.bSyncWithServer));
		PGPclCloseClientPrefs (prefref, FALSE);

		if (DialogBoxParam (g_hInst, MAKEINTRESOURCE(IDD_REVOKECERT), 
							pKM->hWndParent, sRevokeCertDlgProc, 
							(LPARAM)&rcs)) {
			return FALSE;
		}
		
		// get valid passphrase
		LoadString (g_hInst, IDS_SIGNKEYPASSPHRASE, sz128, 128); 
		err = KMGetKeyPhrase (pKM->Context, pKM->tlsContext,
						pKM->hWndParent, sz128,
						pKM->KeySetMain, keySigning,
						NULL, &pPasskey, &sizePasskey);
		PGPclErrorBox (NULL, err);

		// now we have a valid passphrase, if required
		if (IsntPGPError (err)) {

			// update from server
			if (rcs.bSyncWithServer) {
				if (!KMGetFromServerInternal (pKM, FALSE, FALSE, FALSE)) {
					if (KMMessageBox (pKM->hWndParent, IDS_CAPTION, 
									IDS_QUERYCONTINUEREVOKINGCERT, 
									MB_YESNO|MB_ICONEXCLAMATION) == IDNO) {
						bRetVal = FALSE;
					}
				}
			}
		
			if (bRetVal) {

				// make sure we have enough entropy
				PGPclRandom (pKM->Context, pKM->hWndParent, 0);

				err = PGPRevokeSig (
						(PGPSigRef) KMFocusedObject (pKM), 
						pKM->KeySetMain,
						pPasskey ? 
							PGPOPasskeyBuffer (pKM->Context, 
								pPasskey, sizePasskey) :
							PGPONullOption (pKM->Context),
						PGPOLastOption (pKM->Context));
						
				if (IsntPGPError (PGPclErrorBox (NULL, err))) {
					keyParent = KMGetKeyFromCert (pKM, cert);
					KMUpdateKeyInTree (pKM, keyParent, FALSE);

					KMCommitKeyRingChanges (pKM);
					KMUpdateAllValidities (pKM);

					// send key to server, if selected
					if (rcs.bSyncWithServer) {
						KMSendToServer (pKM, PGPCL_DEFAULTSERVER);
					}
				}
				else bRetVal = FALSE;
			}
		}
		else bRetVal = FALSE;
	}
	else bRetVal = FALSE;

	if (pPasskey) {
		KMFreePasskey (pPasskey, sizePasskey);
		pPasskey = NULL;
	}

	return bRetVal;
}


//	___________________________________________________
//
//  Add designated revoker to key

BOOL 
KMAddRevoker (PKEYMAN pKM) 
{
	PGPKeySetRef	keysetToChoose		= kInvalidPGPKeySetRef;
	PGPKeySetRef	keysetToRemove		= kInvalidPGPKeySetRef;
	PGPKeySetRef	keysetThisKey		= kInvalidPGPKeySetRef;
	PGPKeySetRef	keysetSelected		= kInvalidPGPKeySetRef;
	PGPFilterRef	filterRSA			= kInvalidPGPFilterRef;
	PGPError		err					= kPGPError_NoErr;
	PGPByte*		pbyte				= NULL;
	BOOL			bRet				= FALSE;
	PGPUInt32		uCount				= 0;
	PGPBoolean		bSyncWithServer		= FALSE;

	PGPPrefRef		prefref;
	PGPSize			size;
	PGPKeyRef		key;
	CHAR			szPrompt[256];


	key = (PGPKeyRef)KMFocusedObject (pKM);

	PGPclOpenClientPrefs (PGPGetContextMemoryMgr (pKM->Context), &prefref);
	PGPGetPrefBoolean (prefref, kPGPPrefKeyServerSyncOnAdd, 
						&bSyncWithServer);
	PGPclCloseClientPrefs (prefref, FALSE);

	err = PGPNewKeySet (pKM->Context, &keysetToChoose); CKERR;
	err = PGPAddKeys (pKM->KeySetMain, keysetToChoose); CKERR;
	err = PGPCommitKeyRingChanges (keysetToChoose); CKERR;

	err = PGPNewKeyEncryptAlgorithmFilter (pKM->Context, 
						kPGPPublicKeyAlgorithm_RSA, &filterRSA); CKERR;
	err = PGPFilterKeySet (pKM->KeySetMain, 
						filterRSA, &keysetToRemove); CKERR;

	err = PGPCommitKeyRingChanges (keysetToRemove); CKERR;
	err = PGPRemoveKeys (keysetToRemove, keysetToChoose); CKERR;
	err = PGPCommitKeyRingChanges (keysetToChoose); CKERR;

	err = PGPCountKeys (keysetToChoose, &uCount); CKERR;
	if (uCount <= 1) {
		KMMessageBox (pKM->hWndParent, IDS_PGP, IDS_NOTENOUGHKEYSTOADDREVOKER,
						MB_OK|MB_ICONINFORMATION);
		goto done;
	}

	err = PGPFreeKeySet (keysetToRemove); CKERR;
	err = PGPNewSingletonKeySet (key, &keysetToRemove); CKERR;
	err = PGPRemoveKeys (keysetToRemove, keysetToChoose); CKERR;

	err = PGPCommitKeyRingChanges (keysetToChoose); CKERR;

	LoadString (g_hInst, IDS_ADDREVOKERPROMPT, szPrompt, sizeof(szPrompt));
	err = PGPclSelectKeys (pKM->Context, pKM->tlsContext, 
					pKM->hWndParent, szPrompt,
					keysetToChoose, pKM->KeySetMain, &keysetSelected);

	if (IsntPGPError (err) && PGPKeySetRefIsValid (keysetSelected))
	{
		if (KMMessageBox (pKM->hWndParent, IDS_CAPTION, 
				IDS_ADDREVOKERCONFIRM, MB_YESNO|MB_ICONEXCLAMATION) == IDYES) 
		{
			err = KMGetKeyPhrase (pKM->Context, pKM->tlsContext,
									pKM->hWndParent, NULL, 
									pKM->KeySetMain, key, 
									NULL, &pbyte, &size); CKERR;

			// update from server
			if (IsntPGPError (err) && bSyncWithServer) {
				if (!KMGetFromServerInternal (pKM, FALSE, FALSE, FALSE)) {
					if (KMMessageBox (pKM->hWndParent, IDS_CAPTION, 
								IDS_QUERYCONTINUEADDING, 	
								MB_YESNO|MB_ICONEXCLAMATION) == IDNO) 
					{
						err = kPGPError_UserAbort;
					}
				}
			}
		
			if (IsntPGPError (err)) {
				err = PGPAddKeyOptions (key, 
					PGPORevocationKeySet (pKM->Context, keysetSelected),
					pbyte ?
						PGPOPasskeyBuffer (pKM->Context, pbyte, size) :
						PGPONullOption (pKM->Context),
					PGPOLastOption (pKM->Context)); CKERR;
			}
		}
		else
			err = kPGPError_UserAbort;
	}

	// send to server
	if (IsntPGPError (err) && bSyncWithServer) {
		KMSendToServer (pKM, PGPCL_DEFAULTSERVER);
	}

	if (IsntPGPError (err)) {
		KMCommitKeyRingChanges (pKM);
		bRet = TRUE;

		if (bSyncWithServer) {
			KMMessageBox (pKM->hWndParent, IDS_PGP, IDS_ADDEDSENTREVOKERS,
						MB_OK|MB_ICONINFORMATION);
		}
		else {
			KMMessageBox (pKM->hWndParent, IDS_PGP, IDS_ADDEDREVOKERS,
						MB_OK|MB_ICONINFORMATION);
		}
	}

done :
	if (IsntNull (pbyte))
		KMFreePasskey (pbyte, size);
	if (PGPKeySetRefIsValid (keysetToChoose))
		PGPFreeKeySet (keysetToChoose);
	if (PGPKeySetRefIsValid (keysetToRemove))
		PGPFreeKeySet (keysetToRemove);
	if (PGPKeySetRefIsValid (keysetThisKey))
		PGPFreeKeySet (keysetThisKey);
	if (PGPKeySetRefIsValid (keysetSelected))
		PGPFreeKeySet (keysetSelected);
	if (PGPFilterRefIsValid (filterRSA))
		PGPFreeFilter (filterRSA);

	PGPclErrorBox (pKM->hWndParent, err);

	return bRet;
}