/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PGPkeys.c - PGPkeys WinMain, associated routines, and global variables
	

	$Id: PGPkeys.c,v 1.58 1998/08/11 15:20:07 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

// project header files
#include "pgpkeysx.h"
#include "..\include\PGPpk.h"

// pgp header files
#include "pgpRandomPool.h"

// system header files
#include <ole2.h>
#include <zmouse.h>

// constant definitions
#define SPLASH_TIME		2500

// typedefs
typedef struct _DLGLISTSTRUCT {
	HWND	hwndDlg;
	struct	_DLGLISTSTRUCT* nextItem;
} DLGLISTSTRUCT, *PDLGLISTSTRUCT;

// Application global variables
HINSTANCE			g_hInst;	           //handle of current instance
HWND				g_hWndMain;
UINT				g_uReloadKeyringMessage = 0;	//broadcast messages
UINT				g_uReloadPrefsMessage = 0;	
UINT				g_uReloadKeyserverPrefsMessage = 0;	
UINT				g_uPurgePassphraseCacheMessage = 0;
UINT				g_uMouseWheelMessage = 0;
PGPContextRef		g_Context;
PGPtlsContextRef	g_TLSContext = kInvalidPGPtlsContextRef;
CHAR				g_szHelpFile[MAX_PATH];
PGPBoolean			g_bKeyGenEnabled = TRUE;
PGPBoolean			g_bExpertMode = FALSE;
PGPBoolean			g_bMarginalAsInvalid = FALSE;
BOOL				g_bReadOnly = FALSE;

BOOL				g_bShowGroups;
INT					g_iGroupsPercent;
INT					g_iToolHeight;

// Local globals
static HHOOK	hhookKeyboard;				//handle of keyboard hook 
static HHOOK	hhookMouse;					//handle of mouse hook 
static PDLGLISTSTRUCT	pDlgList = NULL;

//	____________________________________
//
//	Check for correct version of Windows

BOOL 
IsCorrectVersion (VOID) 
{
	OSVERSIONINFO osid;

	osid.dwOSVersionInfoSize = sizeof (osid);
	GetVersionEx (&osid);

	switch (osid.dwPlatformId) {

	case VER_PLATFORM_WIN32s :
		return FALSE;

	// Windows 95
	case VER_PLATFORM_WIN32_WINDOWS :
 		return TRUE;

	// Windows NT
	case VER_PLATFORM_WIN32_NT :
		if (osid.dwMajorVersion >= 4) return TRUE;
//		return FALSE;
//>>>
		MessageBox (NULL, "PGPkeys is not supported on Windows NT 3.51\n\n"
			"Use at your own risk.",
		"PGPkeys Warning", MB_OK|MB_ICONEXCLAMATION);
		return TRUE;
//<<<
	}
}

//	____________________________________
//
//	Check for previously running instances

BOOL 
DoIExist (
		LPSTR lpszSemName, 
		LPSTR lpszClassName, 
		LPSTR lpszWindowTitle) 
{
    HANDLE			hSem;
    HWND			hWndMe;
	COPYDATASTRUCT	cds;

    // Create or open a named semaphore. 
    hSem = CreateSemaphore (NULL, 0, 1, lpszSemName);

    // Close handle and return NULL if existing semaphore was opened.
    if ((hSem != NULL) && (GetLastError() == ERROR_ALREADY_EXISTS)) {
        CloseHandle(hSem);
        hWndMe = FindWindow (lpszClassName, lpszWindowTitle);
        if (hWndMe) {
			if (!IsZoomed (hWndMe)) ShowWindow (hWndMe, SW_RESTORE);
			SetForegroundWindow (hWndMe);
			cds.dwData = PGPPK_IMPORTKEYCOMMANDLINE | PGPPK_SELECTIVEIMPORT;
			cds.lpData = GetCommandLine ();
			cds.cbData = lstrlen (cds.lpData) +1;
			SendMessage (hWndMe, WM_COPYDATA, 0, (LPARAM)&cds);
		}
        return TRUE;
    }

    // If new semaphore was created, return FALSE.
    return FALSE;
}


//	____________________________________________________________
//
//	get PGPkeys path from registry and append Help file name 

