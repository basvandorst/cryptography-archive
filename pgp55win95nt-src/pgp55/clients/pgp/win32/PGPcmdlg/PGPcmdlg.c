/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	PGPcomdlg.c - PGP common dialogs DLL utility routines
	

	$Id: PGPcmdlg.c,v 1.25 1997/10/10 18:28:47 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"	

#include "pgpcdlgx.h"
#include "pgpphras.h"

#define YTEXTINC	16

#pragma data_seg ("PGPcomdlg_shared")
HWND g_hWndPreferences = NULL;
HWND g_hWndEntropy = NULL;
HWND g_hWndSplash = NULL;
HWND g_hWndCheckKeyrings = NULL;
#pragma data_seg ()

// DLL globals
HINSTANCE g_hInst;
CHAR g_szHelpFile[MAX_PATH];

//	__________________________
//
//	Memory allocation routines
//

VOID* 
pcAlloc (UINT uBytes) 
{
	return (HeapAlloc (GetProcessHeap (), 0, uBytes));
}


VOID 
pcFree (VOID* p) 
{
	if (p)
		HeapFree (GetProcessHeap (), 0, p);
}

//	___________________________________________________
//
//	Message box routine using string table resource IDs

LRESULT 
PGPcomdlgMessageBox (
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

//	____________________________________________________________
//
//	get PGPkeys path from registry and append Help file name 

VOID 
GetHelpFilePath (VOID) 
{
	CHAR sz[128];

	PGPcomdlgGetPGPPath (g_szHelpFile, sizeof(g_szHelpFile));

	LoadString (g_hInst, IDS_HELPFILENAME, sz, sizeof(sz));
	lstrcat (g_szHelpFile, sz);
}

//	______________________________________________
//
//	create logical palette from bitmap color table

HPALETTE 
CreateDIBPalette (
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
 
//	______________________________________
//
//	Load DIB bitmap and associated palette

HBITMAP
LoadResourceBitmap (
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
 

//	_______________________________________
//
//	Paint user info and registration number 

VOID 
PaintUserInfo (
	   HDC hDC, 
	   INT iXpos, 
	   INT iYpos) 
{
	HFONT		hFontOld;
	CHAR		sz[256];
	PGPError	err;
	PGPPrefRef	PrefRef;

	sz[0] = '\0';
#if PGP_BUSINESS_SECURITY
	if (PGPcomdlgOpenAdminPrefs (&PrefRef, FALSE) != kPGPError_NoErr)
		return;
	err = PGPGetPrefStringBuffer (PrefRef, kPGPPrefSiteLicenseNumber, 
										sizeof(sz), sz);
	if (IsPGPError (err)) sz[0] = '\0';
	PGPcomdlgCloseAdminPrefs (PrefRef, FALSE);
#endif

	if (PGPcomdlgOpenClientPrefs (&PrefRef) != kPGPError_NoErr)
		return;

	SetTextColor (hDC, RGB (255, 255, 255));
	SetBkMode (hDC, TRANSPARENT);
	hFontOld = SelectObject (hDC, GetStockObject (DEFAULT_GUI_FONT));

	// license number
	if (sz[0] == '\0') {
		err = PGPGetPrefStringBuffer (PrefRef, kPGPPrefLicenseNumber, 
										sizeof(sz), sz);
	}
	TextOut (hDC, iXpos, iYpos+(2*YTEXTINC), sz, lstrlen (sz));

	// user name
	sz[0] = '\0';
	err = PGPGetPrefStringBuffer (PrefRef, kPGPPrefOwnerName, 
										sizeof(sz), sz);
	TextOut (hDC, iXpos, iYpos, sz, lstrlen (sz));

	// company name
	sz[0] = '\0';
	err = PGPGetPrefStringBuffer (PrefRef, kPGPPrefCompanyName, 
										sizeof(sz), sz);
	TextOut (hDC, iXpos, iYpos+YTEXTINC, sz, lstrlen (sz));

	SelectObject (hDC, hFontOld);
	PGPcomdlgCloseClientPrefs (PrefRef, FALSE);
}

//	________________________
//
//	Broadcast reload message

VOID PGPcdExport 
PGPcomdlgNotifyKeyringChanges (LPARAM lParam) 
{
	UINT uMessageID;

	uMessageID = RegisterWindowMessage (RELOADKEYRINGMSG);
	PostMessage (HWND_BROADCAST, uMessageID, 0, lParam);
}

//	____________________________________________________________
//
//	get path of PGP installation from registry key 
//	note: includes trailing '\'

PGPError PGPcdExport 
PGPcomdlgGetPGPPath (LPSTR szPath, UINT uLen) 
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

//	____________________________________________________________
//
//	get keyring and randseed file paths from SDK 

PGPError PGPcdExport 
PGPcomdlgGetSDKFilePaths (
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

//	____________________________________________________________
//
//	set keyring and randseed file paths using SDK 

PGPError PGPcdExport 
PGPcomdlgSetSDKFilePaths (
		LPSTR	pszPubRingPath,
		LPSTR	pszPrivRingPath,
		LPSTR	pszRandSeedPath,
		BOOL	bCreateFiles)
{
	PGPError err;
	PGPFileSpecRef	fileref;
	PGPContextRef	context;
	PGPKeySetRef	keysetDummy;

	err = PGPNewContext ( kPGPsdkAPIVersion, &context);
	PGPsdkLoadDefaultPrefs (context);

	if (IsntPGPError (err)) {
	
		if (pszPubRingPath) {
			err = PGPNewFileSpecFromFullPath (context, 
									pszPubRingPath, &fileref);
			if (IsntPGPError (err) && fileref) {
				err = PGPsdkPrefSetFileSpec (context, 
								kPGPsdkPref_PublicKeyring, fileref);
				PGPFreeFileSpec (fileref);
			}
		}

		if (pszPrivRingPath && IsntPGPError (err)) {
			err = PGPNewFileSpecFromFullPath (context, 
									pszPrivRingPath, &fileref);
			if (IsntPGPError (err) && fileref) {
				err = PGPsdkPrefSetFileSpec (context, 
								kPGPsdkPref_PrivateKeyring, fileref);
				PGPFreeFileSpec (fileref);
			}
		}

		if (pszRandSeedPath && IsntPGPError (err)) {
			err = PGPNewFileSpecFromFullPath (context, 
									pszRandSeedPath, &fileref);
			if (IsntPGPError (err) && fileref) {
				err = PGPsdkPrefSetFileSpec (context, 
								kPGPsdkPref_RandomSeedFile, fileref);
				PGPFreeFileSpec (fileref);
			}
		}

		PGPsdkSavePrefs (context);

		if (IsntPGPError (err) && bCreateFiles) {
			PGPOpenDefaultKeyRings (context, 
					kPGPKeyRingOpenFlags_Create, &keysetDummy);
			PGPFreeKeySet (keysetDummy);
		}

		PGPFreeContext (context);
	}

	return err;
}

//	____________________________________________________________
//
//	set keyring and randseed file paths using SDK 

PGPError PGPcdExport 
PGPcomdlgCloseLibrary (VOID) 
{
	while (PGPPurgeSignCachedPhrase ()) Sleep (250);

	return kPGPError_NoErr;
}

//	____________________
//
//	DLL entry/exit point

BOOL WINAPI 
DllMain (
		 HINSTANCE hInstDll, 
		 DWORD dwReason, 
		 LPVOID lpvReserved) 
{

	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		g_hInst = hInstDll;
		GetHelpFilePath ();
		break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
		break;
    case DLL_PROCESS_DETACH:
		break;
    }
    return TRUE;
}


