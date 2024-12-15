/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */
//:PGPads.c - display Advertisement dialog(s)
//
//	$Id: PGPads.c,v 1.3 1997/05/09 16:25:43 pbj Exp $
//

#include "pgpcomdlgx.h"
	
#define XINCREMENT	6
#define YINCREMENT	10
#define EDGEWIDTH	6

// external globals
extern HINSTANCE g_hInst;
extern CHAR g_szbuf[G_BUFLEN];

// local globals
static HWND hWndParent;
static CHAR szWebLink[128];
static CHAR szWebLinkText[64];
static HBITMAP hBitmapAd;
static HPALETTE hPaletteAd;
static UINT uXsize, uYsize;
static UINT uXpos, uYpos;


//----------------------------------------------------|
//	Dialog procedure for Advertisement dialog

BOOL CALLBACK AdvertisementDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam,
							LPARAM lParam) {
	HDC hDC, hMemDC;
	PAINTSTRUCT ps;
	HBITMAP hBitmapOld;
	HWND hWndControl;
	RECT rc;
	UINT uX, uY;

	switch(uMsg) {

	case WM_INITDIALOG:
		SetWindowPos (hDlg, HWND_TOP, uXpos, uYpos, uXsize+6, uYsize, 0);

		hWndControl = GetDlgItem (hDlg, IDCANCEL);
		GetClientRect (hWndControl, &rc);
		uY = uYsize - YINCREMENT - rc.bottom;
		uX = uXsize - XINCREMENT - rc.right;
		SetWindowPos (hWndControl, NULL, uX, uY,
				0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);

		hWndControl = GetDlgItem (hDlg, IDC_WWWLINK);
		GetClientRect (hWndControl, &rc);
		uY = uYsize - YINCREMENT - rc.bottom;
		uX = uX - XINCREMENT - rc.right;
		SetWindowPos (hWndControl, NULL, uX, uY,
				0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);

		SetDlgItemText (hDlg, IDC_WWWLINK, szWebLinkText);
		return TRUE;

	case WM_PAINT:
		hDC = BeginPaint (hDlg, &ps);
		hMemDC = CreateCompatibleDC (hDC);
		if (hPaletteAd) {
			SelectPalette (hDC, hPaletteAd, FALSE);
			RealizePalette (hDC);
			SelectPalette (hMemDC, hPaletteAd, FALSE);
			RealizePalette (hMemDC);
		}
		hBitmapOld = SelectObject (hMemDC, hBitmapAd);
		BitBlt (hDC, 0, 0, uXsize, uYsize, hMemDC, 0, 0, SRCCOPY);
		SelectObject (hMemDC, hBitmapOld);
		DeleteDC (hMemDC);
		EndPaint (hDlg, &ps);
		return FALSE;

	case WM_COMMAND:
		switch(LOWORD (wParam)) {

		case IDC_WWWLINK :
			ShellExecute (hWndParent, "open", szWebLink, NULL, "C:\\",
							SW_SHOWNORMAL);
			EndDialog (hDlg, 0);
			break;

		case IDCANCEL:
			EndDialog (hDlg, 0);
			break;
		}
		return TRUE;
	}
	return FALSE;
}


//----------------------------------------------------|
//	Post Advertisement dialog with button for web link
//	to URL.

UINT PGPExport PGPcomdlgAdvertisement (HWND hWnd, UINT uIndex) {

	BITMAP bm;
	HDC hDC;
	INT iAdBitmap;
	INT iNumBits;

	hWndParent = hWnd;
	LoadString (g_hInst, IDS_FVURL, szWebLink, sizeof (szWebLink));
	LoadString (g_hInst, IDS_FVBUTTONTEXT, szWebLinkText,
					sizeof (szWebLinkText));

	hDC = GetDC (NULL);		// DC for desktop
	iNumBits = GetDeviceCaps (hDC, BITSPIXEL) * GetDeviceCaps (hDC, PLANES);
	ReleaseDC (NULL, hDC);

	iAdBitmap = 0;

	if (iNumBits <= 1) {
		switch (uIndex) {
		case 0 :
			iAdBitmap = IDB_SPLASH1;
			break;
		}
	 }
	else if (iNumBits <= 4) {
		switch (uIndex) {
		case 0 :
			iAdBitmap = IDB_SPLASH4;
			break;
		}
	}
	else {
		switch (uIndex) {
		case 0 :
			iAdBitmap = IDB_SPLASH8;
			break;
		}
	}

	if (iAdBitmap == 0) return PGPCOMDLG_OK;

	hBitmapAd = LoadResourceBitmap (g_hInst, MAKEINTRESOURCE (iAdBitmap),
										&hPaletteAd);
	GetObject (hBitmapAd, sizeof(BITMAP), (LPSTR)&bm);

	uXsize = bm.bmWidth;
	uYsize = bm.bmHeight;

	uXpos = (GetSystemMetrics (SM_CXFULLSCREEN) - uXsize - EDGEWIDTH) / 2;
	uYpos = (GetSystemMetrics (SM_CYFULLSCREEN) - uYsize) / 2;

	DialogBox (g_hInst, MAKEINTRESOURCE (IDD_ADVERTISEMENT), hWnd,
					AdvertisementDlgProc);

	DeleteObject (hBitmapAd);
	DeleteObject (hPaletteAd);

	return PGPCOMDLG_OK;
}
