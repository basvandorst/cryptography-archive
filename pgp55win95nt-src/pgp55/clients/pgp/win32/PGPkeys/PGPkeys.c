/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	PGPkeys.c - PGPkeys WinMain, associated routines, and global variables
	

	$Id: PGPkeys.c,v 1.32.2.1 1997/11/20 20:55:24 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

#include "pgpkeysx.h"
#include <ole2.h>

#include "pgpRandomPool.h"
#include "resource.h"

#include "pgpBuildFlags.h"

#include "..\include\PGPpk.h"
#include "..\include\pgpphras.h"

typedef struct _SEARCHDLGLISTSTRUCT {
	HWND	hwndSearchDlg;
	struct	_SEARCHDLGLISTSTRUCT* nextItem;
} SEARCHDLGLISTSTRUCT, *PSEARCHDLGLISTSTRUCT;


#define SPLASH_TIME		2500

// Application global variables
HINSTANCE		g_hInst;	            		//handle of current instance
HWND			g_hWndMain;
UINT			g_uReloadKeyringMessage = 0;	//broadcast message
PGPContextRef	g_ContextRef;
CHAR			g_szHelpFile[MAX_PATH];
PGPBoolean		g_bKeyGenEnabled = TRUE;
PGPBoolean		g_bRSAEnabled = FALSE;
PGPBoolean		g_bExpertMode = FALSE;
PGPBoolean		g_bMarginalAsInvalid = FALSE;
BOOL			g_bReadOnly = FALSE;

BOOL			g_bShowGroups;
INT				g_iGroupsPercent;


// Local globals
static HHOOK	hhookKeyboard;				//handle of keyboard hook 
static HHOOK	hhookMouse;					//handle of mouse hook 
static PSEARCHDLGLISTSTRUCT	pSearchDlgList = NULL;

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

	PGPcomdlgGetPGPPath (g_szHelpFile, sizeof(g_szHelpFile));

	LoadString (g_hInst, IDS_HELPFILENAME, sz, sizeof(sz));
	lstrcat (g_szHelpFile, sz);
}


//	____________________________________________
//
//	Initializations for all instances of program

