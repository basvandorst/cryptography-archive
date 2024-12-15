/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	PGPsplash.c - display splash screen
	

	$Id: CDsplash.c,v 1.14.2.3 1997/11/20 20:55:22 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

#include "pgpcdlgx.h"
#include <process.h>
#include <time.h>

#define TIMER_ID		4321
#define TIMER_PERIOD	200
#define MAINWINDOWDELAY	500

#define ADDHEIGHT		0
#define XINCREMENT		6
#define YINCREMENT		10

// external globals
extern HINSTANCE	g_hInst;
extern HWND			g_hWndSplash;

// local globals
static HWND		hWndSplashParent;
static HBITMAP	hBitmapSplash;
static HPALETTE hPaletteSplash;
static UINT		uTimeOut;
static UINT		uTimerTime;
static UINT		uXsize, uYsize;
static UINT		uXpos, uYpos;

//	______________________________
//
//	Splash screen dialog procedure

BOOL CALLBACK 
SplashDlgProc (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	HDC			hDC, hMemDC;
	PAINTSTRUCT ps;
	HBITMAP		hBitmapOld;
	HWND		hWndControl;
//#if PGP_FREEWARE
//	RECT		rc;
//	UINT		uX;
//	UINT		uY;
//#endif

	switch (uMsg) {

	case WM_INITDIALOG:
		g_hWndSplash = hDlg;
		SetWindowPos (hDlg, HWND_TOP, uXpos, uYpos, uXsize, uYsize, 0);

//#if PGP_FREEWARE
//		hWndControl = GetDlgItem (hDlg, IDOK);
//		if (!hWndSplashParent) EnableWindow (hWndControl, FALSE);
//		GetClientRect (hWndControl, &rc);
//		uY = uYsize + ADDHEIGHT - YINCREMENT - rc.bottom;
//		uX = uXsize - XINCREMENT - rc.right;
//		SetWindowPos (hWndControl, NULL, uX, uY, 
//				0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
//
//		hWndControl = GetDlgItem (hDlg, IDC_BUYNOW);
//		GetClientRect (hWndControl, &rc);
//		uX -= (XINCREMENT + rc.right);
//		SetWindowPos (hWndControl, NULL, uX, uY, 
//				0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
//#else
		hWndControl = GetDlgItem (hDlg, IDOK);
		EnableWindow (hWndControl, FALSE);
		ShowWindow (hWndControl, SW_HIDE);
		hWndControl = GetDlgItem (hDlg, IDC_BUYNOW);
		EnableWindow (hWndControl, FALSE);
		ShowWindow (hWndControl, SW_HIDE);
		uTimerTime = 0;
		SetTimer (hDlg, TIMER_ID, TIMER_PERIOD, NULL);
//#endif
		return TRUE;

	case WM_PAINT:
		hDC = BeginPaint (hDlg, &ps);
		hMemDC = CreateCompatibleDC (hDC);
		if (hPaletteSplash) {
			SelectPalette (hDC, hPaletteSplash, FALSE);
			RealizePalette (hDC);
			SelectPalette (hMemDC, hPaletteSplash, FALSE);
			RealizePalette (hMemDC);
		}
		hBitmapOld = SelectObject (hMemDC, hBitmapSplash);
//#if (PGP_FREEWARE == 0)
		PaintUserInfo (hMemDC, LINCENSEEX, LINCENSEEY);
//#endif
		BitBlt (hDC, 0, 0, uXsize, uYsize, hMemDC, 0, 0, SRCCOPY);
		SelectObject (hMemDC, hBitmapOld);
		DeleteDC (hMemDC);
		EndPaint (hDlg, &ps);
		return FALSE;

//#if PGP_FREEWARE
//	case WM_APP:
//		EnableWindow (GetDlgItem (hDlg, IDOK), TRUE);
//		SetFocus (GetDlgItem (hDlg, IDOK));
//		break;
//#else
	case WM_LBUTTONDOWN:
		if (hWndSplashParent) EndDialog (hDlg, 0);
		return TRUE;

	case WM_DESTROY:
		KillTimer (hDlg, TIMER_ID);
		return FALSE;
//#endif

	case WM_TIMER:
		if (uTimerTime < uTimeOut) 
			uTimerTime += TIMER_PERIOD;
		else 
			if (hWndSplashParent) EndDialog (hDlg, 0);
		return TRUE;

	case WM_CLOSE :
		EndDialog (hDlg, 0);
		break;

	case WM_COMMAND :
		switch(LOWORD (wParam)) {

//		case IDC_BUYNOW: 
//			if (hWndSplashParent) {
//				CHAR szURL[256];
//				LoadString (g_hInst, IDS_HARDSELLURL, szURL, sizeof(szURL));
//				PGPcomdlgWebBrowse (szURL);
//				EndDialog (hDlg, 0);
//			}
//			break;

		case IDOK: 
			if (hWndSplashParent) EndDialog (hDlg, 0);
			break;
		}
		return TRUE;

	default:
		return FALSE;

	}
}

