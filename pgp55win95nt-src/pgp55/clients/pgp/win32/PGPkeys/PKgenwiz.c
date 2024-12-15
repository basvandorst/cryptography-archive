/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved
	
	PGPkeygenwiz.c - handle wizard for creating new keys

	This code attempts to thwart some common situations
	which could compromise the security of the passphrase.
	For the most part, this code has been removed or comment-
	out.
	See PGPkeysChange.c for a description of methods used here.


	$Id: PKgenwiz.c,v 1.71 1997/10/21 13:36:34 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

#include "pgpkeysx.h"
#include "resource.h"
#include "pgpRandomPool.h"

#include <process.h>

#include "pgpKeys.h"
#include "pgpBuildFlags.h"

#include "..\include\pgpCmdlg.h"

#define MAX_FULL_NAME_LEN		126
#define MAX_EMAIL_LEN			126

#define BADPHRASE_LENGTH		0x01
#define BADPHRASE_QUALITY		0x02
#define BADPHRASE_CONFIRMATION	0x04

#define MIN_KEY_SIZE			512
#define MAX_RSA_KEY_SIZE		2048
#define MAX_DSA_KEY_SIZE		4096

#define DEFAULT_MINPHRASELEN	8
#define DEFAULT_MINPHRASEQUAL	60
#define DEFAULT_KEYSIZE			2048
#define DEFAULT_SUBKEYSIZE		1024
#define DEFAULT_KEYTYPE			kPGPPublicKeyAlgorithm_DSA

#define TIMERPERIOD				1000L
#define AVI_TIMER				2345L
#define AVI_RUNTIME				11000L  // duration of AVI in ms

#define LEDTIMER		111L
#define LEDTIMERPERIOD	100L

#define NUMLEDS			10
#define LEDSPACING		2


#define KGWIZ_INTRO		0
#define KGWIZ_NAME		1
#define KGWIZ_TYPE		2
#define KGWIZ_SIZE		3
#define KGWIZ_EXPIRE	4
#define KGWIZ_ADK		5
#define KGWIZ_CORPCERT	6
#define KGWIZ_PHRASE	7
#define KGWIZ_BADPHRASE	8
#define KGWIZ_ENTROPY   9
#define KGWIZ_KEYGEN	10
#define KGWIZ_SIGN		11
#define KGWIZ_PRESEND   12
#define KGWIZ_SEND		13
#define KGWIZ_DONE		14
#define NUM_WIZ_PAGES	15


// external globals
extern HINSTANCE		g_hInst;
extern PGPContextRef	g_ContextRef;
extern CHAR				g_szHelpFile[MAX_PATH];	 // name of help file

// local globals
static HHOOK	hHookKeyboard;
static HWND		hWndCollectEntropy = NULL;
static WNDPROC	wpOrigPhrase1Proc;  
static WNDPROC	wpOrigPhrase2Proc;  
static BOOL		bHideTyping = TRUE;
static LPSTR	szPhrase1 = NULL;
static LPSTR	szPhrase2 = NULL;
static LONG		lPhrase1Len = 0;
static LONG		lPhrase2Len = 0;

static PGPKeySetRef	KeySetMain;


// typedefs
typedef struct _KeyGenInfo
{
	PGPContextRef	Context;
	PGPPrefRef		PrefRefAdmin;
	PGPPrefRef		PrefRefClient;
	HWND			hWndWiz;
	HBITMAP			hBitmap;
	HPALETTE		hPalette;
	LPSTR			pszFullName;
	LPSTR			pszEmail;
	LPSTR			pszUserID;
	LPSTR			pszPassPhrase;
	PGPKeyRef		ADK;
	BOOL			bEnforceADK;
	PGPKeyRef		CorpKey;
	BOOL			bMetaCorpKey;
	BOOL			bMinPhraseQuality;
	INT				iMinPhraseQuality;
	BOOL			bMinPhraseLength;
	INT				iMinPhraseLength;
	BOOL			bAllowRSAGen;
	UINT			uMinKeySize;
	UINT			uPhraseFlags;
	UINT			uKeyType;
	UINT			uKeySize;
	UINT			uSubKeySize;
	UINT			uExpireDays;
	HWND			hwndExpirationDate;
	LONG			lRandomBitsNeeded;
	LONG			lOriginalEntropy;
	PGPKeyRef		Key;
	PGPKeyRef		OldKey;
	BOOL			bFinishSelected;
	BOOL			bCancelPending;
	BOOL			bInGeneration;
	UINT			uWorkingPhase;
	BOOL			bDoSend;
	BOOL			bSendInProgress;
	INT				iStatusValue;
	INT				iStatusDirection;
	BOOL			bSendComplete;
	INT				iFinalResult;
} KEYGENINFO, *PKEYGENINFO;


//___________________________
//
// Secure memory allocation routines
//

static VOID* 
secAlloc (UINT uBytes) 
{
	return (PGPNewSecureData (g_ContextRef, uBytes, NULL));
}


static VOID 
secFree (VOID* p) 
{
	if (p) {
		FillMemory (p, lstrlen (p), '\0');
		PGPFreeData (p);
	}
}

//----------------------------------------------------|
//  Hook procedure for WH_KEYBOARD hook -- 
 
LRESULT CALLBACK WizKeyboardHookProc (INT iCode, 
									  WPARAM wParam, LPARAM lParam) { 
 
	if (hWndCollectEntropy) {
		PGPGlobalRandomPoolAddKeystroke (wParam);
		PostMessage (hWndCollectEntropy, WM_MOUSEMOVE, 0, 0);
		return 1;
	}

	return CallNextHookEx (hHookKeyboard, iCode, wParam, lParam);
} 
 
//----------------------------------------------------|
//  test admin prefs for validity

static BOOL 
ValidateConfiguration (PKEYGENINFO pkgi) 
{
	BOOL		bPrefsOK = TRUE;
	BOOL		bPrefsCorrupt = FALSE;

#if PGP_BUSINESS_SECURITY
	CHAR		szID[20];
	PGPError	err;
	PGPBoolean	b;
	PGPUInt32	u;
#endif

	pkgi->pszFullName		= NULL;
	pkgi->pszEmail			= NULL;
	pkgi->pszUserID			= NULL;
	pkgi->uKeyType			= DEFAULT_KEYTYPE;
	pkgi->uKeySize			= DEFAULT_KEYSIZE;
	pkgi->uSubKeySize		= DEFAULT_SUBKEYSIZE;
	pkgi->uExpireDays		= 0;
	pkgi->pszPassPhrase		= NULL;
	pkgi->uPhraseFlags		= 0;
	pkgi->lRandomBitsNeeded	= 0;
	pkgi->lOriginalEntropy	= 0;
	pkgi->Key				= NULL;
	pkgi->OldKey			= NULL;
	pkgi->bFinishSelected	= FALSE;
	pkgi->bCancelPending	= FALSE;
	pkgi->bInGeneration		= FALSE;
	pkgi->uWorkingPhase		= 0;
	pkgi->bDoSend			= FALSE;
	pkgi->bSendComplete		= FALSE;
	pkgi->iFinalResult		= 0;

	// minimum passphrase length
#if PGP_BUSINESS_SECURITY
	b = FALSE;
	PGPGetPrefBoolean (pkgi->PrefRefAdmin, kPGPPrefEnforceMinChars, &b);
	pkgi->bMinPhraseLength = b;
	if (b) {
		u = DEFAULT_MINPHRASELEN;
		err = PGPGetPrefNumber (pkgi->PrefRefAdmin, kPGPPrefMinChars, &u);
		if (IsPGPError (err)) bPrefsCorrupt = TRUE;
		pkgi->iMinPhraseLength = (INT)u;
	}
	else pkgi->iMinPhraseLength = DEFAULT_MINPHRASELEN;
#else
	pkgi->bMinPhraseLength = FALSE;
	pkgi->iMinPhraseLength = DEFAULT_MINPHRASELEN;
#endif // PGP_BUSINESS_SECURITY

	// minimum passphrase quality
#if PGP_BUSINESS_SECURITY
	b = FALSE;
	PGPGetPrefBoolean (pkgi->PrefRefAdmin, kPGPPrefEnforceMinQuality, &b);
	pkgi->bMinPhraseQuality = b;
	if (b) {
		u = DEFAULT_MINPHRASEQUAL;
		err = PGPGetPrefNumber (pkgi->PrefRefAdmin, kPGPPrefMinQuality, &u);
		if (IsPGPError (err)) bPrefsCorrupt = TRUE;
		pkgi->iMinPhraseQuality = (INT)u;
	}
	else pkgi->iMinPhraseQuality = 0;
#else
	pkgi->bMinPhraseQuality = FALSE;
	pkgi->iMinPhraseQuality = 0;
#endif // PGP_BUSINESS_SECURITY

	// RSA key generation
#if NO_RSA_KEYGEN
	pkgi->bAllowRSAGen = FALSE;
#else
#if PGP_BUSINESS_SECURITY
	b = FALSE;
	PGPGetPrefBoolean (pkgi->PrefRefAdmin, kPGPPrefAllowRSAKeyGen, &b);
	pkgi->bAllowRSAGen = b;
#else 
	pkgi->bAllowRSAGen = TRUE;
#endif
#endif

	// minimum key size 
#if PGP_BUSINESS_SECURITY
	u = MIN_KEY_SIZE;
	PGPGetPrefNumber (pkgi->PrefRefAdmin, kPGPPrefMinimumKeySize, &u);
	pkgi->uMinKeySize = u;
#else
	pkgi->uMinKeySize = MIN_KEY_SIZE;
#endif

	// ADK stuff
#if PGP_BUSINESS_SECURITY
	b = FALSE;
	PGPGetPrefBoolean (pkgi->PrefRefAdmin, kPGPPrefEnforceIncomingADK, &b);
	pkgi->bEnforceADK = b;

	b = FALSE;
	PGPGetPrefBoolean (pkgi->PrefRefAdmin, kPGPPrefUseDHADK, &b);
	if (b) {
		err = PGPGetPrefStringBuffer (pkgi->PrefRefAdmin, kPGPPrefDHADKID, 
			sizeof(szID), szID);
		if (IsntPGPError (err)) {
			err = PGPcomdlgGetKeyFromKeyID (pkgi->Context, KeySetMain, 
					szID, kPGPPublicKeyAlgorithm_DSA, &(pkgi->ADK));
		}
		if (IsPGPError (err)) {
			bPrefsOK = FALSE;
			PGPkeysMessageBox (NULL, IDS_CAPTION, IDS_MISSINGADK, 
							MB_OK|MB_ICONSTOP);
		}
		else {
			PGPGetKeyBoolean (pkgi->ADK, kPGPKeyPropIsExpired, &b);
			if (b) {
				bPrefsOK = FALSE;
				PGPkeysMessageBox (NULL, IDS_CAPTION, IDS_EXPIREDADK, 
							MB_OK|MB_ICONSTOP);
			}
			else {
				PGPGetKeyBoolean (pkgi->ADK, kPGPKeyPropIsRevoked, &b);
				if (b) {
					bPrefsOK = FALSE;
					PGPkeysMessageBox (NULL, IDS_CAPTION, IDS_REVOKEDADK, 
								MB_OK|MB_ICONSTOP);
				}
			}
		}
	}
	else pkgi->ADK = NULL;
#else
	pkgi->ADK = NULL;
	pkgi->bEnforceADK = FALSE;
#endif // PGP_BUSINESS_SECURITY

	// Corporate Key stuff
#if PGP_BUSINESS_SECURITY
	// corp key signing type
	b = FALSE;
	PGPGetPrefBoolean (pkgi->PrefRefAdmin, kPGPPrefMetaIntroducerCorp, &b);
	pkgi->bMetaCorpKey = b;

	// corp key
	b = FALSE;
	PGPGetPrefBoolean (pkgi->PrefRefAdmin, kPGPPrefAutoSignTrustCorp, &b);
	if (b) {
		err = PGPGetPrefStringBuffer (pkgi->PrefRefAdmin, 
								kPGPPrefCorpKeyID, sizeof(szID), szID);
		if (IsntPGPError (err)) {
			err = PGPGetPrefNumber (pkgi->PrefRefAdmin,
							kPGPPrefCorpKeyPublicKeyAlgorithm, &u);
			if (IsntPGPError (err)) {
				err = PGPcomdlgGetKeyFromKeyID (pkgi->Context, KeySetMain, 
						szID, u, &(pkgi->CorpKey));
			}					
		}
		if (IsPGPError (err)) {
			bPrefsOK = FALSE;
			PGPkeysMessageBox (NULL, IDS_CAPTION, IDS_MISSINGCORPKEY, 
							MB_OK|MB_ICONSTOP);
		}
		else {
			PGPGetKeyBoolean (pkgi->CorpKey, kPGPKeyPropIsExpired, &b);
			if (b) {
				bPrefsOK = FALSE;
				PGPkeysMessageBox (NULL, IDS_CAPTION, IDS_EXPIREDCORPKEY, 
							MB_OK|MB_ICONSTOP);
			}
			else {
				PGPGetKeyBoolean (pkgi->CorpKey, kPGPKeyPropIsRevoked, &b);
				if (b) {
					bPrefsOK = FALSE;
					PGPkeysMessageBox (NULL, IDS_CAPTION, IDS_REVOKEDCORPKEY, 
								MB_OK|MB_ICONSTOP);
				}
			}
		}
	}
	else pkgi->CorpKey = NULL;
#else
	pkgi->bMetaCorpKey = FALSE;
	pkgi->CorpKey = NULL;
#endif // PGP_BUSINESS_SECURITY


	// everything OK ?
	if (bPrefsCorrupt) {
		bPrefsOK = FALSE;
		PGPkeysMessageBox (NULL, IDS_CAPTION, IDS_ADMINPREFSCORRUPT, 
							MB_OK|MB_ICONSTOP);
	}
	return (bPrefsOK);  
}


//----------------------------------------------------|
//  return DSA key size on basis of requested ElGamal key size

static ULONG GetDSAKeySize (ULONG ulRequested)
{
	ULONG ulActualBits = ulRequested;

	if (ulRequested > 1024) ulActualBits = 1024;

	return (ulActualBits);
}


//	____________________________________
//
//  Draw the "LED" progress indicator

VOID
DrawSendStatus (HWND hwnd,
				PKEYGENINFO pkgi) 
{
	HBRUSH			hBrushLit, hBrushUnlit, hBrushOld;
	HPEN			hPen, hPenOld;
	INT				i;
	INT				itop, ibot, ileft, iright, iwidth;
	RECT			rc;
	HDC				hdc;
	PAINTSTRUCT		ps;

	if (pkgi->iStatusValue < -1) return;

	hdc = BeginPaint (hwnd, &ps);

	// draw 3D shadow
	GetClientRect (hwnd, &rc);
	itop = rc.top+1;
	ibot = rc.bottom-2;

	iwidth = (rc.right-rc.left) / NUMLEDS;
	iwidth -= LEDSPACING;

	ileft = rc.left + 4;
	for (i=0; i<NUMLEDS; i++) {
		iright = ileft + iwidth;

		MoveToEx (hdc, ileft, ibot, NULL);
		LineTo (hdc, iright, ibot);
		LineTo (hdc, iright, itop);

		ileft += iwidth + LEDSPACING;
	}

	hPen = CreatePen (PS_SOLID, 0, RGB (128, 128, 128));
	hPenOld = SelectObject (hdc, hPen);
	hBrushLit = CreateSolidBrush (RGB (0, 255, 0));
	hBrushUnlit = CreateSolidBrush (RGB (0, 128, 0));

	ileft = rc.left + 4;

	// draw "Knight Rider" LEDs
	if (pkgi->iStatusDirection) {
		hBrushOld = SelectObject (hdc, hBrushUnlit);
		for (i=0; i<NUMLEDS; i++) {
			iright = ileft + iwidth;
	
			if (i == pkgi->iStatusValue) {
				SelectObject (hdc, hBrushLit);
				Rectangle (hdc, ileft, itop, iright, ibot);
				SelectObject (hdc, hBrushUnlit);
			}
			else  {
				Rectangle (hdc, ileft, itop, iright, ibot);
			}
	
			ileft += iwidth + LEDSPACING;
		}
	}

	// draw "progress bar" LEDs
	else { 
		if (pkgi->iStatusValue >= 0) 
			hBrushOld = SelectObject (hdc, hBrushLit);
		else
			hBrushOld = SelectObject (hdc, hBrushUnlit);

		for (i=0; i<NUMLEDS; i++) {
			iright = ileft + iwidth;
	
			if (i > pkgi->iStatusValue) {
				SelectObject (hdc, hBrushUnlit);
			}
			Rectangle (hdc, ileft, itop, iright, ibot);
	
			ileft += iwidth + LEDSPACING;
		}
	}

	SelectObject (hdc, hBrushOld);
	SelectObject (hdc, hPenOld);
	DeleteObject (hPen);
	DeleteObject (hBrushLit);
	DeleteObject (hBrushUnlit);

	EndPaint (hwnd, &ps);
}

