/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	KMProps.c - handle Key properties dialogs

	$Id: KMProps.c,v 1.100.2.1 1999/06/11 06:14:47 heller Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

// project header files
#include "pgpkmx.h"
#include "pgpclx.h"
#include "..\include\treelist.h"
#include "pgpHashWords.h"

// system header files
#include <process.h>

// constant definitions
#define ENFORCELISTWIDTH	52

#define MAXSHEETS			16		// max number of simultaneous dialogs

#define MINSUBKEYSIZE		768
#define MAXSUBKEYSIZE		4096
#define DEFAULTSUBKEYSIZE	2048

#define SERIALIZE			1
#define NOSERIALIZE			0

#define AVI_TIMER			4321L
#define AVI_RUNTIME			11000L

#define NOTIFYTIMER			1111L
#define NOTIFYTIMEMS		100

#define NUMHASHWORDCOLS		4

// typedefs
typedef struct {
	PKEYMAN			pKM;
	WNDPROC			wpOrigPhotoIDProc;
	HIMAGELIST		hIml;
	PGPKeyRef		key;
	PGPKeySetRef	keyset;
	PGPUserIDRef	userid;
	PGPSigRef		cert;
	UINT			algKey;
	INT				iIndex;
	UINT			uTrust;
	UINT			uValidity;
	INT				iExpireDays;
	UINT			uNumberADKs;
	UINT			uNumberRevokers;
	PGPBoolean		bX509;
	PGPBoolean		bReadOnly;
	PGPBoolean		bShowHexFingerprint;
	PGPBoolean		bSecret;
	PGPBoolean		bSplit;
	PGPBoolean		bDisabled;
	PGPBoolean		bAxiomatic;
	PGPBoolean		bInvalid;
	PGPBoolean		bRevoked;
	PGPBoolean		bExpired;
	PGPBoolean		bPhotoInvalid;
	PGPBoolean		bKeyGenEnabled;
	BOOL			bReadyToPaint;
	BOOL			bNeedsCommit;
	HWND			hwndValidity;
	HWND			hwndStartDate;
	HWND			hwndExpireDate;
	HWND			hwndSubKeys;
	HWND			hwndADKs;
	HWND			hwndRevokers;
	HWND			hwndRevokerDlg;
	INT				iNumPhotoIDs;
	INT				iImageIndex;
	HBITMAP			hbitmapPhotoID;
	HPALETTE		hpalettePhotoID;
	INT				iwidthPhotoID;
	INT				iheightPhotoID;
	LPBYTE			pPhotoBuffer;
	PGPSize			iPhotoBufferLength;
	PGPUInt32		uMinSubkeySize;
} KMPROPSHEETSTRUCT;

typedef struct {
	FARPROC			lpfnCallback;
	PKEYMAN			pKM;
	BOOL			bCertProps;
} PROPSTRUCT;

typedef struct {
	PKEYMAN			pKM;
	PGPKeyRef		key;
	PGPSigRef		cert;
	PGPKeySetRef	keyset;
	PGPUserIDRef	userid;
} THREADSTRUCT;

typedef struct {
	PGPSubKeyRef	subkey;
	PGPTime			timeStart;
	INT				iExpireDays;
	BOOL			bNeverExpires;
	UINT			uSize;
} SUBKEYSTRUCT, *PSUBKEYSTRUCT;

typedef struct {
	KMPROPSHEETSTRUCT*	pkmpss;
	PSUBKEYSTRUCT		psks;
	LPSTR				pszPhrase;
	PGPByte*			pPasskey;
	PGPSize				sizePasskey;
	HWND				hwndProgress;
	BOOL				bCancelPending;
	BOOL				bGenDone;
	BOOL				bGenOK;
	BOOL				bAVIStarted;
	BOOL				bAVIFinished;
} SUBKEYGENSTRUCT, *PSUBKEYGENSTRUCT;

// external globals
extern HINSTANCE g_hInst;

// local globals
static UINT uSubkeySizes[] = {768, 1024, 1536, 2048, 3072};

static DWORD aKeyPropIds[] = {
	IDC_CHANGEPHRASE,		IDH_PGPKM_CHANGEPHRASE,	// this must be first item
	IDC_KEYID,				IDH_PGPKM_KEYID,
	IDC_KEYTYPE,			IDH_PGPKM_KEYTYPE,
	IDC_KEYSIZE,			IDH_PGPKM_KEYSIZE,
	IDC_CREATEDATE,			IDH_PGPKM_CREATEDATE,
	IDC_EXPIREDATE,			IDH_PGPKM_KEYEXPIRES,
	IDC_CIPHER,				IDH_PGPKM_CIPHER,
	IDC_HEXFINGERPRINT,		IDH_PGPKM_HEXFINGERPRINT,
	IDC_FINGERPRINT1,		IDH_PGPKM_FINGERPRINT,
	IDC_FINGERPRINT2,		IDH_PGPKM_FINGERPRINT,
	IDC_FINGERPRINT3,		IDH_PGPKM_FINGERPRINT,
	IDC_FINGERPRINT4,		IDH_PGPKM_FINGERPRINT,
	IDC_USEHEXFINGERPRINT,	IDH_PGPKM_DISPLAYHEXFINGERPRINT,
	IDC_ENABLED,			IDH_PGPKM_ENABLED,
	IDC_PHOTOID,			IDH_PGPKM_PHOTOID,
	IDC_VALIDITYBAR,		IDH_PGPKM_VALIDITYBAR,
	IDC_TRUSTSLIDER,		IDH_PGPKM_TRUSTSLIDER,
	IDC_AXIOMATIC,			IDH_PGPKM_AXIOMATIC,
    0,0 
}; 

static DWORD aNewSubkeyIds[] = {
	IDC_SUBKEYSIZE,		IDH_PGPKM_NEWSUBKEYSIZE,
	IDC_STARTDATE,		IDH_PGPKM_NEWSUBKEYSTARTDATE,
	IDC_NEVEREXPIRES,	IDH_PGPKM_NEWSUBKEYNEVEREXPIRES,
	IDC_EXPIRESON,		IDH_PGPKM_NEWSUBKEYEXPIRES,
	IDC_EXPIRATIONDATE,	IDH_PGPKM_NEWSUBKEYEXPIREDATE,
    0,0 
}; 

static DWORD aSubkeyIds[] = {
	IDC_SUBKEYLIST,		IDH_PGPKM_SUBKEYLIST,
	IDC_NEWSUBKEY,		IDH_PGPKM_SUBKEYCREATE,
	IDC_REVOKESUBKEY,	IDH_PGPKM_SUBKEYREVOKE,
	IDC_REMOVESUBKEY,	IDH_PGPKM_SUBKEYREMOVE,
    0,0 
}; 

static DWORD aADKIds[] = {
	IDC_FRAME,			IDH_PGPKM_ADKLIST,
	IDC_ADKTREELIST,	IDH_PGPKM_ADKLIST,
    0,0 
}; 

static DWORD aRevokerIds[] = {
	IDC_FRAME,			IDH_PGPKM_REVOKERLIST,
	IDC_REVOKERTREELIST,IDH_PGPKM_REVOKERLIST,
    0,0 
}; 

static DWORD aPGPcertIds[] = {
	IDC_NAME,			IDH_PGPKM_PGPCERTNAME,
	IDC_KEYID,			IDH_PGPKM_PGPCERTKEYID,
	IDC_CREATIONDATE,	IDH_PGPKM_PGPCERTCREATION,
	IDC_EXPIRATIONDATE,	IDH_PGPKM_PGPCERTEXPIRATION,
	IDC_EXPORTABLE,		IDH_PGPKM_PGPCERTEXPORTABLE,
	IDC_EXPIRED,		IDH_PGPKM_PGPCERTEXPIRED,
	IDC_REVOKED,		IDH_PGPKM_PGPCERTREVOKED,
	IDC_SHOWSIGNER,		IDH_PGPKM_PGPCERTSHOWSIGNER,
    0,0 
}; 

static DWORD aX509certIds[] = {
	IDC_NAME,			IDH_PGPKM_X509CERTNAME,
	IDC_ISSUER,			IDH_PGPKM_X509CERTISSUER,
	IDC_LASTCRL,		IDH_PGPKM_X509CERTLASTCRL,
	IDC_NEXTCRL,		IDH_PGPKM_X509CERTNEXTCRL,
	IDC_CREATIONDATE,	IDH_PGPKM_X509CERTCREATION,
	IDC_EXPIRATIONDATE,	IDH_PGPKM_X509CERTEXPIRATION,
	IDC_EXPORTABLE,		IDH_PGPKM_X509CERTEXPORTABLE,
	IDC_EXPIRED,		IDH_PGPKM_X509CERTEXPIRED,
	IDC_REVOKED,		IDH_PGPKM_X509CERTREVOKED,
	IDC_SHOWSIGNER,		IDH_PGPKM_X509CERTSHOWSIGNER,
    0,0 
}; 

// prototypes
static VOID 
sSingleKeyProperties (
		PKEYMAN			pKM,
		PGPKeyRef		key,
		PGPKeySetRef	keyset,
		PGPUserIDRef	userid,
		PBOOL			pbContinue);

static VOID 
sSingleCertProperties (
		PKEYMAN			pKM,
		PGPSigRef		cert,
		PGPKeyRef		key,
		PGPKeySetRef	keyset,
		PGPUserIDRef	userid,
		PBOOL			pbContinue);

//	___________________________________________________
//
//	copy the appropriate hash word to the string buffer

static VOID
sGetHashWord (
		PGPByte		bIndex,
		PGPBoolean	bEven,
		LPSTR		psz)
{
	if (bEven)
		lstrcpy (psz, &hashWordListEven[bIndex][0]);
	else
		lstrcpy (psz, &hashWordListOdd[bIndex][0]);
}

//	___________________________________________________
//
//	set the fingerprint controls on basis of "usehex" flag

static VOID
sSetFingerprintControls (
		 HWND		hwnd,
		 PGPBoolean	bUseHex,
		 PGPKeyRef	key,
		 PGPUInt32	uAlg)
{
	UINT		u, uWordsTotal;
	PGPBoolean	bEven;
	CHAR		sz[64];
	CHAR		sz1[64];
	CHAR		sz2[256];

	if (bUseHex) {
		ShowWindow (GetDlgItem (hwnd, IDC_HEXFINGERPRINT), TRUE);
		ShowWindow (GetDlgItem (hwnd, IDC_FINGERPRINT1), FALSE);
		ShowWindow (GetDlgItem (hwnd, IDC_FINGERPRINT2), FALSE);
		ShowWindow (GetDlgItem (hwnd, IDC_FINGERPRINT3), FALSE);
		ShowWindow (GetDlgItem (hwnd, IDC_FINGERPRINT4), FALSE);

		PGPGetKeyPropertyBuffer (key, 
				kPGPKeyPropFingerprint, sizeof(sz), sz, &u);
		KMConvertStringFingerprint (uAlg, sz);
		SetDlgItemText (hwnd, IDC_HEXFINGERPRINT, sz);
	}
	else {
		ShowWindow (GetDlgItem (hwnd, IDC_HEXFINGERPRINT), FALSE);
		ShowWindow (GetDlgItem (hwnd, IDC_FINGERPRINT1), TRUE);
		ShowWindow (GetDlgItem (hwnd, IDC_FINGERPRINT2), TRUE);
		ShowWindow (GetDlgItem (hwnd, IDC_FINGERPRINT3), TRUE);
		ShowWindow (GetDlgItem (hwnd, IDC_FINGERPRINT4), TRUE);

		PGPGetKeyPropertyBuffer (key, 
				kPGPKeyPropFingerprint, sizeof(sz1), sz1, &u);

		if (uAlg == kPGPPublicKeyAlgorithm_RSA) 
			uWordsTotal = 16;
		else
			uWordsTotal = 20;

		sz2[0] = '\0';
		bEven = TRUE;
		for (u=0; u<uWordsTotal; u+=NUMHASHWORDCOLS) {
			sGetHashWord (sz1[u], bEven, sz);
			lstrcat (sz2, sz);
			lstrcat (sz2, "\n");
		}
		SetDlgItemText (hwnd, IDC_FINGERPRINT1, sz2);

		sz2[0] = '\0';
		bEven = !bEven;
		for (u=1; u<uWordsTotal; u+=NUMHASHWORDCOLS) {
			sGetHashWord (sz1[u], bEven, sz);
			lstrcat (sz2, sz);
			lstrcat (sz2, "\n");
		}
		SetDlgItemText (hwnd, IDC_FINGERPRINT2, sz2);

		sz2[0] = '\0';
		bEven = !bEven;
		for (u=2; u<uWordsTotal; u+=NUMHASHWORDCOLS) {
			sGetHashWord (sz1[u], bEven, sz);
			lstrcat (sz2, sz);
			lstrcat (sz2, "\n");
		}
		SetDlgItemText (hwnd, IDC_FINGERPRINT3, sz2);

		sz2[0] = '\0';
		bEven = !bEven;
		for (u=3; u<uWordsTotal; u+=NUMHASHWORDCOLS) {
			sGetHashWord (sz1[u], bEven, sz);
			lstrcat (sz2, sz);
			lstrcat (sz2, "\n");
		}
		SetDlgItemText (hwnd, IDC_FINGERPRINT4, sz2);
	}
}

//	___________________________________________________
//
//	convert SYSTEMTIME structure to number of days from today

static PGPError
sSystemTimeToPGPTime (
		 SYSTEMTIME*	pst, 
		 PGPTime*		ptime) 
{
	struct tm	tmstruct;
	time_t		timeStd;

	pgpAssert (pst != NULL);
	pgpAssert (ptime != NULL);

	*ptime = 0;

	if (pst->wYear > 2037) 
		return kPGPError_BadParams;

	tmstruct.tm_mday = pst->wDay;
	tmstruct.tm_mon = pst->wMonth -1;
	tmstruct.tm_year = pst->wYear -1900;
	tmstruct.tm_hour = 0;
	tmstruct.tm_min = 0;
	tmstruct.tm_sec = 0;
	tmstruct.tm_isdst = -1;

	timeStd = mktime (&tmstruct);
	if (timeStd == (time_t)-1) return kPGPError_BadParams;

	*ptime = PGPGetPGPTimeFromStdTime (timeStd);

	return kPGPError_NoErr;
}


//	____________________________________
//
//  display keygen AVI file in specified window

static VOID
sStartKeyGenAVI (HWND hwnd, LPSTR szHelpFile)
{
	CHAR	szFile[32];
	CHAR	szAnimationFile[MAX_PATH];
	LPSTR	p;

	lstrcpy (szAnimationFile, szHelpFile);

	p = strrchr (szAnimationFile, '\\');
	if (!p)
		p = szAnimationFile;
	else
		++p;
	*p = '\0';

	LoadString (g_hInst, IDS_ANIMATIONFILE, szFile, sizeof(szFile));
	lstrcat (szAnimationFile, szFile);

	Animate_Open (hwnd, szAnimationFile);
	Animate_Play (hwnd, 0, -1, -1);
}


//	______________________________________________
//
//  callback routine called by library key generation routine
//  every so often with status of keygen.  Returning a nonzero
//  value cancels the key generation.

static PGPError 
sSubkeyGenEventHandler (
		PGPContextRef	context, 
		PGPEvent*		event,
		PGPUserValue	userValue)
{
	INT					iReturnCode = kPGPError_NoErr;
	PSUBKEYGENSTRUCT	pskgs;

	pskgs = (PSUBKEYGENSTRUCT) userValue;

	if (pskgs->bCancelPending) 
		iReturnCode = kPGPError_UserAbort;
	
	return (iReturnCode);
}


//	___________________________________________________
//
//  subkey generation thread