//	______________________________
//
//	Splash screen thread procedure

DWORD WINAPI 
SplashThread (PVOID pvoid) 
{
	BITMAP	bm;
	HDC		hDC;
	INT		iNumBits, iSplashBitmap;

	hDC = GetDC (NULL);		// DC for desktop
	iNumBits = GetDeviceCaps (hDC, BITSPIXEL) * GetDeviceCaps (hDC, PLANES);
	ReleaseDC (NULL, hDC);

//#if PGP_FREEWARE
//	if (iNumBits <= 1) iSplashBitmap = IDB_NAGSPLASH1;
//	else if (iNumBits <= 4) iSplashBitmap = IDB_NAGSPLASH4;
//	else iSplashBitmap = IDB_NAGSPLASH8;
//#else
	if (iNumBits <= 1) iSplashBitmap = IDB_SPLASH1;
	else if (iNumBits <= 4) iSplashBitmap = IDB_SPLASH4;
	else iSplashBitmap = IDB_SPLASH8;
//#endif

	hBitmapSplash = LoadResourceBitmap (g_hInst, 
										MAKEINTRESOURCE (iSplashBitmap),
										&hPaletteSplash);
	GetObject (hBitmapSplash, sizeof(BITMAP), (LPSTR)&bm);

	uXsize = bm.bmWidth;
	uYsize = bm.bmHeight;

	uXpos = (GetSystemMetrics (SM_CXFULLSCREEN) - uXsize) / 2;
	uYpos = (GetSystemMetrics (SM_CYFULLSCREEN) - uYsize) / 2;

	DialogBox (g_hInst, MAKEINTRESOURCE (IDD_SPLASH), 
					hWndSplashParent, SplashDlgProc);

	g_hWndSplash = NULL;
	if (hWndSplashParent) {
		EnableWindow (hWndSplashParent, TRUE);
		SetForegroundWindow (hWndSplashParent);
	}

	DeleteObject (hBitmapSplash);
	DeleteObject (hPaletteSplash);

	g_hWndSplash = NULL;

	return kPGPError_NoErr;
}

//	__________________
//
//	Post Splash screen

PGPError PGPcdExport 
PGPcomdlgSplash (
		PGPContextRef	Context,
		HWND			hWndParent, 
		UINT			uMS) 
{
	PGPTime			LastSplash			= 0;
	PGPTime			Today;
	DWORD			dwID;
	PGPPrefRef		PrefRef;
	struct tm *		stm;
	PGPError		err;
	INT				iYear, iDay;
	time_t			tt;
	BOOL			bShowSplash;

	PGPcomdlgOpenClientPrefs (&PrefRef);

	Today = PGPGetTime ();
	tt = PGPGetStdTimeFromPGPTime (Today);
	stm = localtime (&tt);
	iYear = stm->tm_year;
	iDay = stm->tm_yday;

	if (PGPRefIsValid (PrefRef)) {
		err = PGPGetPrefNumber (PrefRef, kPGPPrefDateOfLastSplashScreen, 
						&LastSplash);
		if (IsPGPError (err)) LastSplash = 0;
	}
	tt = PGPGetStdTimeFromPGPTime (LastSplash);
	stm = localtime (&tt);

	bShowSplash = FALSE;
	if (uMS == 0) {
		bShowSplash = TRUE;
	}
	if ((stm != NULL) &&
		((stm->tm_year != iYear) || 
		 (stm->tm_yday != iDay))) bShowSplash = TRUE;

	if (bShowSplash)
	{
		if (g_hWndSplash) return kPGPError_Win32_AlreadyOpen;
		g_hWndSplash = (HWND)1;

		hWndSplashParent = hWndParent;
		uTimeOut = uMS;

		_beginthreadex (NULL, 0, SplashThread, NULL, 0, &dwID); 

		PGPSetPrefNumber  (PrefRef, kPGPPrefDateOfLastSplashScreen, 
							Today);
		Sleep (MAINWINDOWDELAY);
	}

	PGPcomdlgCloseClientPrefs (PrefRef, TRUE);

	return kPGPError_NoErr;
}

//	__________________
//
//	Post Splash screen

PGPError PGPcdExport 
PGPcomdlgSetSplashParent (HWND hWndParent) 
{
	if (hWndParent) {
		hWndSplashParent = hWndParent;
		if (g_hWndSplash) {
			SendMessage (g_hWndSplash, WM_APP, 0, 0);
			EnableWindow (hWndParent, FALSE);
		}
	}
	else {
//#if (PGP_FREEWARE == 0)
		SendMessage (g_hWndSplash, WM_CLOSE, 0, 0);
//#endif
	}

	return kPGPError_NoErr;
}