//----------------------------------------------------|
//  find key with specified user id, if it exists

static VOID 
GetOldKey (PGPContextRef context, 
		   LPSTR szUserID, 
		   PGPKeyRef* pOldKey) 
{
	PGPKeyListRef	KeyList;
	PGPKeyIterRef	KeyIter;
	PGPKeySetRef	KeySet;
	PGPKeyRef		Key;
	PGPBoolean		bSecret;
	PGPError		err;
	PGPFilterRef	filter	= kPGPInvalidRef;
	PGPUInt32		uAlg;

	*pOldKey = NULL;

	err	= PGPNewUserIDStringFilter (context, szUserID, kPGPMatchSubString, 
									&filter);
	if (IsntPGPError (err)) {
		err	= PGPFilterKeySet (KeySetMain, filter, &KeySet);	
		PGPFreeFilter (filter);
	}

	if (KeySet) {

		PGPOrderKeySet (KeySet, kPGPCreationOrdering, &KeyList);
		if (KeyList) {

			PGPNewKeyIter (KeyList, &KeyIter);
			if (KeyIter) {

				PGPKeyIterNext (KeyIter, &Key);
				while (Key && !*pOldKey) {
					PGPGetKeyBoolean (Key, kPGPKeyPropIsSecret, &bSecret);
					if (bSecret) {
						PGPGetKeyNumber (Key, kPGPKeyPropAlgID, &uAlg);
						if (uAlg == kPGPPublicKeyAlgorithm_RSA) {
							*pOldKey = Key;
						}
					}
					PGPKeyIterNext (KeyIter, &Key);
				}
				PGPFreeKeyIter (KeyIter);
			}
			PGPFreeKeyList (KeyList);
		}
		PGPFreeKeySet(KeySet);
	}
	
}


//----------------------------------------------------|
//  create standard PGP userid from name and email address

static ULONG CreatePGPUserID (LPSTR* pszUserID, LPSTR szFullName, 
									 LPSTR szEmail)
{
	INT iReturnCode = kPGPError_NoErr;
	UINT uUserIDLen = 0;
	BOOL bEmail = FALSE;

	/*+4 is:  1 for the \0, one for the space, two for the broquets.*/
	uUserIDLen = lstrlen (szFullName) +1;
	if (lstrlen (szEmail)) {
		bEmail = TRUE;
		uUserIDLen += lstrlen (szEmail) +3;
	}

	*pszUserID = pkAlloc (sizeof(char) * uUserIDLen);
	if (*pszUserID) {
		if (bEmail)
			wsprintf (*pszUserID, "%s <%s>", szFullName, szEmail);
		else 
			lstrcpy (*pszUserID, szFullName);
	}
	else
		iReturnCode = kPGPError_OutOfMemory;

	return (iReturnCode);
}


//----------------------------------------------------|
//  callback routine called by library key generation routine
//  every so often with status of keygen.  Returning a nonzero
//  value cancels the key generation.

PGPError 
KeyGenCallback (PGPContextRef context, PGPEvent* event,
						PGPUserValue userValue)
{
	INT					iReturnCode = kPGPError_NoErr;
	UINT				uOriginalPhase;
	PKEYGENINFO			pkgi;
	PGPEventKeyGenData* pkgd;

	switch (event->type) {
	case kPGPEvent_KeyGenEvent:
		pkgd = &event->data.keyGenData;
		pkgi = (PKEYGENINFO) userValue;
		uOriginalPhase = pkgi->uWorkingPhase;

		if (!pkgi->bCancelPending) {
			if (pkgd->state == ' ') {
				if (pkgi->uWorkingPhase == IDS_KEYGENPHASE1)
					pkgi->uWorkingPhase = IDS_KEYGENPHASE2;
				else
					pkgi->uWorkingPhase = IDS_KEYGENPHASE1;
			}
			if (uOriginalPhase != pkgi->uWorkingPhase)
				PostMessage (pkgi->hWndWiz, 
							KM_M_CHANGEPHASE, 
							0,
							(LPARAM) pkgi->uWorkingPhase);
		}
		else //Let the kernel know we're canceling
			iReturnCode = kPGPError_UnknownError;
	}	
	
	return (iReturnCode);
}


//----------------------------------------------------|
//  thread for actually creating key

void KeyGenerationThread (void *pArgs)
{
	PKEYGENINFO			pkgi = NULL;
	BOOL				bNewDefaultKey = TRUE;
	PGPKeyRef			DefaultKey;
	PGPSubKeyRef		SubKey;
	PGPUserIDRef		CorpUserID;
	PGPUInt32			AlgFlags = 0;
	PGPSize				prefDataSize = 0;
	PGPCipherAlgorithm*	pAlgs;
	PGPBoolean			bFastGen;
	PGPContextRef		ctx;
	PGPKeySetRef		KeySetADK;
	PGPByte				enforce = 0;
	PGPError			err;
	INT					iTrustLevel;
	PGPUInt32			numAlgs;

	pkgi = (PKEYGENINFO) pArgs;
	ctx = pkgi->Context;

	PGPGetDefaultPrivateKey (KeySetMain, &DefaultKey);
	if (PGPRefIsValid (DefaultKey)) bNewDefaultKey = FALSE;

	// construct userid and check for existing key with same userid
	err = CreatePGPUserID (&pkgi->pszUserID, pkgi->pszFullName, 
											pkgi->pszEmail);
	if (pkgi->uKeyType == kPGPPublicKeyAlgorithm_DSA)
		GetOldKey (ctx, pkgi->pszUserID, &pkgi->OldKey);

	// get client preferences
	PGPGetPrefBoolean (pkgi->PrefRefClient, kPGPPrefFastKeyGen, &bFastGen);

	PGPGetPrefData (pkgi->PrefRefClient, kPGPPrefAllowedAlgorithmsList,
							  &prefDataSize, &pAlgs);

	// check for ADK enforcement
	if (pkgi->bEnforceADK) enforce = 0x80;

	if (err == kPGPError_NoErr) {
		numAlgs	= prefDataSize / sizeof(PGPCipherAlgorithm);
		
		if (PGPRefIsValid (pkgi->ADK)) {
			err = PGPNewSingletonKeySet (pkgi->ADK, &KeySetADK);
			if (IsntPGPError (err)) {
				err = PGPGenerateKey (
					ctx, &(pkgi->Key),
					PGPOKeySetRef (ctx, KeySetMain),
					PGPOKeyGenParams (ctx, pkgi->uKeyType, pkgi->uKeySize),
					PGPOKeyGenFast (ctx, bFastGen),
					PGPOKeyGenName (ctx, pkgi->pszUserID, 
											lstrlen (pkgi->pszUserID)),
					PGPOPassphrase (ctx, pkgi->pszPassPhrase),
					PGPOExpiration (ctx, pkgi->uExpireDays),
					PGPOPreferredAlgorithms (ctx, pAlgs, numAlgs),
					PGPOAdditionalRecipientRequestKeySet (ctx, 
														KeySetADK, enforce),
					PGPOEventHandler (ctx, KeyGenCallback, pkgi),
					PGPOLastOption (ctx));
			}
		}
		else {
			err = PGPGenerateKey (
				ctx, &(pkgi->Key),
				PGPOKeySetRef (ctx, KeySetMain),
				PGPOKeyGenParams (ctx, pkgi->uKeyType, pkgi->uKeySize),
				PGPOKeyGenFast (ctx, bFastGen),
				PGPOKeyGenName (ctx, pkgi->pszUserID, 
										lstrlen (pkgi->pszUserID)),
				PGPOPassphrase (ctx, pkgi->pszPassPhrase),
				PGPOExpiration (ctx, pkgi->uExpireDays),
				PGPOPreferredAlgorithms (ctx, pAlgs, numAlgs),
				PGPOEventHandler (ctx, KeyGenCallback, pkgi),
				PGPOLastOption (ctx));
		}

		if (err == kPGPError_NoErr) {
			if (pkgi->uKeyType == kPGPPublicKeyAlgorithm_DSA) {
				err = PGPGenerateSubKey (
					ctx, &SubKey,
					PGPOKeyGenMasterKey (ctx, pkgi->Key),
					PGPOKeyGenParams (ctx, kPGPPublicKeyAlgorithm_ElGamal, 
												pkgi->uSubKeySize),
					PGPOKeyGenFast (ctx, bFastGen),
					PGPOPassphrase (ctx, pkgi->pszPassPhrase),
					PGPOExpiration (ctx, pkgi->uExpireDays),
					PGPOEventHandler (ctx, KeyGenCallback, pkgi),
					PGPOLastOption (ctx));
			}
		}
	}
	if (pAlgs) PGPDisposePrefData (pkgi->PrefRefClient, pAlgs);

	if ((err == kPGPError_NoErr) && !pkgi->bCancelPending) {
		// sign and trust corporate key
		if (PGPRefIsValid (pkgi->CorpKey)) {
			err = PGPGetPrimaryUserID (pkgi->CorpKey, &CorpUserID);
			if (IsntPGPError (err)) {
				if (pkgi->bMetaCorpKey) iTrustLevel = 2;
				else iTrustLevel = 0;

				// make sure we have enough entropy
				PGPcomdlgRandom (ctx, pkgi->hWndWiz, 0);

				err = PGPSignUserID (CorpUserID, pkgi->Key, 
					PGPOPassphrase (ctx, pkgi->pszPassPhrase),
					PGPOExpiration (ctx, 0),
					PGPOExportable (ctx, FALSE),
					PGPOSigTrust (ctx, iTrustLevel, kPGPKeyTrust_Complete),
					PGPOLastOption (ctx));
				if (IsntPGPError (err)) {
					err = PGPSetKeyTrust (pkgi->CorpKey, 
											kPGPKeyTrust_Complete);
					// ignore errors here.  If key is axiomatic, 
					// setting trust will cause an error.
					err = kPGPError_NoErr; 
				}
			}
		}
		// commit everything now so if there is a problem
		// during keyserver stuff, at least the key is saved
		if (!PGPcomdlgErrorMessage (err)) {
			PGPCommitKeyRingChanges (KeySetMain);
			pkgi->iFinalResult = KCD_NEWKEY;
			if (bNewDefaultKey)
				pkgi->iFinalResult |= KCD_NEWDEFKEY;
		}
	}
	else {
		if (!pkgi->bCancelPending)
			PGPcomdlgErrorMessage (err);
	}

	pkgi->bInGeneration = FALSE;
	if (pkgi->bCancelPending) {
		PropSheet_PressButton (GetParent (pkgi->hWndWiz), PSBTN_CANCEL);
	}
	else {
		PostMessage (pkgi->hWndWiz, KM_M_GENERATION_COMPLETE, 0, 0);
	}
}


//----------------------------------------------------|
//  Check if message is dangerous to pass to passphrase edit box

BOOL WizCommonNewKeyPhraseMsgProc (HWND hWnd, UINT uMsg, 
								   WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_RBUTTONDOWN :
	case WM_LBUTTONDBLCLK :
	case WM_MOUSEMOVE :
	case WM_COPY :
	case WM_CUT :
	case WM_PASTE :
	case WM_CLEAR :
	case WM_CONTEXTMENU :
		return TRUE;

	case WM_LBUTTONDOWN :
		if (GetKeyState (VK_SHIFT) & 0x8000) return TRUE;
		break;

	case WM_PAINT :
		if (wParam) {
			SetBkColor ((HDC)wParam, GetSysColor (COLOR_WINDOW));
			if (bHideTyping) 
				SetTextColor ((HDC)wParam, GetSysColor (COLOR_WINDOW));
			else 
				SetTextColor ((HDC)wParam, GetSysColor (COLOR_WINDOWTEXT));
		}
		break; 

	case WM_SETFOCUS :
		SendMessage (hWnd, EM_SETSEL, 0xFFFF, 0xFFFF);
		break;

	case WM_KILLFOCUS :
		break;
	}
	return FALSE; 
} 


//----------------------------------------------------|
//  New passphrase 1 edit box subclass procedure

LRESULT APIENTRY WizPhrase1SubclassProc (HWND hWnd, UINT uMsg, 
								   WPARAM wParam, LPARAM lParam) {
	UINT				uQuality;
	CHAR				szBuf[256];
	LRESULT				lResult;

	if (WizCommonNewKeyPhraseMsgProc (hWnd, uMsg, wParam, lParam)) return 0;
	switch (uMsg) {
	case WM_GETTEXT :
		wParam = lPhrase1Len;
		lParam = (LPARAM)szPhrase1;
		break;

	case WM_CHAR :
		switch (wParam) {
		case VK_TAB :
			if (GetKeyState (VK_SHIFT) & 0x8000) 
				SetFocus (GetDlgItem (GetParent (hWnd), IDC_DUMMYSTOP));
			else 
				SetFocus (GetDlgItem (GetParent (hWnd), 
											IDC_EDIT_PASSPHRASE2));
			break;

		case VK_RETURN :
			{
				HWND hGrandParent = GetParent (GetParent (hWnd));
				PropSheet_PressButton (hGrandParent, PSBTN_NEXT);
			}
			break;


		default :
			lResult = CallWindowProc (wpOrigPhrase1Proc, 
						hWnd, uMsg, wParam, lParam); 
			CallWindowProc (wpOrigPhrase1Proc, 
						hWnd, WM_GETTEXT, sizeof(szBuf), (LPARAM)szBuf); 
			uQuality = PGPcomdlgPassphraseQuality (szBuf);
			memset (szBuf, 0, sizeof(szBuf));
			SendDlgItemMessage (GetParent (hWnd), IDC_PHRASEQUALITY, 
						PBM_SETPOS, uQuality, 0);
			return lResult;

		}
		break;

	}
	return CallWindowProc(wpOrigPhrase1Proc, hWnd, uMsg, wParam, lParam); 
} 


//----------------------------------------------------|
//  New passphrase 2 edit box subclass procedure

LRESULT APIENTRY WizPhrase2SubclassProc (HWND hWnd, UINT uMsg, 
								   WPARAM wParam, LPARAM lParam) {

	if (WizCommonNewKeyPhraseMsgProc (hWnd, uMsg, wParam, lParam)) return 0;
	switch (uMsg) {
	case WM_GETTEXT :
		wParam = lPhrase2Len;
		lParam = (LPARAM)szPhrase2;
		break;

	case WM_CHAR :
		switch (wParam) {
		case VK_TAB :
			if (GetKeyState (VK_SHIFT) & 0x8000) 
				SetFocus (GetDlgItem (GetParent (hWnd), IDC_EDIT_PASSPHRASE));
			else 
				SetFocus (GetDlgItem (GetParent (hWnd), 
										IDC_CHECK_HIDE_TYPING));
			break;

		case VK_RETURN :
			{
				HWND hGrandParent = GetParent (GetParent (hWnd));
				PropSheet_PressButton (hGrandParent, PSBTN_NEXT);
			}
			break;
		}
		break;
	}
	return CallWindowProc(wpOrigPhrase2Proc, hWnd, uMsg, wParam, lParam); 
} 