static VOID 
sSubkeyGenerationThread (void *pArgs)
{
	PSUBKEYGENSTRUCT	pskgs		= (PSUBKEYGENSTRUCT)pArgs;

	BOOL			bRetVal				= FALSE;

	PGPBoolean		bFastGen;
	PGPPrefRef		prefref;
	PGPContextRef	ctx;
	UINT			uEntropyNeeded;
	PGPError		err;

	// get client preferences
	KMRequestSDKAccess (pskgs->pkmpss->pKM);
	PGPclOpenClientPrefs (
		PGPGetContextMemoryMgr (pskgs->pkmpss->pKM->Context), &prefref);
	PGPGetPrefBoolean (prefref, kPGPPrefFastKeyGen, &bFastGen);
	PGPclCloseClientPrefs (prefref, FALSE);

	// generate subkey
	ctx = pskgs->pkmpss->pKM->Context;
	uEntropyNeeded = PGPGetKeyEntropyNeeded (ctx,
			PGPOKeyGenParams (ctx, 
						kPGPPublicKeyAlgorithm_ElGamal, 
						pskgs->psks->uSize),
			PGPOKeyGenFast (ctx, bFastGen),
			PGPOLastOption (ctx));
	PGPclRandom (ctx, pskgs->hwndProgress, uEntropyNeeded);

	if (pskgs->pszPhrase) {
		err = PGPGenerateSubKey (
			ctx, &pskgs->psks->subkey,
			PGPOKeyGenMasterKey (ctx, pskgs->pkmpss->key),
			PGPOKeyGenParams (ctx, 
						kPGPPublicKeyAlgorithm_ElGamal, 
						pskgs->psks->uSize),
			PGPOKeyGenFast (ctx, bFastGen),
			PGPOPassphrase (ctx, pskgs->pszPhrase),
			PGPOCreationDate (ctx, pskgs->psks->timeStart),
			PGPOExpiration (ctx, pskgs->psks->iExpireDays),
			PGPOEventHandler (ctx, sSubkeyGenEventHandler, pskgs),
			PGPOLastOption (ctx));
	}
	else {
		err = PGPGenerateSubKey (
			ctx, &pskgs->psks->subkey,
			PGPOKeyGenMasterKey (ctx, pskgs->pkmpss->key),
			PGPOKeyGenParams (ctx, 
						kPGPPublicKeyAlgorithm_ElGamal, 
						pskgs->psks->uSize),
			PGPOKeyGenFast (ctx, bFastGen),
			PGPOPasskeyBuffer (ctx, pskgs->pPasskey, pskgs->sizePasskey),
			PGPOCreationDate (ctx, pskgs->psks->timeStart),
			PGPOExpiration (ctx, pskgs->psks->iExpireDays),
			PGPOEventHandler (ctx, sSubkeyGenEventHandler, pskgs),
			PGPOLastOption (ctx));
	}
	// note: PGPGenerateSubKey returns kPGPError_OutOfMemory 
	// when user aborts!
	if (err == kPGPError_OutOfMemory) 
		err = kPGPError_UserAbort;
	KMReleaseSDKAccess (pskgs->pkmpss->pKM);

	PGPclErrorBox (pskgs->hwndProgress, err);

	pskgs->bGenDone = TRUE;
	if (IsntPGPError (err))
		pskgs->bGenOK = TRUE;

	SendMessage (pskgs->hwndProgress, WM_CLOSE, 0, 0);

	return;
}


//	___________________________________________________
//
//  check for valid date settings

static VOID 
sValidateSubKeyDates (
		HWND				hDlg, 
		KMPROPSHEETSTRUCT*	pkmpss) 
{
	BOOL		bOK;
	SYSTEMTIME	st;
	INT			iStartDays;
	INT			iExpireDays;

	// get starting date
	SendMessage (pkmpss->hwndStartDate, DTM_GETSYSTEMTIME, 0, 
					(LPARAM)&st);
	PGPclSystemTimeToDays (&st, &iStartDays);

	// get expiration date
	iExpireDays = iStartDays+1;
	if (IsDlgButtonChecked (hDlg, IDC_NEVEREXPIRES) == BST_UNCHECKED) {
		SendMessage (pkmpss->hwndExpireDate, DTM_GETSYSTEMTIME, 0, 
					(LPARAM)&st);
		PGPclSystemTimeToDays (&st, &iExpireDays);
	}

	if ((iStartDays >= 0) && 
		(iExpireDays > iStartDays) &&
		((pkmpss->iExpireDays == -1) ||
		 (iExpireDays <= pkmpss->iExpireDays)))
		bOK = TRUE;
	else 
		bOK = FALSE;

	if (bOK)
		EnableWindow (GetDlgItem (hDlg, IDOK), TRUE);
	else
		EnableWindow (GetDlgItem (hDlg, IDOK), FALSE);
}

//	___________________________________________________
//
//  subkey generation progress dialog procedure

