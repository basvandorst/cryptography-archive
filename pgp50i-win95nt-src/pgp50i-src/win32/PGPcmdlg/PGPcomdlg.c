/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */
//:PGPcomdlg.c - handle PGP common dialogs
//
//	$Id: PGPcomdlg.c,v 1.2 1997/05/09 16:25:44 pbj Exp $
//

#include "pgpcomdlgx.h"
#include "pgpcomdlgver.h"

#define YTEXTINC	16

#pragma data_seg ("PGPcomdlg_shared")
HWND g_hWndPreferences = NULL;
HWND g_hWndEntropy = NULL;
HWND g_hWndSplash = NULL;
HWND g_hWndCheckKeyrings = NULL;
#pragma data_seg ()

// globals
HINSTANCE g_hInst;
CHAR g_szHelpFile[MAX_PATH];
CHAR g_szbuf[G_BUFLEN];


//----------------------------------------------------|
// Message box routine
//
//	iCaption and iMessage are string table resource IDs.

LRESULT PGPcomdlgMessageBox (HWND hWnd, INT iCaption, INT iMessage,
					  ULONG ulFlags) {
	CHAR szCaption [128];
	CHAR szMessage [256];

	LoadString (g_hInst, iCaption, szCaption, sizeof(szCaption));
	LoadString (g_hInst, iMessage, szMessage, sizeof(szMessage));

	ulFlags |= MB_SETFOREGROUND;
	return (MessageBox (hWnd, szMessage, szCaption, ulFlags));
}


//----------------------------------------------------|
// get PGPkeys path from registry and substitute Help file name

VOID GetHelpFilePath (void) {
	HKEY hKey;
	LONG lResult;
	CHAR sz[MAX_PATH];
	CHAR* p;
	DWORD dwValueType, dwSize;

	lstrcpy (g_szHelpFile, "");

	LoadString (g_hInst, IDS_PGPKEYSREGID, g_szbuf, G_BUFLEN);
	lResult = RegOpenKeyEx (HKEY_CURRENT_USER, g_szbuf, 0, KEY_READ, &hKey);

	if (lResult == ERROR_SUCCESS) {
		dwSize = sizeof (g_szHelpFile);
		lResult = RegQueryValueEx (hKey, "ExePath", 0, &dwValueType,
									(LPBYTE)g_szHelpFile, &dwSize);
		RegCloseKey (hKey);
		p = strrchr (g_szHelpFile, '\\');
		if (p) {
			p++;
			*p = '\0';
		}
		else lstrcpy (g_szHelpFile, "");
	}

	LoadString (g_hInst, IDS_HELPFILENAME, sz, sizeof(sz));
	lstrcat (g_szHelpFile, sz);
}


//-------------------------------------------------------------------|
// create logical palette from bitmap color table

HPALETTE CreateDIBPalette (LPBITMAPINFO lpbmi, LPINT lpiNumColors) {
	LPBITMAPINFOHEADER lpbi;
	LPLOGPALETTE lpPal;
	HANDLE hLogPal;
	HPALETTE hPal = NULL;
	INT i;

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


//-------------------------------------------------------------------|
// Load DIB bitmap and associated palette

HBITMAP LoadResourceBitmap (HINSTANCE hInstance, LPSTR lpString,
                           HPALETTE FAR* lphPalette) {
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
// Paint user info and registration number

VOID PaintUserInfo (HDC hDC, INT iXpos, INT iYpos) {
	HFONT hFontOld;
	HKEY hKey;
	LONG lResult;
	DWORD dwSize, dwValueType;

	SetTextColor (hDC, RGB (255, 255, 255));
	SetBkMode (hDC, TRANSPARENT);
	hFontOld = SelectObject (hDC, GetStockObject (DEFAULT_GUI_FONT));

	lstrcpy (g_szbuf, "");
	LoadString (g_hInst, IDS_PGPMAILREGID, g_szbuf, G_BUFLEN);
	lResult = RegOpenKeyEx (HKEY_CURRENT_USER, g_szbuf, 0, KEY_READ, &hKey);

	if (lResult == ERROR_SUCCESS) {
		dwSize = G_BUFLEN;
		lResult = RegQueryValueEx (hKey, "User", 0, &dwValueType,
									(LPBYTE)g_szbuf, &dwSize);
		if (lResult != ERROR_SUCCESS) lstrcpy (g_szbuf, "");
		TextOut (hDC, iXpos, iYpos, g_szbuf, lstrlen (g_szbuf));

		dwSize = G_BUFLEN;
		lResult = RegQueryValueEx (hKey, "Company Name", 0, &dwValueType,
										(LPBYTE)g_szbuf, &dwSize);
		if (lResult != ERROR_SUCCESS) lstrcpy (g_szbuf, "");
		TextOut (hDC, iXpos, iYpos+YTEXTINC, g_szbuf, lstrlen (g_szbuf));

		dwSize = G_BUFLEN;
		lResult = RegQueryValueEx (hKey, "Serial Number", 0, &dwValueType,
										(LPBYTE)g_szbuf, &dwSize);
		if (lResult != ERROR_SUCCESS) lstrcpy (g_szbuf, "");
		TextOut (hDC, iXpos, iYpos+(2*YTEXTINC), g_szbuf, lstrlen (g_szbuf));

		RegCloseKey (hKey);
	}

	SelectObject (hDC, hFontOld);
}


//----------------------------------------------------|
//	Broadcast reload message

VOID PGPExport PGPcomdlgNotifyKeyringChanges (LPARAM lParam) {
	UINT uMessageID;

	uMessageID = RegisterWindowMessage (RELOADKEYRINGMSG);
	PostMessage (HWND_BROADCAST, uMessageID, 0, lParam);
}


//----------------------------------------------------|
// DLL entry/exit point

BOOL WINAPI DllMain (HINSTANCE hInstDll, DWORD dwReason, LPVOID lpvReserved) {

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
