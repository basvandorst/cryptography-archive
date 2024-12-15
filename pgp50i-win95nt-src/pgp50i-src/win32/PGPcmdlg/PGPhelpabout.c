/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */
//:PGPhelpabout.c - display "help about" dialog
//
//	$Id: PGPhelpabout.c,v 1.5.2.1 1997/06/09 15:54:51 dgal Exp $
//

#include "pgpcomdlgx.h"
#include "..\include\PGPversion.h"
	
#define ADDHEIGHT	0
#define XINCREMENT	6
#define YINCREMENT	10
#define EDGEWIDTH	6

#define VERSIONX	32
#define VERSIONY	26

// external globals
extern HINSTANCE g_hInst;
extern CHAR g_szHelpFile[MAX_PATH];
extern CHAR g_szbuf[G_BUFLEN];

// local globals
static HWND hWndParent;
static LPSTR szVersionInfo;
static CHAR szWebLink[64];
static LPSTR szWebLinkText;
static INT iNumBits;
static HBITMAP hBitmapAbout;
static HBITMAP hBitmapCredits;
static COLORREF crVersionTextColor;
static HPALETTE hPaletteAbout;
static HPALETTE hPaletteCredits;
static UINT uXsize, uYsize;
static UINT uXpos, uYpos;

static HWND hWndParentNag;
static CHAR szOrderLink[64];
static LPSTR szOrderLinkText;
static INT iNumBitsNag;
static HBITMAP hBitmapNag;
static HPALETTE hPaletteNag;
static UINT uXsizeNag, uYsizeNag;
static UINT uXposNag, uYposNag;

//----------------------------------------------------|
//	Dialog procedure for Help About dialog

BOOL CALLBACK HelpAboutDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam,
							LPARAM lParam) {
	static BOOL bCredits;
	HDC hDC, hMemDC;
	PAINTSTRUCT ps;
	HFONT hFontOld;
	HBITMAP hBitmapOld;
	HWND hWndControl;
	RECT rc;
	UINT uX, uY;

	switch(uMsg) {

	case WM_INITDIALOG:
		bCredits = FALSE;
		SetWindowPos (hDlg, HWND_TOP, uXpos, uYpos, uXsize+6,
							uYsize+ADDHEIGHT, 0);

		hWndControl = GetDlgItem (hDlg, IDOK);
		GetClientRect (hWndControl, &rc);
		uY = uYsize + ADDHEIGHT - YINCREMENT - rc.bottom;
		uX = uXsize - XINCREMENT - rc.right;
		SetWindowPos (hWndControl, NULL, uX, uY,
				0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);

		hWndControl = GetDlgItem (hDlg, IDC_WWWLINK);
		GetClientRect (hWndControl, &rc);
		uY = uYsize + ADDHEIGHT - YINCREMENT - rc.bottom;
		uX = uX - XINCREMENT - rc.right;
		SetWindowPos (hWndControl, NULL, uX, uY,
				0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);

		hWndControl = GetDlgItem (hDlg, IDC_CREDITS);
		GetClientRect (hWndControl, &rc);
		uY = uYsize + ADDHEIGHT - YINCREMENT - rc.bottom;
		uX = uX - XINCREMENT - rc.right;
		SetWindowPos (hWndControl, NULL, uX, uY,
				0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);

		hWndControl = GetDlgItem (hDlg, IDC_VERSIONINFO);
		GetClientRect (hWndControl, &rc);
		uY = uYsize + ADDHEIGHT - YINCREMENT - rc.bottom;
		uX = XINCREMENT;
		SetWindowPos (hWndControl, NULL, uX, uY,
				0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);

		if (szWebLinkText)
			SetDlgItemText (hDlg, IDC_WWWLINK, szWebLinkText);
		return TRUE;

	case WM_PAINT:
		hDC = BeginPaint (hDlg, &ps);
		hMemDC = CreateCompatibleDC (hDC);
		if (bCredits) {
			if (hPaletteCredits) {
				SelectPalette (hDC, hPaletteCredits, FALSE);
				RealizePalette (hDC);
				SelectPalette (hMemDC, hPaletteCredits, FALSE);
				RealizePalette (hMemDC);
			}
			hBitmapOld = SelectObject (hMemDC, hBitmapCredits);
		}
		else {
			if (hPaletteAbout) {
				SelectPalette (hDC, hPaletteAbout, FALSE);
				RealizePalette (hDC);
				SelectPalette (hMemDC, hPaletteAbout, FALSE);
				RealizePalette (hMemDC);
			}
			hBitmapOld = SelectObject (hMemDC, hBitmapAbout);
			PaintUserInfo (hMemDC, LINCENSEEX, LINCENSEEY);
			SetTextColor (hMemDC, crVersionTextColor);
			SetBkMode (hMemDC, TRANSPARENT);
			hFontOld = SelectObject (hMemDC,
									 GetStockObject (DEFAULT_GUI_FONT));
			if (szVersionInfo)
				TextOut (hMemDC, VERSIONX, uYsize-VERSIONY,
					 szVersionInfo, lstrlen (szVersionInfo));
			else
				TextOut (hMemDC, VERSIONX, uYsize-VERSIONY,
					 PGPVERSIONSTRING, lstrlen (PGPVERSIONSTRING));
			SelectObject (hMemDC, hFontOld);
		}
		BitBlt (hDC, 0, 0, uXsize, uYsize, hMemDC, 0, 0, SRCCOPY);
		SelectObject (hMemDC, hBitmapOld);
		DeleteDC (hMemDC);
		EndPaint (hDlg, &ps);
		return FALSE;

	case WM_COMMAND:
		switch(LOWORD (wParam)) {

		case IDC_WWWLINK :
			if ((ULONG)ShellExecute (hWndParent, "open", szWebLink, NULL,
						   "C:\\", SW_SHOWNORMAL) <= 32) {
				PGPcomdlgMessageBox (hDlg, IDS_CAPTION,
					IDS_BROWSERERROR, MB_OK|MB_ICONHAND);
			}
			else EndDialog (hDlg, 0);
			break;

		case IDC_CREDITS :
			bCredits = !bCredits;
			if (bCredits) {
				LoadString (g_hInst, IDS_INFOTEXT, g_szbuf, G_BUFLEN);
				SetDlgItemText (hDlg, IDC_CREDITS, g_szbuf);
			}
			else {
				LoadString (g_hInst, IDS_CREDITTEXT, g_szbuf, G_BUFLEN);
				SetDlgItemText (hDlg, IDC_CREDITS, g_szbuf);
			}
			InvalidateRect (hDlg, NULL, FALSE);
			break;

		case IDOK:
			EndDialog (hDlg, 0);
			break;
		}
		return TRUE;
	}
	return FALSE;
}


