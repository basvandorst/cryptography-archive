/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */
//:PGPkeys.c - PGPkeys WinMain, associated routines, and global variables

#include <windows.h>
#include "pgpkeysver.h"
#include "pgpkeys.h"
#include "resource.h"
#include "commctrl.h"
#include "..\include\pgpkeyserversupport.h"

#define WINCLASSNAME	("PGPkeysMainWinClass")
#define WINDOWTITLE		("PGPkeys")
#define SEMAPHORENAME	("PGPkeysInstSem")

#define SPLASH_TIME		2500

// Application global variables
HINSTANCE g_hInst;	         	 	 //handle of current instance
HWND g_hWndMain;						//handle of main window
HWND g_hWndTree;						//handle of TreeList control
CHAR g_szHelpFile[MAX_PATH];			//string containing name of help file
PGPKeySet* g_pKeySetMain = NULL;		//pointer to main keyset
LONG g_lKeyListSortField;				//keylist sort order
UINT g_uReloadKeyringMessage = 0;		//broadcast message
BOOL g_bKeyServerSupportLoaded = FALSE;	//TRUE is keyserver DLL is loaded
BOOL g_bReadOnly = FALSE;

CHAR g_szbuf[G_BUFLEN];			//character buffer for general purpose use


//-------------------------------------------------------------------|
// Check for correct version of Windows

BOOL IsCorrectVersion (void) {

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
//>>>
	}
}


//-------------------------------------------------------------------|
// Check for previously running instances

BOOL DoIExist (LPSTR lpszSemName,
			   LPSTR lpszClassName, LPSTR lpszWindowTitle) {

      HANDLE hSem;
      HWND hWndMe;

      // Create or open a named semaphore.
      hSem = CreateSemaphore (NULL, 0, 1, lpszSemName);

      // Close handle and return NULL if existing semaphore was opened.
      if ((hSem != NULL) && (GetLastError() == ERROR_ALREADY_EXISTS)) {
          CloseHandle(hSem);
          hWndMe = FindWindow (lpszClassName, lpszWindowTitle);
          if (hWndMe) {
			if (!IsZoomed (hWndMe)) ShowWindow (hWndMe, SW_RESTORE);			
			SetForegroundWindow (hWndMe);
		}
        return TRUE;
     }

     // If new semaphore was created, return FALSE.
     return FALSE;
}


//-------------------------------------------------------------------|
// Load any PGP support DLLs

BOOL LoadSupportDLLs (void) {
	g_bKeyServerSupportLoaded = (LoadKeyserverSupport () == SUCCESS);
    return TRUE;
}


//-------------------------------------------------------------------|
// Unload any PGP support DLLs

BOOL UnloadSupportDLLs (void) {
	UnLoadKeyserverSupport ();
    return TRUE;
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

	LoadString (g_hInst, IDS_REGISTRYKEY, g_szbuf, G_BUFLEN);
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
// Initializations for all instances of program

BOOL InitApplication (HINSTANCE hInstance) {
	WNDCLASS wndclass;

// check correct version of windows
	if (!IsCorrectVersion ()) return FALSE;

// check for previous instances
	if (DoIExist (SEMAPHORENAME, WINCLASSNAME, WINDOWTITLE)) return FALSE;

// save instance handle
	g_hInst = hInstance;

// Ensure that the common control DLL is loaded.
	InitCommonControls ();

// Ensure that the common control DLL is loaded.
	InitTreeListControl ();

// Initialize the aurora library
	if (PGPcomdlgErrorMessage (pgpLibInit ())) return FALSE;
	if (pgpGetTrustModel () != PGPTRUST0) {
		PGPcomdlgErrorMessage (PGPERR_TROUBLE_BADTRUST);
		return FALSE;
	}

	wndclass.style = 0;
	wndclass.lpfnWndProc = (WNDPROC)KeyManagerWndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon (hInstance, MAKEINTRESOURCE(IDI_EXEICON));
	wndclass.hCursor = LoadCursor (NULL, IDC_ARROW);
	wndclass.hbrBackground = NULL;
    wndclass.lpszMenuName = MAKEINTRESOURCE (IDR_MENUKEYMAN);
	wndclass.lpszClassName = WINCLASSNAME;

	return (RegisterClass (&wndclass));
}


//-------------------------------------------------------------------|
// Main Windows routine

int APIENTRY WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
					LPSTR lpCmdLine, int nCmdShow) {
	MSG msg;
	HACCEL haccel;
	INT lWindowPosX, lWindowPosY, lWindowWidth, lWindowHeight;
	DWORD dwShow;

	if (!InitApplication (hInstance)) return 0;

//	check for timed-out software
	if (PGPcomdlgIsExpired (NULL)) return 0;

//	post splash screen
	LoadString (hInstance, IDS_NOSPLASHSWITCH, g_szbuf, G_BUFLEN);
	if (!strstr (lpCmdLine, g_szbuf))
		PGPcomdlgSplash (NULL, SPLASH_TIME);

//  initialize OLE for drag and drop functionality
    if (OleInitialize (NULL) != NOERROR) return 0;

//  get window position and size information from registry
	KMGetWindowPosFromRegistry (&dwShow, &lWindowPosX, &lWindowPosY,
							&lWindowWidth, &lWindowHeight);
		
//  get sort order from registry
	KMGetSortOrderFromRegistry (&g_lKeyListSortField);
		
//  store path name of application
	KMSetPathRegistryData ();

//	load name of help file
	GetHelpFilePath ();

//	load support DLLs, if possible
	LoadSupportDLLs ();

	g_hWndMain = CreateWindow (WINCLASSNAME, WINDOWTITLE,
		WS_OVERLAPPEDWINDOW, lWindowPosX,
		lWindowPosY, lWindowWidth,
		lWindowHeight, NULL, NULL, hInstance, NULL);	

	if (!g_hWndMain) return 0;

	ShowWindow (g_hWndMain, nCmdShow);	// Show the window
	UpdateWindow (g_hWndMain);			// Sends WM_PAINT message

	haccel = LoadAccelerators (g_hInst, MAKEINTRESOURCE (IDR_KEYMANACCEL));

	while (GetMessage (&msg, NULL, 0,0)) {
		if (!TranslateAccelerator (g_hWndMain, haccel, &msg)) {
	        TranslateMessage(&msg);
		    DispatchMessage(&msg);
		}
	}

//  unload support DLLs
	UnloadSupportDLLs ();

//  close down the library
	pgpLibCleanup ();

//  required by OLE
    OleUninitialize ();

	return (msg.wParam);
}