VOID 
GetHelpFilePath (VOID) 
{
	CHAR sz[128];

	PGPclGetPGPPath (g_szHelpFile, sizeof(g_szHelpFile));

	LoadString (g_hInst, IDS_HELPFILENAME, sz, sizeof(sz));
	lstrcat (g_szHelpFile, sz);
}


//	____________________________________________
//
//	Initializations for all instances of program

BOOL 
InitApplication (HINSTANCE hInstance, BOOL bIgnoreComctlError) 
{
	WNDCLASS				wndclass;
	PGPError				err;

	// check correct version of windows
	if (!IsCorrectVersion ()) 
		return FALSE;

	// check for previous instances
	if (DoIExist (SEMAPHORENAME, WINCLASSNAME, WINDOWTITLE)) 
		return FALSE;

	// save instance handle
	g_hInst = hInstance;

	// register the reload messages
	g_uReloadKeyringMessage = 
			RegisterWindowMessage (RELOADKEYRINGMSG);
	g_uReloadPrefsMessage = 
			RegisterWindowMessage (RELOADPREFSMSG);
	g_uReloadKeyserverPrefsMessage = 
			RegisterWindowMessage (RELOADKEYSERVERPREFSMSG);
	g_uPurgePassphraseCacheMessage = 
			RegisterWindowMessage (PURGEPASSPHRASECACEHMSG);

	// register the mouse wheel message
	g_uMouseWheelMessage = 
			RegisterWindowMessage (MSH_MOUSEWHEEL);

	// Initialize the sdk library
	err = PGPNewContext (kPGPsdkAPIVersion, &g_Context);
	if (IsPGPError (err)) {
		if (err == kPGPError_FeatureNotAvailable) {
			PKMessageBox (NULL, IDS_CAPTION, IDS_SDKEXPIRED,
							MB_OK|MB_ICONSTOP);
		}
		else
			PGPclErrorBox (NULL, err);
		return FALSE;
	}

	PGPclErrorBox (NULL, PGPNewTLSContext (g_Context, &g_TLSContext));

	// initialize client library
	// note: this fails if common control DLL is not correct version
	err = PGPclInitLibrary (g_Context);
	if (IsPGPError (err)) {
		if (err == kPGPError_Win32_InvalidComctl32 && bIgnoreComctlError)
			err = kPGPError_NoErr;
		else {
			PGPclErrorBox (NULL, err);
			return FALSE;
		}
	}

	wndclass.style = 0;                     
	wndclass.lpfnWndProc = (WNDPROC)KeyManagerWndProc; 
	wndclass.cbClsExtra = 0;              
	wndclass.cbWndExtra = 0;              
	wndclass.hInstance = hInstance;       
	wndclass.hIcon = LoadIcon (hInstance, MAKEINTRESOURCE(IDI_EXEICON));
	wndclass.hCursor = LoadCursor (NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1); 
    wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = WINCLASSNAME;

	return (RegisterClass (&wndclass));
}

//	____________________________________
//
//  Hook procedure for WH_MOUSE hook.
  