static BOOL CALLBACK
sSubkeyGenProgressDlgProc (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam,
		LPARAM	lParam) 
{
	PSUBKEYGENSTRUCT	pskgs;
	DWORD				dw;

	switch (uMsg) {

	case WM_INITDIALOG:
		// store pointer to data structure
		SetWindowLong (hDlg, GWL_USERDATA, lParam);
		pskgs = (PSUBKEYGENSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
		pskgs->bAVIStarted = FALSE;
		pskgs->bAVIFinished = FALSE;
		pskgs->hwndProgress = hDlg;

		// Kick off generation proc, here
		_beginthreadex (NULL, 0, 
				(LPTHREAD_START_ROUTINE)sSubkeyGenerationThread, 
				(void *)pskgs, 0, &dw);
		SetTimer (hDlg, AVI_TIMER, 100, NULL);  // delay a few ms
												// before drawing
												// AVI
		return TRUE;

	case WM_TIMER :
		if (wParam == AVI_TIMER) {
			pskgs = (PSUBKEYGENSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
			KillTimer (hDlg, AVI_TIMER);
			if (pskgs->bAVIStarted) {
				if (!pskgs->bAVIFinished) {
					pskgs->bAVIFinished = TRUE;
					if (pskgs->bGenDone) 
						PostMessage (hDlg, WM_CLOSE, 0, 0);
				}
			}
			else {
				sStartKeyGenAVI (GetDlgItem (hDlg, IDC_SUBKEYAVI),
									pskgs->pkmpss->pKM->szHelpFile);
				SetTimer (hDlg, AVI_TIMER, AVI_RUNTIME, NULL);
				pskgs->bAVIStarted = TRUE;
			}
		}
		break;
	
	case WM_DESTROY :
		pskgs = (PSUBKEYGENSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
		pskgs->bAVIFinished = TRUE;
		Animate_Close (GetDlgItem (hDlg, IDC_SUBKEYAVI));
		break;

	case WM_CLOSE :
		pskgs = (PSUBKEYGENSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
		EndDialog (hDlg, pskgs->bGenOK);
		break;

	case WM_COMMAND :
		switch(LOWORD (wParam)) {
		case IDCANCEL :
			pskgs = (PSUBKEYGENSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
			pskgs->bCancelPending = TRUE;
			break;
		}
		break;
	}

	return FALSE;
}

//	___________________________________________________
//
//  generate new subkey

static BOOL 
sAddNewSubkey (
		HWND				hDlg, 
		KMPROPSHEETSTRUCT*	pkmpss) 
{
	BOOL				bRetVal				= FALSE;

	PSUBKEYSTRUCT		psks;
	SUBKEYGENSTRUCT		skgs;

	SYSTEMTIME			st;
	UINT				uSubKeySize;
	CHAR				szSize[16];
	CHAR				szValid[32];
	CHAR				szExpires[32];
	LV_ITEM				lvI;
	INT					iItem, iStartDays;
	CHAR				sz[64];
	CHAR				sz2[128];
	PGPError			err;

	// initialize structs
	skgs.pkmpss = pkmpss;
	skgs.pszPhrase = NULL;
	skgs.pPasskey = NULL;
	skgs.bCancelPending = FALSE;
	skgs.bGenDone = FALSE;
	skgs.bGenOK = FALSE;

	// get subkey size
	uSubKeySize = 0;
	GetDlgItemText (hDlg, IDC_SUBKEYSIZE, szSize, sizeof(szSize) -1);
	uSubKeySize = atoi (szSize);
	if ((uSubKeySize < pkmpss->uMinSubkeySize) || 
		(uSubKeySize > MAXSUBKEYSIZE)) {

		LoadString (g_hInst, IDS_BADSUBKEYSIZE, sz, sizeof(sz));
		wsprintf (sz2, sz, pkmpss->uMinSubkeySize);
		LoadString (g_hInst, IDS_CAPTION, sz, sizeof(sz));
		MessageBox (hDlg, sz2, sz, MB_OK | MB_ICONEXCLAMATION);
		return FALSE;

	}
	wsprintf (szSize, "%i", uSubKeySize);

	// allocate new structure
	psks = KMAlloc (sizeof(SUBKEYSTRUCT));
	if (!psks) return FALSE;
	skgs.psks = psks;

	// initialize structure
	psks->subkey = NULL;
	psks->timeStart = 0;
	psks->iExpireDays = 0;
	psks->uSize = uSubKeySize;

	// get starting date
	SendMessage (pkmpss->hwndStartDate, DTM_GETSYSTEMTIME, 0, 
					(LPARAM)&st);
	sSystemTimeToPGPTime (&st, &psks->timeStart);
	GetDateFormat (LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, 
						NULL, szValid, sizeof(szValid));
	PGPclSystemTimeToDays (&st, &iStartDays);

	// get expiration date
	if (IsDlgButtonChecked (hDlg, IDC_NEVEREXPIRES) == BST_CHECKED) {
		psks->bNeverExpires = TRUE;
		psks->iExpireDays = 0;
	}
	else {
		psks->bNeverExpires = FALSE;
		SendMessage (pkmpss->hwndExpireDate, DTM_GETSYSTEMTIME, 0, 
					(LPARAM)&st);
		PGPclSystemTimeToDays (&st, &psks->iExpireDays);
		psks->iExpireDays -= iStartDays;
	}
	if (psks->bNeverExpires)
		LoadString (g_hInst, IDS_NEVER, szExpires, sizeof(szExpires));
	else
		GetDateFormat (LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, 
						NULL, szExpires, sizeof(szExpires));

	// get phrase from user
	LoadString (g_hInst, IDS_SELKEYPASSPHRASE, sz, sizeof(sz)); 
	KMRequestSDKAccess (pkmpss->pKM);
	err = KMGetKeyPhrase (pkmpss->pKM->Context, pkmpss->pKM->tlsContext,
					hDlg, sz, pkmpss->keyset, pkmpss->key,
					&skgs.pszPhrase, &skgs.pPasskey, &skgs.sizePasskey);
	KMReleaseSDKAccess (pkmpss->pKM);
	PGPclErrorBox (NULL, err);

	if (IsntPGPError (err)) {
		if (DialogBoxParam (g_hInst, MAKEINTRESOURCE (IDD_SUBKEYGENPROG), 
						hDlg, sSubkeyGenProgressDlgProc, (LPARAM)&skgs))
		{
			pkmpss->bNeedsCommit = TRUE;

			// figure item index to use
			iItem = ListView_GetItemCount (pkmpss->hwndSubKeys);

			// insert listview item
			lvI.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE | LVIF_PARAM;
			lvI.state = 0;      
			lvI.stateMask = 0;
			lvI.iImage = IDX_DSAPUBKEY;
		
			lvI.iItem = iItem;
			lvI.iSubItem = 0;
			lvI.pszText	= szValid; 
			lvI.cchTextMax = 0;
			lvI.lParam = (LPARAM)psks;

			iItem = ListView_InsertItem (pkmpss->hwndSubKeys, &lvI);
			if (iItem == -1) 
				KMFree (psks);
			else {
				// add strings for other columns
				ListView_SetItemText (pkmpss->hwndSubKeys, 
										iItem, 1, szExpires);
				ListView_SetItemText (pkmpss->hwndSubKeys, 
										iItem, 2, szSize);
				bRetVal = TRUE;
			}
		}
	}

	if (skgs.pszPhrase)
		KMFreePhrase (skgs.pszPhrase);

	if (skgs.pPasskey) 
		KMFreePasskey (skgs.pPasskey, skgs.sizePasskey);

	return bRetVal;
}


//	___________________________________________________
//
//  new subkey dialog procedure

static BOOL CALLBACK 
sNewSubkeyDlgProc (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam,
		LPARAM	lParam) 
{
	KMPROPSHEETSTRUCT*	pkmpss;
	RECT				rc;
	INT					i;
	INT					iMinDefaultSize;			
	CHAR				sz[8];
	SYSTEMTIME			st;

	pkmpss = (KMPROPSHEETSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);

	switch (uMsg) {

	case WM_INITDIALOG:
		// store pointer to data structure
		SetWindowLong (hDlg, GWL_USERDATA, lParam);
		pkmpss = (KMPROPSHEETSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);

		// create and initialize start date control
		GetWindowRect (GetDlgItem (hDlg, IDC_STARTDATE), &rc);
		MapWindowPoints (NULL, hDlg, (LPPOINT)&rc, 2);
		pkmpss->hwndStartDate = CreateWindowEx (0, DATETIMEPICK_CLASS,
                             "DateTime",
                             WS_BORDER|WS_CHILD|WS_VISIBLE|WS_TABSTOP,
                             rc.left, rc.top, 
							 rc.right-rc.left, rc.bottom-rc.top, 
							 hDlg, (HMENU)IDC_STARTDATE, 
							 g_hInst, NULL);
		SetWindowPos (pkmpss->hwndStartDate, 
					GetDlgItem (hDlg, IDC_STARTDATETEXT),
					0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
		SendMessage (pkmpss->hwndStartDate, DTM_SETMCCOLOR, 
						MCSC_MONTHBK, (LPARAM)GetSysColor (COLOR_3DFACE));

		// create and initialize expire date control
		GetWindowRect (GetDlgItem (hDlg, IDC_EXPIRATIONDATE), &rc);
		MapWindowPoints (NULL, hDlg, (LPPOINT)&rc, 2);
		pkmpss->hwndExpireDate = CreateWindowEx (0, DATETIMEPICK_CLASS,
                             "DateTime",
                             WS_BORDER|WS_CHILD|WS_VISIBLE|WS_TABSTOP,
                             rc.left, rc.top, 
							 rc.right-rc.left, rc.bottom-rc.top, 
							 hDlg, (HMENU)IDC_EXPIRATIONDATE, 
							 g_hInst, NULL);
		SendMessage (pkmpss->hwndExpireDate, DTM_SETMCCOLOR, 
						MCSC_MONTHBK, (LPARAM)GetSysColor (COLOR_3DFACE));
		SetWindowPos (pkmpss->hwndExpireDate, 
					GetDlgItem (hDlg, IDC_EXPIRESON),
					0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
		EnableWindow (pkmpss->hwndExpireDate, FALSE);
		CheckDlgButton (hDlg, IDC_NEVEREXPIRES, BST_CHECKED);

		GetLocalTime (&st);
		st.wYear++;
		SendMessage (pkmpss->hwndExpireDate, DTM_SETSYSTEMTIME,
							GDT_VALID, (LPARAM)&st);

		// initialize subkey size combo box
		iMinDefaultSize = 0;
		for (i=0; i<(sizeof(uSubkeySizes)/sizeof(UINT)); i++) {
			if (uSubkeySizes[i] >= pkmpss->uMinSubkeySize) {
				wsprintf (sz, "%i", uSubkeySizes[i]);
				SendDlgItemMessage (hDlg, IDC_SUBKEYSIZE, CB_ADDSTRING, 
									0, (LPARAM)sz);
				if (iMinDefaultSize == 0) 
					iMinDefaultSize = uSubkeySizes[i];
			}
		}
		if (iMinDefaultSize < DEFAULTSUBKEYSIZE)
			iMinDefaultSize = DEFAULTSUBKEYSIZE;
		wsprintf (sz, "%i", iMinDefaultSize);
		SetDlgItemText (hDlg, IDC_SUBKEYSIZE, sz);

		sValidateSubKeyDates (hDlg, pkmpss);

		return TRUE;

	case WM_COMMAND:
		switch(LOWORD (wParam)) {
		case IDCANCEL :
			EndDialog (hDlg, FALSE);
			break;

		case IDC_NEVEREXPIRES :
		case IDC_EXPIRESON :
			if (IsDlgButtonChecked (hDlg, IDC_EXPIRESON) == BST_CHECKED) {
				EnableWindow (pkmpss->hwndExpireDate, TRUE);
				sValidateSubKeyDates (hDlg, pkmpss);
			}
			else {
				EnableWindow (pkmpss->hwndExpireDate, FALSE);
				sValidateSubKeyDates (hDlg, pkmpss);
			}
			break;

		case IDOK :
			if (sAddNewSubkey (hDlg, pkmpss))
				EndDialog (hDlg, TRUE);
			break;
		}
		return TRUE;

    case WM_HELP: 
        WinHelp (((LPHELPINFO) lParam)->hItemHandle, pkmpss->pKM->szHelpFile, 
            HELP_WM_HELP, (DWORD) (LPSTR) aNewSubkeyIds); 
        break; 
 
    case WM_CONTEXTMENU: 
        WinHelp ((HWND) wParam, pkmpss->pKM->szHelpFile, HELP_CONTEXTMENU, 
            (DWORD) (LPVOID) aNewSubkeyIds); 
        break; 

	case WM_DESTROY :
		break;

	case WM_NOTIFY :
		sValidateSubKeyDates (hDlg, pkmpss);
		break;
	}
	return FALSE;
}

//	___________________________________________________
//
//  remove subkey from listview control

static BOOL 
sRemoveSubKey (HWND hDlg, KMPROPSHEETSTRUCT* pkmpss)
{
	INT				iIndex;
	INT				iNumItems;
	INT				ids;
	LV_ITEM			lvI;
	PGPError		err;
	PSUBKEYSTRUCT	psks;

	iNumItems = ListView_GetItemCount (pkmpss->hwndSubKeys);
	if (iNumItems > 1) ids = IDS_REMOVESUBKEYCONF;
	else ids = IDS_REMOVEONLYSUBKEYCONF;

	if (KMMessageBox (hDlg, IDS_CAPTION, ids, 
			MB_YESNO | MB_ICONEXCLAMATION) == IDNO) 
		return FALSE;

	iIndex = ListView_GetNextItem (pkmpss->hwndSubKeys, -1, LVNI_SELECTED);

	if (iIndex > -1) {
		lvI.mask = LVIF_PARAM;
		lvI.iItem = iIndex;
		lvI.iSubItem = 0;
		ListView_GetItem (pkmpss->hwndSubKeys, &lvI);

		psks = (PSUBKEYSTRUCT)(lvI.lParam);

		KMRequestSDKAccess (pkmpss->pKM);
		err = PGPRemoveSubKey (psks->subkey);
		KMReleaseSDKAccess (pkmpss->pKM);
	
		if (IsntPGPError (PGPclErrorBox (hDlg, err))) {
			pkmpss->bNeedsCommit = TRUE;
			KMFree ((VOID*)lvI.lParam);
			ListView_DeleteItem (pkmpss->hwndSubKeys, iIndex);
		}
	}

	return TRUE;
}


//	___________________________________________________
//
//  revoke subkey from listview control

static BOOL 
sRevokeSubKey (HWND hDlg, KMPROPSHEETSTRUCT* pkmpss)
{
	PGPByte*		pPasskey			= NULL;
	PGPSize			sizePasskey			= 0;

	INT				iIndex;
	LV_ITEM			lvI;
	PGPError		err;
	PSUBKEYSTRUCT	psks;
	CHAR			sz[64];

	if (KMMessageBox (hDlg, IDS_CAPTION, IDS_REVOKESUBKEYCONF, 
			MB_YESNO | MB_ICONEXCLAMATION) == IDNO) 
		return FALSE;

	// get phrase from user if necessary
	LoadString (g_hInst, IDS_SELKEYPASSPHRASE, sz, sizeof(sz)); 
	KMRequestSDKAccess (pkmpss->pKM);
	err = KMGetKeyPhrase (pkmpss->pKM->Context, pkmpss->pKM->tlsContext,
					hDlg, sz, pkmpss->keyset, pkmpss->key,
					NULL, &pPasskey, &sizePasskey);
	KMReleaseSDKAccess (pkmpss->pKM);
	PGPclErrorBox (NULL, err);

	if (IsntPGPError (err)) {
		iIndex = ListView_GetNextItem (pkmpss->hwndSubKeys, 
										-1, LVNI_SELECTED);
		if (iIndex > -1) {
			lvI.mask = LVIF_PARAM;
			lvI.iItem = iIndex;
			lvI.iSubItem = 0;
			ListView_GetItem (pkmpss->hwndSubKeys, &lvI);

			psks = (PSUBKEYSTRUCT)(lvI.lParam);

			KMRequestSDKAccess (pkmpss->pKM);
			err = PGPRevokeSubKey (psks->subkey, 
							pPasskey ?
								PGPOPasskeyBuffer (pkmpss->pKM->Context, 
									pPasskey, sizePasskey) :
								PGPONullOption (pkmpss->pKM->Context),
							PGPOLastOption (pkmpss->pKM->Context));
			KMReleaseSDKAccess (pkmpss->pKM);

			if (IsntPGPError (PGPclErrorBox (hDlg, err))) {
				pkmpss->bNeedsCommit = TRUE;
				lvI.mask = LVIF_IMAGE;
				lvI.iItem = iIndex;
				lvI.iSubItem = 0;
				lvI.iImage = IDX_DSAPUBREVKEY;
				ListView_SetItem (pkmpss->hwndSubKeys, &lvI);
			}
		}
	}

	if (pPasskey) 
		KMFreePasskey (pPasskey, sizePasskey);

	return TRUE;
}


//	___________________________________________________
//
//  Populate ListView with subkeys

static BOOL 
sInsertSubkeysIntoList (
		HWND			hwndList, 
		PGPKeyRef		key,
		PGPKeySetRef	keyset) 
{
	PGPSubKeyRef	subkey;
	PGPKeyListRef	keylist;
	PGPKeyIterRef	keyiter;
	LV_ITEM			lvI;
	CHAR			szSize[16];
	CHAR			szValid[32];
	CHAR			szExpires[32];
	PGPBoolean		bRevoked;
	PGPBoolean		bExpired;
	INT				iItem;
	PGPTime			time;
	PSUBKEYSTRUCT	psks;

	iItem = 0;

	PGPOrderKeySet (keyset, kPGPAnyOrdering, &keylist);
	PGPNewKeyIter (keylist, &keyiter);
	PGPKeyIterSeek (keyiter, key);
	PGPKeyIterNextSubKey (keyiter, &subkey);
	while (subkey) {

		// allocate structure to hold info
		psks = KMAlloc (sizeof(SUBKEYSTRUCT));
		if (!psks) break;

		// initialize structure
		psks->subkey = subkey;
		psks->uSize = 0;

		// get subkey info
		PGPGetSubKeyNumber (subkey, kPGPKeyPropBits, &psks->uSize);
		wsprintf (szSize, "%i", psks->uSize);

		PGPGetSubKeyBoolean (subkey, kPGPKeyPropIsExpired, &bExpired);
		PGPGetSubKeyBoolean (subkey, kPGPKeyPropIsRevoked, &bRevoked);

		PGPGetSubKeyTime (subkey, kPGPKeyPropCreation, &psks->timeStart);
		KMConvertTimeToString (psks->timeStart, szValid, sizeof (szValid));

		PGPGetSubKeyTime (subkey, kPGPKeyPropExpiration, &time);
		if (time != kPGPExpirationTime_Never) {
			psks->bNeverExpires = FALSE;
			KMConvertTimeToDays (time, &psks->iExpireDays);
			KMConvertTimeToString (time, szExpires, sizeof (szExpires));
		}
		else {
			psks->bNeverExpires = TRUE;
			psks->iExpireDays = 0;
			LoadString (g_hInst, IDS_NEVER, szExpires, sizeof (szExpires));
		}

		// insert listview item
		lvI.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE | LVIF_PARAM;
		lvI.state = 0;      
		lvI.stateMask = 0;
		if (bRevoked) 
			lvI.iImage = IDX_DSAPUBREVKEY;
		else if (bExpired)
			lvI.iImage = IDX_DSAPUBEXPKEY;
		else
			lvI.iImage = IDX_DSAPUBKEY;

		lvI.iItem = iItem;
		lvI.iSubItem = 0;
		lvI.pszText	= szValid; 
		lvI.cchTextMax = 0;
		lvI.lParam = (LPARAM)psks;

		if (ListView_InsertItem (hwndList, &lvI) == -1) return FALSE;

		// add strings for other columns
		ListView_SetItemText (hwndList, iItem, 1, szExpires);
		ListView_SetItemText (hwndList, iItem, 2, szSize);

		PGPKeyIterNextSubKey (keyiter, &subkey);

		iItem++;
	}

	PGPFreeKeyIter (keyiter);
	PGPFreeKeyList (keylist);

	return TRUE;
}


//	___________________________________________________
//
//  destroy all data structures associated with ListView

static VOID
sDestroySubKeyListAndStructures (KMPROPSHEETSTRUCT* pkmpss)
{
	INT		iIndex;
	INT		iNumItems;
	LV_ITEM	lvI;

	iNumItems = ListView_GetItemCount (pkmpss->hwndSubKeys);
	for (iIndex=0; iIndex<iNumItems; iIndex++) {
		lvI.mask = LVIF_PARAM;
		lvI.iItem = iIndex;
		lvI.iSubItem = 0;
		ListView_GetItem (pkmpss->hwndSubKeys, &lvI);
		KMFree ((VOID*)(lvI.lParam));
	}

	ListView_DeleteAllItems (pkmpss->hwndSubKeys);

	ImageList_Destroy (pkmpss->hIml);
}


//	___________________________________________________
//
//  create imagelist 

static VOID 
sCreateImageList (KMPROPSHEETSTRUCT* pkmpss) 
{
	HBITMAP		hBmp;
	HDC			hDC;
	INT			iNumBits;

	if (pkmpss->hIml)
		return;

	// create image list
	hDC = GetDC (NULL);		// DC for desktop
	iNumBits = GetDeviceCaps (hDC, BITSPIXEL) * GetDeviceCaps (hDC, PLANES);
	ReleaseDC (NULL, hDC);

	if (!pkmpss->hIml) {
		if (iNumBits <= 8) {
			pkmpss->hIml = ImageList_Create (16, 16, ILC_COLOR|ILC_MASK, 
											NUM_BITMAPS, 0); 
			hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_IMAGES4BIT));
			ImageList_AddMasked (pkmpss->hIml, hBmp, RGB(255, 0, 255));
			DeleteObject (hBmp);
		}
		else {
			pkmpss->hIml = ImageList_Create (16, 16, ILC_COLOR24|ILC_MASK, 
											NUM_BITMAPS, 0); 
			hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_IMAGES24BIT));
			ImageList_AddMasked (pkmpss->hIml, hBmp, RGB(255, 0, 255));
			DeleteObject (hBmp);
		}
	}
}


//	___________________________________________________
//
//  setup subkey ListView 

static VOID 
sFillSubKeyList (HWND hDlg, KMPROPSHEETSTRUCT* pkmpss) 
{

	LV_COLUMN	lvC; 
	CHAR		sz[256];

	sCreateImageList (pkmpss);
	ListView_SetImageList (pkmpss->hwndSubKeys, pkmpss->hIml, LVSIL_SMALL);

	lvC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lvC.fmt = LVCFMT_CENTER; 
	lvC.pszText = sz;
	
	LoadString (g_hInst, IDS_VALIDFROM, sz, sizeof(sz));
	lvC.cx = 120;  
	lvC.iSubItem = 0;
	if (ListView_InsertColumn (pkmpss->hwndSubKeys, 0, &lvC) == -1) return;

	LoadString (g_hInst, IDS_EXPIRES, sz, sizeof(sz));
	lvC.cx = 70;   
	lvC.iSubItem = 1;
	if (ListView_InsertColumn (pkmpss->hwndSubKeys, 1, &lvC) == -1) return;

	LoadString (g_hInst, IDS_SIZE, sz, sizeof(sz));
	lvC.cx = 70;   
	lvC.iSubItem = 2;
	if (ListView_InsertColumn (pkmpss->hwndSubKeys, 2, &lvC) == -1) return;

	// populate control by iterating through subkeys
	sInsertSubkeysIntoList (pkmpss->hwndSubKeys, pkmpss->key, 
							pkmpss->keyset);
}


//	___________________________________________________
//
//  Key Properties Dialog Message procedure - subkey panel

static BOOL CALLBACK 
sKeyPropDlgProcSubkey (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam,
		LPARAM	lParam) 
{
	KMPROPSHEETSTRUCT*	pkmpss;
	INT					iIndex;
	NMHDR*				pnmh;

	pkmpss = (KMPROPSHEETSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);

	switch (uMsg) {

	case WM_INITDIALOG:
		// store pointer to data structure
		SetWindowLong (hDlg, GWL_USERDATA, ((PROPSHEETPAGE*)lParam)->lParam);
		pkmpss = (KMPROPSHEETSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
		pkmpss->hwndSubKeys = GetDlgItem (hDlg, IDC_SUBKEYLIST);

		// hide explanatory text for non-keypairs
		if (!pkmpss->bSecret) {
			ShowWindow (GetDlgItem (hDlg, IDC_STATICTEXT1), SW_HIDE);
			ShowWindow (GetDlgItem (hDlg, IDC_STATICTEXT2), SW_HIDE);
		}

		// initialize all controls
		PostMessage (hDlg, WM_APP, SERIALIZE, 0);
		return TRUE;

	case WM_APP :
		if (wParam == SERIALIZE)
			KMRequestSDKAccess (pkmpss->pKM);
		sFillSubKeyList (hDlg, pkmpss);
		if (wParam == SERIALIZE)
			KMReleaseSDKAccess (pkmpss->pKM);
		if (pkmpss->bKeyGenEnabled) {
			if (pkmpss->bSecret && !pkmpss->bRevoked && 
					!pkmpss->bExpired && !pkmpss->bReadOnly)
				EnableWindow (GetDlgItem (hDlg, IDC_NEWSUBKEY), TRUE);
		}
		break;

	case WM_COMMAND:
		switch(LOWORD (wParam)) {
		case IDC_NEWSUBKEY :
			DialogBoxParam (g_hInst, MAKEINTRESOURCE (IDD_NEWSUBKEY), 
						hDlg, sNewSubkeyDlgProc, (LPARAM)pkmpss);
			break;

		case IDC_REVOKESUBKEY :
			if (sRevokeSubKey (hDlg, pkmpss)) {
				EnableWindow (GetDlgItem (hDlg, IDC_REVOKESUBKEY), FALSE);
				EnableWindow (GetDlgItem (hDlg, IDC_REMOVESUBKEY), FALSE);
			}
			break;

		case IDC_REMOVESUBKEY :
			if (sRemoveSubKey (hDlg, pkmpss)) {
				EnableWindow (GetDlgItem (hDlg, IDC_REVOKESUBKEY), FALSE);
				EnableWindow (GetDlgItem (hDlg, IDC_REMOVESUBKEY), FALSE);
			}
			break;
		}
		return TRUE;

    case WM_HELP: 
        WinHelp (((LPHELPINFO) lParam)->hItemHandle, pkmpss->pKM->szHelpFile, 
            HELP_WM_HELP, (DWORD) (LPSTR) aSubkeyIds); 
        break; 
 
    case WM_CONTEXTMENU: 
        WinHelp ((HWND) wParam, pkmpss->pKM->szHelpFile, HELP_CONTEXTMENU, 
            (DWORD) (LPVOID) aSubkeyIds); 
        break; 

	case WM_DESTROY :
		sDestroySubKeyListAndStructures (pkmpss);
		break;

	case WM_NOTIFY :
		pnmh = (NMHDR*)lParam;
		switch (pnmh->code) {

		case PSN_HELP :
			WinHelp (hDlg, pkmpss->pKM->szHelpFile, HELP_CONTEXT, 
				IDH_PGPKM_PROPDIALOG); 
			break;

		case NM_CLICK :
		case LVN_KEYDOWN :
			if ((pnmh->idFrom == IDC_SUBKEYLIST) && 
				 pkmpss->bSecret &&
				!pkmpss->bReadOnly) 
			{
				iIndex = 
					ListView_GetNextItem (pkmpss->hwndSubKeys,
														-1, LVNI_SELECTED);
				if (iIndex > -1) {
					LV_ITEM lvI;

					lvI.mask = LVIF_IMAGE|LVIF_PARAM;
					lvI.iItem = iIndex;
					lvI.iSubItem = 0;
					ListView_GetItem(pkmpss->hwndSubKeys, &lvI);
					if ((lvI.iImage == IDX_DSAPUBKEY) && (pkmpss->bSecret))
						EnableWindow (
							GetDlgItem (hDlg, IDC_REVOKESUBKEY), TRUE);
					else
						EnableWindow (
							GetDlgItem (hDlg,IDC_REVOKESUBKEY), FALSE);

					EnableWindow (GetDlgItem (hDlg,IDC_REMOVESUBKEY), TRUE);
				}
				else {
					EnableWindow (GetDlgItem (hDlg,IDC_REVOKESUBKEY), FALSE);
					EnableWindow (GetDlgItem (hDlg,IDC_REMOVESUBKEY), FALSE);
				}
			}
			break;
		}
	}
	return FALSE;
}

//	___________________________________________________
//
//  Populate ListView with ADKs

static VOID 
sInsertADKsIntoTree (
		PKEYMAN			pKM,
		HWND			hwndTree, 
		PGPKeyRef		key,
		PGPKeySetRef	keyset,
		UINT			uNumberADKs) 
{
	TL_TREEITEM		tlI;
	TL_LISTITEM		tlL;
	TL_INSERTSTRUCT tlIns;
	HTLITEM			hTNew;
	PGPKeyRef		keyADK;
	PGPKeyID		keyidADK;
	CHAR			szName[256];
	PGPError		err;
	CHAR			szID[kPGPMaxKeyIDStringSize];
	UINT			u;
	PGPByte			byteClass;
	BOOL			bItalics;

	tlI.hItem = NULL;
	tlI.mask = TLIF_TEXT | TLIF_IMAGE | TLIF_STATE;
	tlI.stateMask = TLIS_ITALICS;
	tlI.pszText = szName;

	for (u=0; u<uNumberADKs; u++) {
		byteClass = 0;
		tlI.iImage = IDX_DSAPUBDISKEY;
		tlI.state = 0;

		err = PGPGetIndexedAdditionalRecipientRequestKey (key, 
				keyset, u, &keyADK, &keyidADK, &byteClass);

		if (IsntPGPError (err)) { 

			if (PGPKeyRefIsValid (keyADK)) {
				KMGetKeyName (keyADK, szName, sizeof(szName));
				tlI.iImage = KMDetermineKeyIcon (pKM, keyADK, &bItalics);
				if (bItalics) tlI.state = TLIS_ITALICS;
			}
			else {
				tlI.state = TLIS_ITALICS;

				LoadString (g_hInst, IDS_UNKNOWNADK, 
									szName, sizeof(szName));
				err = PGPGetKeyIDString (&keyidADK, 
							kPGPKeyIDString_Abbreviated, szID);
				if (IsntPGPError (err)) {
					LoadString (g_hInst, IDS_UNKNOWNADKID, 
									szName, sizeof(szName));
					lstrcat (szName, szID);
				}
			}
		}

		else {
			LoadString (g_hInst, IDS_ERRONEOUSADK, szName, sizeof(szName));
		}

		tlI.cchTextMax = lstrlen (szName);
		tlIns.hInsertAfter = (HTLITEM)TLI_SORT;
		tlIns.item = tlI;
		tlIns.hParent = NULL;
		hTNew = TreeList_InsertItem (hwndTree, &tlIns);

		tlL.pszText = NULL;
		tlL.hItem = hTNew;
		tlL.stateMask = TLIS_VISIBLE;
		tlL.iSubItem = 1;
		tlL.mask = TLIF_DATAVALUE | TLIF_STATE;
		tlL.state = TLIS_VISIBLE;

		if (byteClass & 0x80) tlL.lDataValue = IDX_ADK;
		else tlL.lDataValue = IDX_NOADK;

		TreeList_SetListItem (hwndTree, &tlL, FALSE);
	}
}


//	_____________________________________________________
//
//  add columns to treelist

static BOOL 
sAddADKColumns (HWND hWndTree, INT iWidth) {
	TL_COLUMN tlc;
	CHAR sz[64];

	TreeList_DeleteAllColumns (hWndTree);

	tlc.mask = TLCF_FMT | TLCF_WIDTH | TLCF_TEXT | 
				TLCF_SUBITEM | TLCF_DATATYPE | TLCF_DATAMAX;
	tlc.pszText = sz;

	tlc.iSubItem = 0;
	tlc.fmt = TLCFMT_LEFT;
	tlc.iDataType = TLC_DATASTRING;
	tlc.cx = iWidth - ENFORCELISTWIDTH - 20;
	tlc.bMouseNotify = FALSE;
	LoadString (g_hInst, IDS_NAMEFIELD, sz, sizeof(sz));
	TreeList_InsertColumn (hWndTree, 0, &tlc);

	tlc.fmt = TLCFMT_IMAGE;
	tlc.iDataType = TLC_DATALONG;
	tlc.cx = ENFORCELISTWIDTH;
	tlc.bMouseNotify = FALSE;
	LoadString (g_hInst, IDS_ENFORCEFIELD, sz, sizeof(sz));
	TreeList_InsertColumn (hWndTree, 1, &tlc);

	return TRUE;
}


//	___________________________________________________
//
//  Key Properties Dialog Message procedure - ADK panel

static BOOL CALLBACK 
sKeyPropDlgProcADK (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam,
		LPARAM	lParam) 
{
	KMPROPSHEETSTRUCT*	pkmpss;
	RECT				rc;

	pkmpss = (KMPROPSHEETSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);

	switch (uMsg) {

	case WM_INITDIALOG:
		// store pointer to data structure
		SetWindowLong (hDlg, GWL_USERDATA, ((PROPSHEETPAGE*)lParam)->lParam);
		pkmpss = (KMPROPSHEETSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);

		// create tree view window
		GetClientRect (GetDlgItem (hDlg, IDC_FRAME), &rc);
		rc.left += 8;
		rc.right -= 8;
		rc.top += 16;
		rc.bottom -= 8;

		pkmpss->hwndADKs = CreateWindowEx (WS_EX_CLIENTEDGE, WC_TREELIST, "",
			WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP | WS_GROUP |
			TLS_AUTOSCROLL | TLS_SINGLESELECT,
			rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top,
			GetDlgItem (hDlg, IDC_FRAME), 
			(HMENU)IDC_ADKTREELIST, g_hInst, NULL);

		// create image list
		sCreateImageList (pkmpss);
		TreeList_SetImageList (pkmpss->hwndADKs, pkmpss->hIml);
		sAddADKColumns (pkmpss->hwndADKs, rc.right-rc.left);

		// initialize all controls
		PostMessage (hDlg, WM_APP, SERIALIZE, 0);
		return TRUE;

	case WM_APP :
		if (wParam == SERIALIZE)
			KMRequestSDKAccess (pkmpss->pKM);
		TreeList_DeleteTree (pkmpss->hwndADKs, TRUE);
		sInsertADKsIntoTree (pkmpss->pKM, pkmpss->hwndADKs, pkmpss->key, 
								pkmpss->keyset, pkmpss->uNumberADKs);
		if (wParam == SERIALIZE)
			KMReleaseSDKAccess (pkmpss->pKM);
		break;

    case WM_HELP: 
        WinHelp (((LPHELPINFO) lParam)->hItemHandle, pkmpss->pKM->szHelpFile, 
            HELP_WM_HELP, (DWORD) (LPSTR) aADKIds); 
        break; 
 
    case WM_CONTEXTMENU: 
        WinHelp ((HWND) wParam, pkmpss->pKM->szHelpFile, HELP_CONTEXTMENU, 
            (DWORD) (LPVOID) aADKIds); 
        break; 

	case WM_NOTIFY :
		switch (((NMHDR FAR *) lParam)->code) {

		case PSN_HELP :
			WinHelp (hDlg, pkmpss->pKM->szHelpFile, HELP_CONTEXT, 
				IDH_PGPKM_PROPDIALOG); 
			break;
		}
	}
	return FALSE;
}


//	___________________________________________________
//
//  Populate ListView with Designated Revokers

static VOID 
sInsertRevokersIntoTree (
		PKEYMAN			pKM,
		HWND			hwndTree, 
		PGPKeyRef		key,
		PGPKeySetRef	keyset,
		UINT			uNumberRevokers) 
{
	TL_TREEITEM		tlI;
	TL_INSERTSTRUCT tlIns;
	HTLITEM			hTNew;
	PGPKeyRef		keyRevoker;
	PGPKeyID		keyidRevoker;
	CHAR			szName[256];
	PGPError		err;
	CHAR			szID[kPGPMaxKeyIDStringSize];
	UINT			u;
	BOOL			bItalics;

	tlI.hItem = NULL;
	tlI.mask = TLIF_TEXT | TLIF_IMAGE | TLIF_STATE;
	tlI.stateMask = TLIS_ITALICS;
	tlI.pszText = szName;

	for (u=0; u<uNumberRevokers; u++) {

		tlI.iImage = IDX_DSAPUBDISKEY;
		tlI.state = 0;

		err = PGPGetIndexedRevocationKey (key, 
				keyset, u, &keyRevoker, &keyidRevoker);

		if (IsntPGPError (err)) { 

			if (!PGPKeyRefIsValid (keyRevoker)) {
				// currently only DH/DSS designated revokers are allowed
				// so we assume that's what it is
				err = PGPGetKeyByKeyID (pKM->KeySetMain, &keyidRevoker,
							kPGPPublicKeyAlgorithm_DSA, &keyRevoker);
			}

			if (PGPKeyRefIsValid (keyRevoker)) {
				KMGetKeyName (keyRevoker, szName, sizeof(szName));
				tlI.iImage = KMDetermineKeyIcon (pKM, keyRevoker, &bItalics);
				if (bItalics) tlI.state = TLIS_ITALICS;
			}
			else {
				tlI.state = TLIS_ITALICS;

				LoadString (g_hInst, IDS_UNKNOWNADK, 
									szName, sizeof(szName));
				err = PGPGetKeyIDString (&keyidRevoker, 
							kPGPKeyIDString_Abbreviated, szID);
				if (IsntPGPError (err)) {
					LoadString (g_hInst, IDS_UNKNOWNADKID, 
									szName, sizeof(szName));
					lstrcat (szName, szID);
				}
			}
		}

		else {
			LoadString (g_hInst, IDS_ERRONEOUSREVOKER, 
									szName, sizeof(szName));
		}

		tlI.cchTextMax = lstrlen (szName);
		tlIns.hInsertAfter = (HTLITEM)TLI_SORT;
		tlIns.item = tlI;
		tlIns.hParent = NULL;

		hTNew = TreeList_InsertItem (hwndTree, &tlIns);

	}
}


//	_____________________________________________________
//
//  add columns to treelist

static BOOL 
sAddRevokerColumns (HWND hWndTree, INT iWidth) {
	TL_COLUMN tlc;
	CHAR sz[64];

	TreeList_DeleteAllColumns (hWndTree);

	tlc.mask = TLCF_FMT | TLCF_WIDTH | TLCF_TEXT | 
				TLCF_SUBITEM | TLCF_DATATYPE | TLCF_DATAMAX;
	tlc.pszText = sz;

	tlc.iSubItem = 0;
	tlc.fmt = TLCFMT_LEFT;
	tlc.iDataType = TLC_DATASTRING;
	tlc.cx = iWidth - 8;
	tlc.bMouseNotify = FALSE;
	LoadString (g_hInst, IDS_NAMEFIELD, sz, sizeof(sz));
	TreeList_InsertColumn (hWndTree, 0, &tlc);

	return TRUE;
}


//	___________________________________________________
//
//  Key Properties Dialog Message procedure - Revokers panel

static BOOL CALLBACK 
sKeyPropDlgProcRevokers (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam,
		LPARAM	lParam) 
{
	KMPROPSHEETSTRUCT*	pkmpss;
	RECT				rc;

	pkmpss = (KMPROPSHEETSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);

	switch (uMsg) {

	case WM_INITDIALOG:
		// store pointer to data structure
		SetWindowLong (hDlg, GWL_USERDATA, ((PROPSHEETPAGE*)lParam)->lParam);
		pkmpss = (KMPROPSHEETSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
		pkmpss->hwndRevokerDlg = hDlg;

		// create tree view window
		GetClientRect (GetDlgItem (hDlg, IDC_FRAME), &rc);
		rc.left += 8;
		rc.right -= 8;
		rc.top += 16;
		rc.bottom -= 8;

		pkmpss->hwndRevokers = CreateWindowEx (WS_EX_CLIENTEDGE, WC_TREELIST,
			"", WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP | WS_GROUP |
			TLS_AUTOSCROLL | TLS_SINGLESELECT,
			rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top,
			GetDlgItem (hDlg, IDC_FRAME), 
			(HMENU)IDC_REVOKERTREELIST, g_hInst, NULL);

		// create image list
		sCreateImageList (pkmpss);
		TreeList_SetImageList (pkmpss->hwndRevokers, pkmpss->hIml);
		sAddRevokerColumns (pkmpss->hwndRevokers, rc.right-rc.left);

		// initialize all controls
		PostMessage (hDlg, WM_APP, SERIALIZE, 0);
		return TRUE;

	case WM_APP :
		if (wParam == SERIALIZE)
			KMRequestSDKAccess (pkmpss->pKM);
		TreeList_DeleteTree (pkmpss->hwndRevokers, TRUE);
		PGPCountRevocationKeys (pkmpss->key, &pkmpss->uNumberRevokers);
		sInsertRevokersIntoTree (pkmpss->pKM, pkmpss->hwndRevokers, 
					pkmpss->key, pkmpss->keyset, pkmpss->uNumberRevokers);
		if (wParam == SERIALIZE)
			KMReleaseSDKAccess (pkmpss->pKM);
		break;

    case WM_HELP: 
        WinHelp (((LPHELPINFO) lParam)->hItemHandle, pkmpss->pKM->szHelpFile, 
            HELP_WM_HELP, (DWORD) (LPSTR) aRevokerIds); 
        break; 
 
    case WM_CONTEXTMENU: 
        WinHelp ((HWND) wParam, pkmpss->pKM->szHelpFile, HELP_CONTEXTMENU, 
            (DWORD) (LPVOID) aRevokerIds); 
        break; 

	case WM_NOTIFY :
		switch (((NMHDR FAR *) lParam)->code) {

		case PSN_HELP :
			WinHelp (hDlg, pkmpss->pKM->szHelpFile, HELP_CONTEXT, 
				IDH_PGPKM_PROPDIALOG); 
			break;
		}
	}
	return FALSE;
}


//	___________________________________________________
//
//  update controls dealing with validity

static VOID
sSetValidityControls (HWND hwnd, KMPROPSHEETSTRUCT* pkmpss, UINT uVal) 
{
	UINT u;

	u = KMConvertFromPGPValidity (uVal);
	SendMessage (pkmpss->hwndValidity, PBM_SETPOS, u, 0);

	if (u < (UINT)pkmpss->pKM->iValidityThreshold)
		pkmpss->bInvalid = TRUE;
	else
		pkmpss->bInvalid = FALSE;

	InvalidateRect (hwnd, NULL, FALSE);
}


//	___________________________________________________
//
//  convert slider control values

static UINT
sConvertFromPGPTrust (UINT u) 
{
#if 0 // vertical trust slider
	return (KMConvertFromPGPTrust (kPGPKeyTrust_Complete) -
			KMConvertFromPGPTrust (u));
#else // horizontal trust slider
	return (KMConvertFromPGPTrust (u));
#endif
}


static UINT
sConvertToPGPTrust (UINT u) 
{
#if 0 // vertical trust slider
	return KMConvertToPGPTrust (
		KMConvertFromPGPTrust (kPGPKeyTrust_Complete) - u);
#else // horizontal trust slider
	return KMConvertToPGPTrust (u);
#endif
}


//	___________________________________________________
//
//  update controls dealing with trust

static VOID
sSetTrustControls (HWND hDlg, KMPROPSHEETSTRUCT* pkmpss, UINT uTrust) 
{
	INT i;

	i = sConvertFromPGPTrust (uTrust);
	SendDlgItemMessage (hDlg, IDC_TRUSTSLIDER, TBM_SETPOS, (WPARAM)TRUE, i);
}


//	___________________________________________________
//
//  display photo userID with appropriate overwriting

static VOID 
sPaintPhotoID (
		HWND		hWnd,
		HBITMAP		hbitmapID,
		HPALETTE	hpaletteID,
		INT			iwidthBM,
		INT			iheightBM,
		BOOL		bInvalid,
		BOOL		bRevoked,
		BOOL		bExpired)
{
	HPALETTE		hpaletteOld		= NULL;
	HDC				hdc;
	HDC				hdcMem;
	HDC				hdcMask;
	HBITMAP			hbitmap;
	PAINTSTRUCT		ps;
	RECT			rc;
	INT				icent;
	INT				ileft, itop, iwidth, iheight;
	BITMAP			bm;

	hdc = BeginPaint (hWnd, &ps);

	GetWindowRect (GetDlgItem (hWnd, IDC_PHOTOID), &rc);
	rc.left += 2;
	rc.top += 2;
	rc.right -= 2;
	rc.bottom -= 2;
	MapWindowPoints (NULL, hWnd, (LPPOINT)&rc, 2);
	FillRect (hdc, &rc, (HBRUSH)(COLOR_3DFACE+1));

	// if photoid is available ... draw it
	if (hbitmapID) {
		// check if bitmap needs shrinking
		if ((iheightBM > (rc.bottom-rc.top-2)) ||
			(iwidthBM  > (rc.right-rc.left-2))) 
		{
			if (iheightBM > (iwidthBM * 1.25)) {
				itop = rc.top +1;
				iheight = rc.bottom-rc.top -2;
				icent = (rc.right+rc.left) / 2;
				iwidth = ((rc.bottom-rc.top) * iwidthBM) / iheightBM;
				ileft = icent -(iwidth/2);
			}
			else {
				ileft = rc.left +1;
				iwidth = rc.right-rc.left -2;
				icent = (rc.bottom+rc.top) / 2;
				iheight = ((rc.right-rc.left) * iheightBM) / iwidthBM;
				itop = icent - (iheight/2);
			}
		}
		// otherwise draw it at its real size
		else {
			iwidth = iwidthBM;
			iheight = iheightBM;
			icent = (rc.right+rc.left) / 2;
			ileft = icent - (iwidth/2);
			icent = (rc.bottom+rc.top) / 2;
			itop = icent - (iheight/2);
		}

		hdcMem = CreateCompatibleDC (hdc);

		if (hpaletteID) {
			hpaletteOld = SelectPalette (hdc, hpaletteID, FALSE);
			RealizePalette (hdc);
		}

		SetStretchBltMode (hdc, COLORONCOLOR);
		SelectObject (hdcMem, hbitmapID);
		StretchBlt (hdc, ileft, itop, iwidth, iheight,
					hdcMem, 0, 0, iwidthBM, iheightBM, SRCCOPY);

		// overlay the question mark
		if (bInvalid && !bRevoked && !bExpired) {
			hdcMask = CreateCompatibleDC (hdc);
			hbitmap = 
				LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_QUESTIONMARK));
			GetObject (hbitmap, sizeof(BITMAP), (LPSTR)&bm);
	
			SelectObject (hdcMask, hbitmap);
	
			SetTextColor(hdc, RGB (255,0,0));
			SetBkColor(hdc, RGB (0,0,0));
			StretchBlt (hdc, rc.left, rc.top, 
					rc.right-rc.left-2, rc.bottom-rc.top-2,
					hdcMask, 0, 0, bm.bmWidth, bm.bmHeight, SRCINVERT);
	
			SetTextColor(hdc, RGB (0,0,0));
			SetBkColor(hdc, RGB (255,255,255));
			StretchBlt (hdc, rc.left, rc.top, 
					rc.right-rc.left-2, rc.bottom-rc.top-2,
					hdcMask, 0, 0, bm.bmWidth, bm.bmHeight, SRCAND);
	
			SetTextColor(hdc, RGB (255,0,0));
			SetBkColor(hdc, RGB (0,0,0));
			StretchBlt (hdc, rc.left, rc.top, 
					rc.right-rc.left-2, rc.bottom-rc.top-2,
					hdcMask, 0, 0, bm.bmWidth, bm.bmHeight, SRCINVERT);
	
			DeleteDC (hdcMask);
		}

		if (hpaletteOld) {
			SelectPalette (hdc, hpaletteOld, TRUE);
			RealizePalette (hdc);
		}

		DeleteDC (hdcMem);
	}

	// overlay the text
	if (bRevoked) {
		hdcMask = CreateCompatibleDC (hdc);
		hbitmap = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_REVOKED));
		GetObject (hbitmap, sizeof(BITMAP), (LPSTR)&bm);

		SelectObject (hdcMask, hbitmap);

		SetTextColor(hdc, RGB (255,0,0));
		SetBkColor(hdc, RGB (0,0,0));
		StretchBlt (hdc, rc.left, rc.top, 
				rc.right-rc.left-2, rc.bottom-rc.top-2,
				hdcMask, 0, 0, bm.bmWidth, bm.bmHeight, SRCINVERT);

		SetTextColor(hdc, RGB (0,0,0));
		SetBkColor(hdc, RGB (255,255,255));
		StretchBlt (hdc, rc.left, rc.top, 
				rc.right-rc.left-2, rc.bottom-rc.top-2,
				hdcMask, 0, 0, bm.bmWidth, bm.bmHeight, SRCAND);

		SetTextColor(hdc, RGB (255,0,0));
		SetBkColor(hdc, RGB (0,0,0));
		StretchBlt (hdc, rc.left, rc.top, 
				rc.right-rc.left-2, rc.bottom-rc.top-2,
				hdcMask, 0, 0, bm.bmWidth, bm.bmHeight, SRCINVERT);

		DeleteDC (hdcMask);
	}
	else if (bExpired) {
		hdcMask = CreateCompatibleDC (hdc);
		hbitmap = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_EXPIRED));
		GetObject (hbitmap, sizeof(BITMAP), (LPSTR)&bm);

		SelectObject (hdcMask, hbitmap);

		SetTextColor(hdc, RGB (255,0,0));
		SetBkColor(hdc, RGB (0,0,0));
		StretchBlt (hdc, rc.left, rc.top, 
				rc.right-rc.left-2, rc.bottom-rc.top-2,
				hdcMask, 0, 0, bm.bmWidth, bm.bmHeight, SRCINVERT);

		SetTextColor(hdc, RGB (0,0,0));
		SetBkColor(hdc, RGB (255,255,255));
		StretchBlt (hdc, rc.left, rc.top, 
				rc.right-rc.left-2, rc.bottom-rc.top-2,
				hdcMask, 0, 0, bm.bmWidth, bm.bmHeight, SRCAND);

		SetTextColor(hdc, RGB (255,0,0));
		SetBkColor(hdc, RGB (0,0,0));
		StretchBlt (hdc, rc.left, rc.top, 
				rc.right-rc.left-2, rc.bottom-rc.top-2,
				hdcMask, 0, 0, bm.bmWidth, bm.bmHeight, SRCINVERT);

		DeleteDC (hdcMask);
	}

	EndPaint (hWnd, &ps);
} 


