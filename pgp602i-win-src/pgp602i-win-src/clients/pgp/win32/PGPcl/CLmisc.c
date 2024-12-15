/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	CLmisc.c - PGP ClientLib DLL miscellaneous routines
	

	$Id: CLmisc.c,v 1.36 1998/08/11 14:43:10 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"	

// project header files
#include "pgpclx.h"
#include "pgpkmx.h"
#include "..\include\PGPversion.h"
#include "..\include\PGPpk.h"

// constants
#define TEXTXPOS	0.65
#define TEXTYPOS	0.73

typedef struct {
	PGPContextRef	context;
	LPSTR			pszRemoteHost;
	PGPKeyRef		keyAuthenticating;
	PGPKeySetRef	keysetMain;
	CHAR			szName[kPGPMaxUserIDSize];
	UINT			uFlags;
	PGPValidity		validityThreshold;
} CONFIRMAUTHSTRUCT, *PCONFIRMAUTHSTRUCT;

// external globals
extern HINSTANCE g_hInst;
extern CHAR g_szHelpFile[MAX_PATH];

// local globals
static DWORD aIds[] = {			// Help IDs
    IDC_SERVERNAME,	IDH_PGPCLMISC_AUTHSERVERNAME, 
    IDC_KEYNAME,	IDH_PGPCLMISC_AUTHKEYNAME, 
    IDC_FINGERPRINT,IDH_PGPCLMISC_AUTHKEYFINGERPRINT, 
    IDC_VALIDITY,	IDH_PGPCLMISC_AUTHKEYVALIDITY, 
    IDC_IMPORTKEY,	IDH_PGPCLMISC_AUTHIMPORTKEY, 
	IDOK,			IDH_PGPCLMISC_AUTHCONFIRM,
    0,0 
}; 

//	___________________________________________________
//
//	Internal memory allocation routines
//

VOID* 
clAlloc (UINT uBytes) 
{
	VOID* p;
	p = malloc (uBytes);
	if (p) {
		memset (p, 0, uBytes);
	}
	return p;
}


VOID 
clFree (VOID* p) 
{
	if (p)
		free (p);
}

//	___________________________________________________
//
//	Message box routine using string table resource IDs

LRESULT 
PGPclMessageBox (
		 HWND	hWnd, 
		 INT	iCaption, 
		 INT	iMessage,
		 ULONG	ulFlags) 
{
	CHAR szCaption [128];
	CHAR szMessage [256];

	LoadString (g_hInst, iCaption, szCaption, sizeof(szCaption));
	LoadString (g_hInst, iMessage, szMessage, sizeof(szMessage));

	ulFlags |= MB_SETFOREGROUND;
	return (MessageBox (hWnd, szMessage, szCaption, ulFlags));
}

//	___________________________________________________
//
//	convert SYSTEMTIME structure to number of days from today

PGPError PGPclExport
PGPclSystemTimeToDays (
		 SYSTEMTIME*	pst, 
		 INT*			piDays) 
{
	SYSTEMTIME	stToday;
	struct tm	tmstruct;
	time_t		timeToday;
	time_t		timeInQuestion;
	UINT		uDayToday;
	UINT		uDayInQuestion;

	pgpAssert (pst != NULL);
	pgpAssert (piDays != NULL);

	*piDays = -1;

	if (pst->wYear > 2037) 
		return kPGPError_BadParams;

	GetLocalTime (&stToday);

	tmstruct.tm_mday = stToday.wDay;
	tmstruct.tm_mon = stToday.wMonth -1;
	tmstruct.tm_year = stToday.wYear -1900;
	tmstruct.tm_hour = 12;
	tmstruct.tm_min = 0;
	tmstruct.tm_sec = 0;
	tmstruct.tm_isdst = -1;

	timeToday = mktime (&tmstruct);
	if (timeToday == (time_t)-1) return kPGPError_BadParams;

	tmstruct.tm_mday = pst->wDay;
	tmstruct.tm_mon = pst->wMonth -1;
	tmstruct.tm_year = pst->wYear -1900;
	tmstruct.tm_hour = 12;
	tmstruct.tm_min = 0;
	tmstruct.tm_sec = 0;
	tmstruct.tm_isdst = -1;

	timeInQuestion = mktime (&tmstruct);
	if (timeInQuestion == (time_t)-1) return kPGPError_BadParams;

	uDayToday = timeToday / 86400; 
	uDayInQuestion = timeInQuestion / 86400; 

	*piDays = uDayInQuestion - uDayToday;

	return kPGPError_NoErr;
}


//	___________________________________________________
//
//	create logical palette from bitmap color table

