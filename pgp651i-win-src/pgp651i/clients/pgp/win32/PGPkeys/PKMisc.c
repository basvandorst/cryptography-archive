/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PKMisc.c - miscellaneous routines
	

	$Id: PKMisc.c,v 1.45 1999/03/29 20:58:58 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"	/* or pgpConfig.h in the CDK */

// project header files
#include "pgpkeysx.h"

// typedefs
typedef struct {
	PGPUInt32	uWindowShow;
	PGPInt32	iWindowPosX;
	PGPInt32	iWindowPosY;
	PGPInt32	iWindowWidth;
	PGPInt32	iWindowHeight;
	PGPBoolean	bGroupShow;
	PGPInt32	iGroupPercent;
	PGPInt32	iToolHeight;

} KEYSWINDOWSTRUCT, *PKEYSWINDOWSTRUCT;


// external global variables
extern HINSTANCE		g_hInst;
extern HWND				g_hWndMain;
extern UINT				g_uReloadKeyringMessage;
extern PGPContextRef	g_Context;
extern PGPtlsContextRef	g_TLSContext;
extern PGPBoolean		g_bExpertMode;
extern PGPBoolean		g_bMarginalAsInvalid;

//	____________________________________
//
//	Private memory allocation routine

VOID* 
pkAlloc (LONG size) 
{
	void* p;
	p = malloc (size);
	if (p) {
		memset (p, 0, size);
	}
	return p;
}

//	____________________________________
//
//	Private memory deallocation routine

VOID 
pkFree (VOID* p) 
{
	if (p) {
		free (p);
	}
}

//	____________________________________
//
//	Get PGPkeys private info from prefs file

BOOL 
PKGetPrivatePrefData (
			DWORD*	dwShow, 
			INT*	iX, 
			INT*	iY, 
			INT*	iWidth, 
			INT*	iHeight,
			BOOL*	bGroupShow,
			INT*	iGroupPercent,
			INT*	iToolHeight) 
{
	PKEYSWINDOWSTRUCT		pkws;
	PGPPrefRef				prefref;
	PGPError				err;
	PGPSize					size;

	*dwShow			= SW_SHOWNORMAL;
	*iX				= DEFAULTWINDOWX;
	*iY				= DEFAULTWINDOWY;
	*iWidth			= DEFAULTWINDOWWIDTH; 
	*iHeight		= DEFAULTWINDOWHEIGHT; 
	*bGroupShow		= DEFAULTGROUPSHOW;
	*iGroupPercent	= DEFAULTGROUPPERCENT;
	*iToolHeight	= DEFAULTTOOLHEIGHT;

	err = PGPclOpenClientPrefs (PGPGetContextMemoryMgr (g_Context), 
									&prefref);
	if (IsntPGPError (err)) {
		size = 0;
		err = PGPGetPrefData (prefref, kPGPPrefPGPkeysWinMainWinPos,
						&size, &pkws);
		PGPclCloseClientPrefs (prefref, FALSE);

		if (IsntPGPError (err)) {
			if (size == sizeof(KEYSWINDOWSTRUCT)) {
				*dwShow			= pkws->uWindowShow;
				*iX				= pkws->iWindowPosX;
				*iY				= pkws->iWindowPosY;
				*iWidth			= pkws->iWindowWidth;
				*iHeight		= pkws->iWindowHeight;
				*bGroupShow		= pkws->bGroupShow;
				*iGroupPercent	= pkws->iGroupPercent;
				*iToolHeight	= pkws->iToolHeight;
			}
			PGPDisposePrefData (prefref, pkws);
		}
	}

	return FALSE;
}

//	____________________________________
//
//	 Put window position information in registry

