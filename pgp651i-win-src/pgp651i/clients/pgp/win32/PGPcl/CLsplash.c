/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	CLsplash.c - display splash screen
	

	$Id: CLsplash.c,v 1.7 1999/04/01 03:32:22 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

// project header files
#include "pgpclx.h"

// system header files
#include <process.h>

// constant definitions
#define TIMER_ID		4321
#define TIMER_PERIOD	200
#define MAINWINDOWDELAY	500

#define ADDHEIGHT		0
#define XINCREMENT		6
#define YINCREMENT		10

// external globals
extern HINSTANCE	g_hInst;
extern HWND			g_hWndSplash;

// typedefs
typedef struct _SPLASHSTRUCT {
	PGPContextRef	context;
	HBITMAP			hBitmapSplash;
	HPALETTE		hPaletteSplash;
	UINT			uTimeOut;
	UINT			uTimerTime;
	UINT			uXsize;
	UINT			uYsize;
	UINT			uXpos;
	UINT			uYpos;
} SPLASHSTRUCT, *PSPLASHSTRUCT;

// local globals
static HWND				hWndSplashParent;
static PGPMemoryMgrRef	MemMgr;

//	______________________________
//
//	Splash screen dialog procedure

static BOOL CALLBACK 
sSplashDlgProc (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	PSPLASHSTRUCT	pss;

	switch (uMsg) {

	case WM_INITDIALOG:
		{
			HWND hWndControl;

			SetWindowLong (hDlg, GWL_USERDATA, lParam);
			pss = (PSPLASHSTRUCT)lParam;

			g_hWndSplash = hDlg;
			SetWindowPos (hDlg, HWND_TOP, pss->uXpos, pss->uYpos, 
											pss->uXsize, pss->uYsize, 0);

			hWndControl = GetDlgItem (hDlg, IDOK);
			EnableWindow (hWndControl, FALSE);
			ShowWindow (hWndControl, SW_HIDE);
			hWndControl = GetDlgItem (hDlg, IDC_BUYNOW);
			EnableWindow (hWndControl, FALSE);
			ShowWindow (hWndControl, SW_HIDE);
			pss->uTimerTime = 0;
			SetTimer (hDlg, TIMER_ID, TIMER_PERIOD, NULL);
		}
		return TRUE;

	case WM_PAINT:
		{
			HDC			hDC, hMemDC;
			PAINTSTRUCT ps;
			HBITMAP		hBitmapOld;

			pss = (PSPLASHSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);

			hDC = BeginPaint (hDlg, &ps);
			hMemDC = CreateCompatibleDC (hDC);
			if (pss->hPaletteSplash) {
				SelectPalette (hDC, pss->hPaletteSplash, FALSE);
				RealizePalette (hDC);
			}
			hBitmapOld = SelectObject (hMemDC, pss->hBitmapSplash);
			CLPaintUserInfo (MemMgr, hDlg, hMemDC, NULL);
			BitBlt (hDC, 0, 0, pss->uXsize, pss->uYsize, 
											hMemDC, 0, 0, SRCCOPY);
			SelectObject (hMemDC, hBitmapOld);
			DeleteDC (hMemDC);
			EndPaint (hDlg, &ps);
		}
		return FALSE;

	case WM_LBUTTONDOWN:
		if (hWndSplashParent) EndDialog (hDlg, 0);
		return TRUE;

	case WM_DESTROY:
		KillTimer (hDlg, TIMER_ID);
		return FALSE;

	case WM_TIMER:
		pss = (PSPLASHSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
		if (pss->uTimerTime < pss->uTimeOut) 
			pss->uTimerTime += TIMER_PERIOD;
		else 
			if (hWndSplashParent) EndDialog (hDlg, 0);
		return TRUE;

	case WM_CLOSE :
		EndDialog (hDlg, 0);
		break;

	case WM_COMMAND :
		switch(LOWORD (wParam)) {

		case IDOK: 
			if (hWndSplashParent) EndDialog (hDlg, 0);
			break;
		}
		return TRUE;

	default:
		return FALSE;

	}

	return FALSE;
}

//	______________________________
//
//	Splash screen thread procedure

static DWORD WINAPI 
sSplashThread (PVOID pvoid) 
{
	SPLASHSTRUCT	ss;
	BITMAP			bm;
	HDC				hDC;
	INT				iNumBits, iSplashBitmap;

	hDC = GetDC (NULL);		// DC for desktop
	iNumBits = GetDeviceCaps (hDC, BITSPIXEL) * GetDeviceCaps (hDC, PLANES);
	ReleaseDC (NULL, hDC);

	if (iNumBits <= 1) iSplashBitmap = IDB_SPLASH1;
	else if (iNumBits <= 4) iSplashBitmap = IDB_SPLASH4;
	else iSplashBitmap = IDB_SPLASH8;

	ss.uTimeOut = (UINT)pvoid;
	ss.hBitmapSplash = CLLoadResourceBitmap (g_hInst, 
										MAKEINTRESOURCE (iSplashBitmap),
										&ss.hPaletteSplash);
	GetObject (ss.hBitmapSplash, sizeof(BITMAP), (LPSTR)&bm);

	ss.uXsize = bm.bmWidth;
	ss.uYsize = bm.bmHeight;

	ss.uXpos = (GetSystemMetrics (SM_CXFULLSCREEN) - ss.uXsize) / 2;
	ss.uYpos = (GetSystemMetrics (SM_CYFULLSCREEN) - ss.uYsize) / 2;

	DialogBoxParam (g_hInst, MAKEINTRESOURCE (IDD_SPLASH), 
					hWndSplashParent, sSplashDlgProc, (LPARAM)&ss);

	g_hWndSplash = NULL;
	if (hWndSplashParent) {
		EnableWindow (hWndSplashParent, TRUE);
		SetForegroundWindow (hWndSplashParent);
	}

	DeleteObject (ss.hBitmapSplash);
	DeleteObject (ss.hPaletteSplash);

	g_hWndSplash = NULL;

	return kPGPError_NoErr;
}

//	__________________
//
//	Post Splash screen

PGPError PGPclExport 
PGPclSplash (
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

	MemMgr = PGPGetContextMemoryMgr (Context);
	PGPclOpenClientPrefs (MemMgr, &PrefRef);

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

		_beginthreadex (NULL, 0, sSplashThread, (PVOID)uMS, 0, &dwID); 

		PGPSetPrefNumber (PrefRef, kPGPPrefDateOfLastSplashScreen, Today);
		Sleep (MAINWINDOWDELAY);
	}

	PGPclCloseClientPrefs (PrefRef, TRUE);

	return kPGPError_NoErr;
}

//	__________________
//
//	Identify splash screen parent window

PGPError PGPclExport 
PGPclSetSplashParent (HWND hWndParent) 
{
	if (hWndParent) {
		hWndSplashParent = hWndParent;
		if (g_hWndSplash) {
			SendMessage (g_hWndSplash, WM_APP, 0, 0);
			EnableWindow (hWndParent, FALSE);
		}
	}
	else {
		hWndSplashParent = NULL;
		if (g_hWndSplash)
			SendMessage (g_hWndSplash, WM_CLOSE, 0, 0);
	}

	return kPGPError_NoErr;
}