//----------------------------------------------------|
//  Signing passphrase edit box subclass procedure

LRESULT APIENTRY WizPhrase3SubclassProc (HWND hWnd, UINT uMsg, 
								   WPARAM wParam, LPARAM lParam) {

	if (WizCommonNewKeyPhraseMsgProc (hWnd, uMsg, wParam, lParam)) return 0;
	switch (uMsg) {
	case WM_GETTEXT :
		wParam = lPhrase1Len;
		lParam = (LPARAM)szPhrase1;
		break;

	case WM_CHAR :
		switch (wParam) {
		case VK_TAB :
			if (GetKeyState (VK_SHIFT) & 0x8000) 
				SetFocus (GetDlgItem (GetParent (hWnd), IDC_CHECK_SIGN_KEY));
			else 
				SetFocus (GetDlgItem (GetParent (hWnd), 
											IDC_CHECK_HIDE_TYPING));
			break;

		case VK_RETURN :
			{
				HWND hGrandParent = GetParent (GetParent (hWnd));
				PropSheet_PressButton (hGrandParent, PSBTN_NEXT);
			}
			break;
		}
		break;
	}
	return CallWindowProc(wpOrigPhrase1Proc, hWnd, uMsg, wParam, lParam); 
} 



//----------------------------------------------------|
//  see if selected date is valid

BOOL 
ValidateSelectedDate (HWND hwndDate, UINT uDaysToday, UINT* puDaysExpire) 
{
	UINT		uMaxDays;
	UINT		uDay, uMonth, uYear;
	struct tm	exptm;
	time_t		exptime;

	uYear = Date_GetYear (hwndDate);
	uMonth = Date_GetMonth (hwndDate);
	uDay = Date_GetDay (hwndDate);

	switch (uMonth) {
		case 1 :
		case 3 :
		case 5 :
		case 7 :
		case 8 :
		case 10 :
		case 12 :
			uMaxDays = 31;
			break;

		case 4 :
		case 6 :
		case 9 :
		case 11 :
			uMaxDays = 30;
			break;

		case 2 : 
			if(!(uYear % 4) && ((uYear % 100) || !(uYear % 400)))
				uMaxDays = 29;	// its a leap year
			else
				uMaxDays = 28;
			break;
	}

	exptm.tm_mday = uDay;
	exptm.tm_mon = uMonth -1;
	exptm.tm_year = uYear -1900;
	exptm.tm_hour = 12;
	exptm.tm_min = 0;
	exptm.tm_sec = 0;
	exptm.tm_isdst = -1;

	exptime = mktime (&exptm);
	if (exptime == (time_t)-1) return FALSE;

	*puDaysExpire = exptime / 86400; 
	if (*puDaysExpire <= uDaysToday) return FALSE;
	*puDaysExpire = *puDaysExpire - uDaysToday;

	if (uDay > uMaxDays) return FALSE;
	if (uYear > 2037) return FALSE;

	return TRUE;

}


//----------------------------------------------------|
//  Dialog procedure for "Finish" dialog

LRESULT WINAPI 
KeyWizardDoneDlgProc (HWND hDlg, UINT uMsg, 
					  WPARAM wParam, LPARAM lParam) 
{
	PKEYGENINFO		pkgi;
	BOOL			bReturnCode = FALSE;

	switch (uMsg) {
	case WM_INITDIALOG:
		{
			PROPSHEETPAGE *ppspMsgRec = (PROPSHEETPAGE *) lParam;
			pkgi = (PKEYGENINFO) ppspMsgRec->lParam;
			SetWindowLong (hDlg, GWL_USERDATA, (LPARAM)pkgi);
			break;
		}

	case WM_PAINT :
		pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
		if (pkgi->hPalette)
		{
			PAINTSTRUCT ps;
			HDC	hDC = BeginPaint (hDlg, &ps);
			SelectPalette (hDC, pkgi->hPalette, FALSE);
			RealizePalette (hDC);
			EndPaint (hDlg, &ps);
		}
		break;

	case WM_NOTIFY:
		{	
			LPNMHDR pnmh = (LPNMHDR) lParam;

			switch (pnmh->code) {
			case PSN_SETACTIVE:
				pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
				if (pkgi->bDoSend)
					PostMessage (GetParent (hDlg), 
						PSM_SETWIZBUTTONS, 0, PSWIZB_FINISH);
				else 
					PostMessage (GetParent (hDlg), 
						PSM_SETWIZBUTTONS, 0, PSWIZB_BACK|PSWIZB_FINISH);

				SendDlgItemMessage (hDlg, IDC_WIZBITMAP, STM_SETIMAGE, 
					IMAGE_BITMAP, (LPARAM) pkgi->hBitmap);
				if ((!pkgi->bDoSend) || (!pkgi->bSendComplete)) {
					CHAR szText[256];
					LoadString (g_hInst, IDS_KW_SENDLATER, 
						szText, sizeof(szText));
					SetDlgItemText(hDlg, IDC_STATIC_KS_TEXT, szText);
				}
				else {
					EnableWindow (GetDlgItem (GetParent (hDlg), IDCANCEL),
									FALSE);
				}
				SetWindowLong (hDlg, DWL_MSGRESULT, 0L);
				bReturnCode = TRUE;
				break;

			case PSN_WIZFINISH:
				pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
				pkgi->bFinishSelected = TRUE;
				SetWindowLong(hDlg, DWL_MSGRESULT, 0L);
				bReturnCode = TRUE;
				break;

			case PSN_HELP:
				WinHelp (hDlg, g_szHelpFile, HELP_CONTEXT, 
							IDH_PGPPK_WIZ_DONE); 
				break;

			case PSN_QUERYCANCEL:
				break;
			}
			break;
		}
	}

	return bReturnCode;
}


//----------------------------------------------------|
//  Dialog procedure for displaying keyserver communication status

LRESULT WINAPI 
KeyWizardSendToServerDlgProc (HWND hDlg, UINT uMsg, 
							  WPARAM wParam, LPARAM lParam) 
{
	PKEYGENINFO			pkgi;
	BOOL				bReturnCode = FALSE;
	static PGPKeySetRef	keysetSend;			

	switch (uMsg) {
	case WM_INITDIALOG:
		{
			PROPSHEETPAGE *ppspMsgRec = (PROPSHEETPAGE *) lParam;
			pkgi = (PKEYGENINFO) ppspMsgRec->lParam;
			SetWindowLong (hDlg, GWL_USERDATA, (LPARAM)pkgi);
			break;
		}

	case WM_PAINT :
		pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
		if (pkgi->hPalette) {
			PAINTSTRUCT ps;
			HDC	hDC = BeginPaint (hDlg, &ps);
			SelectPalette (hDC, pkgi->hPalette, FALSE);
			RealizePalette (hDC);
			EndPaint (hDlg, &ps);
		}
		DrawSendStatus (GetDlgItem (hDlg, IDC_PROGRESS), pkgi);
		break;
	
	case WM_TIMER :
		pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
		pkgi->iStatusValue += pkgi->iStatusDirection;
		if (pkgi->iStatusValue <= 0) {
			pkgi->iStatusValue = 0;
			pkgi->iStatusDirection = 1;
		}
		else if (pkgi->iStatusValue >= NUMLEDS-1) {
			pkgi->iStatusValue = NUMLEDS-1;
			pkgi->iStatusDirection = -1;
		}
		InvalidateRect (hDlg, NULL, FALSE);
		break;

	case WM_NOTIFY:
		pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
		{
			LPNMHDR pnmh = (LPNMHDR) lParam;

			switch (pnmh->code) {
			case PSN_SETACTIVE:
				pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
				if (!pkgi->bDoSend) {
					SetWindowLong(hDlg, DWL_MSGRESULT, -1L); // skip
					bReturnCode = TRUE;
				}
				else {
					pkgi->bSendInProgress = FALSE;
					pkgi->iStatusValue = -1;
					pkgi->iStatusDirection = 1;
					PostMessage (GetParent(hDlg), 
						PSM_SETWIZBUTTONS, 0, 0);
					SendDlgItemMessage (hDlg, IDC_WIZBITMAP, STM_SETIMAGE, 
						IMAGE_BITMAP, (LPARAM) pkgi->hBitmap);
					PGPNewSingletonKeySet (pkgi->Key, &keysetSend);
					PGPcomdlgSendKeysToServerNotify (pkgi->Context, hDlg, 
								NULL, KeySetMain, keysetSend);
					SetWindowLong(hDlg, DWL_MSGRESULT, 0L); 
					bReturnCode = TRUE;
				}
				break;

			case PSN_KILLACTIVE :
				KillTimer (hDlg, LEDTIMER);
				break;

			case PGPCOMDLG_SERVERPROGRESS :
				{
					PPGPcdSERVEREVENT pEvent = (PPGPcdSERVEREVENT)lParam;
					if (!(pkgi->bSendInProgress)) {
						pkgi->bSendInProgress = TRUE;
						pkgi->iStatusValue = 0;
						if (pEvent->step == PGPCOMDLG_SERVERINFINITE) {
							pkgi->iStatusDirection = 1;
							SetTimer (hDlg, LEDTIMER, LEDTIMERPERIOD, NULL);
						}
						else {
							pkgi->iStatusDirection = 0;
							pkgi->iStatusValue = 0;
						}
					}
					else {
						if (pEvent->step != PGPCOMDLG_SERVERINFINITE) {
							pkgi->iStatusDirection = 0;
							pkgi->iStatusValue = (pEvent->step * 9) /
													pEvent->total;
							InvalidateRect (GetDlgItem (hDlg, IDC_PROGRESS),
											NULL, FALSE);
						}
					}
					SetDlgItemText (hDlg, IDC_PROGRESSTEXT, 
											pEvent->szmessage);
					pEvent->cancel = pkgi->bCancelPending;
					return FALSE;
				}

			case PGPCOMDLG_SERVERDONE : 
					pkgi->bSendComplete = TRUE;
			case PGPCOMDLG_SERVERABORT : 
			case PGPCOMDLG_SERVERERROR : 
				pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
				{
					PPGPcdSERVEREVENT	pEvent = (PPGPcdSERVEREVENT)lParam;

					KillTimer (hDlg, LEDTIMER);
					pkgi->iStatusValue = -1;
					pkgi->iStatusDirection = 1;
					InvalidateRect (GetDlgItem (hDlg, IDC_PROGRESS),
											NULL, FALSE);

					SetDlgItemText (hDlg, IDC_PROGRESSTEXT, 
											pEvent->szmessage);
					PostMessage (GetParent(hDlg), 
						PSM_SETWIZBUTTONS, 0, PSWIZB_NEXT);
					EnableWindow (GetDlgItem (GetParent (hDlg), IDCANCEL),
									FALSE);
					ShowWindow (GetDlgItem (hDlg, IDC_PROGRESS), SW_HIDE);

					PGPFreeKeySet (keysetSend);

					pEvent->cancel = pkgi->bCancelPending;
					return FALSE;
				}

			case PSN_HELP:
				WinHelp (hDlg, g_szHelpFile, HELP_CONTEXT, 
							IDH_PGPPK_WIZ_SEND); 
				break;

			case PSN_QUERYCANCEL:
				pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
				if (!pkgi->bSendComplete) {
					CHAR				sz[128];

					pkgi->bCancelPending = TRUE;
					LoadString (g_hInst, IDS_KW_CANCELING, sz, sizeof(sz));
					SetDlgItemText (hDlg, IDC_PROGRESSTEXT, sz);
					SetWindowLong (hDlg, DWL_MSGRESULT, -1L); 
					bReturnCode = TRUE;
				}
				break;
			}
			break;
		}
	}

	return bReturnCode;
}


//----------------------------------------------------|
//  Dialog procedure for querying about sending to keyserver

LRESULT WINAPI 
KeyWizardPreSendDlgProc (HWND hDlg, UINT uMsg, 
						 WPARAM wParam, LPARAM lParam) 
{
	PKEYGENINFO		pkgi;
	BOOL			bReturnCode = FALSE;

	switch (uMsg) {
	case WM_INITDIALOG:
		{
			PROPSHEETPAGE *ppspMsgRec = (PROPSHEETPAGE *) lParam;
			pkgi = (PKEYGENINFO) ppspMsgRec->lParam;
			SetWindowLong (hDlg, GWL_USERDATA, (LPARAM)pkgi);

			SendDlgItemMessage (hDlg, IDC_CHECK_SEND, 
				BM_SETCHECK, BST_UNCHECKED, 0);
			break;
		}

	case WM_PAINT :
		pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
		if (pkgi->hPalette)
		{
			PAINTSTRUCT ps;
			HDC	hDC = BeginPaint (hDlg, &ps);
			SelectPalette (hDC, pkgi->hPalette, FALSE);
			RealizePalette (hDC);
			EndPaint (hDlg, &ps);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD (wParam)) {
		case IDC_CHECK_SEND:
			pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
			if (IsDlgButtonChecked (hDlg, IDC_CHECK_SEND) == 
				BST_CHECKED) {
				pkgi->bDoSend = TRUE;
			}
			else {
				pkgi->bDoSend = FALSE;
			}
			break;
		}
		break;

	case WM_NOTIFY:
		{
			LPNMHDR pnmh = (LPNMHDR) lParam;

			switch (pnmh->code) {
			case PSN_SETACTIVE:
				pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
				PostMessage (GetParent(hDlg), 
					PSM_SETWIZBUTTONS, 0, PSWIZB_NEXT);
				SendDlgItemMessage (hDlg, IDC_WIZBITMAP, STM_SETIMAGE, 
					IMAGE_BITMAP, (LPARAM) pkgi->hBitmap);
				SetWindowLong (hDlg, DWL_MSGRESULT, 0L);
				bReturnCode = TRUE;
				break;

			case PSN_KILLACTIVE :
				pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
				if (IsDlgButtonChecked (hDlg, IDC_CHECK_SEND) 
												== BST_CHECKED) {
					pkgi->bDoSend = TRUE;
				}
				else pkgi->bDoSend = FALSE;
				SetWindowLong (hDlg, DWL_MSGRESULT, 0L);
				bReturnCode = TRUE;
				break;

			case PSN_HELP:
				WinHelp (hDlg, g_szHelpFile, HELP_CONTEXT, 
							IDH_PGPPK_WIZ_PRESEND); 
				break;

			case PSN_QUERYCANCEL:
				break;
			}
			break;
		}
	}

	return bReturnCode;
}


//----------------------------------------------------|
//  Dialog procedure for querying user about signing new
//  key with old key