VOID 
PKSetPrivatePrefData (
		HWND	hWnd, 
		BOOL	bGroupShow,
		INT		iGroupPercent,
		INT		iToolHeight) 
{
	WINDOWPLACEMENT		wp;
	KEYSWINDOWSTRUCT	kws;
	PGPPrefRef			prefref;
	PGPError			err;

	wp.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement (hWnd, &wp);

	err = PGPclOpenClientPrefs (PGPGetContextMemoryMgr (g_Context), 
									&prefref);
	if (IsntPGPError (err)) {
		kws.uWindowShow = wp.showCmd;
		kws.iWindowPosX = wp.rcNormalPosition.left;
		kws.iWindowPosY = wp.rcNormalPosition.top;
		kws.iWindowWidth = 
			wp.rcNormalPosition.right - wp.rcNormalPosition.left;
		kws.iWindowHeight = 
			wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
		kws.bGroupShow = bGroupShow;
		kws.iGroupPercent = iGroupPercent;
		kws.iToolHeight = iToolHeight;
		PGPSetPrefData (prefref, kPGPPrefPGPkeysWinMainWinPos, 
						sizeof(kws), &kws);

		PGPclCloseClientPrefs (prefref, TRUE);
	}
}

//	____________________________________
//
//	Put PGPkeys application path into registry

VOID 
PKSetPathRegistryData (void) 
{
	HKEY	hKey;
	LONG	lResult;
	CHAR	szPath[MAX_PATH];
	DWORD	dw;
	CHAR	sz[256];
	CHAR*	p;

	LoadString (g_hInst, IDS_REGISTRYKEY, sz, sizeof(sz));
	lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE, sz, 0, NULL,
				REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dw);

	if (lResult == ERROR_SUCCESS) {

		// get full path of PGPkeys.exe
		GetModuleFileName (NULL, szPath, sizeof(szPath));

		// lop off file name
		if (p = strrchr (szPath, '\\'))
			*(++p) = '\0';

		// write it to the registry
		RegSetValueEx (hKey, "InstallPath", 0, REG_SZ, (LPBYTE)szPath, 
			lstrlen(szPath)+1);
		RegCloseKey (hKey);
	}
}

//	____________________________________
//
//  Put up preferences property sheet

VOID 
PKReloadPGPPreferences (PGPKEYSSTRUCT* ppks) 
{
	PGPPrefRef	prefs;
	PGPBoolean	b, bUpdate;

	bUpdate = FALSE;
	PGPclOpenClientPrefs (PGPGetContextMemoryMgr (g_Context), &prefs);

	// has the validity dot vs. bar pref changed?
	PGPGetPrefBoolean (prefs, kPGPPrefDisplayMarginalValidity, &b);
	if (b != g_bExpertMode) {
		g_bExpertMode = b;
		bUpdate = TRUE;
		if (!g_bExpertMode) {
			ppks->kmConfig.ulOptionFlags |= KMF_NOVICEMODE;
			ppks->gmConfig.ulOptionFlags |= GMF_NOVICEMODE;
		}
		else {
			ppks->kmConfig.ulOptionFlags &= ~KMF_NOVICEMODE;
			ppks->gmConfig.ulOptionFlags &= ~GMF_NOVICEMODE;
		}
	}

	// has the display marginal validity as invalid pref changed?
	PGPGetPrefBoolean (prefs, kPGPPrefMarginalIsInvalid, &b);
	if (b != g_bMarginalAsInvalid) {
		g_bMarginalAsInvalid = b;
		bUpdate = TRUE;
		if (g_bMarginalAsInvalid) {
			ppks->kmConfig.ulOptionFlags |= KMF_MARGASINVALID;
			ppks->gmConfig.ulOptionFlags |= GMF_MARGASINVALID;
		}
		else {
			ppks->kmConfig.ulOptionFlags &= ~KMF_MARGASINVALID;
			ppks->gmConfig.ulOptionFlags &= ~GMF_MARGASINVALID;
		}
	}

	PGPclCloseClientPrefs (prefs, FALSE);

	// if prefs have changed => redraw the windows
	if (bUpdate) {
		ppks->kmConfig.ulMask = PGPKM_OPTIONS;
		PGPkmConfigure (ppks->hKM, &(ppks->kmConfig));
		PGPkmLoadKeySet (ppks->hKM, ppks->KeySetMain, ppks->KeySetMain);
		PGPgmConfigure (ppks->hGM, &(ppks->gmConfig));
		PGPgmLoadGroups (ppks->hGM);
	}
}