LRESULT CALLBACK 
MouseHookProc (
		INT		iCode, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{ 
	MOUSEHOOKSTRUCT* pmhs;

    if (iCode >= 0) {
		pmhs = (MOUSEHOOKSTRUCT*) lParam; 
		PGPGlobalRandomPoolAddMouse (pmhs->pt.x, pmhs->pt.y);
	}

	return CallNextHookEx (hhookMouse, iCode, wParam, lParam);
} 

//	____________________________________
//
//  Hook procedure for WH_KEYBOARD hook.
  
LRESULT CALLBACK 
KeyboardHookProc (
		INT		iCode, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{ 
    if (iCode >= 0) {
		PGPGlobalRandomPoolAddKeystroke (wParam);
	}

	return CallNextHookEx (hhookKeyboard, iCode, wParam, lParam);
} 

//	____________________________________
//
//	Reload all search windows in list

VOID
PKPostMessageDlgList (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PDLGLISTSTRUCT pdls;

	pdls = pDlgList;

	while (pdls) {
		PostMessage (pdls->hwndDlg, uMsg, wParam, lParam);
		pdls = pdls->nextItem;
	}
}

//	____________________________________
//
//	Enable/Disable all search windows in list

VOID
PKEnableDlgList (BOOL bEnabled)
{
	PDLGLISTSTRUCT pdls;

	pdls = pDlgList;

	while (pdls) {
		EnableWindow (pdls->hwndDlg, bEnabled);
		pdls = pdls->nextItem;
	}
}

//	____________________________________
//
//	Add dlg hwnd to linked list

static VOID
sAddDlgToList (HWND hwndDlg)
{
	PDLGLISTSTRUCT pdls;

	pdls = pkAlloc (sizeof (DLGLISTSTRUCT));
	if (pdls) {
		pdls->hwndDlg = hwndDlg;
		pdls->nextItem = pDlgList;
		pDlgList = pdls;
	}
}

//	____________________________________
//
//	Remove dlg hwnd from linked list

static VOID
sRemoveDlgFromList (HWND hwndDlg)
{
	PDLGLISTSTRUCT pdls;
	PDLGLISTSTRUCT pdlsPrev;

	if (!pDlgList) return;

	pdls = pDlgList;

	if (pdls->hwndDlg == hwndDlg) {
		pDlgList = pDlgList->nextItem;
	}
	else {
		pdls = pDlgList->nextItem;
		pdlsPrev = pDlgList;
		while (pdls) {
			if (pdls->hwndDlg == hwndDlg) {
				pdlsPrev->nextItem = pdls->nextItem;
				break;
			}
			pdlsPrev = pdls;
			pdls = pdls->nextItem;
		}
	}

	if (pdls) pkFree (pdls);
}

//	____________________________________
//
//	Add or remove dlg hwnd from linked list

VOID
PKDialogListFunc (HWND hwndDlg, BOOL bAdd, POINT* ppt, HWND* phwnd)
{
	PDLGLISTSTRUCT	pdls;
	RECT			rc;

	// if a hwnd is specified, then add/remove if from list
	if (hwndDlg) {
		if (bAdd)
			sAddDlgToList (hwndDlg);
		else
			sRemoveDlgFromList (hwndDlg);
	}

	// otherwise this is a query to find out if a point is in a window
	else {
		if (!phwnd) return;
		*phwnd = NULL;

		if (!ppt) return;
		if (!pDlgList) return;

		pdls = pDlgList;

		while (pdls) {
			GetWindowRect (pdls->hwndDlg, &rc);
			if (PtInRect (&rc, *ppt)) {
				*phwnd = pdls->hwndDlg;
				return;
			}
			pdls = pdls->nextItem;
		}
	}
}

//	____________________________________
//
//	Main Windows routine

INT APIENTRY 
WinMain (
		HINSTANCE hInstance, 
		HINSTANCE hPrevInstance,
		LPSTR lpCmdLine, 
		INT nCmdShow) 
{
	MSG			msg;
	HACCEL		haccelMain, haccelDialog;
	INT			lWindowPosX, lWindowPosY, lWindowWidth, lWindowHeight;
	DWORD		dwShow;
	CHAR		szSwitch[32];
	HMENU		hMenuMain;
	PGPPrefRef	prefRef;
	PGPError	err;
	BOOL		bDlgMessage;
	BOOL		bIgnoreComctlError;

	PDLGLISTSTRUCT	pdls;

	//  store path name of application
	PKSetPathRegistryData ();

	//	initialize application stuff
	LoadString (hInstance, IDS_COMCTLERRORSWITCH, szSwitch, sizeof(szSwitch));
	if (strstr (lpCmdLine, szSwitch))
		bIgnoreComctlError = TRUE;
	else
		bIgnoreComctlError = FALSE;
	if (!InitApplication (hInstance, bIgnoreComctlError)) return 0;

	//	attempt to set event hooks
	hhookMouse = SetWindowsHookEx (WH_MOUSE, MouseHookProc, 
									NULL, GetCurrentThreadId());
	hhookKeyboard = SetWindowsHookEx (WH_KEYBOARD, KeyboardHookProc, 
									NULL, GetCurrentThreadId());
	if (!hhookMouse || !hhookKeyboard) return 0;

	//	check for timed-out software
	if (IsPGPError (PGPclIsExpired (NULL))) return 0;

	//	check that admin file can be found
#if PGP_BUSINESS_SECURITY
#if PGP_ADMIN_BUILD
	err = PGPclOpenAdminPrefs (
			PGPGetContextMemoryMgr (g_Context), 
			&prefRef, TRUE);
#else
	err = PGPclOpenAdminPrefs (
			PGPGetContextMemoryMgr (g_Context), 
			&prefRef, FALSE);
#endif	// PGP_ADMIN_BUILD
	if (IsPGPError (err)) return 0;
	PGPGetPrefBoolean (prefRef, kPGPPrefAllowKeyGen, 
							&g_bKeyGenEnabled);
	PGPclCloseAdminPrefs (prefRef, FALSE);
#else
	err = 0;
	g_bKeyGenEnabled = TRUE;
#endif	// PGP_BUSINESS_SECURITY

	//	post splash screen
	LoadString (hInstance, IDS_NOSPLASHSWITCH, szSwitch, sizeof(szSwitch));
	if (!strstr (lpCmdLine, szSwitch))
		PGPclSplash (g_Context, NULL, SPLASH_TIME);

	//  initialize OLE for drag and drop functionality
    if (OleInitialize (NULL) != NOERROR) return 0; 

	//  get window position and size information from registry
	PGPclOpenClientPrefs (PGPGetContextMemoryMgr (g_Context), &prefRef);
	PGPGetPrefBoolean (prefRef, kPGPPrefDisplayMarginalValidity, 
							&g_bExpertMode);
	PGPGetPrefBoolean (prefRef, kPGPPrefMarginalIsInvalid, 
							&g_bMarginalAsInvalid);
	PKGetPrivatePrefData (&dwShow, &lWindowPosX, &lWindowPosY, 
							&lWindowWidth, &lWindowHeight, &g_bShowGroups,
							&g_iGroupsPercent, &g_iToolHeight);
	PGPclCloseClientPrefs (prefRef, FALSE);
				
	//	load name of help file
	GetHelpFilePath ();

	hMenuMain = LoadMenu (hInstance, MAKEINTRESOURCE (IDR_MENUKEYMAN));

	g_hWndMain = CreateWindow (WINCLASSNAME, WINDOWTITLE, 
		WS_OVERLAPPEDWINDOW, lWindowPosX, 
		lWindowPosY, lWindowWidth,
		lWindowHeight, NULL, hMenuMain, hInstance, NULL);	

	if (!g_hWndMain) return 0;

	ShowWindow (g_hWndMain, nCmdShow);	// Show the window
	UpdateWindow (g_hWndMain);			// Sends WM_PAINT message

	haccelMain = 
		LoadAccelerators (g_hInst, MAKEINTRESOURCE (IDR_KEYMANACCEL));  
	haccelDialog = 
		LoadAccelerators (g_hInst, MAKEINTRESOURCE (IDR_DIALOGACCEL));  

	while (GetMessage (&msg, NULL, 0,0)) {
		if (pDlgList) {
			bDlgMessage = FALSE;
			pdls = pDlgList;
			while (pdls) {
				if (pdls->hwndDlg != g_hWndMain) {
					if (IsDialogMessage (pdls->hwndDlg, &msg)) {
						TranslateAccelerator (
								pdls->hwndDlg, haccelDialog, &msg);
						bDlgMessage = TRUE;
						break;
					}
				}
				pdls = pdls->nextItem;
			}
			if (bDlgMessage) continue;
		}
		if (!TranslateAccelerator (g_hWndMain, haccelMain, &msg)) { 
			TranslateMessage(&msg); 
			DispatchMessage(&msg); 
		} 
	}

	// clean out caches
	PGPclCloseLibrary ();

	if (PGPtlsContextRefIsValid (g_TLSContext))
		PGPFreeTLSContext (g_TLSContext);

	// close down SDK
	PGPFreeContext (g_Context);

	// required by OLE
    OleUninitialize ();

	// remove hook procedures
	UnhookWindowsHookEx (hhookKeyboard);
	UnhookWindowsHookEx (hhookMouse);

	return (msg.wParam);  
}