LRESULT WINAPI 
KeyWizardSignOldDlgProc (HWND hDlg, UINT uMsg, 
						 WPARAM wParam, LPARAM lParam) 
{
	PKEYGENINFO		pkgi;
	BOOL			bReturnCode = FALSE;
	PGPBoolean		bNeedsPhrase;
	static HBRUSH	hBrushEdit;

	switch (uMsg) {
	case WM_INITDIALOG:
		{
			PROPSHEETPAGE *ppspMsgRec = (PROPSHEETPAGE *) lParam;
			pkgi = (PKEYGENINFO) ppspMsgRec->lParam;
			SetWindowLong (hDlg, GWL_USERDATA, (LPARAM)pkgi);

			wpOrigPhrase1Proc = (WNDPROC) SetWindowLong (GetDlgItem (hDlg, 
										IDC_EDIT_PASSPHRASE), 
										GWL_WNDPROC, 
										(LONG) WizPhrase3SubclassProc); 

			hBrushEdit = CreateSolidBrush (GetSysColor (COLOR_WINDOW));
			break;
		}

	case WM_PAINT :
		pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
		if (pkgi->hPalette)
		{
			PAINTSTRUCT ps;
			HDC	hDC = BeginPaint (hDlg, &ps);
			SelectPalette (hDC, pkgi->hPalette, FALSE);
			RealizePalette (hDC);
			EndPaint (hDlg, &ps);
		}
		break;

	case WM_CTLCOLOREDIT:
		if (((HWND)lParam == GetDlgItem (hDlg, IDC_EDIT_PASSPHRASE)) ||
			((HWND)lParam == GetDlgItem (hDlg, IDC_EDIT_PASSPHRASE2))) {
			SetBkColor ((HDC)wParam, GetSysColor (COLOR_WINDOW));
			if (bHideTyping) 
				SetTextColor ((HDC)wParam, GetSysColor(COLOR_WINDOW));
			else 
				SetTextColor ((HDC)wParam, GetSysColor(COLOR_WINDOWTEXT));
			return (BOOL)hBrushEdit;
		}
		break;

	case WM_DESTROY: 
		SetWindowLong (GetDlgItem (hDlg, IDC_EDIT_PASSPHRASE), 
						GWL_WNDPROC, (LONG)wpOrigPhrase1Proc); 
		DeleteObject (hBrushEdit);
		PGPkeysWipeEditBox (hDlg, IDC_EDIT_PASSPHRASE);
		break; 

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_CHECK_HIDE_TYPING:
			if (IsDlgButtonChecked (hDlg, IDC_CHECK_HIDE_TYPING) ==
					BST_CHECKED) 
				bHideTyping = TRUE;
			else 
				bHideTyping = FALSE;
			InvalidateRect (GetDlgItem (hDlg, IDC_EDIT_PASSPHRASE), 
								NULL, TRUE);
			break;
		}
		break;

	case WM_NOTIFY:
		{
			LPNMHDR pnmh = (LPNMHDR) lParam;

			switch (pnmh->code) {
			case PSN_SETACTIVE:
				pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
				if (!pkgi->OldKey) {
					SetWindowLong(hDlg, DWL_MSGRESULT, -1L); // skip this page
					bReturnCode = TRUE;
				}
				else {
					PostMessage (GetParent(hDlg), 
						PSM_SETWIZBUTTONS, 0, PSWIZB_NEXT);
					SendDlgItemMessage (hDlg, IDC_WIZBITMAP, STM_SETIMAGE, 
						IMAGE_BITMAP, (LPARAM) pkgi->hBitmap);
					bHideTyping = TRUE;
					CheckDlgButton (hDlg, IDC_CHECK_HIDE_TYPING, 
						BST_CHECKED);
					SendDlgItemMessage (hDlg, IDC_CHECK_SIGN_KEY, 
						BM_SETCHECK, BST_CHECKED, 0);
					PGPGetKeyBoolean (pkgi->OldKey, 
							kPGPKeyPropNeedsPassphrase, &bNeedsPhrase);
					if (!bNeedsPhrase) {
						SetDlgItemText (hDlg, IDC_EDIT_PASSPHRASE, "");
						EnableWindow (GetDlgItem (hDlg,IDC_EDIT_PASSPHRASE),
										FALSE);
						EnableWindow (GetDlgItem (hDlg,IDC_CHECK_HIDE_TYPING),
										FALSE);
					}
					SetWindowLong (hDlg, DWL_MSGRESULT, 0L);
					bReturnCode = TRUE;
				}
				break;

			case PSN_KILLACTIVE:
				if (SendDlgItemMessage (hDlg, IDC_CHECK_SIGN_KEY, 
								BM_GETCHECK, 0, 0) == BST_CHECKED) {
					BOOL bAllOk = FALSE;
					PGPUserIDRef UserID;
					CHAR szDummy[4];

					pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
	
					if (szPhrase1) {
						secFree (szPhrase1);
						szPhrase1 = NULL;
					}
					lPhrase1Len = SendDlgItemMessage (hDlg, 
									IDC_EDIT_PASSPHRASE, 
									WM_GETTEXTLENGTH, 0, 0) +1;
					szPhrase1 = secAlloc (lPhrase1Len);
					if (szPhrase1) 
						GetDlgItemText (hDlg, IDC_EDIT_PASSPHRASE, 
										szDummy, sizeof(szDummy));
 
					if (szPhrase1) {
						PGPGetPrimaryUserID (pkgi->Key, &UserID);

						if (UserID) {
							PGPError err = kPGPError_BadPassphrase;

							// make sure we have enough entropy
							PGPcomdlgRandom (pkgi->Context, pkgi->hWndWiz, 0);

							err = PGPSignUserID (UserID, pkgi->OldKey, 
								PGPOPassphrase (pkgi->Context, szPhrase1),
								PGPOLastOption (pkgi->Context));
							if (err == kPGPError_BadPassphrase) {
								PGPkeysMessageBox (hDlg, IDS_KW_TITLE,
											IDS_KW_BAD_PASSPHRASE, 
											MB_OK | MB_ICONERROR);
							}
							else {
								if (err != kPGPError_NoErr)
									PGPcomdlgErrorMessage (err);
								else
									bAllOk = TRUE;
							}
						}					   
						secFree (szPhrase1);
						szPhrase1 = NULL;
						PGPkeysWipeEditBox (hDlg, IDC_EDIT_PASSPHRASE);
						SetDlgItemText (hDlg, IDC_EDIT_PASSPHRASE, "");
					}
					if (!bAllOk) {
						SetWindowLong (hDlg, DWL_MSGRESULT, -1L);
						bReturnCode = TRUE;
					}
				}
				break;

			case PSN_HELP:
				WinHelp (hDlg, g_szHelpFile, HELP_CONTEXT, 
							IDH_PGPPK_WIZ_SIGNOLD); 
				break;

			case PSN_QUERYCANCEL:
				break;
			}
			break;
		}
	}

	return bReturnCode;
}


//----------------------------------------------------|
//  Dialog procedure for actually generating the key(s)

LRESULT WINAPI 
KeyWizardGenerationDlgProc (HWND hDlg, UINT uMsg, 
							WPARAM wParam, LPARAM lParam) 
{
	PKEYGENINFO		pkgi;
	BOOL			bReturnCode = FALSE;
	DWORD			dwThreadID;
	char			szPhaseString[128];
	static BOOL		bAVIFinished;
	static BOOL		bAVIStarted;

	switch (uMsg) {
	case WM_INITDIALOG:
		{
			PROPSHEETPAGE *ppspMsgRec = (PROPSHEETPAGE *) lParam;
			pkgi = (PKEYGENINFO) ppspMsgRec->lParam;
			SetWindowLong (hDlg, GWL_USERDATA, (LPARAM)pkgi);

			pkgi->uWorkingPhase = IDS_KEYGENPHASE1;
			LoadString (g_hInst, pkgi->uWorkingPhase, 
					   szPhaseString, sizeof(szPhaseString));
			SetDlgItemText (hDlg, IDC_PHASE, szPhaseString);

			pkgi->hWndWiz = hDlg;
			pkgi->bInGeneration = TRUE;

			// Kick off generation proc, here
			_beginthreadex (NULL, 0, 
				(LPTHREAD_START_ROUTINE)KeyGenerationThread, 
				(void *) pkgi, 0, &dwThreadID);
			break;
		}

	case WM_PAINT :
		pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
		if (pkgi->hPalette)
		{
			PAINTSTRUCT ps;
			HDC	hDC = BeginPaint (hDlg, &ps);
			SelectPalette (hDC, pkgi->hPalette, FALSE);
			RealizePalette (hDC);
			EndPaint (hDlg, &ps);
		}
		break;

	case KM_M_CHANGEPHASE:
		bReturnCode = TRUE;
		LoadString (g_hInst, lParam, szPhaseString, sizeof(szPhaseString));
		SetDlgItemText (hDlg, IDC_PHASE, szPhaseString);
		break;

	case WM_TIMER :
		if (wParam == AVI_TIMER) {
			KillTimer (hDlg, AVI_TIMER);
			bReturnCode = TRUE;
			if (bAVIStarted) {
				if (!bAVIFinished) {
					bAVIFinished = TRUE;
					pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
					if (!pkgi->bInGeneration) 
						PostMessage (hDlg, KM_M_GENERATION_COMPLETE, 0, 0);
				}
			}
			else {
				CHAR szFile[32];
				CHAR szAnimationFile[_MAX_PATH], *p;
				lstrcpy (szAnimationFile, g_szHelpFile);

				p = strrchr (szAnimationFile, '\\');
				if (!p)
					p = szAnimationFile;
				else
					++p;

				*p = '\0';
				LoadString (g_hInst, IDS_ANIMATIONFILE,
					szFile, sizeof(szFile));
				lstrcat (szAnimationFile, szFile);

				Animate_Open (GetDlgItem (hDlg, IDC_WORKINGAVI), 
									szAnimationFile);
				Animate_Play (GetDlgItem (hDlg, IDC_WORKINGAVI), 
										0, -1, -1);

				SetTimer (hDlg, AVI_TIMER, AVI_RUNTIME, NULL);
				bAVIStarted = TRUE;
			}
		}
		break;
	
	case KM_M_GENERATION_COMPLETE:
		pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
		if (bAVIFinished) 
			Animate_Close (GetDlgItem (hDlg, IDC_WORKINGAVI));
	
		if (pkgi->bCancelPending) {
			LoadString (g_hInst, IDS_KW_CANCELING, 
				szPhaseString, sizeof(szPhaseString));
		}
		else {
			if (pkgi->iFinalResult) {
				LoadString (g_hInst, IDS_KW_COMPLETE, 
					szPhaseString, sizeof(szPhaseString));
				SendMessage (GetParent(hDlg), 
					PSM_SETWIZBUTTONS, 0, PSWIZB_NEXT);
			}
			else {
				LoadString (g_hInst, IDS_KW_UNABLETOCOMPLETE, 
					szPhaseString, sizeof(szPhaseString));
			}
		}
		SetDlgItemText (hDlg, IDC_PHASE, szPhaseString);
						
		bReturnCode = TRUE;
		break;
		
	case WM_COMMAND:
		switch (HIWORD(wParam)) {
		case BN_CLICKED:
			if (LOWORD(wParam) == IDC_RADIO_CUSTOM_DAYS) {
				SendDlgItemMessage (hDlg, IDC_EDIT_CUSTOM_DAYS, 
						WM_ENABLE, TRUE, 0);
			}
			else {
				SendDlgItemMessage (hDlg, IDC_EDIT_CUSTOM_DAYS, 
						WM_ENABLE, FALSE, 0);
			}
			break;
		}
		break;

	case WM_NOTIFY:
		{
			LPNMHDR pnmh = (LPNMHDR) lParam;

			switch (pnmh->code) {
			case PSN_SETACTIVE:
				pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
				PostMessage (GetParent(hDlg), 
						PSM_SETWIZBUTTONS, 0, 0);
				SendDlgItemMessage (hDlg, IDC_WIZBITMAP, STM_SETIMAGE, 
						IMAGE_BITMAP, (LPARAM) pkgi->hBitmap);
				bAVIFinished = FALSE;
				bAVIStarted = FALSE;
				SetTimer (hDlg, AVI_TIMER, 100, NULL);  // delay a few ms
														// before drawing
														// AVI
				SetWindowLong (hDlg, DWL_MSGRESULT, 0L);
				bReturnCode = TRUE;
				break;

			case PSN_KILLACTIVE:
				LoadString (g_hInst, IDS_KW_CANCELING, 
						szPhaseString, sizeof(szPhaseString));
				SetDlgItemText (hDlg, IDC_PHASE, szPhaseString);
				bAVIFinished = TRUE;
				Animate_Close (GetDlgItem (hDlg, IDC_WORKINGAVI));
				break;

			case PSN_HELP:
				WinHelp (hDlg, g_szHelpFile, HELP_CONTEXT, 
							IDH_PGPPK_WIZ_GENERATION); 
				break;

			case PSN_QUERYCANCEL:
				//If we're generating a key, don't let the user press
				//cancel without asking.  If he says, "yes, I want to cancel,"
				//then we'll still reject the message, but set CancelPending
				//to TRUE, so that the next time the library comes around, we
				//can nuke the thread.  The thread will then clear the
				//InGeneration flag and re-send us the cancel message.
				pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
				
				if (pkgi->bInGeneration) {
					if (!pkgi->bCancelPending &&
						(PGPkeysMessageBox (hDlg, IDS_KW_TITLE, 
								IDS_KW_CONFIRM_CANCEL,
								MB_YESNO | MB_ICONQUESTION) == IDYES)) {
						if (pkgi->bInGeneration) {
							pkgi->bCancelPending = TRUE;
							PostMessage (hDlg, KM_M_GENERATION_COMPLETE, 
									0, 0L);
						}
					}
				}
				if (pkgi->bInGeneration) {
					SetWindowLong (hDlg, DWL_MSGRESULT, 1L); 
					bReturnCode = TRUE;
				}
				else {
					SetWindowLong (hDlg, DWL_MSGRESULT, 0L); 
					bReturnCode = TRUE;
				}
				break;
			}
			break;
		}
	}

	return bReturnCode;
}


//----------------------------------------------------|
//  Dialog procedure for getting entropy
//  from user