//	____________________________________
//
//  Put up preferences property sheet

BOOL 
PKPGPPreferences (
		PGPKEYSSTRUCT*	ppks, 
		HWND			hWnd, 
		INT				iPage) 
{
	PGPError	err;

	EnableWindow (hWnd, FALSE);
	err = PGPclPreferences (g_Context, hWnd, iPage, ppks->KeySetMain);
	if (err != kPGPError_Win32_AlreadyOpen) SetForegroundWindow (hWnd);

	EnableWindow (hWnd, TRUE);
	SetFocus (ppks->hWndTreeList);

	PKReloadPGPPreferences (ppks);

	if (err == kPGPError_NoErr) return TRUE;
	else return FALSE;
}


//	___________________________________________________
//
//  commit keyring changes and broadcast reload message

INT 
PKCommitKeyRingChanges (PGPKeySetRef KeySet, BOOL bBroadcast) 
{
	PGPError	err;
	HCURSOR		hcursorOld;

	hcursorOld = SetCursor (LoadCursor (NULL, IDC_WAIT));
	err = PGPCommitKeyRingChanges (KeySet);
	if (IsntPGPError (PGPclErrorBox (NULL, err)) && bBroadcast) {
		PostMessage (HWND_BROADCAST, g_uReloadKeyringMessage,
			MAKEWPARAM (LOWORD (g_hWndMain), FALSE),
			(LPARAM)GetCurrentProcessId ());
	}
	SetCursor (hcursorOld);
	return err;
}


//	____________________________________
//
//	Auto update all keys in keyring

BOOL 
PKAutoUpdateAllKeys (
		HWND			hwnd, 
		PGPKeySetRef	keysetMain,
		BOOL			bForce) 
{
	PGPError		err					= kPGPError_NoErr;
	PGPKeySetRef	keysetUpdated		= kInvalidPGPKeySetRef;
	BOOL			bImported			= FALSE;

	INT				iNumKeys;
	HCURSOR			hcursorOld;

	// update entire keyset from key-based server
	err = PGPclUpdateKeySetFromServer (g_Context, g_TLSContext, hwnd, 
					keysetMain, PGPCL_USERIDBASEDSERVER, keysetMain, 
					&keysetUpdated); CKERR;

	// if we got updated keys, query user and commit the changes
	if (PGPKeySetRefIsValid (keysetUpdated)) 
	{
		if (bForce) 
		{
			hcursorOld = SetCursor (LoadCursor (NULL, IDC_WAIT));
			err = PGPAddKeys (keysetUpdated, keysetMain);
			if (IsntPGPError (err)) {
				bImported = TRUE;
				err = PKCommitKeyRingChanges (keysetMain, TRUE);
			}
			SetCursor (hcursorOld);
		}
		else 
		{
			PGPCountKeys (keysetUpdated, &iNumKeys);
			if (iNumKeys > 0)
			{
				err = PGPclQueryAddKeys (g_Context, 
										 g_TLSContext,
										 hwnd,
										 keysetUpdated, 
										 keysetMain);
				if (IsntPGPError (err)) {
					bImported = TRUE;
					err = PKCommitKeyRingChanges (keysetMain, TRUE);
				}
			}
			else
				PKMessageBox (hwnd, IDS_AUTOUPDATECAPTION, 
						IDS_NOUPDATEDKEYS, MB_OK|MB_ICONEXCLAMATION);
		}
	}

done :
	if (PGPKeySetRefIsValid (keysetUpdated))
		PGPFreeKeySet (keysetUpdated);

	PGPclErrorBox (hwnd, err);

	return bImported;
}


//	____________________________________
//
//	derive meta introducers from keyset

