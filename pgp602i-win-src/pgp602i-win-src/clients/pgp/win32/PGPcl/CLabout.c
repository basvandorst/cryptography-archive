/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	CLabout.c - display "help about" dialog
	

	$Id: CLabout.c,v 1.12 1998/08/11 14:43:04 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

// project header files
#include "pgpclx.h"

// system header files
#include <shellapi.h>
	
#define ADDWIDTH	2
#define ADDHEIGHT	2
#define XINCREMENT	6
#define YINCREMENT	10
#define EDGEWIDTH	6

#define VERSIONX	32
#define VERSIONY	26

typedef struct _DIALOGINFOSTRUCT {
	PGPContextRef	context;
	HWND			hWndParent;
	LPSTR			szVersionInfo;
	CHAR			szWebLink[128];
	LPSTR			szWebLinkText;
	HBITMAP			hBitmapMain;
	HBITMAP			hBitmapCredits;
	COLORREF		crVersionTextColor;
	HPALETTE		hPaletteMain;
	HPALETTE		hPaletteCredits;
	UINT			uXsize;
	UINT			uYsize;
	UINT			uXpos;
	UINT			uYpos;
	BOOL			bCredits;
} DIALOGINFOSTRUCT, *PDIALOGINFOSTRUCT;

// external globals
extern HINSTANCE	g_hInst;
extern CHAR			g_szHelpFile[MAX_PATH];

//	______________________________________
//
//	Thread routine to delay and then open URL.
//	If we don't wait for calling dialog box to
//	close, browser will be launched and then forced
//	to background as parent window of calling
//	dialog box is activated.

#define THREAD_DELAY_MS	150

DWORD WINAPI
ExecuteShellThread (LPVOID lpvoid) 
{
	LPSTR pszURL;
	pszURL = (LPSTR)lpvoid;

	// delay to allow calling dialog box to close
	// and activate parent
	Sleep (THREAD_DELAY_MS);

	// now that parent is active, open the URL
	if ((ULONG)ShellExecute (NULL, "open", pszURL, NULL, 
				  "C:\\", SW_SHOWNORMAL) <= 32) {
		PGPclMessageBox (NULL, IDS_CAPTION, 
			IDS_BROWSERERROR, MB_OK|MB_ICONHAND);
	}

	free (pszURL);

	return 0;
}

//	_______________________________________________
//
//	launch web browser with specified link

PGPError PGPclExport 
PGPclWebBrowse (LPSTR pszURL)
{
	DWORD	dwThreadID;
	LPSTR	pszURLbuffer;

	if (!pszURL) return kPGPError_BadParams;

	pszURLbuffer = malloc (lstrlen(pszURL) +1);
	lstrcpy (pszURLbuffer, pszURL);

	CreateThread (NULL, 0, ExecuteShellThread, 
							(LPVOID)pszURLbuffer, 0, &dwThreadID);

	return kPGPError_NoErr;
}

//	______________________________________
//
//	Dialog procedure for Help About dialog