LRESULT WINAPI 
KeyWizardRandobitsDlgProc (HWND hDlg, UINT uMsg, 
							WPARAM wParam, LPARAM lParam) 
{
	PKEYGENINFO		pkgi;
	BOOL			bReturnCode = FALSE;

	switch (uMsg) {
	case WM_INITDIALOG:
		{
			PROPSHEETPAGE *ppspMsgRec = (PROPSHEETPAGE *) lParam;
			pkgi = (PKEYGENINFO) ppspMsgRec->lParam;
			SetWindowLong (hDlg, GWL_USERDATA, (LPARAM)pkgi);
			break;
		}

	case WM_PAINT :
		pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
		if (pkgi->hPalette)
		{
			PAINTSTRUCT ps;
			HDC	hDC = BeginPaint (hDlg, &ps);
			SelectPalette (hDC, pkgi->hPalette, FALSE);
			RealizePalette (hDC);
			EndPaint (hDlg, &ps);
		}
		break;

	case WM_MOUSEMOVE:
		{
			INT		iPercentComplete = 0;
			FLOAT	fTotal, fSoFar;

			pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
			if (lParam) {
				PGPGlobalRandomPoolAddMouse (LOWORD (lParam), 
											 HIWORD (lParam));
			}

			if (pkgi->lRandomBitsNeeded) {
				fSoFar = (float) PGPGlobalRandomPoolGetEntropy();
				fTotal = (float) pkgi->lRandomBitsNeeded;
				fSoFar -= (float) pkgi->lOriginalEntropy;
				fTotal -= (float) pkgi->lOriginalEntropy;
				iPercentComplete = (INT) ((fSoFar / fTotal) * 100.0);

				if (fSoFar >= fTotal) {
					pkgi->lRandomBitsNeeded = 0;
					iPercentComplete = 100;
					hWndCollectEntropy = NULL;
					SendMessage (GetParent(hDlg), PSM_SETWIZBUTTONS, 
										0, PSWIZB_NEXT|PSWIZB_BACK);
				}
				SendDlgItemMessage (hDlg, IDC_PROGRESS, PBM_SETPOS, 
										iPercentComplete, 0);
			}
			break;
		}

	case WM_NOTIFY:
		{
			LPNMHDR pnmh = (LPNMHDR) lParam;

			switch(pnmh->code) {
			case PSN_SETACTIVE:
				pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
				PostMessage(GetParent(hDlg), 
						PSM_SETWIZBUTTONS, 0, 0);
				SendDlgItemMessage(hDlg, IDC_WIZBITMAP, STM_SETIMAGE, 
						IMAGE_BITMAP, (LPARAM) pkgi->hBitmap);
				{
					LONG			lTotalBitsNeeded, lRandPoolSize;
					PGPBoolean		bFastGen;
					PGPContextRef	ctx = pkgi->Context;

					hWndCollectEntropy = hDlg;

					// Check to see if there are random bits needed
					PGPGetPrefBoolean (pkgi->PrefRefClient, 
										kPGPPrefFastKeyGen,
										&bFastGen);
					lTotalBitsNeeded = 
						PGPGetKeyEntropyNeeded (ctx,
							PGPOKeyGenParams (ctx, pkgi->uKeyType, 
													pkgi->uKeySize),
							PGPOKeyGenFast (ctx, bFastGen),
							PGPOLastOption (ctx));
					if (pkgi->uKeyType == kPGPPublicKeyAlgorithm_DSA) {
						lTotalBitsNeeded += 
							PGPGetKeyEntropyNeeded (pkgi->Context,
								PGPOKeyGenParams (ctx, 
										kPGPPublicKeyAlgorithm_ElGamal, 
										pkgi->uSubKeySize),
								PGPOKeyGenFast (ctx, bFastGen),
								PGPOLastOption (ctx));
					}
					lRandPoolSize = PGPGlobalRandomPoolGetSize ();

					pkgi->lRandomBitsNeeded = 
								min (lTotalBitsNeeded, lRandPoolSize);
				}
				if ((pkgi->lRandomBitsNeeded -
							(LONG)PGPGlobalRandomPoolGetEntropy()) > 0) {
					pkgi->lOriginalEntropy = PGPGlobalRandomPoolGetEntropy();
					SendDlgItemMessage (hDlg, IDC_PROGRESS, 
							PBM_SETRANGE, 0, MAKELPARAM(0, 100));
					SendDlgItemMessage (hDlg, IDC_PROGRESS, 
							PBM_SETPOS, 0, 0);
					SetWindowLong(hDlg, DWL_MSGRESULT, 0L);
					bReturnCode = TRUE;
				}
				else {
					SetWindowLong (hDlg, DWL_MSGRESULT, -1L); // skip page
					bReturnCode = TRUE;
					hWndCollectEntropy = NULL;
				}
				break;

			case PSN_KILLACTIVE:
				SetWindowLong (hDlg, DWL_MSGRESULT, 0);
				bReturnCode = TRUE;
				hWndCollectEntropy = NULL;
				break;

			case PSN_HELP:
				WinHelp (hDlg, g_szHelpFile, HELP_CONTEXT, 
							IDH_PGPPK_WIZ_RANDOBITS); 
				break;

			case PSN_QUERYCANCEL:
				hWndCollectEntropy = NULL;
				break;
			}
			break;
		}
	}

	return bReturnCode;
}


//----------------------------------------------------|
//  Dialog procedure for "Bad Passphrase" dialog

LRESULT WINAPI 
KeyWizardBadPassphraseDlgProc (HWND hDlg, UINT uMsg, 
					  WPARAM wParam, LPARAM lParam) 
{
	PKEYGENINFO		pkgi;
	BOOL			bReturnCode = FALSE;

	switch (uMsg) {
	case WM_INITDIALOG:
		{
			PROPSHEETPAGE *ppspMsgRec = (PROPSHEETPAGE *) lParam;
			pkgi = (PKEYGENINFO) ppspMsgRec->lParam;
			SetWindowLong (hDlg, GWL_USERDATA, (LPARAM)pkgi);

			break;
		}

	case WM_PAINT :
		pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
		if (pkgi->hPalette)
		{
			PAINTSTRUCT ps;
			HDC	hDC = BeginPaint (hDlg, &ps);
			SelectPalette (hDC, pkgi->hPalette, FALSE);
			RealizePalette (hDC);
			EndPaint (hDlg, &ps);
		}
		break;

	case WM_NOTIFY:
		{	
			LPNMHDR pnmh		= (LPNMHDR) lParam;
			BOOL	bPhraseOK	= TRUE;
			INT		iShow;
			CHAR	sz1[128], sz2[128];

			switch (pnmh->code) {
			case PSN_SETACTIVE:
			{
				BOOL	bRejected;
				BOOL	bLengthRejected, bQualityRejected, bConfirmRejected;
				INT		ids;

				pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
				if (pkgi->uPhraseFlags == 0) {
					SetWindowLong (hDlg, DWL_MSGRESULT, -1L); // skip page
					bReturnCode = TRUE;
					break;
				}

				SendDlgItemMessage (hDlg, IDC_WIZBITMAP, STM_SETIMAGE, 
					IMAGE_BITMAP, (LPARAM) pkgi->hBitmap);

				if (pkgi->bMinPhraseLength) ids = IDS_REQPHRASELENGTH;
				else ids = IDS_SUGPHRASELENGTH;
				LoadString (g_hInst, ids, sz1, sizeof(sz1));
				wsprintf (sz2, sz1, pkgi->iMinPhraseLength);
				SetDlgItemText (hDlg, IDC_BADLENGTH, sz2);

				if (pkgi->bMinPhraseQuality) ids = IDS_REQPHRASEQUALITY;
				else ids = IDS_SUGPHRASEQUALITY;
				LoadString (g_hInst, ids, sz1, sizeof(sz1));
				wsprintf (sz2, sz1, pkgi->iMinPhraseQuality);
				SetDlgItemText (hDlg, IDC_BADQUALITY1, sz2);

				bRejected = FALSE;
				bLengthRejected = FALSE;
				bQualityRejected = FALSE;
				bConfirmRejected = FALSE;
				if ((pkgi->bMinPhraseLength) && 
					(pkgi->uPhraseFlags & BADPHRASE_LENGTH)) {
					bLengthRejected = TRUE;
					bRejected = TRUE;
				}
				if ((pkgi->bMinPhraseQuality) && 
					(pkgi->uPhraseFlags & BADPHRASE_QUALITY)) {
					bQualityRejected = TRUE;
					bRejected = TRUE;
				}
				if (pkgi->uPhraseFlags & BADPHRASE_CONFIRMATION) {
					bConfirmRejected = TRUE;
					bRejected = TRUE;
				}

				if (bRejected) {
					LoadString (g_hInst, IDS_PHRASEREJECTED, 
													sz1, sizeof(sz1));
					SetDlgItemText (hDlg, IDC_REJECTTEXT, sz1);
					PostMessage (GetParent (hDlg), 
						PSM_SETWIZBUTTONS, 0, PSWIZB_BACK);
				}
				else {
					LoadString (g_hInst, IDS_PHRASEWARNED, 
													sz1, sizeof(sz1));
					SetDlgItemText (hDlg, IDC_REJECTTEXT, sz1);
					PostMessage (GetParent (hDlg), 
						PSM_SETWIZBUTTONS, 0, PSWIZB_BACK|PSWIZB_NEXT);
				}

				iShow = SW_HIDE;
				if (pkgi->uPhraseFlags & BADPHRASE_LENGTH) 
					if (!bRejected || bLengthRejected)
						iShow = SW_SHOW;
				ShowWindow (GetDlgItem (hDlg, IDC_BADLENGTH), iShow);
				ShowWindow (GetDlgItem (hDlg, IDC_BOX1), iShow);

				if (iShow == SW_HIDE) {
					if (pkgi->uPhraseFlags & BADPHRASE_CONFIRMATION)
						if (!bRejected || bConfirmRejected)
							iShow = SW_SHOW;
					ShowWindow (GetDlgItem (hDlg, IDC_BADCONFIRM), iShow);
					ShowWindow (GetDlgItem (hDlg, IDC_BOX1), iShow);
				}
				else
					ShowWindow (GetDlgItem (hDlg, IDC_BADCONFIRM), SW_HIDE);

				iShow = SW_HIDE;
				if (pkgi->uPhraseFlags & BADPHRASE_QUALITY)
					if (!bRejected || bQualityRejected)
						iShow = SW_SHOW;
				ShowWindow (GetDlgItem (hDlg, IDC_BADQUALITY1), iShow);
				ShowWindow (GetDlgItem (hDlg, IDC_BADQUALITY2), iShow);
				ShowWindow (GetDlgItem (hDlg, IDC_BOX2), iShow);

				SetWindowLong (hDlg, DWL_MSGRESULT, 0L);
				bReturnCode = TRUE;
				break;
			}

			case PSN_HELP:
				WinHelp (hDlg, g_szHelpFile, HELP_CONTEXT, 
							IDH_PGPPK_WIZ_BADPHRASE); 
				break;

			case PSN_QUERYCANCEL:
				break;
			}
			break;
		}
	}

	return bReturnCode;
}


//----------------------------------------------------|
//  Dialog procedure for getting passphrase
//  from user

LRESULT WINAPI 
KeyWizardPassphraseDlgProc (HWND hDlg, UINT uMsg, 
							WPARAM wParam, LPARAM lParam) 
{
	PKEYGENINFO		pkgi;
	BOOL			bReturnCode = FALSE;
	CHAR			szDummy[4];
	static HBRUSH	hBrushEdit;

	switch (uMsg) {
	case WM_INITDIALOG:
		{
			CHAR sz1[256], sz2[256];
			PROPSHEETPAGE *ppspMsgRec = (PROPSHEETPAGE *) lParam;
			pkgi = (PKEYGENINFO) ppspMsgRec->lParam;
			SetWindowLong (hDlg, GWL_USERDATA, (LPARAM)pkgi);

			wpOrigPhrase1Proc = (WNDPROC) SetWindowLong (GetDlgItem (hDlg, 
							IDC_EDIT_PASSPHRASE), 
							GWL_WNDPROC, 
							(LONG) WizPhrase1SubclassProc); 
			wpOrigPhrase2Proc = (WNDPROC) SetWindowLong (GetDlgItem (hDlg, 
							IDC_EDIT_PASSPHRASE2), 
							GWL_WNDPROC, 
							(LONG) WizPhrase2SubclassProc); 

			hBrushEdit = CreateSolidBrush (GetSysColor (COLOR_WINDOW));

			LoadString (g_hInst, IDS_PHRASELENGTHTEXT, sz1, sizeof(sz1));
			wsprintf (sz2, sz1, pkgi->iMinPhraseLength);
			SetDlgItemText (hDlg, IDC_PHRASELENGTHTEXT, sz2);
			break;
		}

	case WM_PAINT :
		pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
		if (pkgi->hPalette)
		{
			PAINTSTRUCT ps;
			HDC	hDC = BeginPaint (hDlg, &ps);
			SelectPalette (hDC, pkgi->hPalette, FALSE);
			RealizePalette (hDC);
			EndPaint (hDlg, &ps);
		}
		break;

	case WM_CTLCOLOREDIT:
		if (((HWND)lParam == GetDlgItem (hDlg, IDC_EDIT_PASSPHRASE)) ||
			((HWND)lParam == GetDlgItem (hDlg, IDC_EDIT_PASSPHRASE2))) {
			SetBkColor ((HDC)wParam, GetSysColor (COLOR_WINDOW));
			if (bHideTyping) 
				SetTextColor ((HDC)wParam, GetSysColor (COLOR_WINDOW));
			else 
				SetTextColor ((HDC)wParam, GetSysColor (COLOR_WINDOWTEXT));
			return (BOOL)hBrushEdit;
		}
		break;

	case WM_DESTROY: 
		SetWindowLong(GetDlgItem(hDlg, IDC_EDIT_PASSPHRASE),
						GWL_WNDPROC, (LONG)wpOrigPhrase1Proc); 
		SetWindowLong(GetDlgItem(hDlg, IDC_EDIT_PASSPHRASE2), 
						GWL_WNDPROC, (LONG)wpOrigPhrase2Proc);
		DeleteObject (hBrushEdit);
		PGPkeysWipeEditBox (hDlg, IDC_EDIT_PASSPHRASE);
		PGPkeysWipeEditBox (hDlg, IDC_EDIT_PASSPHRASE2);
		break; 

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDC_CHECK_HIDE_TYPING:
			if (IsDlgButtonChecked (hDlg, IDC_CHECK_HIDE_TYPING) ==
							BST_CHECKED) 
				bHideTyping = TRUE;
			else 
				bHideTyping = FALSE;

			InvalidateRect (GetDlgItem (hDlg, IDC_EDIT_PASSPHRASE), 
											NULL, TRUE);
			InvalidateRect (GetDlgItem (hDlg, IDC_EDIT_PASSPHRASE2), 
											NULL, TRUE);
			break;
		}
		break;

	case WM_NOTIFY:
		{
			LPNMHDR pnmh = (LPNMHDR) lParam;

			switch (pnmh->code) {
			case PSN_SETACTIVE:
				pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
				PostMessage (GetParent(hDlg), 
						PSM_SETWIZBUTTONS, 0, PSWIZB_BACK|PSWIZB_NEXT);
				SendDlgItemMessage (hDlg, IDC_WIZBITMAP, STM_SETIMAGE, 
						IMAGE_BITMAP, (LPARAM) pkgi->hBitmap);
				SetWindowLong (hDlg, DWL_MSGRESULT, 0L);
				bHideTyping = TRUE;
				CheckDlgButton (hDlg, IDC_CHECK_HIDE_TYPING, BST_CHECKED);
				if (pkgi->bMinPhraseQuality) {
					SendDlgItemMessage (hDlg, IDC_MINPHRASEQUALITY, 
								PBM_SETRANGE, 0, MAKELPARAM (0, 100));
					SendDlgItemMessage (hDlg, IDC_MINPHRASEQUALITY, 
								PBM_SETPOS, pkgi->iMinPhraseQuality, 0);
				}
				else {
					ShowWindow (GetDlgItem (hDlg, IDC_MINPHRASEQUALITY), 
										SW_HIDE);
					ShowWindow (GetDlgItem (hDlg, IDC_MINQUALITYTEXT), 
										SW_HIDE);
				}
				SendDlgItemMessage (hDlg, IDC_PHRASEQUALITY, PBM_SETRANGE, 
										0, MAKELPARAM (0, 100));
				SendDlgItemMessage (hDlg, IDC_PHRASEQUALITY, PBM_SETPOS, 
												0, 0);
				bReturnCode = TRUE;
				break;

			case PSN_WIZNEXT:
				pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);

				// get entered phrase into buffer
				if (szPhrase1) {
					secFree (szPhrase1);
					szPhrase1 = NULL;
				}
				lPhrase1Len = SendDlgItemMessage (hDlg, 
						IDC_EDIT_PASSPHRASE, WM_GETTEXTLENGTH, 0, 0) +1;
				szPhrase1 = secAlloc (lPhrase1Len);
				if (szPhrase1) 
					GetDlgItemText (hDlg, IDC_EDIT_PASSPHRASE, 
										szDummy, sizeof(szDummy));

				// get confirmation phrase
				if (szPhrase2) {
					secFree (szPhrase2);
					szPhrase2 = NULL;
				}
				lPhrase2Len = SendDlgItemMessage (hDlg, 
						IDC_EDIT_PASSPHRASE2, WM_GETTEXTLENGTH, 0, 0) +1;
				szPhrase2 = secAlloc (lPhrase2Len);
				if (szPhrase2) 
						GetDlgItemText (hDlg, IDC_EDIT_PASSPHRASE2, 
										szDummy, sizeof(szDummy));
 
				if (szPhrase1 && szPhrase2) {
					pkgi->uPhraseFlags = 0;
					if (strcmp (szPhrase1, szPhrase2) != 0) 
						pkgi->uPhraseFlags |= BADPHRASE_CONFIRMATION;

					if (lstrlen (szPhrase1) < pkgi->iMinPhraseLength) 
						pkgi->uPhraseFlags |= BADPHRASE_LENGTH;

					if (PGPcomdlgPassphraseQuality (szPhrase1) < 
												pkgi->iMinPhraseQuality) 
						pkgi->uPhraseFlags |= BADPHRASE_QUALITY;

					if (pkgi->pszPassPhrase) {
						secFree (pkgi->pszPassPhrase);
						pkgi->pszPassPhrase = NULL;
					}
					pkgi->pszPassPhrase = 
									secAlloc (lPhrase1Len * sizeof(char));
					lstrcpy (pkgi->pszPassPhrase, szPhrase1);

					PGPkeysWipeEditBox (hDlg, IDC_EDIT_PASSPHRASE);
					PGPkeysWipeEditBox (hDlg, IDC_EDIT_PASSPHRASE2);
					SetDlgItemText (hDlg, IDC_EDIT_PASSPHRASE, "");
					SetDlgItemText (hDlg, IDC_EDIT_PASSPHRASE2, "");
					SetFocus (GetDlgItem (hDlg, IDC_EDIT_PASSPHRASE));

					SetWindowLong (hDlg, DWL_MSGRESULT, 0L);
					bReturnCode = TRUE;
				}
				if (szPhrase1) {
					secFree (szPhrase1);
					szPhrase1 = NULL;
				}
				if (szPhrase2) {
					secFree (szPhrase2);
					szPhrase2 = NULL;
				}
				break;

			case PSN_HELP:
				WinHelp (hDlg, g_szHelpFile, HELP_CONTEXT, 
							IDH_PGPPK_WIZ_PASSPHRASE); 
				break;

			case PSN_QUERYCANCEL:
				break;
			}
			break;
		}
	}

	return bReturnCode;
}