static PGPError 
sGetMetaIntroducers (
		PGPKeySetRef	keysetMain,
		PGPKeySetRef*	pkeysetMeta)
{
	PGPError		err					= kPGPError_NoErr;
	PGPKeyListRef	keylist				= kInvalidPGPKeyListRef;
	PGPKeyIterRef	keyiter				= kInvalidPGPKeyIterRef;
	PGPKeyRef		key					= kInvalidPGPKeyRef;
	PGPKeySetRef	keysetMeta			= kInvalidPGPKeySetRef;
	PGPKeySetRef	keysetSingle		= kInvalidPGPKeySetRef;
	PGPBoolean		bMargIsInvalid;
	PGPValidity		validityThreshold;
	PGPPrefRef		prefs;

	// use prefs to determine validity threshold
	PGPclOpenClientPrefs (PGPGetContextMemoryMgr (g_Context), &prefs);
	PGPGetPrefBoolean (prefs, kPGPPrefMarginalIsInvalid, &bMargIsInvalid);
	PGPclCloseClientPrefs (prefs, FALSE);

	if (bMargIsInvalid) 
		validityThreshold = kPGPValidity_Complete;
	else
		validityThreshold = kPGPValidity_Marginal;

	// create the new keyset to collect the meta-introducer keys
	err = PGPNewKeySet (g_Context, &keysetMeta); CKERR;

	// now we'll iterate the entire keyset
	err = PGPOrderKeySet (keysetMain, kPGPAnyOrdering, &keylist); CKERR;
	err = PGPNewKeyIter (keylist, &keyiter); CKERR;

	while (IsntPGPError (PGPKeyIterNext (keyiter, &key)) && 
			PGPKeyRefIsValid (key)) 
	{
		PGPBoolean		bDisabled;
		PGPBoolean		bExpired;
		PGPBoolean		bRevoked;
		PGPBoolean		bMetaIntroducer;
		PGPValidity		validity;
		PGPUserIDRef	userid;
		PGPSigRef		sig;

		bMetaIntroducer = FALSE;

		// we'll exclude expired, revoked, disabled and invalid keys
		PGPGetKeyBoolean (key, kPGPKeyPropIsExpired, &bExpired);
		PGPGetKeyBoolean (key, kPGPKeyPropIsRevoked, &bRevoked);
		PGPGetKeyBoolean (key, kPGPKeyPropIsDisabled, &bDisabled);
		PGPGetPrimaryUserIDValidity (key, &validity);

		if (!bDisabled && !bExpired && !bRevoked &&
			(validity >= validityThreshold)) 
		{
			while (IsntPGPError (PGPKeyIterNextUserID (keyiter, &userid)) && 
					PGPUserIDRefIsValid (userid) &&
					!bMetaIntroducer) 
			{
				while (IsntPGPError (PGPKeyIterNextUIDSig (keyiter, &sig)) && 
						PGPSigRefIsValid (sig) &&
						!bMetaIntroducer) 
				{
					PGPBoolean		bExportable;
					PGPUInt32		uTrustLevel;

					PGPGetSigBoolean (sig, 
							kPGPSigPropIsRevoked, &bRevoked);
					PGPGetSigBoolean (sig, 
							kPGPSigPropIsExportable, &bExportable);
					PGPGetSigNumber  (sig, 
							kPGPSigPropTrustLevel, &uTrustLevel);

					if (!bRevoked && !bExportable && (uTrustLevel == 2))
						 bMetaIntroducer = TRUE;

				}
			}
			
			// this is a meta introducer, add it to our growing collection
			if (bMetaIntroducer) 
			{
				err = PGPNewSingletonKeySet (key, &keysetSingle); CKERR;
				err = PGPAddKeys (keysetSingle, keysetMeta); CKERR;
				PGPFreeKeySet (keysetSingle);
				keysetSingle = kInvalidPGPKeySetRef;
			}
		}
	}

	// return keyset
	*pkeysetMeta = keysetMeta;
	keysetMeta = kInvalidPGPKeySetRef;

done :
	if (PGPKeySetRefIsValid (keysetMeta))
		PGPFreeKeySet (keysetMeta);
	if (PGPKeySetRefIsValid (keysetSingle))
		PGPFreeKeySet (keysetSingle);
	if (PGPKeyIterRefIsValid (keyiter))
		PGPFreeKeyIter (keyiter);
	if (PGPKeyListRefIsValid (keylist))
		PGPFreeKeyList (keylist);
	
	return err;
}