//	___________________________________________________
//
//  PhotoID subclass procedure

static LRESULT APIENTRY 
sPhotoIDSubclassProc (
		HWND	hWnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	KMPROPSHEETSTRUCT* pkmpss;
	pkmpss = 
		(KMPROPSHEETSTRUCT*)GetWindowLong (GetParent (hWnd), GWL_USERDATA);

    switch (uMsg) {

	case WM_CONTEXTMENU :
		{
			HMENU	hMC;
			HMENU	hMenuTrackPopup;

			hMC = LoadMenu (g_hInst, MAKEINTRESOURCE (IDR_MENUPHOTOID));
			if (pkmpss->hbitmapPhotoID)
				EnableMenuItem (hMC, IDM_COPYBITMAP, 
										MF_BYCOMMAND|MF_ENABLED);
			else
				EnableMenuItem (hMC, IDM_COPYBITMAP, 
										MF_BYCOMMAND|MF_GRAYED);

			hMenuTrackPopup = GetSubMenu (hMC, 0);

			TrackPopupMenu (hMenuTrackPopup, TPM_LEFTALIGN|TPM_RIGHTBUTTON,
				LOWORD(lParam), HIWORD(lParam), 0, GetParent (hWnd), NULL);

			DestroyMenu (hMC);
		}
		break;

	default :
		return CallWindowProc (pkmpss->wpOrigPhotoIDProc, hWnd, uMsg, 
								wParam, lParam); 
	}
	return TRUE;
} 


