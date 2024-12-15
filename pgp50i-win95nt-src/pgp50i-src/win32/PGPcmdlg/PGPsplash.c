/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
//:PGPhelpabout.c - display "help about" dialog
//
//

#include <process.h>
#include "pgpcomdlgx.h"

#define TIMER_ID		4321
#define TIMER_PERIOD	200
#define MAINWINDOWDELAY	500

// external globals
extern HINSTANCE g_hInst;
extern HWND g_hWndSplash;

// local globals
static HWND hWndSplashParent;
static HBITMAP hBitmapSplash;
static HPALETTE hPaletteSplash;
static UINT uTimeOut;
static UINT uTimerTime;
static UINT uXsize, uYsize;
static UINT uXpos, uYpos;


//-------------------------------------------------------------------|
// Splash screen dialog procedure

BOOL CALLBACK SplashDlgProc (HWND hDlg, UINT uMsg,
							 WPARAM wParam, LPARAM lParam) {

	HDC hDC, hMemDC;
	PAINTSTRUCT ps;
	HBITMAP hBitmapOld;

	switch (uMsg) {

	case WM_INITDIALOG:
		g_hWndSplash = hDlg;
		uTimerTime = 0;
		SetTimer (hDlg, TIMER_ID, TIMER_PERIOD, NULL);
		SetWindowPos (hDlg, HWND_TOP, uXpos, uYpos, uXsize, uYsize, 0);
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
		PaintUserInfo (hMemDC, LINCENSEEX, LINCENSEEY);
		BitBlt (hDC, 0, 0, uXsize, uYsize, hMemDC, 0, 0, SRCCOPY);
		SelectObject (hMemDC, hBitmapOld);
		DeleteDC (hMemDC);
		EndPaint (hDlg, &ps);
		return FALSE;

	case WM_TIMER:
		if (uTimerTime < uTimeOut)
			uTimerTime += TIMER_PERIOD;
		else
			if (hWndSplashParent) {
				SetForegroundWindow (hWndSplashParent);
				EndDialog (hDlg, 0);
			}
		return TRUE;

	case WM_LBUTTONDOWN:
		if (hWndSplashParent) {
			SetForegroundWindow (hWndSplashParent);
			EndDialog (hDlg, 0);
		}
		return TRUE;

	case WM_DESTROY:
		KillTimer (hDlg, TIMER_ID);
		return FALSE;

	default:
		return FALSE;

	}
}


//----------------------------------------------------|
//	Splash screen thread procedure

DWORD WINAPI SplashThread (PVOID pvoid) {
	BITMAP bm;
	HDC hDC;
	INT iNumBits, iSplashBitmap;

	hDC = GetDC (NULL);		// DC for desktop
	iNumBits = GetDeviceCaps (hDC, BITSPIXEL) * GetDeviceCaps (hDC, PLANES);
	ReleaseDC (NULL, hDC);

	if (iNumBits <= 1) iSplashBitmap = IDB_SPLASH1;
	else if (iNumBits <= 4) iSplashBitmap = IDB_SPLASH4;
	else if (iNumBits <= 8) iSplashBitmap = IDB_SPLASH8;
	else iSplashBitmap = IDB_SPLASH24;

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

	DeleteObject (hBitmapSplash);
	DeleteObject (hPaletteSplash);

	g_hWndSplash = NULL;

	return PGPCOMDLG_OK;
}


//----------------------------------------------------|
//	Post Splash screen

UINT PGPExport PGPcomdlgSplash (HWND hWndParent, UINT uMS) {
	DWORD dwID;
	ULONG uLastSplashTime;
	ULONG uTodayTime;
	struct tm* ptm;
	INT iTodayYear, iTodayDay;

	uTodayTime = pgpGetTime ();
	ptm = localtime (&uTodayTime);
	iTodayYear = ptm->tm_year;
	iTodayDay = ptm->tm_yday;

	pgpGetPrefNumber (kPGPPrefDateOfLastSplashScreen, &uLastSplashTime);
	ptm = localtime (&uLastSplashTime);

	if ((iTodayYear != ptm->tm_year) || (iTodayDay != ptm->tm_yday)) {
		if (g_hWndSplash) return PGPCOMDLG_ALREADYOPEN;
		g_hWndSplash = (HWND)1;

		hWndSplashParent = hWndParent;
		uTimeOut = uMS;

		_beginthreadex (NULL, 0, SplashThread, NULL, 0, &dwID);

		pgpSetPrefNumber  (kPGPPrefDateOfLastSplashScreen, uTodayTime);
		Sleep (MAINWINDOWDELAY);
	}

	return PGPCOMDLG_OK;
}


//----------------------------------------------------|
//	Post Splash screen

UINT PGPExport PGPcomdlgSetSplashParent (HWND hWndParent) {

	hWndSplashParent = hWndParent;

	return PGPCOMDLG_OK;
}