//	____________________________________
//
//	find keys signed by specified key and update them from keyserver

static PGPError 
sFindAndUpdateKeysIntroducedBy (
		HWND			hwnd,
		PGPKeyRef		key,
		PGPKeySetRef	keysetMain,
		PGPKeySetRef	keysetSigned)
{
	PGPError		err				= kPGPError_NoErr;
	PGPFilterRef	filter			= kInvalidPGPFilterRef;
	PGPKeyListRef	keylist			= kInvalidPGPKeyListRef;
	PGPKeyIterRef	keyiter			= kInvalidPGPKeyIterRef;
	PGPKeySetRef	keysetFound		= kInvalidPGPKeySetRef;
	PGPKeySetRef	keysetSingle	= kInvalidPGPKeySetRef;

	PGPKeyID		keyidMeta;

	// get the keyid of this meta introducer key
	err = PGPGetKeyIDFromKey (key, &keyidMeta); CKERR;

	// create a filter to find keys signed by this key
	err = PGPNewSigKeyIDFilter (g_Context, &keyidMeta, &filter); CKERR;

	// go out and search the root keyserver with this filter
	err = PGPclSearchServerWithFilter (g_Context, g_TLSContext,
									hwnd, filter, PGPCL_ROOTSERVER, 
									keysetMain, &keysetFound); CKERR;
	if (!PGPKeySetRefIsValid (keysetFound)) 
		goto done;

	// we found some keys signed by the meta introducer, now iterate them
	err = PGPOrderKeySet (keysetFound, kPGPAnyOrdering, &keylist); CKERR;
	err = PGPNewKeyIter (keylist, &keyiter); CKERR;
 
	while (	IsntPGPError (PGPKeyIterNext (keyiter, &key)) &&
			PGPKeyRefIsValid (key))
	{
		PGPBoolean		bSignedByMeta	= FALSE;
		PGPUserIDRef	userid;
		PGPSigRef		sig;
		PGPBoolean		bExportable;
		PGPUInt32		uTrustLevel;
		PGPKeyID		keyidSigner;

		while (	IsntPGPError (PGPKeyIterNextUserID (keyiter, &userid)) &&
				PGPUserIDRefIsValid (userid) &&
				!bSignedByMeta)
		{
			while (	IsntPGPError (PGPKeyIterNextUIDSig (keyiter, &sig)) &&
					PGPSigRefIsValid (sig) &&
					!bSignedByMeta)
			{
				err = PGPGetKeyIDOfCertifier (sig, &keyidSigner); CKERR;
				err = PGPGetSigBoolean (sig, 
						kPGPSigPropIsExportable, &bExportable); CKERR;
				err =PGPGetSigNumber  (sig, 
						kPGPSigPropTrustLevel, &uTrustLevel); CKERR;

				// check if this cert was created by meta introducer
				// and if it has the appropriate properties
				if (bExportable && (uTrustLevel == 1)) {
					if (PGPCompareKeyIDs (&keyidMeta, &keyidSigner) == 0)
						bSignedByMeta = TRUE;
				}
			}
		}
		
		// this key is one that we're looking for, add it to keyset
		if (bSignedByMeta) {
			err = PGPNewSingletonKeySet (key, &keysetSingle); CKERR;
			err = PGPAddKeys (keysetSingle, keysetSigned); CKERR;
			PGPFreeKeySet (keysetSingle);
			keysetSingle = kInvalidPGPKeySetRef;
		}
	}

done:
	if (PGPKeySetRefIsValid (keysetSingle))
		PGPFreeKeySet (keysetSingle);
	if (PGPKeySetRefIsValid (keysetFound))
		PGPFreeKeySet (keysetFound);
	if (PGPKeyListRefIsValid (keylist))
		PGPFreeKeyList (keylist);
	if (PGPKeyIterRefIsValid (keyiter))
		PGPFreeKeyIter (keyiter);
	if (PGPFilterRefIsValid (filter))
		PGPFreeFilter (filter);

	return err;
}