//----------------------------------------------------|
//	Post Help About dialog with button for web link
//	to specified URL.

UINT PGPExport PGPcomdlgHelpAbout (HWND hWnd, LPSTR szVersion,
								   LPSTR szLinkText, LPSTR szLink) {

	BITMAP bm;
	HDC hDC;
	INT iAboutBitmap, iCreditBitmap;

	hWndParent = hWnd;
	szVersionInfo = szVersion;
	if (szLink) lstrcpy (szWebLink, szLink);
	else LoadString (g_hInst, IDS_WEBLINK, szWebLink, sizeof (szWebLink));
	szWebLinkText = szLinkText;

	hDC = GetDC (NULL);		// DC for desktop
	iNumBits = GetDeviceCaps (hDC, BITSPIXEL) * GetDeviceCaps (hDC, PLANES);
	ReleaseDC (NULL, hDC);

	if (iNumBits <= 1) {
		iAboutBitmap = IDB_SPLASH1;
		iCreditBitmap = IDB_CREDITS1;
		crVersionTextColor = RGB (255,255,255);
	}
	else if (iNumBits <= 4) {
		iAboutBitmap = IDB_SPLASH4;
		iCreditBitmap = IDB_CREDITS4;
		crVersionTextColor = RGB (0,0,0);
	}
	else if (iNumBits <= 8) {
		iAboutBitmap = IDB_SPLASH8;
		iCreditBitmap = IDB_CREDITS8;
		crVersionTextColor = RGB (0,0,0);
	}
	else {
		iAboutBitmap = IDB_SPLASH24;
		iCreditBitmap = IDB_CREDITS8;
		crVersionTextColor = RGB (0,0,0);
	}

	hBitmapAbout = LoadResourceBitmap (g_hInst,
									   MAKEINTRESOURCE (iAboutBitmap),
									   &hPaletteAbout);
	GetObject (hBitmapAbout, sizeof(BITMAP), (LPSTR)&bm);

	hBitmapCredits = LoadResourceBitmap (g_hInst,
										 MAKEINTRESOURCE (iCreditBitmap),
										 &hPaletteCredits);

	uXsize = bm.bmWidth;
	uYsize = bm.bmHeight;

	uXpos = (GetSystemMetrics (SM_CXFULLSCREEN) - uXsize - EDGEWIDTH) / 2;
	uYpos = (GetSystemMetrics (SM_CYFULLSCREEN) - uYsize) / 2;

	DialogBox (g_hInst, MAKEINTRESOURCE (IDD_HELPABOUT),
			   hWnd, HelpAboutDlgProc);

	DeleteObject (hBitmapAbout);
	DeleteObject (hBitmapCredits);
	DeleteObject (hPaletteAbout);
	DeleteObject (hPaletteCredits);

	return PGPCOMDLG_OK;
}