//----------------------------------------------------|
//  Dialog procedure for "ADK" dialog

LRESULT WINAPI 
KeyWizardCorpCertDlgProc (HWND hDlg, UINT uMsg, 
					  WPARAM wParam, LPARAM lParam) 
{
	PKEYGENINFO		pkgi;
	BOOL			bReturnCode = FALSE;

	switch (uMsg) {
	case WM_INITDIALOG:
		{
			PROPSHEETPAGE *ppspMsgRec = (PROPSHEETPAGE *) lParam;
			pkgi = (PKEYGENINFO) ppspMsgRec->lParam;
			SetWindowLong (hDlg, GWL_USERDATA, (LPARAM)pkgi);
			break;
		}

	case WM_PAINT :
		pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
		if (pkgi->hPalette)
		{
			PAINTSTRUCT ps;
			HDC	hDC = BeginPaint (hDlg, &ps);
			SelectPalette (hDC, pkgi->hPalette, FALSE);
			RealizePalette (hDC);
			EndPaint (hDlg, &ps);
		}
		break;

	case WM_NOTIFY:
		{	
			LPNMHDR pnmh = (LPNMHDR) lParam;

			switch (pnmh->code) {
			case PSN_SETACTIVE:
				pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
				if (pkgi->CorpKey) {
					CHAR sz[kPGPMaxUserIDSize];
					UINT u;
					PostMessage (GetParent (hDlg), 
						PSM_SETWIZBUTTONS, 0, PSWIZB_BACK|PSWIZB_NEXT);
					SendDlgItemMessage (hDlg, IDC_WIZBITMAP, STM_SETIMAGE, 
						IMAGE_BITMAP, (LPARAM) pkgi->hBitmap);
					SetWindowLong (hDlg, DWL_MSGRESULT, 0L);
					u = sizeof(sz);
					PGPGetPrimaryUserIDNameBuffer (pkgi->CorpKey, 
													sizeof(sz), sz, &u);
					SetDlgItemText (hDlg, IDC_CORPCERTKEY, sz);
				}
				else 
					SetWindowLong (hDlg, DWL_MSGRESULT, -1L); // skip page

				bReturnCode = TRUE;
				break;

			case PSN_HELP:
				WinHelp (hDlg, g_szHelpFile, HELP_CONTEXT, 
							IDH_PGPPK_WIZ_CORPKEY); 
				break;

			case PSN_QUERYCANCEL:
				break;
			}
			break;
		}
	}

	return bReturnCode;
}


//----------------------------------------------------|
//  Dialog procedure for "ADK" dialog

LRESULT WINAPI 
KeyWizardADKDlgProc (HWND hDlg, UINT uMsg, 
					  WPARAM wParam, LPARAM lParam) 
{
	PKEYGENINFO		pkgi;
	BOOL			bReturnCode = FALSE;

	switch (uMsg) {
	case WM_INITDIALOG:
		{
			PROPSHEETPAGE *ppspMsgRec = (PROPSHEETPAGE *) lParam;
			pkgi = (PKEYGENINFO) ppspMsgRec->lParam;
			SetWindowLong (hDlg, GWL_USERDATA, (LPARAM)pkgi);
			break;
		}

	case WM_PAINT :
		pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
		if (pkgi->hPalette)
		{
			PAINTSTRUCT ps;
			HDC	hDC = BeginPaint (hDlg, &ps);
			SelectPalette (hDC, pkgi->hPalette, FALSE);
			RealizePalette (hDC);
			EndPaint (hDlg, &ps);
		}
		break;

	case WM_NOTIFY:
		{	
			LPNMHDR pnmh = (LPNMHDR) lParam;

			switch (pnmh->code) {
			case PSN_SETACTIVE:
				pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
				if (pkgi->ADK &&
						(pkgi->uKeyType == kPGPPublicKeyAlgorithm_DSA)) {
					CHAR sz[kPGPMaxUserIDSize];
					UINT u;
					PostMessage (GetParent (hDlg), 
						PSM_SETWIZBUTTONS, 0, PSWIZB_BACK|PSWIZB_NEXT);
					SendDlgItemMessage (hDlg, IDC_WIZBITMAP, STM_SETIMAGE, 
						IMAGE_BITMAP, (LPARAM) pkgi->hBitmap);
					SetWindowLong (hDlg, DWL_MSGRESULT, 0L);
					u = sizeof(sz);
					PGPGetPrimaryUserIDNameBuffer (pkgi->ADK, 
													sizeof(sz), sz, &u);
					SetDlgItemText (hDlg, IDC_ADK, sz);
				}
				else 
					SetWindowLong (hDlg, DWL_MSGRESULT, -1L); // skip page

				bReturnCode = TRUE;
				break;

			case PSN_HELP:
				WinHelp (hDlg, g_szHelpFile, HELP_CONTEXT, 
							IDH_PGPPK_WIZ_ADK); 
				break;

			case PSN_QUERYCANCEL:
				break;
			}
			break;
		}
	}

	return bReturnCode;
}


//----------------------------------------------------|
//  Dialog procedure for getting expiration info of key
//  from user

LRESULT WINAPI 
KeyWizardExpirationDlgProc (HWND hDlg, UINT uMsg, 
							WPARAM wParam, LPARAM lParam) 
{
	PKEYGENINFO		pkgi;
	BOOL			bReturnCode = FALSE;
	UINT			uExpireDays;
	static UINT		uDaysToday;

	switch (uMsg) {
	case WM_INITDIALOG:
		{
			RECT			rc;
			time_t			today;
			struct tm*		ptm;
			PROPSHEETPAGE	*ppspMsgRec = (PROPSHEETPAGE *) lParam;

			pkgi = (PKEYGENINFO) ppspMsgRec->lParam;
			SetWindowLong (hDlg, GWL_USERDATA, (LPARAM)pkgi);
			
			PGPcomdlgInitTimeDateControl (g_hInst);

			GetWindowRect (GetDlgItem (hDlg, IDC_RADIO_CUSTOM_DAYS), &rc);
			MapWindowPoints (NULL, hDlg, (LPPOINT)&rc, 2);
			pkgi->hwndExpirationDate = 
					PGPcomdlgCreateTimeDateControl (hDlg, g_hInst, 
							rc.right, rc.top-2, 
							PGPCOMDLG_DISPLAY_DATE);
			SetWindowPos (pkgi->hwndExpirationDate, 
							GetDlgItem (hDlg, IDC_RADIO_CUSTOM_DAYS),
							0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);

			time (&today);
			ptm = localtime (&today);
			ptm->tm_hour = 12;
			ptm->tm_min	= 0;
			ptm->tm_sec = 0;

			today = mktime (ptm);
			uDaysToday = today / 86400;

			ptm->tm_mday++;
			today = mktime (ptm);
			ptm = localtime (&today);

			Date_SetDay (pkgi->hwndExpirationDate, ptm->tm_mday);
			Date_SetMonth (pkgi->hwndExpirationDate, ptm->tm_mon+1);
			Date_SetYear (pkgi->hwndExpirationDate, ptm->tm_year+1900);

			if (!pkgi->uExpireDays) {
				SendDlgItemMessage (hDlg, IDC_RADIO_NEVER, 
					BM_SETCHECK, BST_CHECKED, 0);
				EnableWindow (pkgi->hwndExpirationDate, FALSE);
			}
			else {
				SendDlgItemMessage (hDlg, IDC_RADIO_CUSTOM_DAYS, 
					BM_SETCHECK, BST_CHECKED, 0);
				EnableWindow (pkgi->hwndExpirationDate, TRUE);
			}
			break;
		}

	case WM_PAINT :
		pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
		if (pkgi->hPalette)
		{
			PAINTSTRUCT ps;
			HDC	hDC = BeginPaint (hDlg, &ps);
			SelectPalette (hDC, pkgi->hPalette, FALSE);
			RealizePalette (hDC);
			EndPaint (hDlg, &ps);
		}
		break;

	case WM_COMMAND:
		pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
		switch (LOWORD (wParam)) {
		case IDC_RADIO_NEVER :
		case IDC_RADIO_CUSTOM_DAYS:
			if (LOWORD(wParam) == IDC_RADIO_CUSTOM_DAYS) {
				EnableWindow (pkgi->hwndExpirationDate, TRUE);
				if (ValidateSelectedDate (pkgi->hwndExpirationDate, 
												uDaysToday, &uExpireDays)) 
					PostMessage (GetParent(hDlg), 
						PSM_SETWIZBUTTONS, 0, PSWIZB_BACK|PSWIZB_NEXT);
				else 
					PostMessage (GetParent(hDlg), 
						PSM_SETWIZBUTTONS, 0, PSWIZB_BACK);
			}
			else {
				EnableWindow (pkgi->hwndExpirationDate, FALSE);
				PostMessage (GetParent(hDlg), 
						PSM_SETWIZBUTTONS, 0, PSWIZB_BACK|PSWIZB_NEXT);
			}
			break;
		}
		break;

	case WM_NOTIFY:
		{
			BOOL	bInRange = TRUE;
			LPNMHDR pnmh = (LPNMHDR) lParam;

			switch (pnmh->code) {
			case TDN_TIMECHANGED :
				pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
				if (ValidateSelectedDate (pkgi->hwndExpirationDate, 
											uDaysToday, &uExpireDays)) 
					PostMessage (GetParent(hDlg), 
						PSM_SETWIZBUTTONS, 0, PSWIZB_BACK|PSWIZB_NEXT);
				else 
					PostMessage (GetParent(hDlg), 
						PSM_SETWIZBUTTONS, 0, PSWIZB_BACK);
				break;

			case PSN_SETACTIVE:
				pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
				PostMessage (GetParent(hDlg), 
						PSM_SETWIZBUTTONS, 0, PSWIZB_BACK|PSWIZB_NEXT);
				SendDlgItemMessage (hDlg, IDC_WIZBITMAP, STM_SETIMAGE, 
						IMAGE_BITMAP, (LPARAM) pkgi->hBitmap);
				if (ValidateSelectedDate (pkgi->hwndExpirationDate, 
											uDaysToday, &uExpireDays) ||
					(IsDlgButtonChecked (hDlg, IDC_RADIO_NEVER) == 
															BST_CHECKED))
					PostMessage (GetParent(hDlg), 
						PSM_SETWIZBUTTONS, 0, PSWIZB_BACK|PSWIZB_NEXT);
				else 
					PostMessage (GetParent(hDlg), 
						PSM_SETWIZBUTTONS, 0, PSWIZB_BACK);
				SetWindowLong (hDlg, DWL_MSGRESULT, 0L);
				bReturnCode = TRUE;
				break;

			case PSN_WIZNEXT:
				pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);

				if (SendDlgItemMessage (hDlg, IDC_RADIO_NEVER, 
						BM_GETCHECK, 0, 0) == BST_CHECKED)
					uExpireDays = 0;
				else if (SendDlgItemMessage (hDlg, IDC_RADIO_CUSTOM_DAYS, 
						BM_GETCHECK, 0, 0) == BST_CHECKED) {
					ValidateSelectedDate (pkgi->hwndExpirationDate, 
												uDaysToday, &uExpireDays);
				}
				pkgi->uExpireDays = uExpireDays;
				break;

			case PSN_HELP:
				WinHelp (hDlg, g_szHelpFile, HELP_CONTEXT, 
							IDH_PGPPK_WIZ_EXPIRATION); 
				break;

			case PSN_QUERYCANCEL:
				break;
			}
			break;
		}
	}

	return bReturnCode;
}


//----------------------------------------------------|
//  Dialog procedure for getting size of key (number of bits)
//  from user