//	____________________________________
//
//	Auto update trusted introducers in keyring

BOOL 
PKAutoUpdateIntroducers (
		HWND			hwnd, 
		PGPKeySetRef	keysetMain,
		BOOL			bForce) 
{
	PGPError		err					= kPGPError_NoErr;
	PGPKeySetRef	keysetMeta			= kInvalidPGPKeySetRef;
	PGPKeySetRef	keysetUpdatedMeta	= kInvalidPGPKeySetRef;
	PGPKeySetRef	keysetSigned		= kInvalidPGPKeySetRef;
	PGPKeyListRef	keylist				= kInvalidPGPKeyListRef;
	PGPKeyIterRef	keyiter				= kInvalidPGPKeyIterRef;
	BOOL			bImported			= FALSE;

	PGPKeyRef			key;
	PGPInt32			iNumKeys;
	HCURSOR				hcursorOld;

	// get meta introducers from main keyset
	err = sGetMetaIntroducers (keysetMain, &keysetMeta); CKERR;

	// if we have some, update them from root keyserver
	PGPCountKeys (keysetMeta, &iNumKeys);
	if (iNumKeys > 0) 
	{
		err = PGPclUpdateKeySetFromServer (g_Context, g_TLSContext, hwnd, 
					keysetMeta, PGPCL_ROOTSERVER, keysetMain, 
					&keysetUpdatedMeta); CKERR;
	}

	// if we found keys on keyserver, add them to our main keyset
	if (PGPRefIsValid (keysetUpdatedMeta)) {
		hcursorOld = SetCursor (LoadCursor (NULL, IDC_WAIT));
		err = PGPAddKeys (keysetUpdatedMeta, keysetMain); 
		if (IsntPGPError (err)) 
		{
			bImported = TRUE;
			err = PKCommitKeyRingChanges (keysetMain, TRUE);
		}
		SetCursor (hcursorOld);
		CKERR;
	}

	// create new keyset to collect keys signed by the meta introducers
	err = PGPNewKeySet (g_Context, &keysetSigned); CKERR;

	// now prepare to iterate through the meta introducers
	err = PGPOrderKeySet (keysetMeta, kPGPAnyOrdering, &keylist); CKERR;
	err = PGPNewKeyIter (keylist, &keyiter); CKERR;

	while (	IsntPGPError (PGPKeyIterNext (keyiter, &key)) && 
			PGPKeyRefIsValid (key))
	{
		// go look for keys signed by this meta introducer
		err = sFindAndUpdateKeysIntroducedBy (
					hwnd, key, keysetMain, keysetSigned); CKERR;
	}
	CKERR;

	// now add the updated signed keys to our main keyset
	if (bForce) 
	{
		hcursorOld = SetCursor (LoadCursor (NULL, IDC_WAIT));
		err = PGPAddKeys (keysetSigned, keysetMain);
		if (IsntPGPError (err)) {
			bImported = TRUE;
			err = PKCommitKeyRingChanges (keysetMain, TRUE);
		}
		SetCursor (hcursorOld);
	}
	else 
	{
		PGPCountKeys (keysetSigned, &iNumKeys);
		if (iNumKeys > 0)
		{
			err = PGPclQueryAddKeys (g_Context, 
									 g_TLSContext,
									 hwnd,
									 keysetSigned, 
									 keysetMain);
			if (IsntPGPError (err)) {
				bImported = TRUE;
				err = PKCommitKeyRingChanges (keysetMain, TRUE);
			}
		}
		else
		{
			PKMessageBox (hwnd, IDS_AUTOUPDATECAPTION, 
					IDS_NONEWTRUSTEDINTROS, MB_OK|MB_ICONEXCLAMATION);
		}
	}

done:
	if (PGPKeyListRefIsValid (keylist))
		PGPFreeKeyList (keylist);
	if (PGPKeyIterRefIsValid (keyiter))
		PGPFreeKeyIter (keyiter);
	if (PGPKeySetRefIsValid (keysetUpdatedMeta))
		PGPFreeKeySet (keysetUpdatedMeta);
	if (PGPKeySetRefIsValid (keysetMeta))
		PGPFreeKeySet (keysetMeta);
	if (PGPKeySetRefIsValid (keysetSigned))
		PGPFreeKeySet (keysetSigned);

	PGPclErrorBox (hwnd, err);

	return bImported;
}