static HPALETTE 
sCreateDIBPalette (
		  LPBITMAPINFO	lpbmi, 
		  LPINT			lpiNumColors) 
{
	LPBITMAPINFOHEADER	lpbi;
	LPLOGPALETTE		lpPal;
	HANDLE				hLogPal;
	HPALETTE			hPal = NULL;
	INT					i;
 
	lpbi = (LPBITMAPINFOHEADER)lpbmi;
	if (lpbi->biBitCount <= 8)
		*lpiNumColors = (1 << lpbi->biBitCount);
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
 
//	___________________________________________________
//
//	Load DIB bitmap and associated palette

HBITMAP
CLLoadResourceBitmap (
		HINSTANCE		hInstance, 
		LPSTR			lpString,
		HPALETTE FAR*	lphPalette) 
{
	HRSRC				hRsrc;
	HGLOBAL				hGlobal;
	HBITMAP				hBitmapFinal = NULL;
	LPBITMAPINFOHEADER	lpbi;
	HDC					hdc;
    INT					iNumColors;
 
	if (hRsrc = FindResource (hInstance, lpString, RT_BITMAP)) {
		hGlobal = LoadResource (hInstance, hRsrc);
		lpbi = (LPBITMAPINFOHEADER)LockResource (hGlobal);
 
		hdc = GetDC(NULL);
		*lphPalette =  sCreateDIBPalette ((LPBITMAPINFO)lpbi, &iNumColors);
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
 

//	___________________________________________________
//
//	Paint user info and registration number 

#define YTEXTINC	14

VOID 
CLPaintUserInfo (
		PGPMemoryMgrRef	memMgr,
		HWND			hwnd,
		HDC				hdc, 
		LPSTR			pszVersion)
{
	HFONT		hFontOld;
	UINT		uTAOld;
	CHAR		sz[256];
	PGPError	err;
	PGPPrefRef	PrefRef;
	RECT		rc;
	INT			iXpos, iYpos;

	GetClientRect (hwnd, &rc);
	iXpos = (INT)(TEXTXPOS * rc.right);
	iYpos = (INT)(TEXTYPOS * rc.bottom);

	SetTextColor (hdc, RGB (255, 255, 255));
	SetBkMode (hdc, TRANSPARENT);
	uTAOld = SetTextAlign (hdc, TA_LEFT);
	hFontOld = SelectObject (hdc, GetStockObject (DEFAULT_GUI_FONT));

	sz[0] = '\0';
#if PGP_BUSINESS_SECURITY
	if (PGPclOpenAdminPrefs (memMgr, &PrefRef, FALSE) != kPGPError_NoErr)
		return;
	err = PGPGetPrefStringBuffer (PrefRef, kPGPPrefAdminCompanyName, 
										sizeof(sz), sz);
	if (IsPGPError (err)) sz[0] = '\0';
	PGPclCloseAdminPrefs (PrefRef, FALSE);
#endif

	if (PGPclOpenClientPrefs (memMgr, &PrefRef) != kPGPError_NoErr)
		return;

	// company name
	if (sz[0] == '\0') {
		err = PGPGetPrefStringBuffer (PrefRef, kPGPPrefCompanyName, 
										sizeof(sz), sz);
	}
	if (IsPGPError (err)) sz[0] = '\0';
	rc.left = iXpos;
	rc.right -= 4;
	rc.top = iYpos + (2*YTEXTINC);
	rc.bottom = rc.top + YTEXTINC;
	DrawTextEx (hdc, sz, lstrlen (sz), &rc, DT_END_ELLIPSIS, NULL);

	// user name
	sz[0] = '\0';
	err = PGPGetPrefStringBuffer (PrefRef, kPGPPrefOwnerName, 
										sizeof(sz), sz);
	rc.top = iYpos + (1*YTEXTINC);
	rc.bottom = rc.top + YTEXTINC;
	DrawTextEx (hdc, sz, lstrlen (sz), &rc, DT_END_ELLIPSIS, NULL);

	// version number
	if (pszVersion) 
		lstrcpy (sz, pszVersion);
	else
		lstrcpy (sz, PGPVERSIONSTRING);
	TextOut (hdc, iXpos, iYpos, sz, lstrlen (sz));

	// labels
	uTAOld = SetTextAlign (hdc, TA_RIGHT);
	LoadString (g_hInst, IDS_VERSION, sz, sizeof(sz));
	TextOut (hdc, iXpos-8, iYpos, sz, lstrlen (sz));

	LoadString (g_hInst, IDS_LICENSEDTO, sz, sizeof(sz));
	TextOut (hdc, iXpos-8, iYpos+YTEXTINC, sz, lstrlen (sz));

	SetTextAlign (hdc, uTAOld);
	SelectObject (hdc, hFontOld);
	PGPclCloseClientPrefs (PrefRef, FALSE);
}

//	___________________________________________________
//
//	Broadcast reload message

VOID PGPclExport 
PGPclNotifyKeyringChanges (LPARAM lParam) 
{
	UINT uMessageID;

	uMessageID = RegisterWindowMessage (RELOADKEYRINGMSG);
	PostMessage (HWND_BROADCAST, uMessageID, 0, lParam);
}

//	___________________________________________________
//
//	Broadcast reload message

VOID PGPclExport 
PGPclNotifyPrefsChanges (LPARAM lParam) 
{
	UINT uMessageID;

	uMessageID = RegisterWindowMessage (RELOADPREFSMSG);
	PostMessage (HWND_BROADCAST, uMessageID, 0, lParam);
}

//	___________________________________________________
//
//	Broadcast messages indicating keyserver prefs may have changed

VOID PGPclExport 
PGPclNotifyKeyserverPrefsChanges (LPARAM lParam) 
{
	UINT uMessageID;

	uMessageID = RegisterWindowMessage (RELOADKEYSERVERPREFSMSG);
	PostMessage (HWND_BROADCAST, uMessageID, 0, lParam);
}

//	___________________________________________________
//
//	get path of PGP installation from registry key 
//	note: includes trailing '\'

PGPError PGPclExport 
PGPclGetPGPPath (LPSTR szPath, UINT uLen) 
{
	HKEY		hKey;
	LONG		lResult;
	CHAR*		p;
	DWORD		dwValueType, dwSize;
	CHAR		szKey[128];
	PGPError	err;

	err = kPGPError_FileNotFound;

	lstrcpy (szPath, "");

	LoadString (g_hInst, IDS_REGISTRYKEY, szKey, sizeof(szKey));
	lResult = RegOpenKeyEx (HKEY_CURRENT_USER, szKey, 0, KEY_READ, &hKey);

	if (lResult == ERROR_SUCCESS) {
		err = kPGPError_OutputBufferTooSmall;
		dwSize = uLen;
		lResult = RegQueryValueEx (hKey, "ExePath", 0, &dwValueType, 
			(LPBYTE)szPath, &dwSize);
		RegCloseKey (hKey);
		if (lResult == ERROR_SUCCESS) {
			p = strrchr (szPath, '\\');
			if (p) {
				p++;
				*p = '\0';
			}
			err = kPGPError_NoErr;
		}
	}

	return err;
}

//	___________________________________________________
//
//	get keyring and randseed file paths from SDK 

PGPError PGPclExport 
PGPclGetSDKFilePaths (
		LPSTR	pszPubRingPath,
		INT		iPubRingLen,
		LPSTR	pszPrivRingPath,
		INT		iPrivRingLen,
		LPSTR	pszRandSeedPath,
		INT		iRandSeedLen) 
{
	PGPError err;
	PGPFileSpecRef	fileref;
	PGPContextRef	context;
	LPSTR			lpsz;

	err = PGPNewContext ( kPGPsdkAPIVersion, &context);
	PGPsdkLoadDefaultPrefs (context);

	if (IsntPGPError (err)) {
	
		if (pszPubRingPath) {
			err = PGPsdkPrefGetFileSpec (context, kPGPsdkPref_PublicKeyring,
									&fileref);
			if (IsntPGPError (err) && fileref) {
				err = PGPGetFullPathFromFileSpec (fileref, &lpsz);
				if (IsntPGPError (err)) {
					lstrcpyn (pszPubRingPath, lpsz, iPubRingLen);
					PGPFreeData (lpsz);
				}
				PGPFreeFileSpec (fileref);
			}
		}

		if (pszPrivRingPath && IsntPGPError (err)) {
			err = PGPsdkPrefGetFileSpec (context, kPGPsdkPref_PrivateKeyring,
									&fileref);
			if (IsntPGPError (err) && fileref) {
				err = PGPGetFullPathFromFileSpec (fileref, &lpsz);
				if (IsntPGPError (err)) {
					lstrcpyn (pszPrivRingPath, lpsz, iPrivRingLen);
					PGPFreeData (lpsz);
				}
				PGPFreeFileSpec (fileref);
			}
		}

		if (pszRandSeedPath && IsntPGPError (err)) {
			err = PGPsdkPrefGetFileSpec (context, kPGPsdkPref_RandomSeedFile,
									&fileref);
			if (IsntPGPError (err) && fileref) {
				err = PGPGetFullPathFromFileSpec (fileref, &lpsz);
				if (IsntPGPError (err)) {
					lstrcpyn (pszRandSeedPath, lpsz, iRandSeedLen);
					PGPFreeData (lpsz);
				}
				PGPFreeFileSpec (fileref);
			}
		}
		PGPFreeContext (context);
	}

	return err;
}

//	___________________________________________________
//
//	set keyring and randseed file paths using SDK 

PGPError PGPclExport 
PGPclSetSDKFilePaths (
		LPSTR	pszPubRingPath,
		LPSTR	pszPrivRingPath,
		LPSTR	pszRandSeedPath,
		BOOL	bCreateFiles)
{
	PGPKeySetRef	keysetDummy		= kInvalidPGPKeySetRef;
	PGPFileSpecRef	fileref			= kInvalidPGPFileSpecRef;
	PGPContextRef	context			= kInvalidPGPContextRef;
	PGPError		err				= kPGPError_NoErr;

	err = PGPNewContext (kPGPsdkAPIVersion, &context);
	PGPsdkLoadDefaultPrefs (context);

	if (IsntPGPError (err)) {
	
		if (pszPubRingPath) {
			err = PGPNewFileSpecFromFullPath (context, 
									pszPubRingPath, &fileref); CKERR;
			if (PGPFileSpecRefIsValid (fileref)) {
				err = PGPsdkPrefSetFileSpec (context, 
								kPGPsdkPref_PublicKeyring, fileref);
				PGPFreeFileSpec (fileref);
				fileref = kInvalidPGPFileSpecRef;
			}
		} CKERR;

		if (pszPrivRingPath) {
			err = PGPNewFileSpecFromFullPath (context, 
									pszPrivRingPath, &fileref); CKERR;
			if (PGPFileSpecRefIsValid (fileref)) {
				err = PGPsdkPrefSetFileSpec (context, 
								kPGPsdkPref_PrivateKeyring, fileref);
				PGPFreeFileSpec (fileref);
				fileref = kInvalidPGPFileSpecRef;
			}
		} CKERR;

		if (pszRandSeedPath) {
			err = PGPNewFileSpecFromFullPath (context, 
									pszRandSeedPath, &fileref); CKERR;
			if (PGPFileSpecRefIsValid (fileref)) {
				err = PGPsdkPrefSetFileSpec (context, 
								kPGPsdkPref_RandomSeedFile, fileref);
				PGPFreeFileSpec (fileref);
				fileref = kInvalidPGPFileSpecRef;
			}
		} CKERR;

		PGPsdkSavePrefs (context);

		if (IsntPGPError (err) && bCreateFiles) {
			err = PGPOpenDefaultKeyRings (context, 
				kPGPKeyRingOpenFlags_Create|kPGPKeyRingOpenFlags_Mutable, 
				&keysetDummy);
			if (PGPKeySetRefIsValid (keysetDummy))
				PGPFreeKeySet (keysetDummy);
		}

	}
done :
	if (PGPFileSpecRefIsValid (fileref)) 
		PGPFreeFileSpec (fileref);
	if (PGPContextRefIsValid (context))
		PGPFreeContext (context);

	return err;
}


//	__________________________________________________
//
//	save user information to preferences file 

PGPError PGPclExport 
PGPclSetUserInfo (
		LPSTR	szOwnerName,
		LPSTR	szCompanyName,
		LPSTR	szLicenseNumber)
{
	PGPError		err;
	PGPPrefRef		prefref;
	PGPContextRef	context;

	err = PGPNewContext (kPGPsdkAPIVersion, &context);
	if (IsPGPError (err)) return err;

	err = PGPclOpenClientPrefs (PGPGetContextMemoryMgr (context), &prefref);
	if (IsPGPError (err)) {
		PGPFreeContext (context);
		return err;
	}

	if (szOwnerName) {
		PGPSetPrefString (prefref, kPGPPrefOwnerName, szOwnerName);
	}

	if (szCompanyName) {
		PGPSetPrefString (prefref, kPGPPrefCompanyName, szCompanyName);
	}

#if (PGP_NO_LICENSE_NUMBER == 0)
	if (szLicenseNumber) {
		PGPSetPrefString (prefref, kPGPPrefLicenseNumber, szLicenseNumber);
	}
#endif //PGP_NO_LICENSE_NUMBER

	err = PGPclCloseClientPrefs (prefref, TRUE);

	PGPFreeContext (context);
	return err;
}


//	_________________________________________________
//
//	Wrapper routine for platform independent 
//	word wrap code.

PGPError PGPclExport 
PGPclWrapBuffer (
		LPSTR		szInText,
		PGPUInt16	wrapColumn,
		LPSTR*		pszOutText)
{
	PGPError		err				= kPGPError_NoErr;
	PGPIORef		inRef, outRef;
	PGPMemoryMgrRef	memMgr;
	PGPFileOffset	outSize;
	PGPSize			outRead;
	INT				InSize;

	outRead = 0;

	err = PGPNewMemoryMgr ( 0, &memMgr);

	if (IsntPGPError (err))
	{
		PGPNewMemoryIO (
			memMgr,
			(PGPMemoryIORef *)(&inRef));

		InSize = strlen (szInText);
		PGPIOWrite (inRef,
			InSize,
			szInText);
		PGPIOFlush (inRef);
		PGPIOSetPos (inRef,0);

		PGPNewMemoryIO (
			memMgr,
			(PGPMemoryIORef *)(&outRef));

		err = pgpWordWrapIO(
			  inRef,outRef,
			  wrapColumn,
			  "\r\n");

		if (IsntPGPError (err))
		{
			INT memamt;

			PGPIOGetPos (outRef, &outSize);

			memamt = (INT)outSize+1;
			*pszOutText = (CHAR *)malloc (memamt);

			if (*pszOutText)
			{
				memset (*pszOutText, 0x00, memamt);

				PGPIOSetPos (outRef,0);
				PGPIORead (outRef,
					(INT)outSize,
					*pszOutText, 
					&outRead);
				PGPIOFlush (outRef);
			}
			else err = kPGPError_OutOfMemory;
		}
		PGPFreeIO (inRef); 
		PGPFreeIO (outRef);
	}
	PGPFreeMemoryMgr (memMgr);

	return err;
}

//	__________________________________________________
//
//	free previously-wrapped buffer

PGPError PGPclExport 
PGPclFreeWrapBuffer (LPSTR textBuffer)
{
	memset (textBuffer, 0x00, lstrlen (textBuffer));
	free (textBuffer);

	return kPGPError_NoErr;
}

//	________________________
//
//	Get keyid string from key

PGPError PGPclExport 
PGPclGetKeyFromKeyID (
		PGPContextRef	context,
		PGPKeySetRef	keyset,
		LPSTR			szID,
		UINT			uAlg,
		PGPKeyRef*		pkey) 
{
	PGPError		err; 
	PGPKeyID		keyID;

	err = PGPImportKeyID (szID, &keyID);
	if (IsntPGPError (err)) {
		err = PGPGetKeyByKeyID (keyset, &keyID, uAlg, pkey);
		if (!(*pkey)) 
			err = kPGPError_ItemNotFound;
	}

	return err;
}

//	________________________
//
//	find out if SDK supports the specified public key algorithm

PGPError PGPclExport 
PGPclCheckSDKSupportForPKAlg (
		PGPPublicKeyAlgorithm alg,
		PGPBoolean mustEncrypt,
		PGPBoolean mustSign)
{
	PGPError	err			= kPGPError_FeatureNotAvailable;

	PGPUInt32					i, iNumAlgs;
	PGPPublicKeyAlgorithmInfo	alginfo;

	err = PGPCountPublicKeyAlgorithms (&iNumAlgs);
	if (IsPGPError (err)) return kPGPError_FeatureNotAvailable;

	for (i=0; i<iNumAlgs; i++) {
		err = PGPGetIndexedPublicKeyAlgorithmInfo (i, &alginfo);
		if (IsntPGPError (err)) {
			if (alginfo.algID == alg) {
				err = kPGPError_NoErr;
				if (mustEncrypt && !(alginfo.canEncrypt))
					err = kPGPError_FeatureNotAvailable;
				if (mustSign && !(alginfo.canSign))
					err = kPGPError_FeatureNotAvailable;
				return err;
			}
		}
	}

	return kPGPError_FeatureNotAvailable;
}

//	________________________
//
//	find out if SDK supports the specified cipher algorithm

PGPError PGPclExport 
PGPclCheckSDKSupportForCipherAlg (PGPCipherAlgorithm alg)
{
	PGPError	err		= kPGPError_FeatureNotAvailable;

	PGPUInt32					i, iNumAlgs;
	PGPSymmetricCipherInfo		cipherinfo;

	err = PGPCountSymmetricCiphers (&iNumAlgs);
	if (IsPGPError (err)) return kPGPError_FeatureNotAvailable;

	for (i=0; i<iNumAlgs; i++) {
		err = PGPGetIndexedSymmetricCipherInfo (i, &cipherinfo);
		if (IsntPGPError (err)) {
			if (cipherinfo.algID == alg) {
				return kPGPError_NoErr;
			}
		}
	}

	return kPGPError_FeatureNotAvailable;
}

//	________________________
//
//	sync up keysets to resolve trust information

PGPError PGPclExport 
PGPclSyncKeySets (
		PGPContextRef context,
		PGPKeySetRef keysetMain,
		PGPKeySetRef keysetNew)
{

	PGPKeySetRef	keysetCombined;
	PGPKeySetRef	keysetSync;
	PGPError		err;

	if (!PGPRefIsValid (keysetNew)) return kPGPError_NoErr;

	if (!PGPRefIsValid (keysetMain)) {
		PGPsdkLoadDefaultPrefs (context);
		err = PGPOpenDefaultKeyRings (context, 0, &keysetSync);
		if (IsPGPError (err) || !PGPRefIsValid (keysetSync)) return err;
	}
	else keysetSync = keysetMain;

	err = PGPNewKeySet (context, &keysetCombined);
	if (IsntPGPError (err)) {

		err = PGPAddKeys (keysetNew, keysetCombined);
		if (IsntPGPError (err)) {

			err = PGPAddKeys (keysetSync, keysetCombined);
			if (IsntPGPError (err)) {

				err = PGPCheckKeyRingSigs (keysetNew, 
									keysetCombined, FALSE, NULL, 0);

				if (IsntPGPError (err)) {
					err = PGPPropagateTrust (keysetCombined);
				}
			}
		}

		PGPFreeKeySet (keysetCombined);
	}

	if (!PGPRefIsValid (keysetMain)) PGPFreeKeySet (keysetSync);

	return err;
}

//	__________________________________________________________
//
//  Check to see if it's time to auto-update keys

PGPError PGPclExport
PGPclCheckAutoUpdate(PGPMemoryMgrRef memoryMgr, 
					 PGPBoolean bResetDates,
					 PGPBoolean *pbUpdateAllKeys,
					 PGPBoolean *pbUpdateTrustedIntroducers)
{
	PGPError	err = kPGPError_NoErr;

#if PGP_BUSINESS_SECURITY
	PGPPrefRef	adminPrefs;
	PGPBoolean	bUpdateAllKeys;
	PGPBoolean	bUpdateTrustedIntroducers;
	PGPInt32	nDaysUpdateAllKeys;
	PGPInt32	nDaysUpdateTrustedIntroducers;
	PGPUInt32	nLastUpdate;
	time_t		tToday;
	time_t		tLastUpdate;
	struct tm 	tmToday;
	struct tm 	tmLastUpdate;
#endif //PGP_BUSINESS_SECURITY

	if (pbUpdateAllKeys != NULL)
		*pbUpdateAllKeys = FALSE;

	if (pbUpdateTrustedIntroducers != NULL)
		*pbUpdateTrustedIntroducers = FALSE;

#if PGP_BUSINESS_SECURITY

	err = PGPclOpenAdminPrefs(memoryMgr, &adminPrefs, TRUE);

	if (IsPGPError(err))
		return err;

	tToday = PGPGetStdTimeFromPGPTime(PGPGetTime());
	memcpy(&tmToday, localtime(&tToday), sizeof(struct tm));

	err = PGPGetPrefBoolean(adminPrefs, kPGPPrefUpdateAllKeys, 
			&bUpdateAllKeys);
	if (IsPGPError(err))
	{
		bUpdateAllKeys = FALSE;
		err = kPGPError_NoErr;
	}

	err = PGPGetPrefBoolean(adminPrefs, kPGPPrefUpdateTrustedIntroducers, 
			&bUpdateTrustedIntroducers);
	if (IsPGPError(err))
	{
		bUpdateTrustedIntroducers = FALSE;
		err = kPGPError_NoErr;
	}

	if (bUpdateAllKeys)
	{
		err = PGPGetPrefNumber(adminPrefs, kPGPPrefDaysUpdateAllKeys,
				&nDaysUpdateAllKeys);
		if (IsPGPError(err))
		{
			nDaysUpdateAllKeys = 0;
			err = kPGPError_NoErr;
		}

		if (nDaysUpdateAllKeys > 0)
		{
			err = PGPGetPrefNumber(adminPrefs, kPGPPrefLastAllKeysUpdate,
					&nLastUpdate);
			if (IsPGPError(err))
			{
				nLastUpdate = 0;
				err = kPGPError_NoErr;
			}

			tLastUpdate = PGPGetStdTimeFromPGPTime(nLastUpdate);
			memcpy(&tmLastUpdate, localtime(&tLastUpdate), sizeof(struct tm));

			if (tmToday.tm_year > tmLastUpdate.tm_year)
				tmToday.tm_yday += 366;

			if ((tmToday.tm_yday - tmLastUpdate.tm_yday) >=
				nDaysUpdateAllKeys)
			{
				*pbUpdateAllKeys = TRUE;
				if (bResetDates)
				{
					PGPSetPrefNumber(adminPrefs, kPGPPrefLastAllKeysUpdate,
						PGPGetTime());
				}
			}
		}
	}

	if (bUpdateTrustedIntroducers)
	{
		err = PGPGetPrefNumber(adminPrefs, 
				kPGPPrefDaysUpdateTrustedIntroducers,
				&nDaysUpdateTrustedIntroducers);
		if (IsPGPError(err))
		{
			nDaysUpdateTrustedIntroducers = 0;
			err = kPGPError_NoErr;
		}

		if (nDaysUpdateTrustedIntroducers > 0)
		{
			err = PGPGetPrefNumber(adminPrefs, 
					kPGPPrefLastTrustedIntroducersUpdate,
					&nLastUpdate);
			if (IsPGPError(err))
			{
				nLastUpdate = 0;
				err = kPGPError_NoErr;
			}

			tLastUpdate = PGPGetStdTimeFromPGPTime(nLastUpdate);
			memcpy(&tmLastUpdate, localtime(&tLastUpdate), sizeof(struct tm));

			if (tmToday.tm_year > tmLastUpdate.tm_year)
				tmToday.tm_yday += 366;

			if ((tmToday.tm_yday - tmLastUpdate.tm_yday) >=
				nDaysUpdateTrustedIntroducers)
			{
				*pbUpdateTrustedIntroducers = TRUE;
				if (bResetDates) 
				{
					PGPSetPrefNumber(adminPrefs, 
						kPGPPrefLastTrustedIntroducersUpdate,
						PGPGetTime());
				}
			}
		}
	}

	PGPclCloseAdminPrefs(adminPrefs, TRUE);

#endif	// PGP_BUSINESS_SECURITY

	return err;
}


//----------------------------------------------------|
// Sign key dialog message procedure

static BOOL CALLBACK 
sConfirmAuthDlgProc (
				HWND hDlg, 
				UINT uMsg, 
				WPARAM wParam, 
				LPARAM lParam) 
{
	PCONFIRMAUTHSTRUCT	pcas;
	CHAR				sz[64];
	INT					i;
	PGPValidity			validity;
	PGPUInt32			uAlgorithm;
	UINT				uIDS;

	switch (uMsg) {

	case WM_INITDIALOG :
		SetWindowLong (hDlg, GWL_USERDATA, lParam);
		pcas = (PCONFIRMAUTHSTRUCT)lParam;

		// initialize validity bar
		i = KMConvertFromPGPValidity (kPGPValidity_Complete);
		SendMessage (GetDlgItem (hDlg, IDC_VALIDITY), PBM_SETRANGE, 
									0, MAKELPARAM (0,i));
		PGPGetKeyNumber (pcas->keyAuthenticating, kPGPKeyPropValidity, 
								&validity);
		i = KMConvertFromPGPValidity (validity);
		SendMessage (GetDlgItem (hDlg, IDC_VALIDITY), PBM_SETPOS, 
									(WPARAM)i, 0);

		// initialize text
		uIDS = 0;
		switch (pcas->uFlags) {
		case PGPCL_SHOWAUTHENTICATION :
			ShowWindow (GetDlgItem (hDlg, IDOK), SW_HIDE);
			LoadString (g_hInst, IDS_DONE, sz, sizeof(sz));
			SetDlgItemText (hDlg, IDCANCEL, sz);
			break;

		case PGPCL_AUTHEXPECTEDKEY :
			if (validity >= pcas->validityThreshold) {
				EndDialog (hDlg, 1);
				return FALSE;
			}
			else 
				uIDS = IDS_INVALIDAUTHKEY;
			break;

		case PGPCL_AUTHRECONSTITUTING :
			if (validity < pcas->validityThreshold)
				uIDS = IDS_INVALIDAUTHKEY;
			else 
				uIDS = IDS_VALIDAUTHKEY;
			break;

		case PGPCL_AUTHNEWKEY :
			uIDS = IDS_NEWAUTHKEY;
			break;

		case PGPCL_AUTHUNEXPECTEDKEY :
			uIDS = IDS_UNEXPECTEDAUTHKEY;
			break;
		}

		if (uIDS) {
			CHAR	szText[256];

			LoadString (g_hInst, uIDS, szText, sizeof(szText));
			SetDlgItemText (hDlg, IDC_AUTHTEXT, szText);
		}

		// initialize server name
		SetDlgItemText (hDlg, IDC_SERVERNAME, pcas->pszRemoteHost);

		// initialize key name
		SetDlgItemText (hDlg, IDC_KEYNAME, pcas->szName);

		// initialize fingerprint edit control
		PGPGetKeyPropertyBuffer (pcas->keyAuthenticating, 
				kPGPKeyPropFingerprint, sizeof (sz), sz, &i);
		PGPGetKeyNumber (pcas->keyAuthenticating, 
				kPGPKeyPropAlgID, &uAlgorithm);
		KMConvertStringFingerprint (uAlgorithm, sz);
		SetDlgItemText (hDlg, IDC_FINGERPRINT, sz);

		return TRUE;

	case WM_HELP: 
	    WinHelp (((LPHELPINFO) lParam)->hItemHandle, g_szHelpFile, 
	        HELP_WM_HELP, (DWORD) (LPSTR) aIds); 
	    break; 

	case WM_CONTEXTMENU: 
		WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU, 
		    (DWORD) (LPVOID) aIds); 
		break; 

	case WM_COMMAND :
		switch (LOWORD(wParam)) {
		case IDCANCEL :
			EndDialog (hDlg, 0);
			break;

		case IDOK :
			EndDialog (hDlg, 1);
			break;

		case IDC_IMPORTKEY :
			pcas = (PCONFIRMAUTHSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
			{
				PGPKeySetRef		keysetAuth;
				PGPError			err; 

				err = PGPNewSingletonKeySet (
							pcas->keyAuthenticating, &keysetAuth);
				if (IsntPGPError (err) && PGPKeySetRefIsValid (keysetAuth)) 
				{
					err = CLAddKeysToMain (
								pcas->context,
								hDlg, 
								keysetAuth,
								pcas->keysetMain);
					PGPFreeKeySet (keysetAuth);
				}
			}
			break;
		}
		return TRUE;
	}
	return FALSE;
}

//----------------------------------------------------|
//  post confirmation dialog for authentication key

PGPError PGPclExport
PGPclConfirmRemoteAuthentication (
		PGPContextRef	context,
		HWND			hwndParent, 
		LPSTR			pszServer,
		PGPKeyRef		keyAuth,
		PGPKeySetRef	keysetMain,
		UINT			uFlags)
{
	CONFIRMAUTHSTRUCT	cas;
	UINT				u;
	PGPPrefRef			prefs;
	PGPBoolean			bMargIsInvalid;

	cas.context				= context;
	cas.pszRemoteHost		= pszServer;
	cas.keyAuthenticating	= keyAuth;
	cas.keysetMain			= keysetMain;
	cas.uFlags				= uFlags;
	cas.szName[0]			= '\0';

	if (PGPKeyRefIsValid (keyAuth))
		PGPGetPrimaryUserIDNameBuffer (keyAuth, kPGPMaxUserIDSize,
									cas.szName, &u);

	// use prefs to determine validity threshold
	PGPclOpenClientPrefs (PGPGetContextMemoryMgr (context), &prefs);
	PGPGetPrefBoolean (prefs, kPGPPrefMarginalIsInvalid, &bMargIsInvalid);
	PGPclCloseClientPrefs (prefs, FALSE);
	if (bMargIsInvalid) 
		cas.validityThreshold = kPGPValidity_Complete;
	else
		cas.validityThreshold = kPGPValidity_Marginal;

	if (DialogBoxParam (g_hInst, 
		MAKEINTRESOURCE(IDD_CONFIRMAUTHENTICATION),
		hwndParent, sConfirmAuthDlgProc, (LPARAM)&cas))
		return kPGPError_NoErr;
	else
		return kPGPError_UserAbort;

}

//----------------------------------------------------|
//  find unknown signer key

PGPError PGPclExport
PGPclLookupUnknownSigner(
		PGPContextRef		context,
		PGPKeySetRef		KeySetMain,
		PGPtlsContextRef	tlsContext,
		HWND				hwnd,
		PGPEvent			*event,
		PGPKeyID			signingKeyID,
		PGPBoolean*			pbGotKeys)
{
	PGPKeySetRef newKeySet = NULL;
	PGPUInt32 numKeys = 0;
	PGPError lookupErr;
	int nOleErr;

	if (pbGotKeys == NULL)
		return kPGPError_BadParams;

	*pbGotKeys = FALSE;

	lookupErr = PGPclSearchServerForKeyIDs(context,
					NULL, hwnd, &signingKeyID, 1, 
					PGPCL_DEFAULTSERVER, KeySetMain, &newKeySet);
				
	if (IsPGPError(lookupErr))
		PGPclErrorBox(hwnd, lookupErr);
	else
	{
		PGPCountKeys(newKeySet, &numKeys);
		if (numKeys > 0)
		{
			nOleErr = OleInitialize(NULL);
			if ((nOleErr == S_OK) || (nOleErr == S_FALSE))
			{
				PGPclQueryAddKeys(context, tlsContext, hwnd, newKeySet, 
					NULL);
				OleUninitialize();
			}
			
			PGPAddJobOptions(event->job,
				PGPOKeySetRef(context, newKeySet),
				PGPOLastOption(context));

			PGPFreeKeySet(newKeySet);
			*pbGotKeys = TRUE;
		}
	}

	return kPGPError_NoErr;
}


//	____________________________________
//
//	Get HWND of PGPkeys application

static HWND 
sGetPGPkeysWindow (VOID)
{
    HWND			hWndMe		= NULL;
    HANDLE			hSem;

    // Create or open a named semaphore. 
    hSem = CreateSemaphore (NULL, 0, 1, SEMAPHORENAME);

    // return HWND if existing semaphore was opened.
    if (hSem != NULL) {
		if (GetLastError() == ERROR_ALREADY_EXISTS) {
		    hWndMe = FindWindow (WINCLASSNAME, WINDOWTITLE);
		}
        CloseHandle(hSem);
	}

	return hWndMe;
}

//----------------------------------------------------|
// Add keys in specified keyset to main keyset

BOOL 
CLAddKeysToMain (
		PGPContextRef	context, 
		HWND			hwnd,
		PGPKeySetRef	keysetToAdd,
		PGPKeySetRef	keysetMain) 
{
	PGPError			err					= 0;
	PGPKeySetRef		keyset				= kInvalidPGPKeySetRef;
	UINT				uReloadMessage;
	HWND				hwndPGPkeys;
	LPVOID				pBuffer;
	INT					slen;
	COPYDATASTRUCT		cds;
	HCURSOR				hCursorOld;


	if (PGPKeySetRefIsValid (keysetMain)) {
		err = PGPAddKeys (keysetToAdd, keysetMain);
		if (IsntPGPError (err)) {
			hCursorOld = SetCursor (LoadCursor (NULL, IDC_WAIT));
			err = PGPCommitKeyRingChanges (keysetMain);
			SetCursor (hCursorOld);

			if (IsntPGPError (err)) {
				uReloadMessage = RegisterWindowMessage (RELOADKEYRINGMSG);
				PostMessage (HWND_BROADCAST, uReloadMessage, 
						MAKEWPARAM (LOWORD (hwnd), TRUE), 
						GetCurrentProcessId ());
			}
		}
	}
	// no main keyset, we're supposed to try to add it to default keyring
	else {
		hwndPGPkeys = sGetPGPkeysWindow ();
		// PGPkeys is running ... send it the key block
		if (hwndPGPkeys) {
			err = PGPExportKeySet (keysetToAdd, 
							PGPOAllocatedOutputBuffer (context,
									&pBuffer, 0x40000000, &slen),
							PGPOExportPrivateKeys (context, TRUE),
							PGPOExportFormat (context, 
									kPGPExportFormat_Complete),
							PGPOLastOption (context));
			if (IsntPGPError (err)) {
				cds.dwData = PGPPK_IMPORTKEYBUFFER;
				cds.cbData = slen+1;
				cds.lpData = pBuffer;
				err = SendMessage (hwndPGPkeys, WM_COPYDATA, 
													0, (LPARAM)&cds);
				if (err) err = kPGPError_NoErr;
				else err = kPGPError_UnknownError;
				PGPFreeData (pBuffer);
			}
		}

		// PGPkeys is not running ... try to add keys to default keyring
		else {
			PGPsdkLoadDefaultPrefs (context);
			err = PGPOpenDefaultKeyRings (	context, 
											kPGPKeyRingOpenFlags_Mutable, 
											&keyset);
			if (IsntPGPError (err) && keyset) {
				err = PGPAddKeys (keysetToAdd, keyset);
				if (IsntPGPError (err)) {
					err = PGPCommitKeyRingChanges (keyset);
					if (IsntPGPError (err)) {
						uReloadMessage = 
							RegisterWindowMessage (RELOADKEYRINGMSG);
						PostMessage (HWND_BROADCAST, uReloadMessage, 
							MAKEWPARAM (LOWORD (hwnd), FALSE), 
							GetCurrentProcessId ());
					}
				}
				PGPFreeKeySet (keyset);
			}
		}
	}

	if (IsPGPError (err)) {
		PGPclMessageBox (hwnd, IDS_CAPTION, IDS_IMPORTKEYERROR,
							MB_OK|MB_ICONEXCLAMATION);
	}
	return (IsntPGPError (err));
}