BOOL 
InitApplication (HINSTANCE hInstance) 
{
	WNDCLASS wndclass;

// check correct version of windows
	if (!IsCorrectVersion ()) return FALSE;

// check for previous instances
	if (DoIExist (SEMAPHORENAME, WINCLASSNAME, WINDOWTITLE)) return FALSE;

// save instance handle
	g_hInst = hInstance;

// Ensure that the common control DLL is loaded.
	InitCommonControls ();

// register the reload keyring message
	g_uReloadKeyringMessage = RegisterWindowMessage (RELOADKEYRINGMSG);

// Initialize the cdk library
	if (PGPcomdlgErrorMessage (
		PGPNewContext (kPGPsdkAPIVersion, &g_ContextRef))) return FALSE;

	wndclass.style = 0;                     
	wndclass.lpfnWndProc = (WNDPROC)KeyManagerWndProc; 
	wndclass.cbClsExtra = 0;              
	wndclass.cbWndExtra = 0;              
	wndclass.hInstance = hInstance;       
	wndclass.hIcon = LoadIcon (hInstance, MAKEINTRESOURCE(IDI_EXEICON));
	wndclass.hCursor = LoadCursor (NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)COLOR_BACKGROUND; 
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
PKReloadSearchDlgList (VOID)
{
	PSEARCHDLGLISTSTRUCT psdls;

	psdls = pSearchDlgList;

	while (psdls) {
		SendMessage (psdls->hwndSearchDlg, KM_M_RELOADKEYRINGS, 1, 0);
		psdls = psdls->nextItem;
	}
}

//	____________________________________
//
//	Enable/Disable all search windows in list

VOID
PKEnableSearchDlgList (BOOL bEnabled)
{
	PSEARCHDLGLISTSTRUCT psdls;

	psdls = pSearchDlgList;

	while (psdls) {
		EnableWindow (psdls->hwndSearchDlg, bEnabled);
		psdls = psdls->nextItem;
	}
}

//	____________________________________
//
//	Add search dlg hwnd to linked list

VOID
PKAddSearchDlgToList (HWND hwndSearchDlg)
{
	PSEARCHDLGLISTSTRUCT psdls;

	psdls = pkAlloc (sizeof (SEARCHDLGLISTSTRUCT));
	if (psdls) {
		psdls->hwndSearchDlg = hwndSearchDlg;
		psdls->nextItem = pSearchDlgList;
		pSearchDlgList = psdls;
	}
}

//	____________________________________
//
//	Remove search dlg hwnd from linked list

VOID
PKRemoveSearchDlgFromList (HWND hwndSearchDlg)
{
	PSEARCHDLGLISTSTRUCT psdls;
	PSEARCHDLGLISTSTRUCT psdlsPrev;

	if (!pSearchDlgList) return;

	psdls = pSearchDlgList;

	if (psdls->hwndSearchDlg == hwndSearchDlg) {
		pSearchDlgList = pSearchDlgList->nextItem;
	}
	else {
		psdls = pSearchDlgList->nextItem;
		psdlsPrev = pSearchDlgList;
		while (psdls) {
			if (psdls->hwndSearchDlg == hwndSearchDlg) {
				psdlsPrev->nextItem = psdls->nextItem;
				break;
			}
			psdlsPrev = psdls;
			psdls = psdls->nextItem;
		}
	}

	if (psdls) pkFree (psdls);
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
	HACCEL		haccel;
	INT			lWindowPosX, lWindowPosY, lWindowWidth, lWindowHeight;
	DWORD		dwShow;
	CHAR		szSwitch[32];
	HMENU		hMenuMain;
	PGPPrefRef	prefRef;
	PGPError	err;
	BOOL		bDlgMessage;

	PSEARCHDLGLISTSTRUCT	psdls;

	//	initialize application stuff
	if (!InitApplication (hInstance)) return 0;

	//	attempt to set event hooks
	hhookMouse = SetWindowsHookEx (WH_MOUSE, MouseHookProc, 
									NULL, GetCurrentThreadId());
	hhookKeyboard = SetWindowsHookEx (WH_KEYBOARD, KeyboardHookProc, 
									NULL, GetCurrentThreadId());
	if (!hhookMouse || !hhookKeyboard) return 0;

	//	check for timed-out software
	if (PGPcomdlgIsExpired (NULL)) return 0;

	//  store path name of application
	PGPkeysSetPathRegistryData ();

	//	check that admin file can be found
#if PGP_BUSINESS_SECURITY
#if PGP_ADMIN_BUILD
	err = PGPcomdlgOpenAdminPrefs (&prefRef, TRUE);
#else
	err = PGPcomdlgOpenAdminPrefs (&prefRef, FALSE);
#endif	// PGP_ADMIN_BUILD
	if (IsPGPError (err)) return 0;
	PGPGetPrefBoolean (prefRef, kPGPPrefAllowKeyGen, 
							&g_bKeyGenEnabled);
	PGPcomdlgCloseAdminPrefs (prefRef, FALSE);
#else
	err = 0;
	g_bKeyGenEnabled = TRUE;
#endif	// PGP_BUSINESS_SECURITY

	//	post splash screen
	LoadString (hInstance, IDS_NOSPLASHSWITCH, szSwitch, sizeof(szSwitch));
//#if PGP_FREEWARE
//	PGPcomdlgSplash (g_ContextRef, NULL, SPLASH_TIME);
//#else
	if (!strstr (lpCmdLine, szSwitch))
		PGPcomdlgSplash (g_ContextRef, NULL, SPLASH_TIME);
//#endif

	//  initialize OLE for drag and drop functionality
    if (OleInitialize (NULL) != NOERROR) return 0; 

	//  get window position and size information from registry
	PGPcomdlgOpenClientPrefs (&prefRef);
	PGPGetPrefBoolean (prefRef, kPGPPrefDisplayMarginalValidity, 
							&g_bExpertMode);
	PGPGetPrefBoolean (prefRef, kPGPPrefMarginalIsInvalid, 
							&g_bMarginalAsInvalid);
	PGPkeysGetWindowPosFromRegistry (&dwShow, &lWindowPosX, &lWindowPosY, 
							&lWindowWidth, &lWindowHeight, &g_bShowGroups,
							&g_iGroupsPercent);
	PGPcomdlgCloseClientPrefs (prefRef, FALSE);
				
	//	load name of help file
	GetHelpFilePath ();

	//	find out if RSA operations are enabled in the SDK
	err = PGPcomdlgCheckSDKSupportForPKAlg (
					kPGPPublicKeyAlgorithm_RSA, FALSE, TRUE);
	if (IsPGPError (err)) g_bRSAEnabled = FALSE;
	else g_bRSAEnabled = TRUE;

	hMenuMain = LoadMenu (hInstance, MAKEINTRESOURCE (IDR_MENUKEYMAN));

	g_hWndMain = CreateWindow (WINCLASSNAME, WINDOWTITLE, 
		WS_OVERLAPPEDWINDOW, lWindowPosX, 
		lWindowPosY, lWindowWidth,
		lWindowHeight, NULL, hMenuMain, hInstance, NULL);	

	if (!g_hWndMain) return 0;

	ShowWindow (g_hWndMain, nCmdShow);	// Show the window
	UpdateWindow (g_hWndMain);			// Sends WM_PAINT message

	haccel = LoadAccelerators (g_hInst, MAKEINTRESOURCE (IDR_KEYMANACCEL));

	while (GetMessage (&msg, NULL, 0,0)) {
		if (pSearchDlgList) {
			bDlgMessage = FALSE;
			psdls = pSearchDlgList;
			while (psdls) {
				if (IsDialogMessage (psdls->hwndSearchDlg, &msg)) {
					bDlgMessage = TRUE;
					break;
				}
				psdls = psdls->nextItem;
			}
			if (bDlgMessage) continue;
		}
		if (!TranslateAccelerator (g_hWndMain, haccel, &msg)) { 
			TranslateMessage(&msg); 
			DispatchMessage(&msg); 
		} 
	}

	// clean out caches
	PGPcomdlgCloseLibrary ();
	while (PGPPurgeSignCachedPhrase ()) Sleep (250);

	// close down SDK
	PGPFreeContext (g_ContextRef);

	// required by OLE
    OleUninitialize ();

	// remove hook procedures
	UnhookWindowsHookEx (hhookKeyboard);
	UnhookWindowsHookEx (hhookMouse);

	return (msg.wParam);  
}