//	____________________________________
//
//	Ask CA for any new CRLs 

BOOL 
PKUpdateCARevocations (
		HWND			hwnd, 
		HKEYMAN			hKM,
		PGPKeySetRef	keysetMain) 
{
	PGPError		err					= kPGPError_NoErr;
	PGPInt32		iNumKeys			= 0;
	BOOL			bImported			= FALSE;

	err = PGPclGetCertificateRevocationsFromServer (
								g_Context,
								g_TLSContext,
								hwnd, 
								keysetMain);

	if (err == kPGPError_Win32_NoNewCRL)
	{
		PKMessageBox (hwnd, IDS_CAPTION, IDS_NONEWCRLS, 
				MB_OK|MB_ICONINFORMATION);
	}
	else if (!PGPclErrorBox (hwnd, err))
	{
		PKCommitKeyRingChanges (keysetMain, TRUE);
		PGPkmReLoadKeySet (hKM, FALSE);
	}

	return bImported;
}


//	__________________________________________________________
//
//  Schedule the next CRL update

PGPError
PKScheduleNextCRLUpdate (
		PGPContextRef	context, 
		PGPKeySetRef	keysetMain)
{
	PGPError	err					= kPGPError_NoErr;

#if PGP_BUSINESS_SECURITY

	PGPPrefRef		prefrefAdmin		= kInvalidPGPPrefRef;
	PGPPrefRef		prefrefClient		= kInvalidPGPPrefRef;

	PGPKeyRef		keyRootCA;
	PGPSigRef		sigRootCA;
	PGPBoolean		bAutoUpdate;
	PGPBoolean		bHasCRL;
	PGPUInt32		uNextUpdate;


	err = PGPclOpenAdminPrefs (PGPGetContextMemoryMgr (context), 
				&prefrefAdmin, PGPclIsAdminInstall()); CKERR;

	err = PGPGetPrefBoolean (prefrefAdmin, 
				kPGPPrefAutoUpdateX509CRL, &bAutoUpdate); CKERR;
	if (bAutoUpdate)
	{
		err = PGPclOpenClientPrefs (PGPGetContextMemoryMgr (context),
				&prefrefClient); CKERR;

		err = PGPclGetRootCACertPrefs (context, prefrefClient, keysetMain,
				&keyRootCA, &sigRootCA); CKERR;

		err = PGPGetKeyBoolean (
				keyRootCA, kPGPKeyPropHasCRL, &bHasCRL); CKERR;

		if (bHasCRL)
		{
			err = PGPGetKeyTime (keyRootCA, 
					kPGPKeyPropCRLNextUpdate, &uNextUpdate); CKERR;

			PGPSetPrefNumber (prefrefClient, 
					kPGPPrefNextAutoCRLUpdate,
					uNextUpdate);
		}
		CKERR;
	}

done:
	if (PGPPrefRefIsValid (prefrefClient)) 
		PGPclCloseClientPrefs (prefrefClient, TRUE);
	if (PGPPrefRefIsValid (prefrefAdmin)) 
		PGPclCloseAdminPrefs (prefrefAdmin, FALSE);

#endif	// PGP_BUSINESS_SECURITY

	return err;
}