LRESULT WINAPI 
KeyWizardSizeDlgProc (HWND hDlg, UINT uMsg, 
					   WPARAM wParam, LPARAM lParam) 
{
	PKEYGENINFO		pkgi;
	BOOL			bReturnCode = FALSE;

	switch (uMsg) {
	case WM_INITDIALOG:
		{
			PROPSHEETPAGE *ppspMsgRec = (PROPSHEETPAGE *) lParam;
			pkgi = (PKEYGENINFO) ppspMsgRec->lParam;
			SetWindowLong (hDlg, GWL_USERDATA, (LPARAM)pkgi);

			SendDlgItemMessage (hDlg, IDC_CUSTOM_BITS, EM_SETLIMITTEXT, 4, 0);
			SendDlgItemMessage (hDlg, IDC_CUSTOM_BITS, WM_ENABLE, 
												FALSE, 0);

			if (pkgi->uKeySize < pkgi->uMinKeySize) 
				pkgi->uKeySize = pkgi->uMinKeySize;

			if (pkgi->uMinKeySize > 768) 
				EnableWindow (GetDlgItem (hDlg, IDC_RADIO_768), FALSE);
			if (pkgi->uMinKeySize > 1024) 
				EnableWindow (GetDlgItem (hDlg, IDC_RADIO_1024), FALSE);
			if (pkgi->uMinKeySize > 1536) 
				EnableWindow (GetDlgItem (hDlg, IDC_RADIO_1536), FALSE);
			if (pkgi->uMinKeySize > 2048) 
				EnableWindow (GetDlgItem (hDlg, IDC_RADIO_2048), FALSE);
			if (pkgi->uMinKeySize > 3072) 
				EnableWindow (GetDlgItem (hDlg, IDC_RADIO_3072), FALSE);

			switch (pkgi->uKeySize) {
			case 768:
				SendDlgItemMessage (hDlg, IDC_RADIO_768, 
										BM_SETCHECK, BST_CHECKED, 0);
				break;

			case 1024:
				SendDlgItemMessage (hDlg, IDC_RADIO_1024, 
										BM_SETCHECK, BST_CHECKED, 0);
				break;

			case 1536:
				SendDlgItemMessage (hDlg, IDC_RADIO_1536, 
										BM_SETCHECK, BST_CHECKED, 0);
				break;

			case 2048:
				SendDlgItemMessage (hDlg, IDC_RADIO_2048, 
										BM_SETCHECK, BST_CHECKED, 0);
				break;

			case 3072:
				SendDlgItemMessage (hDlg, IDC_RADIO_3072, 
										BM_SETCHECK, BST_CHECKED, 0);
				break;

			default:
				SendDlgItemMessage (hDlg, IDC_RADIO_CUSTOM, 
										BM_SETCHECK, BST_CHECKED, 0);
				SetDlgItemInt (hDlg, IDC_CUSTOM_BITS, pkgi->uKeySize, FALSE);
				SendDlgItemMessage (hDlg, IDC_CUSTOM_BITS, 
										WM_ENABLE, TRUE, 0);
				break;
			}
			break;
		}

	case WM_PAINT :
		pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
		if (pkgi->hPalette)
		{
			PAINTSTRUCT ps;
			HDC	hDC = BeginPaint (hDlg, &ps);
			SelectPalette (hDC, pkgi->hPalette, FALSE);
			RealizePalette (hDC);
			EndPaint (hDlg, &ps);
		}
		break;

	case WM_COMMAND:
		{
			switch (HIWORD(wParam)) {
			case BN_CLICKED:
				if (LOWORD (wParam) == IDC_RADIO_CUSTOM) {
					SendDlgItemMessage (hDlg, IDC_CUSTOM_BITS, 
										WM_ENABLE, TRUE, 0);
				}
				else {
					SendDlgItemMessage (hDlg, IDC_CUSTOM_BITS, 
										WM_ENABLE, FALSE, 0);
				}
				break;
			}
			break;
		}

	case WM_NOTIFY:
		{
			UINT	uMaxKeySize;
			UINT	uKeySize = 0;
			CHAR	szText[64];
			CHAR	szText2[64];
			LPNMHDR pnmh = (LPNMHDR) lParam;

			switch(pnmh->code) {
			case PSN_SETACTIVE:
				pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
				PostMessage (GetParent(hDlg), 
						PSM_SETWIZBUTTONS, 0, PSWIZB_BACK|PSWIZB_NEXT);
				SendDlgItemMessage(hDlg, IDC_WIZBITMAP, STM_SETIMAGE, 
						IMAGE_BITMAP, (LPARAM) pkgi->hBitmap);
				SetWindowLong (hDlg, DWL_MSGRESULT, 0L);
				bReturnCode = TRUE;

				if (pkgi->uKeyType == kPGPPublicKeyAlgorithm_DSA) {
					ShowWindow (GetDlgItem (hDlg, IDC_RADIO_3072), SW_SHOW);
					ShowWindow (GetDlgItem (hDlg, IDC_STATIC_SMALL_GUIDE), 
											SW_SHOW);
					LoadString (g_hInst, IDS_KW_DSACUSTOM, 
							szText, sizeof(szText));
					wsprintf (szText2, szText, pkgi->uMinKeySize);
					SetDlgItemText (hDlg, IDC_RADIO_CUSTOM, szText2);
					LoadString (g_hInst, IDS_KW_DSA1536, 
							szText, sizeof(szText));
					SetDlgItemText (hDlg, IDC_RADIO_1536, szText);
					LoadString (g_hInst, IDS_KW_DSA2048, 
							szText, sizeof(szText));
					SetDlgItemText (hDlg, IDC_RADIO_2048, szText);
					LoadString (g_hInst, IDS_KW_DSA3072, 
							szText, sizeof(szText));
					SetDlgItemText (hDlg, IDC_RADIO_3072, szText);
				}
				else {
					ShowWindow (GetDlgItem (hDlg, IDC_RADIO_3072), SW_HIDE);
					ShowWindow (GetDlgItem (hDlg, IDC_STATIC_SMALL_GUIDE), 
											SW_HIDE);
					LoadString (g_hInst, IDS_KW_RSACUSTOM, 
							szText, sizeof(szText));
					wsprintf (szText2, szText, pkgi->uMinKeySize);
					SetDlgItemText (hDlg, IDC_RADIO_CUSTOM, szText2);
					LoadString (g_hInst, IDS_KW_RSA1536, 
							szText, sizeof(szText));
					SetDlgItemText (hDlg, IDC_RADIO_1536, szText);
					LoadString (g_hInst, IDS_KW_RSA2048, 
							szText, sizeof(szText));
					SetDlgItemText (hDlg, IDC_RADIO_2048, szText);
				}
				break;

			case PSN_KILLACTIVE:
				pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);

				if (SendDlgItemMessage (hDlg, IDC_RADIO_768, 
						BM_GETCHECK, 0, 0) == BST_CHECKED)
					uKeySize = 768;
				else if (SendDlgItemMessage (hDlg, IDC_RADIO_1024, 
						BM_GETCHECK, 0, 0) == BST_CHECKED)
					uKeySize = 1024;
				else if (SendDlgItemMessage (hDlg, IDC_RADIO_1536, 
						BM_GETCHECK, 0, 0) == BST_CHECKED)
					uKeySize = 1536;
				else if (SendDlgItemMessage (hDlg, IDC_RADIO_2048, 
						BM_GETCHECK, 0, 0) == BST_CHECKED)
					uKeySize = 2048;
				else if (SendDlgItemMessage (hDlg, IDC_RADIO_3072, 
						BM_GETCHECK, 0, 0) == BST_CHECKED)
					uKeySize = 3072;
				else if (SendDlgItemMessage (hDlg, IDC_RADIO_CUSTOM, 
						BM_GETCHECK, 0, 0) == BST_CHECKED) {
					uKeySize = GetDlgItemInt(hDlg, IDC_CUSTOM_BITS, 
						NULL, FALSE);
				}

				if (pkgi->uKeyType == kPGPPublicKeyAlgorithm_RSA)
					uMaxKeySize = MAX_RSA_KEY_SIZE;
				else 
					uMaxKeySize = MAX_DSA_KEY_SIZE;

				if ((uKeySize < pkgi->uMinKeySize) || 
					(uKeySize > uMaxKeySize)) {
					CHAR szError[1024], szTitle[1024], szTemp[1024];
					LoadString (g_hInst, IDS_KW_INVALID_KEY_SIZE, 
											szTemp, sizeof(szTemp));
					LoadString (g_hInst, IDS_KW_TITLE, 
											szTitle, sizeof(szTitle));
					wsprintf (szError, szTemp, 
										pkgi->uMinKeySize, uMaxKeySize);
					MessageBox (hDlg, szError, szTitle, MB_OK|MB_ICONERROR);
					SetWindowLong (hDlg, DWL_MSGRESULT, -1L);
					bReturnCode = TRUE;
				}
				else { 
					if (uKeySize < 768) {
						if (PGPkeysMessageBox (hDlg, IDS_CAPTION, 
										   IDS_SMALLKEYWARNING, 
										   MB_YESNO|MB_ICONEXCLAMATION|
										   MB_DEFBUTTON2) != IDYES) {
							SetWindowLong (hDlg, DWL_MSGRESULT, -1L);
							bReturnCode = TRUE;
						}
					}
					if (pkgi->uKeyType == kPGPPublicKeyAlgorithm_DSA) {
						pkgi->uKeySize = GetDSAKeySize (uKeySize);
						pkgi->uSubKeySize = uKeySize;
					}
					else {
						pkgi->uKeySize = uKeySize;
					}
				}
				break;

			case PSN_HELP:
				WinHelp (hDlg, g_szHelpFile, HELP_CONTEXT, 
							IDH_PGPPK_WIZ_SIZE); 
				break;

			case PSN_QUERYCANCEL:
				break;
			}
			break;
		}
	}

	return bReturnCode;
}


//----------------------------------------------------|
//  Dialog procedure for getting type of key (RSA/DSA)
//  from user

LRESULT WINAPI 
KeyWizardTypeDlgProc (HWND hDlg, UINT uMsg, 
					   WPARAM wParam, LPARAM lParam) 
{
	PKEYGENINFO		pkgi;
	BOOL			bReturnCode = FALSE;

	switch (uMsg) {
	case WM_INITDIALOG:
		{
			PROPSHEETPAGE *ppspMsgRec = (PROPSHEETPAGE *) lParam;
			pkgi = (PKEYGENINFO) ppspMsgRec->lParam;
			SetWindowLong (hDlg, GWL_USERDATA, (LPARAM)pkgi);

			if (!pkgi->bAllowRSAGen) {
				pkgi->uKeyType = kPGPPublicKeyAlgorithm_DSA;
				EnableWindow (GetDlgItem (hDlg, IDC_RADIO_RSA), FALSE);
				ShowWindow (GetDlgItem (hDlg, IDC_NORSAGENTEXT), SW_SHOW);
			}

			switch (pkgi->uKeyType) {
			case kPGPPublicKeyAlgorithm_RSA:
				SendDlgItemMessage (hDlg, IDC_RADIO_RSA, BM_SETCHECK, 
										BST_CHECKED, 0);
				break;

			case kPGPPublicKeyAlgorithm_DSA:
				SendDlgItemMessage (hDlg, IDC_RADIO_ELGAMAL, BM_SETCHECK, 
										BST_CHECKED, 0);
				break;
			}
			break;
		}

	case WM_PAINT :
		pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
		if (pkgi->hPalette)
		{
			PAINTSTRUCT ps;
			HDC	hDC = BeginPaint (hDlg, &ps);
			SelectPalette (hDC, pkgi->hPalette, FALSE);
			RealizePalette (hDC);
			EndPaint (hDlg, &ps);
		}
		break;

	case WM_NOTIFY:
		{
			UINT	uKeyType;
			LPNMHDR pnmh = (LPNMHDR) lParam;


			switch(pnmh->code) {
			case PSN_SETACTIVE:
				pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
				if (!pkgi->bAllowRSAGen) {
					SetWindowLong (hDlg, DWL_MSGRESULT, -1L); // skip page
				}
				else {
					PostMessage(GetParent(hDlg), 
						PSM_SETWIZBUTTONS, 0, PSWIZB_BACK|PSWIZB_NEXT);
					SendDlgItemMessage(hDlg, IDC_WIZBITMAP, STM_SETIMAGE, 
						IMAGE_BITMAP, (LPARAM) pkgi->hBitmap);
					SetWindowLong(hDlg, DWL_MSGRESULT, 0L);
				}
				bReturnCode = TRUE;
				break;

			case PSN_KILLACTIVE:
				uKeyType = kPGPPublicKeyAlgorithm_DSA + 
							kPGPPublicKeyAlgorithm_RSA;
				pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);

				if (SendDlgItemMessage (hDlg, IDC_RADIO_RSA, 
								BM_GETCHECK, 0, 0) == BST_CHECKED)
					uKeyType = kPGPPublicKeyAlgorithm_RSA;
				else if(SendDlgItemMessage(hDlg, IDC_RADIO_ELGAMAL, 
								BM_GETCHECK, 0, 0) == BST_CHECKED)
					uKeyType = kPGPPublicKeyAlgorithm_DSA;

				if ((uKeyType != kPGPPublicKeyAlgorithm_RSA) && 
					(uKeyType != kPGPPublicKeyAlgorithm_DSA)) {
					PGPkeysMessageBox (hDlg, IDS_KW_TITLE, 
									   IDS_KW_INVALID_KEY_TYPE, 
									   MB_OK|MB_ICONERROR);
					SetWindowLong(hDlg, DWL_MSGRESULT, -1L);
					bReturnCode = TRUE;
				}
				else {
					pkgi->uKeyType = uKeyType;
				}
				SetWindowLong(hDlg, DWL_MSGRESULT, 0L);
				bReturnCode = TRUE;
				break;

			case PSN_HELP:
				WinHelp (hDlg, g_szHelpFile, HELP_CONTEXT, 
							IDH_PGPPK_WIZ_TYPE); 
				break;

			case PSN_QUERYCANCEL:
				break;
			}
			break;
		}
	}

	return bReturnCode;
}


//----------------------------------------------------|
//  Dialog procedure for getting name and email address
//  from user

LRESULT WINAPI 
KeyWizardNameDlgProc (HWND hDlg, UINT uMsg, 
					   WPARAM wParam, LPARAM lParam) 
{
	PKEYGENINFO		pkgi;
	BOOL			bReturnCode = FALSE;

	switch (uMsg) {
	case WM_INITDIALOG:
		{
			PROPSHEETPAGE *ppspMsgRec = (PROPSHEETPAGE *) lParam;
			pkgi = (PKEYGENINFO) ppspMsgRec->lParam;
			SetWindowLong (hDlg, GWL_USERDATA, (LPARAM)pkgi);

			SendDlgItemMessage (hDlg, IDC_FULL_NAME, EM_SETLIMITTEXT, 
				MAX_FULL_NAME_LEN, 0);
			SendDlgItemMessage (hDlg, IDC_EMAIL, EM_SETLIMITTEXT, 
				MAX_EMAIL_LEN, 0);
			break;
		}

	case WM_PAINT :
		pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
		if (pkgi->hPalette)
		{
			PAINTSTRUCT ps;
			HDC	hDC = BeginPaint (hDlg, &ps);
			SelectPalette (hDC, pkgi->hPalette, FALSE);
			RealizePalette (hDC);
			EndPaint (hDlg, &ps);
		}
		break;

	case WM_NOTIFY:
		{
			INT		iTextLen;
			BOOL	bContinue = TRUE;
			LPNMHDR pnmh = (LPNMHDR) lParam;

			switch(pnmh->code) {
			case PSN_SETACTIVE:
				pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
				PostMessage(GetParent(hDlg), 
						PSM_SETWIZBUTTONS, 0, PSWIZB_BACK|PSWIZB_NEXT);
				SendDlgItemMessage(hDlg, IDC_WIZBITMAP, STM_SETIMAGE, 
						IMAGE_BITMAP, (LPARAM) pkgi->hBitmap);
				SetWindowLong(hDlg, DWL_MSGRESULT, 0L);
				bReturnCode = TRUE;
				break;

			case PSN_KILLACTIVE:
				SetWindowLong(hDlg, DWL_MSGRESULT, 0L);
				bReturnCode = TRUE;
				break;

			case PSN_HELP:
				WinHelp (hDlg, g_szHelpFile, HELP_CONTEXT, 
							IDH_PGPPK_WIZ_NAME); 
				break;

			case PSN_QUERYCANCEL:
				break;

			case PSN_WIZNEXT:
				pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);

				if (pkgi->pszFullName) {
					pkFree (pkgi->pszFullName);
					pkgi->pszFullName = NULL;
				}

				if (pkgi->pszEmail) {
					pkFree (pkgi->pszEmail);
					pkgi->pszEmail = NULL;
				}

				// allocate string and get name from edit box
				iTextLen = SendDlgItemMessage (hDlg, IDC_FULL_NAME, 
						WM_GETTEXTLENGTH, 0, 0) + 1;
				pkgi->pszFullName = pkAlloc (iTextLen * sizeof(char));
				if (pkgi->pszFullName) 
						GetDlgItemText (hDlg, IDC_FULL_NAME, 
										pkgi->pszFullName, iTextLen);

				// no name entered, warn user
				if (iTextLen <= 1) {
					PGPkeysMessageBox (hDlg, IDS_KW_TITLE, 
									 IDS_KW_NO_FULL_NAME, 
									 MB_OK|MB_ICONWARNING);
					bContinue = FALSE;
					SetFocus (GetDlgItem (hDlg, IDC_FULL_NAME));
				} 
					
				if (bContinue) {
					iTextLen = SendDlgItemMessage (hDlg, IDC_EMAIL, 
											WM_GETTEXTLENGTH, 0, 0) + 1;
					pkgi->pszEmail = pkAlloc (iTextLen * sizeof(char));
					if (pkgi->pszEmail)
						GetDlgItemText (hDlg, IDC_EMAIL, 
										pkgi->pszEmail, iTextLen);
					if (iTextLen <= 1) {
						if ((PGPkeysMessageBox (hDlg, IDS_KW_TITLE, 
								IDS_KW_NO_EMAIL, 
								MB_YESNO|MB_ICONWARNING|
								MB_DEFBUTTON2) != IDYES)) {
							bContinue = FALSE;
							SetFocus (GetDlgItem (hDlg, IDC_EMAIL));
						}
					}
				}
				if (!bContinue) {
					SetWindowLong(hDlg, DWL_MSGRESULT, -1L);
					bReturnCode = TRUE;
				}
			}
		}
	}

	return bReturnCode;
}