//	___________________________________________________
//
//  update system palette

static BOOL
sUpdatePalette (
		HWND				hwnd,
		KMPROPSHEETSTRUCT*	pkmpss)
{
	BOOL		bretval		= FALSE;
	HDC			hdc;
	HPALETTE	hpaletteOld;

	if (pkmpss->hpalettePhotoID == NULL) return FALSE;

	hdc = GetDC (hwnd);

	hpaletteOld = SelectPalette (hdc, pkmpss->hpalettePhotoID, FALSE);
	if (RealizePalette (hdc)) {
		InvalidateRect (hwnd, NULL, TRUE); 
		bretval = TRUE;
	}

	SelectPalette (hdc, hpaletteOld, TRUE);
	RealizePalette (hdc);
	ReleaseDC (hwnd, hdc);

	return bretval;
}


//	___________________________________________________
//
//  Key Properties Dialog Message procedure - General panel

static BOOL CALLBACK 
sKeyPropDlgProcGeneral (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam,
		LPARAM	lParam) 
{
	KMPROPSHEETSTRUCT*	pkmpss;
	CHAR				sz[kPGPMaxUserIDSize];
	INT					i;
	UINT				u, uTrust;
	PGPUInt32			prefAlg[6];
	PGPTime				tm;
	PGPUserIDRef		userid;
	HWND				hwndParent;
	RECT				rc;
	LPBITMAPINFO		lpbmi;

	pkmpss = (KMPROPSHEETSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);

	switch (uMsg) {

	case WM_INITDIALOG:
		// store pointer to data structure
		SetWindowLong (hDlg, GWL_USERDATA, ((PROPSHEETPAGE*)lParam)->lParam);
		pkmpss = (KMPROPSHEETSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);

		// save HWND to table
		pkmpss->pKM->hWndTable[pkmpss->iIndex] = hDlg;

		// center dialog on screen
		hwndParent = GetParent (hDlg);
		if (pkmpss->pKM->iNumberSheets == 1) {
			GetWindowRect (hwndParent, &rc);
			SetWindowPos (hwndParent, NULL,
				(GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2,
				(GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 3,
				0, 0, SWP_NOSIZE | SWP_NOZORDER);
		}

		// subclass photoID control to handle dropping, dragging
		pkmpss->wpOrigPhotoIDProc = 
			(WNDPROC) SetWindowLong(GetDlgItem (hDlg, IDC_PHOTOID), 
						GWL_WNDPROC, (LONG) sPhotoIDSubclassProc); 

		// create smooth progress bar
		GetWindowRect (GetDlgItem (hDlg, IDC_VALIDITYBAR), &rc);
		MapWindowPoints (NULL, hDlg, (LPPOINT)&rc, 2);
		pkmpss->hwndValidity = 
			CreateWindowEx (0, PROGRESS_CLASS, (LPSTR) NULL, 
							WS_CHILD|WS_VISIBLE|PBS_SMOOTH, 
							rc.left, rc.top, 
							rc.right-rc.left, rc.bottom-rc.top, 
							hDlg, (HMENU) 0, g_hInst, NULL); 

		// disable and hide cancel button; and move "OK" button over
		SendMessage (hwndParent, PSM_CANCELTOCLOSE, 0, 0);
		GetWindowRect (GetDlgItem (hwndParent, IDCANCEL), &rc);
		MapWindowPoints (NULL, hwndParent, (LPPOINT)&rc, 2);
		SetWindowPos (GetDlgItem (hwndParent, IDOK), NULL, rc.left,
						rc.top, rc.right-rc.left, rc.bottom-rc.top,
						SWP_NOZORDER);
		ShowWindow (GetDlgItem (hwndParent, IDCANCEL), SW_HIDE);

		// initialize all controls
		PostMessage (hDlg, WM_APP, SERIALIZE, 0);
		return TRUE;

	case WM_APP :
		if (wParam == SERIALIZE)
			KMRequestSDKAccess (pkmpss->pKM);

		PGPGetKeyBoolean (pkmpss->key, kPGPKeyPropIsSecret,
													&pkmpss->bSecret);
		PGPGetKeyBoolean (pkmpss->key, kPGPKeyPropIsSecretShared,
													&pkmpss->bSplit);
		PGPGetKeyBoolean (pkmpss->key, kPGPKeyPropIsRevoked,
													&pkmpss->bRevoked);
		PGPGetKeyBoolean (pkmpss->key, kPGPKeyPropIsExpired,
													&pkmpss->bExpired);
		PGPGetKeyBoolean (pkmpss->key, kPGPKeyPropIsAxiomatic, 
													&pkmpss->bAxiomatic);

		// initialize key id edit control
		KMGetKeyIDFromKey (pkmpss->key, sz, sizeof(sz));
		SetDlgItemText (hDlg, IDC_KEYID, sz);

		// initialize key type edit control
		switch (pkmpss->algKey) {
		case kPGPPublicKeyAlgorithm_RSA :
			LoadString (g_hInst, IDS_RSA, sz, sizeof (sz));
			break;

		case kPGPPublicKeyAlgorithm_DSA :
			LoadString (g_hInst, IDS_DSA_ELGAMAL, sz, sizeof (sz));
			break;

		default :
			LoadString (g_hInst, IDS_UNKNOWN, sz, sizeof (sz));
			break;
		}
		SetDlgItemText (hDlg, IDC_KEYTYPE, sz);

		// initialize key size edit control
		KMGetKeyBitsString (pkmpss->keyset, 
							pkmpss->key, sz, sizeof(sz));
		SetDlgItemText (hDlg, IDC_KEYSIZE, sz);

		// initialize key creation date edit control
		PGPGetKeyTime (pkmpss->key, kPGPKeyPropCreation, &tm);
		KMConvertTimeToString (tm, sz, sizeof (sz));
		SetDlgItemText (hDlg, IDC_CREATEDATE, sz);

		// initialize key expiration date edit control
		PGPGetKeyTime (pkmpss->key, kPGPKeyPropExpiration, &tm);
		if (tm == kPGPExpirationTime_Never) {
			LoadString (g_hInst, IDS_NEVER, sz, sizeof (sz));
			pkmpss->iExpireDays = -1;
		}
		else {
			KMConvertTimeToDays (tm, &pkmpss->iExpireDays);
			KMConvertTimeToString (tm, sz, sizeof (sz));
		}
		SetDlgItemText (hDlg, IDC_EXPIREDATE, sz);

		// initialize preferred cipher edit control
		PGPGetKeyPropertyBuffer (pkmpss->key, kPGPKeyPropPreferredAlgorithms,
							sizeof(prefAlg), (PGPByte*)&prefAlg[0], &u);
		if (u < sizeof(PGPCipherAlgorithm)) 
			prefAlg[0] = kPGPCipherAlgorithm_IDEA;
		switch (prefAlg[0]) {
			case kPGPCipherAlgorithm_CAST5 :
				LoadString (g_hInst, IDS_CAST, sz, sizeof(sz));
				break;

			case kPGPCipherAlgorithm_3DES :
				LoadString (g_hInst, IDS_3DES, sz, sizeof(sz));
				break;

			default :
				LoadString (g_hInst, IDS_IDEA, sz, sizeof(sz));
				break;
		}
		SetDlgItemText (hDlg, IDC_CIPHER, sz);

		// initialize fingerprint edit control
		if (pkmpss->bShowHexFingerprint) 
			CheckDlgButton (hDlg, IDC_USEHEXFINGERPRINT, BST_CHECKED);
		else 
			CheckDlgButton (hDlg, IDC_USEHEXFINGERPRINT, BST_UNCHECKED);
		sSetFingerprintControls (hDlg, 
					pkmpss->bShowHexFingerprint, 
					pkmpss->key, pkmpss->algKey);

		// initialize validity edit and bar controls
		i = KMConvertFromPGPValidity (kPGPValidity_Complete);
		SendMessage (pkmpss->hwndValidity, PBM_SETRANGE, 
									0, MAKELPARAM (0,i));
		PGPGetKeyNumber (pkmpss->key, kPGPKeyPropValidity, 
								&pkmpss->uValidity);
		sSetValidityControls (hDlg, pkmpss, pkmpss->uValidity);

		// initialize "Axiomatic" checkbox control
		if (pkmpss->bAxiomatic) 
			CheckDlgButton (hDlg, IDC_AXIOMATIC, BST_CHECKED);
		else 
			CheckDlgButton (hDlg, IDC_AXIOMATIC, BST_UNCHECKED);

		if (pkmpss->bRevoked || pkmpss->bExpired || pkmpss->bReadOnly)
			EnableWindow (GetDlgItem (hDlg, IDC_AXIOMATIC), FALSE);
		else 
			EnableWindow (GetDlgItem (hDlg, IDC_AXIOMATIC), TRUE);

		if (!pkmpss->bSecret && !pkmpss->bAxiomatic) 
			ShowWindow (GetDlgItem (hDlg, IDC_AXIOMATIC), SW_HIDE);
		else
			ShowWindow (GetDlgItem (hDlg, IDC_AXIOMATIC), SW_SHOW);

		// initialize trust edit and slider controls
		i = KMConvertFromPGPTrust (kPGPKeyTrust_Complete);
		SendDlgItemMessage (hDlg, IDC_TRUSTSLIDER, TBM_SETRANGE, 
								0, MAKELPARAM (0, i));
		PGPGetKeyNumber (pkmpss->key, kPGPKeyPropTrust, &pkmpss->uTrust);
		sSetTrustControls (hDlg, pkmpss, pkmpss->uTrust);

		if (pkmpss->bAxiomatic || pkmpss->bRevoked || pkmpss->bReadOnly)
		{
			EnableWindow (GetDlgItem (hDlg, IDC_TRUSTSLIDER), FALSE);
			EnableWindow (GetDlgItem (hDlg, IDC_TRUSTTEXT1), FALSE);
			EnableWindow (GetDlgItem (hDlg, IDC_TRUSTTEXT2), FALSE);
		}
		else
		{
			EnableWindow (GetDlgItem (hDlg, IDC_TRUSTSLIDER), TRUE);
			EnableWindow (GetDlgItem (hDlg, IDC_TRUSTTEXT1), TRUE);
			EnableWindow (GetDlgItem (hDlg, IDC_TRUSTTEXT2), TRUE);
		}

		// initialize enable/disable checkbox
		PGPGetKeyBoolean (pkmpss->key, kPGPKeyPropIsDisabled, 
							&pkmpss->bDisabled);
		if (pkmpss->bDisabled) 
			CheckDlgButton (hDlg, IDC_ENABLED, BST_UNCHECKED);
		else 
			CheckDlgButton (hDlg, IDC_ENABLED, BST_CHECKED);

		if (pkmpss->bRevoked || pkmpss->bExpired || pkmpss->bReadOnly)
			EnableWindow (GetDlgItem (hDlg, IDC_ENABLED), FALSE);
		else
			EnableWindow (GetDlgItem (hDlg, IDC_ENABLED), TRUE);

		if (pkmpss->bSecret) {
			if (pkmpss->bAxiomatic) 
				EnableWindow (GetDlgItem (hDlg, IDC_ENABLED), FALSE);
			else
				EnableWindow (GetDlgItem (hDlg, IDC_ENABLED), TRUE);
		}

		// initialize change passphrase button
		if (pkmpss->bSplit) {
			aKeyPropIds[1] = IDH_PGPKM_JOINKEY;
			LoadString (g_hInst, IDS_RECONSTITUTEKEY, sz, sizeof(sz));
		}
		else {
			aKeyPropIds[1] = IDH_PGPKM_CHANGEPHRASE;
			LoadString (g_hInst, IDS_CHANGEPHRASE, sz, sizeof(sz));	
		}
		SetDlgItemText (hDlg, IDC_CHANGEPHRASE, sz);

		if (!pkmpss->bSecret) 
			ShowWindow (GetDlgItem (hDlg, IDC_CHANGEPHRASE), SW_HIDE);

		if (pkmpss->bReadOnly)
			EnableWindow (GetDlgItem (hDlg, IDC_CHANGEPHRASE), FALSE);
		else
			EnableWindow (GetDlgItem (hDlg, IDC_CHANGEPHRASE), TRUE);

		// display photo ID
		if (pkmpss->hbitmapPhotoID) {
			DeleteObject (pkmpss->hbitmapPhotoID);
			pkmpss->hbitmapPhotoID = NULL;
		}
		if (pkmpss->pPhotoBuffer) {
			KMFree (pkmpss->pPhotoBuffer);
			pkmpss->pPhotoBuffer = NULL;
		}
		if (pkmpss->hbitmapPhotoID) {
			DeleteObject (pkmpss->hbitmapPhotoID);
			pkmpss->hbitmapPhotoID = NULL;
		}
		if (pkmpss->hpalettePhotoID) {
			DeleteObject (pkmpss->hpalettePhotoID);
			pkmpss->hpalettePhotoID = NULL;
		}

		PGPGetPrimaryAttributeUserID (pkmpss->key, 
										kPGPAttribute_Image, &userid);
		if (userid == NULL) {
			ShowWindow (GetDlgItem (hDlg, IDC_PHOTOSCROLLBAR), SW_HIDE);
			pkmpss->iImageIndex = -1;
		}
		else {
			PGPKeyListRef	keylist;
			PGPKeyIterRef	keyiter;
			INT				iNumPhotoID;
			PGPUInt32		iType;
			PGPBoolean		bAttrib;
			SCROLLINFO		si;

			if (pkmpss->iImageIndex < 0)
				pkmpss->iImageIndex = 0;

			// get number of Photo IDs
			PGPOrderKeySet (pkmpss->pKM->KeySetDisp, 
								kPGPAnyOrdering, &keylist);
			PGPNewKeyIter (keylist, &keyiter);
			PGPKeyIterSeek (keyiter, pkmpss->key);
			PGPKeyIterNextUserID (keyiter, &userid);
			iNumPhotoID = 0;
			while (userid) {
				PGPGetUserIDBoolean (userid, 
						kPGPUserIDPropIsAttribute, &bAttrib);
				if (userid == pkmpss->userid)
					pkmpss->iImageIndex = iNumPhotoID;
				if (bAttrib) {
					PGPGetUserIDNumber (userid, 
								kPGPUserIDPropAttributeType, &iType);
					if (iType == kPGPAttribute_Image)
						iNumPhotoID++;
				}
				PGPKeyIterNextUserID (keyiter, &userid);
			}
			pkmpss->iNumPhotoIDs = iNumPhotoID;

			// setup photoid scrollbar
			if (iNumPhotoID <= 1) 
				ShowWindow (GetDlgItem (hDlg, IDC_PHOTOSCROLLBAR), SW_HIDE);
			else {
				si.cbSize = sizeof(SCROLLINFO);
				si.fMask = SIF_ALL;
				si.nMin = 0;
				si.nMax = iNumPhotoID-1;
				si.nPage = 1;
				si.nPos = pkmpss->iImageIndex;
				SetScrollInfo (GetDlgItem (hDlg, IDC_PHOTOSCROLLBAR), SB_CTL,
								&si, TRUE);
				ShowWindow (GetDlgItem (hDlg, IDC_PHOTOSCROLLBAR), SW_SHOW);
			}

			// reset userid to selected one
			PGPKeyIterSeek (keyiter, pkmpss->key);
			PGPKeyIterNextUserID (keyiter, &userid);
			iNumPhotoID = -1;
			while (userid) {
				PGPGetUserIDBoolean (userid, 
						kPGPUserIDPropIsAttribute, &bAttrib);
				if (bAttrib) {
					PGPGetUserIDNumber (userid, 
								kPGPUserIDPropAttributeType, &iType);
					if (iType == kPGPAttribute_Image)
						iNumPhotoID++;
				}
				if (pkmpss->iImageIndex == iNumPhotoID) {
					PGPGetUserIDNumber (userid,
						kPGPUserIDPropValidity, &u);
					u = KMConvertFromPGPValidity (u);
					if (u < (UINT)pkmpss->pKM->iValidityThreshold)
						pkmpss->bPhotoInvalid = TRUE;
					else
						pkmpss->bPhotoInvalid = FALSE;
					break;
				}
				PGPKeyIterNextUserID (keyiter, &userid);
			}
			PGPFreeKeyIter (keyiter);
			PGPFreeKeyList (keylist);

			// get length of photo id buffer
			PGPGetUserIDStringBuffer (userid, kPGPUserIDPropAttributeData, 
					1, NULL, &(pkmpss->iPhotoBufferLength));

			if (pkmpss->iPhotoBufferLength > 0) {
				// now actually get the buffer data
				pkmpss->pPhotoBuffer = KMAlloc (pkmpss->iPhotoBufferLength);
				PGPGetUserIDStringBuffer (userid, kPGPUserIDPropAttributeData,
						pkmpss->iPhotoBufferLength, pkmpss->pPhotoBuffer, 
						&(pkmpss->iPhotoBufferLength));

				// no more SDK calls, release to avoid palette-related lockup
				if (wParam == SERIALIZE) {
					wParam = NOSERIALIZE;
					KMReleaseSDKAccess (pkmpss->pKM);
				}

				if (IsntPGPError (KMDIBfromPhoto (pkmpss->pPhotoBuffer, 
						pkmpss->iPhotoBufferLength, TRUE, &lpbmi))) {
					if (lpbmi) {
						KMGetDIBSize (lpbmi, &(pkmpss->iwidthPhotoID),
								 &(pkmpss->iheightPhotoID));
						pkmpss->hbitmapPhotoID = KMDDBfromDIB (lpbmi, 
									&(pkmpss->hpalettePhotoID)); 

						KMFree (lpbmi);
					}
				}
			}
		}

		if (wParam == SERIALIZE)
			KMReleaseSDKAccess (pkmpss->pKM);

		if (pkmpss->hwndRevokerDlg) {
			SendMessage (pkmpss->hwndRevokerDlg, WM_APP, wParam, 0);
			InvalidateRect (pkmpss->hwndRevokers, NULL, TRUE);
			UpdateWindow (pkmpss->hwndRevokers);
		}
		
		pkmpss->bReadyToPaint = TRUE;
		InvalidateRect (hDlg, NULL, FALSE);
		break;

	case WM_APP+1 :
		EnableWindow (GetDlgItem (hDlg, IDC_TRUSTSLIDER), FALSE);
		EnableWindow (GetDlgItem (hDlg, IDC_TRUSTTEXT1), FALSE);
		EnableWindow (GetDlgItem (hDlg, IDC_TRUSTTEXT2), FALSE);
		EnableWindow (GetDlgItem (hDlg, IDC_AXIOMATIC), FALSE);
		EnableWindow (GetDlgItem (hDlg, IDC_ENABLED), FALSE);
		EnableWindow (GetDlgItem (hDlg, IDC_CHANGEPHRASE), FALSE);
		EnableWindow (GetDlgItem (GetParent(hDlg), IDOK), FALSE);
		break;

	case WM_APP+2 :
		pkmpss->userid = (PGPUserIDRef)lParam;
		PostMessage (hDlg, WM_APP, 0, 0);
		break;

	case WM_PAINT :
		if (pkmpss->bReadyToPaint)
			sPaintPhotoID (hDlg, 
						pkmpss->hbitmapPhotoID, pkmpss->hpalettePhotoID,
						pkmpss->iwidthPhotoID, pkmpss->iheightPhotoID,
						pkmpss->bPhotoInvalid, pkmpss->bRevoked,
						pkmpss->bExpired);
		else {
			PAINTSTRUCT	ps;

			BeginPaint (hDlg, &ps);
			EndPaint (hDlg, &ps);
		}
		break;

	case WM_PALETTECHANGED :
		if ((HWND)wParam != hDlg) 
			sUpdatePalette (hDlg, pkmpss);
		break;

	case WM_QUERYNEWPALETTE :
		return (sUpdatePalette (hDlg, pkmpss));

	case WM_HSCROLL :
	case WM_VSCROLL :
		// check if originating from trust slider
		if ((HWND)lParam == GetDlgItem (hDlg, IDC_TRUSTSLIDER)) {
			if (LOWORD (wParam) == TB_ENDTRACK) 
			{
				u = SendDlgItemMessage (hDlg, IDC_TRUSTSLIDER, TBM_GETPOS, 
										0, 0);
				uTrust = sConvertToPGPTrust (u);
				if (uTrust == pkmpss->uTrust) break;

				if ((pkmpss->uValidity <= kPGPValidity_Invalid) && 
					(uTrust >= kPGPKeyTrust_Marginal)) 
				{
					sSetTrustControls (hDlg, pkmpss, 
										kPGPKeyTrust_Never);
					KMMessageBox (hDlg, IDS_CAPTION, 
						IDS_TRUSTONINVALIDKEY, 
						MB_OK|MB_ICONEXCLAMATION);
				}
				else 
				{
					pkmpss->uTrust = uTrust;
					KMRequestSDKAccess (pkmpss->pKM);
					if (IsntPGPError (PGPclErrorBox (hDlg, 
						PGPSetKeyTrust (pkmpss->key, uTrust))))
					{
						PGPCommitKeyRingChanges (pkmpss->pKM->KeySetMain);
						// this will cause reload message on closing
						pkmpss->bNeedsCommit = TRUE;

						KMReleaseSDKAccess (pkmpss->pKM);
						SendMessage (pkmpss->pKM->hWndParent, 
									KM_M_KEYPROPACTION, 
									KM_PROPACTION_UPDATEKEY, 
									(LPARAM)(pkmpss->key));
						PostMessage (hDlg, WM_APP, SERIALIZE, 0);
					}
					else
						KMReleaseSDKAccess (pkmpss->pKM);
				}
			}
		}
		// otherwise, must be coming from photoid scrollbar
		else {
			INT	iNewPos;

			iNewPos = pkmpss->iImageIndex;

			switch (LOWORD(wParam)) {
			case SB_TOP :
				iNewPos = 0;
				break;

			case SB_BOTTOM :
				iNewPos = pkmpss->iNumPhotoIDs-1;
				break;

			case SB_PAGELEFT :
			case SB_LINELEFT :
				iNewPos = pkmpss->iImageIndex-1;
				if (iNewPos < 0) iNewPos = 0;
				break;

			case SB_PAGERIGHT :
			case SB_LINERIGHT :
				iNewPos = pkmpss->iImageIndex+1;
				if (iNewPos >= pkmpss->iNumPhotoIDs) 
					iNewPos = pkmpss->iNumPhotoIDs-1;
				break;

			case SB_THUMBPOSITION :
			case SB_THUMBTRACK :
				iNewPos = HIWORD(wParam);

			default :
				break;
			}

			if (iNewPos != pkmpss->iImageIndex) {
				pkmpss->userid = kInvalidPGPUserIDRef;
				pkmpss->iImageIndex = iNewPos;
				PostMessage (hDlg, WM_APP, SERIALIZE, 0);
			}
		}
		return 0;

	case WM_DESTROY: 
		SetWindowLong(GetDlgItem(hDlg, IDC_PHOTOID), 
							GWL_WNDPROC, (LONG)pkmpss->wpOrigPhotoIDProc); 
		break; 

	case WM_COMMAND:
		switch(LOWORD (wParam)) {
			case IDC_CHANGEPHRASE :
				if (pkmpss->bSecret) {
					KMRequestSDKAccess (pkmpss->pKM);
					if (KMChangePhrase (GetParent (hDlg), 
										pkmpss->pKM, 
										pkmpss->pKM->Context, 
										pkmpss->pKM->tlsContext,
										pkmpss->keyset, 
										pkmpss->key))
					{
						pkmpss->bNeedsCommit = TRUE;
					}
					KMReleaseSDKAccess (pkmpss->pKM);
					PostMessage (hDlg, WM_APP, SERIALIZE, 0);
				}
				break;

			case IDC_AXIOMATIC :
				KMRequestSDKAccess (pkmpss->pKM);
				if (IsDlgButtonChecked (
							hDlg, IDC_AXIOMATIC) == BST_CHECKED) 
				{
					if (IsDlgButtonChecked (
							hDlg, IDC_ENABLED) == BST_UNCHECKED) 
					{
						PGPEnableKey (pkmpss->key);
					}
					PGPSetKeyAxiomatic (pkmpss->key,
								PGPOLastOption (pkmpss->pKM->Context));
				}
				else
					PGPUnsetKeyAxiomatic (pkmpss->key);

				PGPCommitKeyRingChanges (pkmpss->pKM->KeySetMain);
				pkmpss->bNeedsCommit = TRUE;

				KMReleaseSDKAccess (pkmpss->pKM);
				SendMessage (pkmpss->pKM->hWndParent, KM_M_KEYPROPACTION, 
						KM_PROPACTION_UPDATEKEY, (LPARAM)(pkmpss->key));
				PostMessage (hDlg, WM_APP, SERIALIZE, 0);
				break;
				
			case IDC_ENABLED :
				KMRequestSDKAccess (pkmpss->pKM);
				if (IsDlgButtonChecked (
							hDlg, IDC_ENABLED) == BST_CHECKED) 
					PGPEnableKey (pkmpss->key);
				else 
					PGPDisableKey (pkmpss->key);

				PGPCommitKeyRingChanges (pkmpss->pKM->KeySetMain);
				pkmpss->bNeedsCommit = TRUE;

				KMReleaseSDKAccess (pkmpss->pKM);
				SendMessage (pkmpss->pKM->hWndParent, KM_M_KEYPROPACTION, 
						KM_PROPACTION_UPDATEKEY, (LPARAM)(pkmpss->key));
				PostMessage (hDlg, WM_APP, SERIALIZE, 0);
				break;

			case IDM_COPYBITMAP :
				KMCopyPhotoToClipboard (hDlg, pkmpss->pPhotoBuffer, 
										pkmpss->iPhotoBufferLength);
				break;

			case IDC_USEHEXFINGERPRINT :
				if (IsDlgButtonChecked (
						hDlg, IDC_USEHEXFINGERPRINT) == BST_CHECKED) 
					pkmpss->bShowHexFingerprint = TRUE;
				else
					pkmpss->bShowHexFingerprint = FALSE;

				sSetFingerprintControls (hDlg, 
					pkmpss->bShowHexFingerprint, 
					pkmpss->key, pkmpss->algKey);
				break;
		}
		return TRUE;

    case WM_HELP: 
        WinHelp (((LPHELPINFO) lParam)->hItemHandle, pkmpss->pKM->szHelpFile, 
            HELP_WM_HELP, (DWORD) (LPSTR) aKeyPropIds); 
        break; 
 
    case WM_CONTEXTMENU: 
        WinHelp ((HWND) wParam, pkmpss->pKM->szHelpFile, HELP_CONTEXTMENU, 
            (DWORD) (LPVOID) aKeyPropIds); 
        break; 

	case WM_NOTIFY :
		switch (((NMHDR FAR *) lParam)->code) {

		case PSN_HELP :
			WinHelp (hDlg, pkmpss->pKM->szHelpFile, HELP_CONTEXT, 
				IDH_PGPKM_PROPDIALOG); 
			break;
		}
		return TRUE;
	}
	return FALSE;
}

//	___________________________________________________
//
//  Key Properties Dialog Message procedure - General panel

static BOOL CALLBACK 
sCertPropDlgProc (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam,
		LPARAM	lParam) 
{
	KMPROPSHEETSTRUCT*	pkmpss;
	CHAR				sz[kPGPMaxUserIDSize];
	PGPTime				tm;
	HWND				hwndParent;
	RECT				rc;
	PGPKeyRef			keySigner;
	PGPSigRef			certSigner;
	PGPError			err;

	pkmpss = (KMPROPSHEETSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);

	switch (uMsg) {

	case WM_INITDIALOG:
		// store pointer to data structure
		SetWindowLong (hDlg, GWL_USERDATA, ((PROPSHEETPAGE*)lParam)->lParam);
		pkmpss = (KMPROPSHEETSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);

		// save HWND to table
		pkmpss->pKM->hWndTable[pkmpss->iIndex] = hDlg;

		// center dialog on screen
		hwndParent = GetParent (hDlg);
		if (pkmpss->pKM->iNumberSheets == 1) {
			GetWindowRect (hwndParent, &rc);
			SetWindowPos (hwndParent, NULL,
				(GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2,
				(GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 3,
				0, 0, SWP_NOSIZE | SWP_NOZORDER);
		}

		// disable and hide cancel button; and move "OK" button over
		SendMessage (hwndParent, PSM_CANCELTOCLOSE, 0, 0);
		GetWindowRect (GetDlgItem (hwndParent, IDCANCEL), &rc);
		MapWindowPoints (NULL, hwndParent, (LPPOINT)&rc, 2);
		SetWindowPos (GetDlgItem (hwndParent, IDOK), NULL, rc.left,
						rc.top, rc.right-rc.left, rc.bottom-rc.top,
						SWP_NOZORDER);
		ShowWindow (GetDlgItem (hwndParent, IDCANCEL), SW_HIDE);

		// initialize all controls
		PostMessage (hDlg, WM_APP, SERIALIZE, 0);
		return TRUE;

	case WM_APP :
	{
		PGPError		err;
		PGPBoolean		b;
		PGPSize			size;

		pkmpss = (KMPROPSHEETSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);

		sCreateImageList (pkmpss);

		if (wParam == SERIALIZE)
			KMRequestSDKAccess (pkmpss->pKM);

		// initialize name control
		if (pkmpss->bX509)
		{
			PGPGetSigPropertyBuffer (pkmpss->cert, 
					kPGPSigPropX509LongName, sizeof(sz), sz, &size);
			SetDlgItemText (hDlg, IDC_NAME, sz);

			PGPGetSigPropertyBuffer (pkmpss->cert, 
					kPGPSigPropX509IssuerLongName, sizeof(sz), sz, &size);
			SetDlgItemText (hDlg, IDC_ISSUER, sz);

			PGPGetKeyBoolean (pkmpss->key, kPGPKeyPropHasCRL, &b);
			if (b)
			{
				PGPGetKeyTime (pkmpss->key,
							kPGPKeyPropCRLThisUpdate, &tm);
				KMConvertTimeToString (tm, sz, sizeof (sz));
				SetDlgItemText (hDlg, IDC_LASTCRL, sz);

				PGPGetKeyTime (pkmpss->key,
							kPGPKeyPropCRLNextUpdate, &tm);
				KMConvertTimeToString (tm, sz, sizeof (sz));
				SetDlgItemText (hDlg, IDC_NEXTCRL, sz);
			}
			else
			{
				LoadString (g_hInst, IDS_NA, sz, sizeof(sz));
				SetDlgItemText (hDlg, IDC_LASTCRL, sz);
				SetDlgItemText (hDlg, IDC_NEXTCRL, sz);
			}
		}
		else
		{
			err = PGPGetSigCertifierKey (
						pkmpss->cert, pkmpss->pKM->KeySetDisp, &keySigner);
			if (err == kPGPError_ItemNotFound) 
			{
				err = kPGPError_NoErr;
				keySigner = NULL;
			}

			if (!keySigner && 
				(pkmpss->pKM->KeySetDisp != pkmpss->pKM->KeySetMain)) 
			{
				err = PGPGetSigCertifierKey (	
						pkmpss->cert, pkmpss->pKM->KeySetMain, &keySigner);
			}

			if (err == kPGPError_ItemNotFound) 
			{
				err = kPGPError_NoErr;
				keySigner = NULL;
			}

			PGPclErrorBox (NULL, err);
			if (keySigner) 
				KMGetKeyName (keySigner, sz, sizeof(sz));
			else 
				LoadString (g_hInst, IDS_UNKNOWN, sz, sizeof(sz));
			SetDlgItemText (hDlg, IDC_NAME, sz);
		}

		// initialize keyid control
		KMGetKeyIDFromCert (pkmpss->cert, sz, sizeof(sz));
		SetDlgItemText (hDlg, IDC_KEYID, sz);

		// initialize cert creation date edit control
		PGPGetSigTime (pkmpss->cert, kPGPSigPropCreation, &tm);
		KMConvertTimeToString (tm, sz, sizeof (sz));
		SetDlgItemText (hDlg, IDC_CREATIONDATE, sz);

		// initialize cert expiration date edit control
		PGPGetSigTime (pkmpss->cert, kPGPSigPropExpiration, &tm);
		if (tm != kPGPExpirationTime_Never) 
			KMConvertTimeToString (tm, sz, sizeof (sz));
		else 
			LoadString (g_hInst, IDS_NEVER, sz, sizeof (sz));
		SetDlgItemText (hDlg, IDC_EXPIRATIONDATE, sz);

		// initialize checkboxes
		PGPGetSigBoolean (pkmpss->cert, kPGPSigPropIsRevoked, &b);
		CheckDlgButton (
				hDlg, IDC_REVOKED, (b ? BST_CHECKED : BST_UNCHECKED));

		PGPGetSigBoolean (pkmpss->cert, kPGPSigPropIsExpired, &b);
		CheckDlgButton (
				hDlg, IDC_EXPIRED, (b ? BST_CHECKED : BST_UNCHECKED));

		PGPGetSigBoolean (pkmpss->cert, kPGPSigPropIsExportable, &b);
		CheckDlgButton (
				hDlg, IDC_EXPORTABLE, (b ? BST_CHECKED : BST_UNCHECKED));

		// initialize show certifier button
		if (pkmpss->bX509)
		{
			err = PGPGetSigX509CertifierSig (
					pkmpss->cert, pkmpss->pKM->KeySetMain, &certSigner);
			// if this is a "self-sig" then disable button
			if (certSigner == pkmpss->cert)
				err = kPGPError_KeyInvalid;
		}
		else
		{
			err = PGPGetSigCertifierKey (
					pkmpss->cert, pkmpss->pKM->KeySetMain, &keySigner);
		}
		EnableWindow (
				GetDlgItem (hDlg, IDC_SHOWSIGNER), (IsntPGPError (err)));

		// initialize icon
		pkmpss->iImageIndex = 
			KMDetermineCertIcon (pkmpss->cert, NULL, NULL);

		if (wParam == SERIALIZE)
			KMReleaseSDKAccess (pkmpss->pKM);

		InvalidateRect (hDlg, NULL, FALSE);
		break;
	}

	case WM_APP+1 :
		EnableWindow (GetDlgItem (GetParent(hDlg), IDOK), FALSE);
		break;

	case WM_APP+2 :
		break;

	case WM_PAINT :
	{
		PAINTSTRUCT ps;
		HDC			hdc;
		RECT		rc;
		INT			ix, iy;

		hdc = BeginPaint (hDlg, &ps);
		if (pkmpss->hIml)
		{
			GetWindowRect (GetDlgItem (hDlg, IDC_ICONFRAME), &rc);
			MapWindowPoints (NULL, hDlg, (LPPOINT)&rc, 2);
			ix = rc.left + (rc.right-rc.left)/2 - 8;
			iy = rc.top + (rc.bottom-rc.top)/2 - 8;
			ImageList_Draw (pkmpss->hIml, pkmpss->iImageIndex, 
						hdc, ix, iy, ILD_TRANSPARENT);
		}
		EndPaint (hDlg, &ps);
		break;
	}

	case WM_HELP: 
		if (pkmpss->bX509)
			WinHelp (((LPHELPINFO) lParam)->hItemHandle, 
				pkmpss->pKM->szHelpFile, HELP_WM_HELP, 
				(DWORD) (LPVOID) aX509certIds); 
		else
			WinHelp (((LPHELPINFO) lParam)->hItemHandle, 
				pkmpss->pKM->szHelpFile, HELP_WM_HELP, 
				(DWORD) (LPVOID) aPGPcertIds); 
		break; 

	case WM_CONTEXTMENU: 
		if (pkmpss->bX509)
			WinHelp ((HWND) wParam, pkmpss->pKM->szHelpFile, 
					HELP_CONTEXTMENU, (DWORD) (LPVOID) aX509certIds); 
		else
			WinHelp ((HWND) wParam, pkmpss->pKM->szHelpFile, 
					HELP_CONTEXTMENU, (DWORD) (LPVOID) aPGPcertIds); 
		break; 

	case WM_NOTIFY :
		switch (((NMHDR FAR *) lParam)->code) {

		case PSN_HELP :
			if (pkmpss->bX509)
				WinHelp (hDlg, pkmpss->pKM->szHelpFile, HELP_CONTEXT, 
						IDH_PGPKM_X509CERTDIALOG); 
			else
				WinHelp (hDlg, pkmpss->pKM->szHelpFile, HELP_CONTEXT, 
						IDH_PGPKM_PGPCERTDIALOG); 
			break;
		}
		break;

	case WM_COMMAND:
		switch(LOWORD (wParam)) {
		case IDC_SHOWSIGNER :
			if (pkmpss->bX509)
			{
				KMRequestSDKAccess (pkmpss->pKM);
				err = PGPGetSigX509CertifierSig (
						pkmpss->cert, pkmpss->pKM->KeySetMain, &certSigner);
				KMReleaseSDKAccess (pkmpss->pKM);
				if (IsntPGPError (PGPclErrorBox (hDlg, err)))
				{
					sSingleCertProperties (pkmpss->pKM, certSigner, 
							NULL, pkmpss->pKM->KeySetMain, NULL, NULL);
				}
			}
			else
			{
				KMRequestSDKAccess (pkmpss->pKM);
				err = PGPGetSigCertifierKey (
						pkmpss->cert, pkmpss->pKM->KeySetMain, &keySigner);
				KMReleaseSDKAccess (pkmpss->pKM);
				if (IsntPGPError (PGPclErrorBox (hDlg, err)))
				{
					sSingleKeyProperties (pkmpss->pKM, keySigner, 
							pkmpss->pKM->KeySetMain, NULL, NULL);
				}
			}
			break;
		}
		return TRUE;
 	}
	return FALSE;
}

//	___________________________________________________
//
//  Post Key Properties Dialog 

static DWORD WINAPI 
sSingleKeyThread (THREADSTRUCT* pts) 
{
	CHAR				szTitle[32];
	CHAR				szUserName[kPGPMaxUserIDSize];
    PROPSHEETPAGE		psp;
    PROPSHEETHEADER		psh;
	HPROPSHEETPAGE		hpsp[4];
	KMPROPSHEETSTRUCT	kmpss;
	INT					i, iRetVal;
	PGPError			err;
	PGPPrefRef			prefref;

	iRetVal = 0;

	for (i=0; i<MAXSHEETS; i++) {
		if (!pts->pKM->KeyTable[i]) {
			if (!pts->pKM->hWndTable[i]) {
				pts->pKM->KeyTable[i] = pts->key;
				kmpss.iIndex = i;
				break;
			}
		}
	}

	kmpss.pKM					= pts->pKM;
	kmpss.key					= pts->key;
	kmpss.keyset				= pts->keyset;
	kmpss.userid				= pts->userid;
	kmpss.cert					= kInvalidPGPSigRef;
	kmpss.bNeedsCommit			= FALSE;
	kmpss.bReadyToPaint			= FALSE;
	kmpss.hIml					= NULL;
	kmpss.iImageIndex			= -1;
	kmpss.hbitmapPhotoID		= NULL;
	kmpss.hpalettePhotoID		= NULL;
	kmpss.pPhotoBuffer			= NULL;
	kmpss.iPhotoBufferLength	= 0;
	kmpss.hwndRevokerDlg		= NULL;

	if ((pts->pKM->ulOptionFlags & KMF_READONLY) ||
		!(pts->pKM->ulOptionFlags & KMF_ENABLECOMMITS))
		kmpss.bReadOnly = TRUE;
	else
		kmpss.bReadOnly = FALSE;

	KMRequestSDKAccess (kmpss.pKM);

	// get the "show hex fingerprint" flag
	err = PGPclOpenClientPrefs (
				PGPGetContextMemoryMgr (kmpss.pKM->Context), 
				&prefref);
	if (IsntPGPError (err))
	{
		PGPGetPrefBoolean (prefref, kPGPPrefUseHexFingerprint, 
							&kmpss.bShowHexFingerprint);
		PGPclCloseClientPrefs (prefref, FALSE);
	}

#if PGP_BUSINESS_SECURITY
	err = PGPclOpenAdminPrefs (
				PGPGetContextMemoryMgr (kmpss.pKM->Context), 
				&prefref, PGPclIsAdminInstall());

	kmpss.bKeyGenEnabled = FALSE;
	kmpss.uMinSubkeySize = MINSUBKEYSIZE;
	if (IsntPGPError (err)) {
		PGPGetPrefBoolean (prefref, kPGPPrefAllowKeyGen, 
							&kmpss.bKeyGenEnabled);
		PGPGetPrefNumber (prefref, kPGPPrefMinimumKeySize, 
							&kmpss.uMinSubkeySize);
		PGPclCloseAdminPrefs (prefref, FALSE);
	}
#else
	kmpss.bKeyGenEnabled = TRUE;
	kmpss.uMinSubkeySize = MINSUBKEYSIZE;
#endif	// PGP_BUSINESS_SECURITY

	PGPCountAdditionalRecipientRequests (pts->key, &kmpss.uNumberADKs);
	PGPCountRevocationKeys (pts->key, &kmpss.uNumberRevokers);
	PGPGetKeyNumber (kmpss.key, kPGPKeyPropAlgID, &kmpss.algKey);

	KMReleaseSDKAccess (kmpss.pKM);

	LoadString (g_hInst, IDS_PROPTITLE0, szTitle, sizeof(szTitle));
	psh.nPages		= 1;		//always show the general sheet
    psp.dwSize		= sizeof(PROPSHEETPAGE);
    psp.dwFlags		= PSP_USETITLE | PSP_HASHELP;
    psp.hInstance	= g_hInst;
	psp.pszTemplate = MAKEINTRESOURCE (IDD_KEYPROPGENERAL);
    psp.pszIcon		= NULL;
    psp.pfnDlgProc	= sKeyPropDlgProcGeneral;
    psp.pszTitle	= szTitle;
    psp.lParam		= (LPARAM)&kmpss;
    psp.pfnCallback = NULL;
	hpsp[0]			= CreatePropertySheetPage (&psp);

	if (kmpss.algKey == kPGPPublicKeyAlgorithm_DSA) {
		LoadString (g_hInst, IDS_PROPTITLE1, szTitle, sizeof(szTitle));
		psp.dwSize		= sizeof(PROPSHEETPAGE);
	    psp.dwFlags		= PSP_USETITLE | PSP_HASHELP;
	    psp.hInstance	= g_hInst;
		psp.pszTemplate = MAKEINTRESOURCE (IDD_KEYPROPSUBKEYS);
		psp.pszIcon		= NULL;
		psp.pfnDlgProc	= sKeyPropDlgProcSubkey;
		psp.pszTitle	= szTitle;
		psp.lParam		= (LPARAM)&kmpss;
		psp.pfnCallback = NULL;
		hpsp[psh.nPages] = CreatePropertySheetPage (&psp);
		psh.nPages++;
	}

	if (kmpss.uNumberADKs > 0) {
		LoadString (g_hInst, IDS_PROPTITLE2, szTitle, sizeof(szTitle));
	    psp.dwSize		= sizeof(PROPSHEETPAGE);
	    psp.dwFlags		= PSP_USETITLE | PSP_HASHELP;
	    psp.hInstance	= g_hInst;
		psp.pszTemplate = MAKEINTRESOURCE (IDD_KEYPROPADK);
	    psp.pszIcon		= NULL;
	    psp.pfnDlgProc	= sKeyPropDlgProcADK;
	    psp.pszTitle	= szTitle;
	    psp.lParam		= (LPARAM)&kmpss;
	    psp.pfnCallback = NULL;
		hpsp[psh.nPages] = CreatePropertySheetPage (&psp);
		psh.nPages++;
	}

	if (kmpss.uNumberRevokers > 0) {
		LoadString (g_hInst, IDS_PROPTITLE3, szTitle, sizeof(szTitle));
	    psp.dwSize		= sizeof(PROPSHEETPAGE);
	    psp.dwFlags		= PSP_USETITLE | PSP_HASHELP;
	    psp.hInstance	= g_hInst;
		psp.pszTemplate = MAKEINTRESOURCE (IDD_KEYPROPREVOKERS);
	    psp.pszIcon		= NULL;
	    psp.pfnDlgProc	= sKeyPropDlgProcRevokers;
	    psp.pszTitle	= szTitle;
	    psp.lParam		= (LPARAM)&kmpss;
	    psp.pfnCallback = NULL;
		hpsp[psh.nPages] = CreatePropertySheetPage (&psp);
		psh.nPages++;
	}

	KMGetKeyName (pts->key, szUserName, sizeof(szUserName));
    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_NOAPPLYNOW;
	if (pts->pKM->ulOptionFlags & KMF_MODALPROPERTIES)
		psh.hwndParent = pts->pKM->hWndParent;
	else
		psh.hwndParent = NULL;
    psh.hInstance = g_hInst;
    psh.pszIcon = NULL;
    psh.pszCaption = (LPSTR) szUserName;

    psh.nStartPage = 0;
    psh.phpage = &hpsp[0];
    psh.pfnCallback = NULL;

    PropertySheet(&psh);

	kmpss.pKM->KeyTable[kmpss.iIndex] = NULL;
	kmpss.pKM->iNumberSheets--;

	// if anything changed, commit the changes
	if (kmpss.pKM->ulOptionFlags & KMF_ENABLECOMMITS) {
		if (kmpss.bNeedsCommit) {
			HCURSOR		hCursorOld; 
			UINT		uReloadMessage;

			hCursorOld = SetCursor (LoadCursor (NULL, IDC_WAIT));
			KMRequestSDKAccess (kmpss.pKM);
			err = PGPCommitKeyRingChanges (kmpss.pKM->KeySetMain);
			SetCursor (hCursorOld);
			KMReleaseSDKAccess (kmpss.pKM);
			SendMessage (kmpss.pKM->hWndParent, KM_M_KEYPROPACTION, 
						KM_PROPACTION_UPDATEKEY, (LPARAM)(kmpss.key));

			KMUpdateKeyPropertiesThread (kmpss.pKM);

			if (kmpss.pKM->ulOptionFlags & KMF_ENABLERELOADS) {
				if (IsntPGPError (err)) {
					uReloadMessage = 
						RegisterWindowMessage (RELOADKEYRINGMSG);
					PostMessage (HWND_BROADCAST, uReloadMessage, 
						MAKEWPARAM (LOWORD (kmpss.pKM->hWndParent), 
						FALSE), GetCurrentProcessId ());
					Sleep (200);
				}
			}
			InvalidateRect (kmpss.pKM->hWndTree, NULL, TRUE);
		}
	}

	// save the "show hex fingerprint" pref
	KMRequestSDKAccess (kmpss.pKM);
	err = PGPclOpenClientPrefs (
			PGPGetContextMemoryMgr (kmpss.pKM->Context), 
			&prefref);
	if (IsntPGPError (err))
	{
		PGPSetPrefBoolean (prefref, kPGPPrefUseHexFingerprint, 
							kmpss.bShowHexFingerprint);
		PGPclCloseClientPrefs (prefref, TRUE);
	}
	KMReleaseSDKAccess (kmpss.pKM);

	kmpss.pKM->hWndTable[kmpss.iIndex] = NULL;

	if (kmpss.pKM->iNumberSheets == 0) {
		SetActiveWindow (kmpss.pKM->hWndParent);
	}

	// free allocated objects
	if (kmpss.hbitmapPhotoID) {
		DeleteObject (kmpss.hbitmapPhotoID);
		kmpss.hbitmapPhotoID = NULL;
	}
	if (kmpss.hpalettePhotoID) 
		DeleteObject (kmpss.hpalettePhotoID);
	if (kmpss.pPhotoBuffer) 
		KMFree (kmpss.pPhotoBuffer);
	if (kmpss.hIml)
		ImageList_Destroy (kmpss.hIml);

	return iRetVal;
}


//	___________________________________________________
//
//  Post Cert Properties Dialog 

static DWORD WINAPI 
sSingleCertThread (THREADSTRUCT* pts) 
{
	CHAR				szTitle[32];
    PROPSHEETPAGE		psp;
    PROPSHEETHEADER		psh;
	HPROPSHEETPAGE		hpsp[4];
	KMPROPSHEETSTRUCT	kmpss;
	INT					i, iRetVal;

	iRetVal = 0;

	for (i=0; i<MAXSHEETS; i++) {
		if (!pts->pKM->SigTable[i]) {
			if (!pts->pKM->hWndTable[i]) {
				pts->pKM->SigTable[i] = pts->cert;
				kmpss.iIndex = i;
				break;
			}
		}
	}

	kmpss.pKM					= pts->pKM;
	kmpss.keyset				= pts->keyset;
	kmpss.userid				= pts->userid;
	kmpss.cert					= pts->cert;
	kmpss.bNeedsCommit			= FALSE;
	kmpss.bReadyToPaint			= FALSE;
	kmpss.hIml					= NULL;
	kmpss.iImageIndex			= -1;
	kmpss.hbitmapPhotoID		= NULL;
	kmpss.hpalettePhotoID		= NULL;
	kmpss.pPhotoBuffer			= NULL;
	kmpss.iPhotoBufferLength	= 0;
	kmpss.hwndRevokerDlg		= NULL;

	kmpss.bKeyGenEnabled		= FALSE;
	kmpss.uMinSubkeySize		= MINSUBKEYSIZE;

	if ((pts->pKM->ulOptionFlags & KMF_READONLY) ||
		!(pts->pKM->ulOptionFlags & KMF_ENABLECOMMITS))
		kmpss.bReadOnly = TRUE;
	else
		kmpss.bReadOnly = FALSE;

	// stuff requiring SDK access
	KMRequestSDKAccess (kmpss.pKM);

	PGPGetSigBoolean (kmpss.cert, kPGPSigPropIsX509, &kmpss.bX509);
	if (kmpss.bX509)
		psp.pszTemplate = MAKEINTRESOURCE (IDD_X509CERTPROP);
	else
		psp.pszTemplate = MAKEINTRESOURCE (IDD_PGPCERTPROP);

	KMReleaseSDKAccess (kmpss.pKM);

	if (kmpss.bX509)
		kmpss.key = KMGetKeyFromCert (kmpss.pKM, kmpss.cert);
	else
		kmpss.key = pts->key;

	psh.nPages		= 1;		
	psp.dwSize		= sizeof(PROPSHEETPAGE);
	psp.dwFlags		= PSP_HASHELP;
	psp.hInstance	= g_hInst;
	psp.pszIcon		= NULL;
	psp.pfnDlgProc	= sCertPropDlgProc;
	psp.pszTitle	= szTitle;
	psp.lParam		= (LPARAM)&kmpss;
	psp.pfnCallback = NULL;
	hpsp[0]			= CreatePropertySheetPage (&psp);


	LoadString (g_hInst, IDS_CERTPROPSTITLE, szTitle, sizeof(szTitle));
	psh.dwSize		= sizeof(PROPSHEETHEADER);
	psh.dwFlags		= PSH_NOAPPLYNOW;
	if (pts->pKM->ulOptionFlags & KMF_MODALPROPERTIES)
		psh.hwndParent = pts->pKM->hWndParent;
	else
		psh.hwndParent = NULL;
	psh.hInstance	= g_hInst;
	psh.pszIcon		= NULL;
	psh.pszCaption	= (LPSTR) szTitle;

	psh.nStartPage	= 0;
	psh.phpage		= &hpsp[0];
	psh.pfnCallback = NULL;

	PropertySheet(&psh);

	kmpss.pKM->SigTable[kmpss.iIndex] = NULL;
	kmpss.pKM->iNumberSheets--;

	kmpss.pKM->hWndTable[kmpss.iIndex] = NULL;

	if (kmpss.pKM->iNumberSheets == 0) {
		SetActiveWindow (kmpss.pKM->hWndParent);
	}

	if (kmpss.hIml)
		ImageList_Destroy (kmpss.hIml);

	return iRetVal;
}


//	___________________________________________________
//
//  if prop sheet does not exist for this key, start thread
//  to display properties 

static VOID 
sSingleKeyProperties (
		PKEYMAN			pKM,
		PGPKeyRef		key,
		PGPKeySetRef	keyset,
		PGPUserIDRef	userid,
		PBOOL			pbContinue) 
{
	THREADSTRUCT*	pts;
	INT				i;
	DWORD			dwID;

	// see if property sheet is already open for this key
	for (i=0; i<MAXSHEETS; i++) {
		if (key == pKM->KeyTable[i]) {
			// if user clicked on userid, communicate this to prop sheet
			if (PGPUserIDRefIsValid (userid)) {
				SendMessage (pKM->hWndTable[i], WM_APP+2, 
							0, (LPARAM)userid);
			}
			SetForegroundWindow (pKM->hWndTable[i]);
			if (pbContinue)
				*pbContinue = TRUE;
			return;
		}
	}

	// no existing property sheet for this key, 
	// see if we've reached the max number of sheets
	pKM->iNumberSheets++;
	if (pKM->iNumberSheets > MAXSHEETS) {
		pKM->iNumberSheets--;
		if (pbContinue)
			*pbContinue = FALSE;
		return;
	}

	// create sheet for this key
	if (key) {
		pts = (THREADSTRUCT*)KMAlloc (sizeof (THREADSTRUCT));
		if (pts) {
			pts->pKM	= pKM;
			pts->key	= key;
			pts->cert	= kInvalidPGPSigRef;
			pts->keyset = keyset;
			pts->userid = userid;
			_beginthreadex (NULL, 0, sSingleKeyThread, 
							(LPVOID)pts, 0, &dwID);
		}
	}
}


//	___________________________________________________
//
//  if prop sheet does not exist for this cert, start thread
//  to display properties 

static VOID 
sSingleCertProperties (
		PKEYMAN			pKM,
		PGPSigRef		cert,
		PGPKeyRef		key,
		PGPKeySetRef	keyset,
		PGPUserIDRef	userid,
		PBOOL			pbContinue) 
{
	THREADSTRUCT*	pts;
	INT				i;
	DWORD			dwID;

	// see if property sheet is already open for this key
	for (i=0; i<MAXSHEETS; i++) {
		if (cert == pKM->SigTable[i]) {
			SetForegroundWindow (pKM->hWndTable[i]);
			if (pbContinue)
				*pbContinue = TRUE;
			return;
		}
	}

	// no existing property sheet for this key, 
	// see if we've reached the max number of sheets
	pKM->iNumberSheets++;
	if (pKM->iNumberSheets > MAXSHEETS) {
		pKM->iNumberSheets--;
		if (pbContinue)
			*pbContinue = FALSE;
		return;
	}

	// create sheet for this key
	if (cert) {
		pts = (THREADSTRUCT*)KMAlloc (sizeof (THREADSTRUCT));
		if (pts) {
			pts->pKM	= pKM;
			pts->key	= key;
			pts->cert	= cert;
			pts->keyset = keyset;
			pts->userid = userid;
			_beginthreadex (NULL, 0, sSingleCertThread, 
							(LPVOID)pts, 0, &dwID);
		}
	}
}


//	___________________________________________________
//
//  Get key or cert ref to display properties for 

static BOOL CALLBACK 
sSingleProperties (TL_TREEITEM* lptli, LPARAM lParam) 
{
	PGPUserIDRef	userid			= kInvalidPGPUserIDRef;
	PGPSigRef		cert			= kInvalidPGPSigRef;
	PROPSTRUCT*		pps				= (PROPSTRUCT*)lParam;
	PGPKeyRef		key				= kInvalidPGPKeyRef;
	PGPKeySetRef	keyset			= kInvalidPGPKeySetRef;

	PGPError		err;
	BOOL			bContinue;

	switch (lptli->iImage) {
	case IDX_RSASECKEY :
	case IDX_RSASECDISKEY :
	case IDX_RSASECREVKEY :
	case IDX_RSASECEXPKEY :
	case IDX_RSASECSHRKEY :
	case IDX_RSAPUBKEY :
	case IDX_RSAPUBDISKEY :
	case IDX_RSAPUBREVKEY :
	case IDX_RSAPUBEXPKEY :
	case IDX_DSASECKEY :
	case IDX_DSASECDISKEY :
	case IDX_DSASECREVKEY :
	case IDX_DSASECEXPKEY :
	case IDX_DSASECSHRKEY :
	case IDX_DSAPUBKEY :
	case IDX_DSAPUBDISKEY :
	case IDX_DSAPUBREVKEY :
	case IDX_DSAPUBEXPKEY :
		key = (PGPKeyRef)(lptli->lParam);
		keyset = pps->pKM->KeySetDisp;
		bContinue = TRUE;
		break;

	case IDX_RSAUSERID :
	case IDX_DSAUSERID :
	case IDX_INVALIDUSERID :
		userid = (PGPUserIDRef)(lptli->lParam);
		key = KMGetKeyFromUserID (pps->pKM, userid);
		keyset = pps->pKM->KeySetDisp;
		userid = kInvalidPGPUserIDRef;
		bContinue = FALSE;
		break;

	case IDX_PHOTOUSERID :
		userid = (PGPUserIDRef)(lptli->lParam);
		key = KMGetKeyFromUserID (pps->pKM, userid);
		keyset = pps->pKM->KeySetDisp;
		bContinue = FALSE;
		break;

	case IDX_CERT :
	case IDX_REVCERT :
	case IDX_BADCERT :
	case IDX_EXPORTCERT :
	case IDX_TRUSTEDCERT :
	case IDX_METACERT :
	case IDX_X509CERT :
	case IDX_X509EXPCERT :
	case IDX_X509REVCERT :
		cert = (PGPSigRef)(lptli->lParam);
		key = NULL;

		err = PGPGetSigCertifierKey (cert, pps->pKM->KeySetDisp, &key);
		if (IsntPGPError (err)) {
			keyset = pps->pKM->KeySetDisp;
		}

		if (!key && (pps->pKM->KeySetDisp != pps->pKM->KeySetMain)) {
			err = PGPGetSigCertifierKey (cert, pps->pKM->KeySetMain, &key);
			if (IsntPGPError (err)) {
				keyset = pps->pKM->KeySetMain;
			}
		}

		bContinue = TRUE;
		break;

	default :
		return FALSE;
	}

	if (PGPSigRefIsValid (cert))
		sSingleCertProperties (
				pps->pKM, cert, key, keyset, userid, &bContinue);
	else
		sSingleKeyProperties (
				pps->pKM, key, keyset, userid, &bContinue);

	return bContinue;
}

	
//	___________________________________________________
//
//  Put up key properties dialog(s)

BOOL 
KMKeyProperties (PKEYMAN pKM) 
{
	INT i;
	PROPSTRUCT ps;

	if (pKM->iNumberSheets == 0) {
		for (i=0; i<MAXSHEETS; i++) {
			pKM->hWndTable[i] = NULL;
			pKM->KeyTable[i] = NULL;
		}
	}

	ps.pKM			= pKM;
	ps.bCertProps	= FALSE;
	ps.lpfnCallback = sSingleProperties;

	TreeList_IterateSelected (pKM->hWndTree, &ps);

	return TRUE;
}


//	___________________________________________________
//
//  Put up key properties dialog(s)

BOOL 
KMCertProperties (PKEYMAN pKM) 
{
	INT i;
	PROPSTRUCT ps;

	if (pKM->iNumberSheets == 0) {
		for (i=0; i<MAXSHEETS; i++) {
			pKM->hWndTable[i] = NULL;
			pKM->KeyTable[i] = NULL;
		}
	}

	ps.pKM			= pKM;
	ps.bCertProps	= TRUE;
	ps.lpfnCallback = sSingleProperties;

	TreeList_IterateSelected (pKM->hWndTree, &ps);

	return TRUE;
}


//	___________________________________________________
//
//  Update all existing propertysheets

VOID 
KMUpdateKeyProperties (PKEYMAN pKM) 
{
	INT i;

	if (pKM->iNumberSheets > 0) {
		for (i=0; i<MAXSHEETS; i++) {
			if (pKM->hWndTable[i] && pKM->KeyTable[i]) {
				SendMessage (pKM->hWndTable[i], WM_APP, NOSERIALIZE, 0);
			}
		}
	}
}


//	___________________________________________________
//
//  Update all existing propertysheets

VOID 
KMUpdateKeyPropertiesThread (PKEYMAN pKM) 
{
	INT i;

	if (pKM->iNumberSheets > 0) {
		for (i=0; i<MAXSHEETS; i++) {
			if (pKM->hWndTable[i] && pKM->KeyTable[i]) {
				PostMessage (pKM->hWndTable[i], WM_APP, SERIALIZE, 0);
			}
		}
	}
}


//	___________________________________________________
//
//  Delete existing propertysheets

VOID 
KMDeletePropertiesKey (PKEYMAN pKM, PGPKeyRef key) 
{
	INT i;

	if (pKM->iNumberSheets > 0) {
		for (i=0; i<MAXSHEETS; i++) {
			if (pKM->KeyTable[i] == key) {
				SendMessage (pKM->hWndTable[i], WM_APP+1, 0, 0);
				pKM->KeyTable[i] = NULL;
			}
		}
	}
}


//	___________________________________________________
//
//  Delete existing propertysheets

VOID 
KMDeleteAllKeyProperties (PKEYMAN pKM, 
						  BOOL bCloseWindows) 
{
	INT i;

	if (pKM->iNumberSheets > 0) {
		for (i=0; i<MAXSHEETS; i++) {
			if (pKM->hWndTable[i]) {
				if (bCloseWindows) {
					PropSheet_PressButton (GetParent (pKM->hWndTable[i]),
										   PSBTN_CANCEL);
				}
				else
					SendMessage (pKM->hWndTable[i], WM_APP+1, 0, 0);	
				pKM->KeyTable[i] = NULL;
			}
		}
	}

	Sleep (200);
}


//	___________________________________________________
//
//  Enable/Disable existing propertysheets

VOID
KMEnableAllKeyProperties (PKEYMAN pKM, 
						  BOOL bEnable) 
{
	INT		i;
	HWND	hwndParent;

	if (pKM->iNumberSheets > 0) {
		for (i=0; i<MAXSHEETS; i++) {
			if (pKM->hWndTable[i]) {
				hwndParent = GetParent (pKM->hWndTable[i]);
				EnableWindow (hwndParent, bEnable);
			}
		}
	}
}