BOOL CALLBACK 
HelpAboutDlgProc (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam,
		LPARAM	lParam) 
{
	PDIALOGINFOSTRUCT	pdis;
	CHAR				sz[256];

	switch(uMsg) {

	case WM_INITDIALOG:
		{
			HWND	hWndControl;
			RECT	rc;
			UINT	uX;
			UINT	uY;

			SetWindowLong (hDlg, GWL_USERDATA, lParam);
			pdis = (PDIALOGINFOSTRUCT)lParam;

			pdis->bCredits = FALSE;
			SetWindowPos (hDlg, HWND_TOP, pdis->uXpos, pdis->uYpos, 
						pdis->uXsize+ADDWIDTH, pdis->uYsize+ADDHEIGHT, 0);

			hWndControl = GetDlgItem (hDlg, IDOK);
			GetClientRect (hWndControl, &rc);
			uY = pdis->uYsize + ADDHEIGHT - YINCREMENT - rc.bottom;
			uX = pdis->uXsize - XINCREMENT - rc.right;
			SetWindowPos (hWndControl, NULL, uX, uY, 
					0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);

			hWndControl = GetDlgItem (hDlg, IDC_WWWLINK);
			GetClientRect (hWndControl, &rc);
			uY = pdis->uYsize + ADDHEIGHT - YINCREMENT - rc.bottom;
			uX = uX - XINCREMENT - rc.right;
			SetWindowPos (hWndControl, NULL, uX, uY, 
					0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);

			hWndControl = GetDlgItem (hDlg, IDC_CREDITS);
			GetClientRect (hWndControl, &rc);
			uY = pdis->uYsize + ADDHEIGHT - YINCREMENT - rc.bottom;
			uX = uX - XINCREMENT - rc.right;
			SetWindowPos (hWndControl, NULL, uX, uY, 
					0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);

			if (pdis->szWebLinkText) 
				SetDlgItemText (hDlg, IDC_WWWLINK, pdis->szWebLinkText);
		}
		return TRUE;

	case WM_PAINT:
		{
			HDC			hDC;
			HDC			hMemDC;
			PAINTSTRUCT	ps;
			HBITMAP		hBitmapOld;

			pdis = (PDIALOGINFOSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);

			hDC = BeginPaint (hDlg, &ps);
			hMemDC = CreateCompatibleDC (hDC);
			if (pdis->bCredits) {
				if (pdis->hPaletteCredits) {
					SelectPalette (hDC, pdis->hPaletteCredits, FALSE);
					RealizePalette (hDC);
				}
				hBitmapOld = SelectObject (hMemDC, pdis->hBitmapCredits);
			}
			else {
				if (pdis->hPaletteMain) {
					SelectPalette (hDC, pdis->hPaletteMain, FALSE);
					RealizePalette (hDC);
				}
				hBitmapOld = SelectObject (hMemDC, pdis->hBitmapMain);
				CLPaintUserInfo (PGPGetContextMemoryMgr (pdis->context),
									hDlg, hMemDC, pdis->szVersionInfo);
			}
			BitBlt (hDC, 0, 0, pdis->uXsize, pdis->uYsize, 
											hMemDC, 0, 0, SRCCOPY);
			SelectObject (hMemDC, hBitmapOld);
			DeleteDC (hMemDC);
			EndPaint (hDlg, &ps);
		}
		return FALSE;

	case WM_COMMAND:
		pdis = (PDIALOGINFOSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);

		switch (LOWORD (wParam)) {

		case IDC_WWWLINK :
			PGPclWebBrowse (pdis->szWebLink);
			EndDialog (hDlg, 0);
			break;

		case IDC_CREDITS :
			pdis->bCredits = !pdis->bCredits;
			if (pdis->bCredits) {
				LoadString (g_hInst, IDS_INFOTEXT, sz, sizeof(sz));
				SetDlgItemText (hDlg, IDC_CREDITS, sz);
			}
			else {
				LoadString (g_hInst, IDS_CREDITTEXT, sz, sizeof(sz));
				SetDlgItemText (hDlg, IDC_CREDITS, sz);
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

//	_______________________________________________
//
//	Post Help About dialog with button for web link
//	to specified URL.

PGPError PGPclExport 
PGPclHelpAbout (
		PGPContextRef	context,
		HWND			hWnd, 
		LPSTR			szVersion, 
		LPSTR			szLinkText, 
		LPSTR			szLink) 
{
	DIALOGINFOSTRUCT	dis;
	BITMAP				bm;
	HDC					hDC;
	INT					iAboutBitmap;
	INT					iCreditBitmap;
	INT					iNumBits;

	dis.context = context;
	dis.hWndParent = hWnd;
	dis.szVersionInfo = szVersion;
	if (szLink) 
		lstrcpy (dis.szWebLink, szLink);
	else 
		LoadString (g_hInst, IDS_WEBLINK, 
								dis.szWebLink, sizeof (dis.szWebLink));
	dis.szWebLinkText = szLinkText;

	hDC = GetDC (NULL);		// DC for desktop
	iNumBits = GetDeviceCaps (hDC, BITSPIXEL) * GetDeviceCaps (hDC, PLANES);
	ReleaseDC (NULL, hDC);

	if (iNumBits <= 1) {
		iAboutBitmap = IDB_SPLASH1;
		iCreditBitmap = IDB_CREDITS1;
		dis.crVersionTextColor = RGB (255,255,255);
	}
	else if (iNumBits <= 4) {
		iAboutBitmap = IDB_SPLASH4;
		iCreditBitmap = IDB_CREDITS4;
		dis.crVersionTextColor = RGB (0,0,0);
	}
	else {
		iAboutBitmap = IDB_SPLASH8;
		iCreditBitmap = IDB_CREDITS8;
		dis.crVersionTextColor = RGB (0,0,0);
	}

	dis.hBitmapMain = CLLoadResourceBitmap (g_hInst, 
										MAKEINTRESOURCE (iAboutBitmap),
										&dis.hPaletteMain);
	GetObject (dis.hBitmapMain, sizeof(BITMAP), (LPSTR)&bm);

	dis.hBitmapCredits = CLLoadResourceBitmap (g_hInst, 
										MAKEINTRESOURCE (iCreditBitmap),
										&dis.hPaletteCredits);

	dis.uXsize = bm.bmWidth;
	dis.uYsize = bm.bmHeight;

	dis.uXpos = (GetSystemMetrics (SM_CXFULLSCREEN) 
									- dis.uXsize - EDGEWIDTH) / 2;
	dis.uYpos = (GetSystemMetrics (SM_CYFULLSCREEN) 
									- dis.uYsize) / 2;

	DialogBoxParam (g_hInst, MAKEINTRESOURCE (IDD_HELPABOUT), 
							hWnd, HelpAboutDlgProc, (LPARAM)&dis);

	DeleteObject (dis.hBitmapMain);
	DeleteObject (dis.hBitmapCredits);
	DeleteObject (dis.hPaletteMain);
	DeleteObject (dis.hPaletteCredits);

	return kPGPError_NoErr;
}

#if PGP_DEMO
//	_____________________________________
//
//	Dialog procedure for nag dialog

BOOL CALLBACK 
NagDlgProc (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam,
		LPARAM	lParam) 
{
	PDIALOGINFOSTRUCT	pdis;

	switch(uMsg) {

	case WM_INITDIALOG:
		{
			HWND	hWndControl;
			RECT	rc;
			UINT	uX, uY;

			SetWindowLong (hDlg, GWL_USERDATA, lParam);
			pdis = (PDIALOGINFOSTRUCT)lParam;

			SetWindowPos (hDlg, HWND_TOP, pdis->uXpos, pdis->uYpos, 
							pdis->uXsize+6, pdis->uYsize+ADDHEIGHT, 0);

			hWndControl = GetDlgItem (hDlg, IDOK);
			GetClientRect (hWndControl, &rc);
			uY = pdis->uYsize * 54 / 100;
			uX = pdis->uXsize * 61 / 100;
			SetWindowPos (hWndControl, NULL, uX, uY, 
					0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);

			hWndControl = GetDlgItem (hDlg, IDC_WWWLINK);
			GetClientRect (hWndControl, &rc);
			uY = pdis->uYsize * 43 / 100;
			uX = pdis->uXsize * 61 / 100;
			SetWindowPos (hWndControl, NULL, uX, uY, 
					0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);

			if (pdis->szWebLinkText) 
				SetDlgItemText (hDlg, IDC_WWWLINK, pdis->szWebLinkText);
		}
		return TRUE;

	case WM_PAINT:
		{
			HDC			hDC, hMemDC;
			PAINTSTRUCT	ps;
			HBITMAP		hBitmapOld;

			pdis = (PDIALOGINFOSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);

			hDC = BeginPaint (hDlg, &ps);
			hMemDC = CreateCompatibleDC (hDC);
			if (pdis->hPaletteMain) {
				SelectPalette (hDC, pdis->hPaletteMain, FALSE);
				RealizePalette (hDC);
			}

			hBitmapOld = SelectObject (hMemDC, pdis->hBitmapMain);
			BitBlt (hDC, 0, 0, pdis->uXsize, pdis->uYsize, 
											hMemDC, 0, 0, SRCCOPY);
			SelectObject (hMemDC, hBitmapOld);
			DeleteDC (hMemDC);
			EndPaint (hDlg, &ps);
		}
		return FALSE;

	case WM_COMMAND:
		switch(LOWORD (wParam)) {

		case IDC_WWWLINK :
			pdis = (PDIALOGINFOSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
			PGPclWebBrowse (pdis->szWebLink);
			EndDialog (hDlg, 0);
			break;

		case IDOK: 
			EndDialog (hDlg, 0);
			break;
		}
		return TRUE;
	}
	return FALSE;
}

//	_________________________________________________________
//
//	Post nag dialog with button for web link to specified URL

PGPError PGPclExport 
PGPclNag(
		 HWND			hWnd, 
		 LPSTR			szLinkText, 
		 LPSTR			szLink) 
{
	DIALOGINFOSTRUCT	dis;
	BITMAP				bm;
	HDC					hDC;
	INT					iNagBitmap;
	INT					iNumBits;

	dis.context = NULL;
	dis.hWndParent = hWnd;
	if (szLink) lstrcpy (dis.szWebLink, szLink);
	else LoadString (g_hInst, IDS_WEBLINK, 
							dis.szWebLink, sizeof (dis.szWebLink));
	dis.szWebLinkText = szLinkText;

	hDC = GetDC (NULL);		// DC for desktop
	iNumBits = GetDeviceCaps (hDC, BITSPIXEL) * GetDeviceCaps (hDC, PLANES); 
	ReleaseDC (NULL, hDC);

	if (iNumBits <= 1) {
		iNagBitmap = IDB_NAG1;
	}
	else if (iNumBits <= 4) {
		iNagBitmap = IDB_NAG4;
	}
	else {
		iNagBitmap = IDB_NAG8;
	}

	dis.hBitmapMain = CLLoadResourceBitmap (g_hInst, 
									   MAKEINTRESOURCE (iNagBitmap),
									   &dis.hPaletteMain);
	GetObject (dis.hBitmapMain, sizeof(BITMAP), (LPSTR)&bm);

	dis.uXsize = bm.bmWidth;
	dis.uYsize = bm.bmHeight;

	dis.uXpos = (GetSystemMetrics (SM_CXFULLSCREEN) - 
										dis.uXsize - EDGEWIDTH) / 2;
	dis.uYpos = (GetSystemMetrics (SM_CYFULLSCREEN) -
										dis.uYsize) / 2;

	DialogBox (g_hInst, MAKEINTRESOURCE (IDD_NAG), 
			   hWnd, NagDlgProc);

	DeleteObject (dis.hBitmapMain);
	DeleteObject (dis.hPaletteMain);

	return kPGPError_NoErr;
}

#endif	// PGP_DEMO