//----------------------------------------------------|
//  Dialog procedure for handling beginning introductory
//  dialog.

LRESULT WINAPI 
KeyWizardIntroDlgProc (HWND hDlg, UINT uMsg, 
					   WPARAM wParam, LPARAM lParam) 
{
	BOOL			bReturnCode = FALSE;
	PKEYGENINFO		pkgi		= NULL;

	switch (uMsg) {
	case WM_INITDIALOG:
		{
			RECT rc;

			PROPSHEETPAGE *ppspMsgRec = (PROPSHEETPAGE *) lParam;
			pkgi = (PKEYGENINFO) ppspMsgRec->lParam;
			SetWindowLong (hDlg, GWL_USERDATA, (LPARAM)pkgi);

			// center dialog on screen
			GetWindowRect(GetParent(hDlg), &rc);
			SetWindowPos (GetParent(hDlg), NULL,
				(GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left))/2,
				(GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top))/2,
				0, 0, SWP_NOSIZE | SWP_NOZORDER);
			break;
		}

	case WM_ACTIVATE :
		InvalidateRect (hDlg, NULL, TRUE);
		break;

	case WM_PAINT :
		pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
		if (pkgi->hPalette)
		{
			PAINTSTRUCT ps;
			HDC	hDC = BeginPaint (hDlg, &ps);
			SelectPalette (hDC, pkgi->hPalette, FALSE);
			RealizePalette (hDC);
			EndPaint (hDlg, &ps);
		}
		break;

	case WM_NOTIFY:
		{
			LPNMHDR pnmh = (LPNMHDR) lParam;

			switch(pnmh->code) {
			case PSN_SETACTIVE:
				pkgi = (PKEYGENINFO)GetWindowLong (hDlg, GWL_USERDATA);
				PostMessage(GetParent(hDlg), 
						PSM_SETWIZBUTTONS, 0, PSWIZB_NEXT);
				SendDlgItemMessage(hDlg, IDC_WIZBITMAP, STM_SETIMAGE, 
						IMAGE_BITMAP, (LPARAM) pkgi->hBitmap);
				SetWindowLong(hDlg, DWL_MSGRESULT, 0L);
				bReturnCode = TRUE;
				break;

			case PSN_KILLACTIVE:
				SetWindowLong(hDlg, DWL_MSGRESULT, 0L);
				bReturnCode = TRUE;
				break;

			case PSN_HELP:
				WinHelp (hDlg, g_szHelpFile, HELP_CONTEXT, 
							IDH_PGPPK_WIZ_ABOUT); 
				break;

			case PSN_QUERYCANCEL:
				break;
			}
			
			break;
		}
	}

	return bReturnCode;
}




//-------------------------------------------------------------------|
// Load DIB bitmap and associated palette

HPALETTE 
CreateDIBPalette (LPBITMAPINFO lpbmi, 
				  LPINT lpiNumColors) 
{
	LPBITMAPINFOHEADER lpbi;
	LPLOGPALETTE lpPal;
	HANDLE hLogPal;
	HPALETTE hPal = NULL;
	INT i;
 
	lpbi = (LPBITMAPINFOHEADER)lpbmi;
	if (lpbi->biBitCount <= 8) {
		*lpiNumColors = (1 << lpbi->biBitCount);
	}
	else
		*lpiNumColors = 0;  // No palette needed for 24 BPP DIB
 
	if (*lpiNumColors) {
		hLogPal = GlobalAlloc (GHND, sizeof (LOGPALETTE) +
                             sizeof (PALETTEENTRY) * (*lpiNumColors));
		lpPal = (LPLOGPALETTE) GlobalLock (hLogPal);
		lpPal->palVersion = 0x300;
		lpPal->palNumEntries = *lpiNumColors;
 
		for (i = 0;  i < *lpiNumColors;  i++) {
			lpPal->palPalEntry[i].peRed   = lpbmi->bmiColors[i].rgbRed;
			lpPal->palPalEntry[i].peGreen = lpbmi->bmiColors[i].rgbGreen;
			lpPal->palPalEntry[i].peBlue  = lpbmi->bmiColors[i].rgbBlue;
			lpPal->palPalEntry[i].peFlags = 0;
		}
		hPal = CreatePalette (lpPal);
		GlobalUnlock (hLogPal);
		GlobalFree (hLogPal);
   }
   return hPal;
}


HBITMAP 
LoadResourceBitmap (HINSTANCE hInstance, 
					LPSTR lpString,
					HPALETTE FAR* lphPalette) 
{
	HRSRC  hRsrc;
	HGLOBAL hGlobal;
	HBITMAP hBitmapFinal = NULL;
	LPBITMAPINFOHEADER lpbi;
	HDC hdc;
    INT iNumColors;
 
	if (hRsrc = FindResource (hInstance, lpString, RT_BITMAP)) {
		hGlobal = LoadResource (hInstance, hRsrc);
		lpbi = (LPBITMAPINFOHEADER)LockResource (hGlobal);
 
		hdc = GetDC(NULL);
		*lphPalette =  CreateDIBPalette ((LPBITMAPINFO)lpbi, &iNumColors);
		if (*lphPalette) {
			SelectPalette (hdc,*lphPalette,FALSE);
			RealizePalette (hdc);
		}
 
		hBitmapFinal = CreateDIBitmap (hdc,
                   (LPBITMAPINFOHEADER)lpbi,
                   (LONG)CBM_INIT,
                   (LPSTR)lpbi + lpbi->biSize + iNumColors * sizeof(RGBQUAD),
                   (LPBITMAPINFO)lpbi,
                   DIB_RGB_COLORS );
 
		ReleaseDC (NULL,hdc);
		UnlockResource (hGlobal);
		FreeResource (hGlobal);
	}
	return (hBitmapFinal);
}

//----------------------------------------------------|
//  Create wizard data structures and call PropertySheet 
//  to actually create wizard

VOID
CreateKeyGenWizard (VOID *pArg)
{
	KEYGENINFO		KGInfo;
	PROPSHEETPAGE   pspWiz[NUM_WIZ_PAGES];
	PROPSHEETHEADER pshWiz;
	INT				iIndex;
	INT				iNumBits, iBitmap;
	HDC				hDC;
	HWND			hWndMain = (HWND)pArg;
	DWORD			dwThreadID;
	PGPError		err;

	// Set defaults
	KGInfo.Context			= g_ContextRef;
	KGInfo.PrefRefAdmin		= NULL;
	KGInfo.PrefRefClient	= NULL;
	KGInfo.hWndWiz			= NULL;
	KGInfo.hPalette			= NULL;

	// Determine which bitmap will be displayed in the wizard

	hDC = GetDC (NULL);	 // DC for desktop
	iNumBits = GetDeviceCaps (hDC, BITSPIXEL) * GetDeviceCaps (hDC, PLANES);
	ReleaseDC (NULL, hDC);

	if (iNumBits <= 1)
		iBitmap = IDB_KEYWIZ1;
	else if (iNumBits <= 4) 
		iBitmap = IDB_KEYWIZ4;
	else 
		iBitmap = IDB_KEYWIZ8;

	KGInfo.hBitmap = LoadResourceBitmap (g_hInst, 
									  MAKEINTRESOURCE (iBitmap),
									  &KGInfo.hPalette);

	// Set the values common to all pages
	for (iIndex=0; iIndex<NUM_WIZ_PAGES; iIndex++)
	{
		pspWiz[iIndex].dwSize		= sizeof(PROPSHEETPAGE);
		pspWiz[iIndex].dwFlags		= PSP_DEFAULT | PSP_HASHELP;
		pspWiz[iIndex].hInstance	= g_hInst;
		pspWiz[iIndex].pszTemplate  = NULL;
		pspWiz[iIndex].hIcon		= NULL;
		pspWiz[iIndex].pszTitle		= NULL;
		pspWiz[iIndex].pfnDlgProc   = NULL;
		pspWiz[iIndex].lParam		= (LPARAM) &KGInfo;
		pspWiz[iIndex].pfnCallback  = NULL;
		pspWiz[iIndex].pcRefParent  = NULL;
	}


	// Set up the intro page
	pspWiz[KGWIZ_INTRO].pszTemplate = MAKEINTRESOURCE(IDD_KEYWIZ_INTRO);
	pspWiz[KGWIZ_INTRO].pfnDlgProc = KeyWizardIntroDlgProc;
	
	// Set up the name page
	pspWiz[KGWIZ_NAME].pszTemplate = MAKEINTRESOURCE(IDD_KEYWIZ_NAME);
	pspWiz[KGWIZ_NAME].pfnDlgProc = KeyWizardNameDlgProc;

	// Set up the type page
	pspWiz[KGWIZ_TYPE].pszTemplate = MAKEINTRESOURCE(IDD_KEYWIZ_TYPE);
	pspWiz[KGWIZ_TYPE].pfnDlgProc = KeyWizardTypeDlgProc;
	
	// Set up the size page
	pspWiz[KGWIZ_SIZE].pszTemplate = MAKEINTRESOURCE(IDD_KEYWIZ_SIZE);
	pspWiz[KGWIZ_SIZE].pfnDlgProc = KeyWizardSizeDlgProc;
	
	// Set up the expiration page
	pspWiz[KGWIZ_EXPIRE].pszTemplate = MAKEINTRESOURCE(IDD_KEYWIZ_EXPIRATION);
	pspWiz[KGWIZ_EXPIRE].pfnDlgProc = KeyWizardExpirationDlgProc;
	
	// Set up the expiration page
	pspWiz[KGWIZ_ADK].pszTemplate = MAKEINTRESOURCE(IDD_KEYWIZ_ADK);
	pspWiz[KGWIZ_ADK].pfnDlgProc = KeyWizardADKDlgProc;
	
	// Set up the expiration page
	pspWiz[KGWIZ_CORPCERT].pszTemplate = MAKEINTRESOURCE(IDD_KEYWIZ_CORPCERT);
	pspWiz[KGWIZ_CORPCERT].pfnDlgProc = KeyWizardCorpCertDlgProc;
	
	// Set up the passphrase page
	pspWiz[KGWIZ_PHRASE].pszTemplate = MAKEINTRESOURCE(IDD_KEYWIZ_PASSPHRASE);
	pspWiz[KGWIZ_PHRASE].pfnDlgProc = KeyWizardPassphraseDlgProc;
	
	// Set up the bad passphrase page
	pspWiz[KGWIZ_BADPHRASE].pszTemplate = 
									MAKEINTRESOURCE(IDD_KEYWIZ_BADPHRASE);
	pspWiz[KGWIZ_BADPHRASE].pfnDlgProc = KeyWizardBadPassphraseDlgProc;
	
	// Set up the entropy page
	pspWiz[KGWIZ_ENTROPY].pszTemplate = MAKEINTRESOURCE(IDD_KEYWIZ_RANDOBITS);
	pspWiz[KGWIZ_ENTROPY].pfnDlgProc = KeyWizardRandobitsDlgProc;
	
	// Set up the key generation page
	pspWiz[KGWIZ_KEYGEN].pszTemplate = MAKEINTRESOURCE(IDD_KEYWIZ_GENERATION);
	pspWiz[KGWIZ_KEYGEN].pfnDlgProc = KeyWizardGenerationDlgProc;
	
	// Set up the sign key page
	pspWiz[KGWIZ_SIGN].pszTemplate = MAKEINTRESOURCE(IDD_KEYWIZ_SIGN_OLD);
	pspWiz[KGWIZ_SIGN].pfnDlgProc = KeyWizardSignOldDlgProc;

	// Set up the presend to server page
	pspWiz[KGWIZ_PRESEND].pszTemplate = MAKEINTRESOURCE(IDD_KEYWIZ_PRESEND);
	pspWiz[KGWIZ_PRESEND].pfnDlgProc = KeyWizardPreSendDlgProc;

	// Set up the send to server page
	pspWiz[KGWIZ_SEND].pszTemplate = MAKEINTRESOURCE(IDD_KEYWIZ_SEND);
	pspWiz[KGWIZ_SEND].pfnDlgProc = KeyWizardSendToServerDlgProc;

	// Set up the done page
	pspWiz[KGWIZ_DONE].pszTemplate = MAKEINTRESOURCE(IDD_KEYWIZ_DONE);
	pspWiz[KGWIZ_DONE].pfnDlgProc = KeyWizardDoneDlgProc;

	// Create the header
	pshWiz.dwSize		= sizeof(PROPSHEETHEADER);
	pshWiz.dwFlags		= PSH_WIZARD | PSH_PROPSHEETPAGE;
	pshWiz.hwndParent   = hWndMain;
	pshWiz.hInstance	= g_hInst;
	pshWiz.hIcon		= NULL;
	pshWiz.pszCaption   = NULL;
	pshWiz.nPages		= NUM_WIZ_PAGES;
	pshWiz.nStartPage   = KGWIZ_INTRO;
	pshWiz.ppsp			= pspWiz;
	pshWiz.pfnCallback  = NULL;

	// Execute the Wizard - doesn't return until Cancel or Save
	dwThreadID = GetCurrentThreadId ();
	hHookKeyboard = 
		SetWindowsHookEx (WH_KEYBOARD, WizKeyboardHookProc, NULL, dwThreadID);

#if PGP_BUSINESS_SECURITY
	err = PGPcomdlgOpenAdminPrefs (&KGInfo.PrefRefAdmin, FALSE);
#else
	err = kPGPError_NoErr;
#endif
	if (IsntPGPError (err)) {
		if (ValidateConfiguration (&KGInfo)) {
			PGPcomdlgOpenClientPrefs (&KGInfo.PrefRefClient);

			PropertySheet(&pshWiz);

			PGPcomdlgCloseClientPrefs (KGInfo.PrefRefClient, FALSE);
		}
		if (KGInfo.PrefRefAdmin)
			PGPcomdlgCloseAdminPrefs (KGInfo.PrefRefAdmin, FALSE);
	}

	UnhookWindowsHookEx (hHookKeyboard);
	hWndCollectEntropy = NULL;

	// Free allocated memory and objects

	if (KGInfo.pszPassPhrase) {
		secFree (KGInfo.pszPassPhrase);
		KGInfo.pszPassPhrase = NULL;
	}

	if (KGInfo.pszFullName) {
		pkFree (KGInfo.pszFullName);
		KGInfo.pszFullName = NULL;
	}

	if (KGInfo.pszEmail) {
		pkFree (KGInfo.pszEmail);
		KGInfo.pszEmail = NULL;
	}

	if (KGInfo.pszUserID) {
		pkFree (KGInfo.pszUserID);
		KGInfo.pszUserID = NULL;
	}

	DeleteObject (KGInfo.hBitmap);

	if (!KGInfo.bFinishSelected) {
		PGPKeySetRef KeySet;
		if (KGInfo.Key) {
			PGPcomdlgErrorMessage (PGPNewSingletonKeySet (KGInfo.Key, 
															&KeySet));
			PGPcomdlgErrorMessage (PGPRemoveKeys (KeySet, KeySetMain));
			PGPFreeKeySet (KeySet);
			PGPCommitKeyRingChanges (KeySetMain);
		}
		KGInfo.iFinalResult = 0;
	}

	SendMessage (hWndMain, KM_M_CREATEDONE, KGInfo.iFinalResult, 
							(LPARAM) KGInfo.Key); 

	return;
}


//----------------------------------------------------|
//  Create thread to handle dialog box 

void PGPkeysCreateKey (HWND hParent, PGPKeySetRef KeySet) {
	DWORD dwNewKeyThreadID;
	ULONG ul;

	PGPcomdlgSetSplashParent (NULL);
	EnableWindow (hParent, FALSE);
	hWndCollectEntropy = NULL;
	KeySetMain = KeySet;

	ul = _beginthreadex (NULL, 0, (LPTHREAD_START_ROUTINE)CreateKeyGenWizard, 
							hParent, 0, &dwNewKeyThreadID);
}