#ifdef EVAL_TIMEOUT

//----------------------------------------------------|
//	Dialog procedure for nag dialog

BOOL CALLBACK NagDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam,
							LPARAM lParam) {
	HDC hDC, hMemDC;
	PAINTSTRUCT ps;
	HBITMAP hBitmapOld;
	HWND hWndControl;
	RECT rc;
	UINT uX, uY;

	switch(uMsg) {

	case WM_INITDIALOG:
		SetWindowPos (hDlg, HWND_TOP, uXpos, uYpos, uXsize+6,
							uYsize+ADDHEIGHT, 0);

		hWndControl = GetDlgItem (hDlg, IDOK);
		GetClientRect (hWndControl, &rc);
		uY = uYsize * 54 / 100;
		uX = uXsize * 61 / 100;
		SetWindowPos (hWndControl, NULL, uX, uY,
				0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);

		hWndControl = GetDlgItem (hDlg, IDC_WWWLINK);
		GetClientRect (hWndControl, &rc);
		uY = uYsize * 43 / 100;
		uX = uXsize * 61 / 100;
		SetWindowPos (hWndControl, NULL, uX, uY,
				0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);

		if (szWebLinkText)
			SetDlgItemText (hDlg, IDC_WWWLINK, szWebLinkText);
		return TRUE;

	case WM_PAINT:
		hDC = BeginPaint (hDlg, &ps);
		hMemDC = CreateCompatibleDC (hDC);
		if (hPaletteNag) {
			SelectPalette (hDC, hPaletteNag, FALSE);
			RealizePalette (hDC);
		}

		hBitmapOld = SelectObject (hMemDC, hBitmapNag);
		BitBlt (hDC, 0, 0, uXsize, uYsize, hMemDC, 0, 0, SRCCOPY);
		SelectObject (hMemDC, hBitmapOld);
		DeleteDC (hMemDC);
		EndPaint (hDlg, &ps);
		return FALSE;

	case WM_COMMAND:
		switch(LOWORD (wParam)) {

		case IDC_WWWLINK :
			if ((ULONG)ShellExecute (hWndParent, "open", szWebLink, NULL,
						  "C:\\", SW_SHOWNORMAL) <= 32) {
				PGPcomdlgMessageBox (hDlg, IDS_CAPTION,
					IDS_BROWSERERROR, MB_OK|MB_ICONHAND);
			}
			else EndDialog (hDlg, 0);
			break;

		case IDOK:
			EndDialog (hDlg, 0);
			break;
		}
		return TRUE;
	}
	return FALSE;
}


//----------------------------------------------------|
//	Post nag dialog with button for web link
//	to specified URL.

UINT PGPExport PGPcomdlgNag(HWND hWnd, LPSTR szLinkText, LPSTR szLink)
{
	BITMAP bm;
	HDC hDC;
	INT iNagBitmap;

	hWndParent = hWnd;
	if (szLink) lstrcpy (szWebLink, szLink);
	else LoadString (g_hInst, IDS_WEBLINK, szWebLink, sizeof (szWebLink));
	szWebLinkText = szLinkText;

	hDC = GetDC (NULL);		// DC for desktop
	iNumBits = GetDeviceCaps (hDC, BITSPIXEL) * GetDeviceCaps (hDC, PLANES);
	ReleaseDC (NULL, hDC);

	if (iNumBits <= 1) {
		iNagBitmap = IDB_NAG1;
	}
	else if (iNumBits <= 4) {
		iNagBitmap = IDB_NAG4;
	}
	else if (iNumBits <= 8) {
		iNagBitmap = IDB_NAG8;
	}
	else {
		iNagBitmap = IDB_NAG24;
	}

	hBitmapNag = LoadResourceBitmap (g_hInst,
									   MAKEINTRESOURCE (iNagBitmap),
									   &hPaletteNag);
	GetObject (hBitmapNag, sizeof(BITMAP), (LPSTR)&bm);

	uXsize = bm.bmWidth;
	uYsize = bm.bmHeight;

	uXpos = (GetSystemMetrics (SM_CXFULLSCREEN) - uXsize - EDGEWIDTH) / 2;
	uYpos = (GetSystemMetrics (SM_CYFULLSCREEN) - uYsize) / 2;

	DialogBox (g_hInst, MAKEINTRESOURCE (IDD_NAG),
			   hWnd, NagDlgProc);

	DeleteObject (hBitmapNag);
	DeleteObject (hPaletteNag);

	return PGPCOMDLG_OK;
}

#endif	// EVAL_TIMEOUT
