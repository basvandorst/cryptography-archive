/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PGPnetApp.c - PGPnet GUI application
	

	$Id: PGPNetApp.c,v 1.39.4.3 1999/06/08 16:32:06 pbj Exp $
____________________________________________________________________________*/

#include <windows.h>
#include <commctrl.h>
#include "resource.h"

#include "PGPnetApp.h"
#include "IPAddressControl.h"
#include "DurationControl.h"
#include "pgpNetIPC.h"
#include "pgpNetPaths.h"
#include "pgpAdminPrefs.h"
#include "pgpClientPrefs.h"

#include "PGPcl.h"
#include "TreeList.h"

#define CERTSELECTDLGCLASS			("PGPCERTSELECTDLGCLASS")
#define PGPNET_SEMAPHORENAME		("PGPnetAppSemaphore")

#define PGPNET_DEFAULTWINDOWWIDTH	570
#define PGPNET_DEFAULTWINDOWHEIGHT	400		

HINSTANCE			g_hinst				= NULL;
INT					g_iMinWindowWidth	= 200;
HWND				g_hwndTab			= NULL;
HWND				g_hwndMain			= NULL;
BOOL				g_bWinsock			= FALSE;
BOOL				g_bReadOnly			= FALSE;	
BOOL				g_bAuthAccess		= FALSE;	

PGPContextRef		g_context			= kInvalidPGPContextRef;
HWND				g_hwndCurrentDlg	= NULL;
HWND				g_hwndOptions		= NULL;
UINT				g_uReloadKeyringMsg	= 0;
INT					g_iCurrentPage;
APPOPTIONSSTRUCT	g_AOS;
CHAR				g_szHelpFile[MAX_PATH];


//	____________________________________
//
//	get start page from command line

static INT 
sGetStartPageFromCommandLine (
		LPSTR	pszLine) 
{
	CHAR*	p;

	p = strchr (pszLine, '/');

	if (!p)
		return PGPNET_STATUSPAGE;

	p++;
	switch (*p) 
	{
	case '1' :
	case 'l' :
	case 'L' :
		return PGPNET_LOGPAGE;

	case '2' :
	case 'h' :
	case 'H' :
		return PGPNET_HOSTPAGE;

	case '3' :
	case 'o' :
	case 'O' :
		return PGPNET_OPTIONSHEET;

	default :
		return PGPNET_STATUSPAGE;
	}
}

//	____________________________________
//
//	Check for previously running instances

static BOOL 
sDoIExist (
		LPSTR	pszSemName, 
		LPSTR	pszClassName, 
		LPSTR	pszWindowTitle,
		LPSTR	pszCommandLine) 
{
    HANDLE			hSem;
    HWND			hWndMe;

    // Create or open a named semaphore. 
    hSem = CreateSemaphore (NULL, 0, 1, pszSemName);

    // Close handle and return NULL if existing semaphore was opened.
    if ((hSem != NULL) && 
		(GetLastError() == ERROR_ALREADY_EXISTS)) 
	{
        CloseHandle(hSem);
        hWndMe = FindWindow (pszClassName, pszWindowTitle);
        if (hWndMe) 
		{
			hWndMe = (HWND)SendMessage (hWndMe, WM_APP, PGPNET_PAGESELECT,
					sGetStartPageFromCommandLine (pszCommandLine));	
			if (hWndMe)
			{
				if (!IsZoomed (hWndMe)) 
					ShowWindow (hWndMe, SW_RESTORE);			
				SetForegroundWindow (hWndMe);
			}
		}
        return TRUE;
    }

    // If new semaphore was created, return FALSE.
    return FALSE;
}


//	____________________________________
//
//	Get PGPnet application window position from registry

static VOID 
sGetWindowPosFromRegistry (
		INT*	piX,
		INT*	piY,
		INT*	piWidth,
		INT*	piHeight) 
{
	HKEY	hKey;
	LONG	lResult;
	DWORD	dwType, dwValue, dwSize;
	CHAR	sz[256];

	*piX = CW_USEDEFAULT;
	*piY = CW_USEDEFAULT;
	*piWidth = PGPNET_DEFAULTWINDOWWIDTH;
	*piHeight = PGPNET_DEFAULTWINDOWHEIGHT;

	LoadString (g_hinst, IDS_PGPNETREGKEY, sz, sizeof(sz));
	lResult = RegOpenKeyEx (HKEY_LOCAL_MACHINE, sz, 0, 
										KEY_QUERY_VALUE, &hKey);

	if (lResult == ERROR_SUCCESS) 
	{
		LoadString (g_hinst, IDS_WINDOWPOS, sz, sizeof(sz));
		dwSize = sizeof(dwValue);
		lResult = RegQueryValueEx (hKey, sz, NULL, 
							&dwType, (LPBYTE)&dwValue, &dwSize);
		if (lResult == ERROR_SUCCESS) 
		{
			*piX = LOWORD (dwValue);
			if (*piX > 0x8000)
				*piX -= 0x10000;
			*piY = HIWORD (dwValue);
			if (*piY > 0x8000)
				*piY -= 0x10000;
		}

		LoadString (g_hinst, IDS_WINDOWSIZE, sz, sizeof(sz));
		dwSize = sizeof(dwValue);
		lResult = RegQueryValueEx (hKey, sz, NULL, 
							&dwType, (LPBYTE)&dwValue, &dwSize);
		if (lResult == ERROR_SUCCESS) 
		{
			*piWidth = LOWORD (dwValue);
			*piHeight = HIWORD (dwValue);
		}

		RegCloseKey (hKey);
	}
}

//	____________________________________
//
//	application entry point

INT WINAPI 
WinMain(
    HINSTANCE	hInstance,
    HINSTANCE	hPrevInstance,
    LPSTR		pszCmdLine,	
    INT			nCmdShow) 	
{
	MSG					msg; 
	WNDCLASS			wc; 
	HACCEL				haccelMain;
	PGPError			err;
	INT					iPosX, iPosY, iWidth, iHeight;
	WSADATA				wsadata;

	// check for already running instances of this app
	if (sDoIExist (PGPNET_SEMAPHORENAME, PGPNET_APPWINDOWCLASS,
					PGPNET_APPWINDOWTITLE, pszCmdLine)) 
		return FALSE;

	// parse command-line switches
	g_iCurrentPage = sGetStartPageFromCommandLine (pszCmdLine);

	// save instance handle
	g_hinst = hInstance; 
 
	// Ensure that the custom control DLL is loaded.
	InitTreeListControl ();

	// initialize OLE for drag and drop functionality
	if (OleInitialize (NULL) != NOERROR) 
		return FALSE; 

	// initialize Winsock -- used only to get local machine info
	g_bWinsock = (WSAStartup (MAKEWORD (2, 2), &wsadata) == 0);

	// register so that we can receive the reload keyring message
	g_uReloadKeyringMsg = RegisterWindowMessage (RELOADKEYRINGMSG);

	// Initialize the sdk library
	err = PGPNewContext (kPGPsdkAPIVersion, &g_context);
	if (IsPGPError (err)) {
		if (err == kPGPError_FeatureNotAvailable) {
			PNMessageBox (NULL, IDS_CAPTION, IDS_SDKEXPIRED,
							MB_OK|MB_ICONSTOP);
		}
		else
			PNErrorBox (NULL, err);
		return FALSE;
	}

	// get help file path
	err = PGPnetGetHelpFullPath (g_szHelpFile, sizeof(g_szHelpFile));
	if (IsPGPError (err))
		return FALSE;

	// initialize client library
	// note: this fails if common control DLL is not correct version
	//	initialize application stuff
	err = PGPclInitLibrary (g_context);
	if (IsPGPError (err)) 
	{
		PGPclErrorBox (NULL, err);
		return FALSE;
	}

	// register window class for main window
	if (!hPrevInstance) { 
		wc.style = 0; 
		wc.lpfnWndProc = (WNDPROC)PGPnetWndProc; 
		wc.cbClsExtra = 0; 
		wc.cbWndExtra = 0; 
		wc.hInstance = hInstance; 
		wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PGPNETAPP)); 
		wc.hCursor = LoadCursor ((HINSTANCE)NULL, IDC_ARROW); 
		wc.hbrBackground = GetSysColorBrush (COLOR_3DFACE); 
		wc.lpszMenuName =  NULL; 
		wc.lpszClassName = PGPNET_APPWINDOWCLASS; 

		if (!RegisterClass(&wc)) 
			return FALSE; 
    } 
 
	// initialize common controls
	InitCommonControls (); 
	InitPGPIPAddressControl ();
	InitPGPDurationControl ();

	// determine if we have Admin privileges
	g_bAuthAccess = TRUE;
	g_bReadOnly = FALSE;

	// get window position info
	sGetWindowPosFromRegistry (&iPosX, &iPosY, &iWidth, &iHeight);

    // create the main window
	g_hwndMain = CreateWindow (
				PGPNET_APPWINDOWCLASS, PGPNET_APPWINDOWTITLE, 
				WS_OVERLAPPEDWINDOW, 
				iPosX, iPosY, iWidth, iHeight, (HWND)NULL, 
				LoadMenu (g_hinst, MAKEINTRESOURCE(IDR_PGPNETMENU)), 
				hInstance, (LPVOID)NULL); 
  
	if (g_hwndMain) 
	{  
		haccelMain = 
			LoadAccelerators (g_hinst, MAKEINTRESOURCE (IDR_PNACCELERATORS));  

		while (GetMessage (&msg, (HWND)NULL, 0, 0)) 
		{
			if (!TranslateAccelerator (g_hwndMain, haccelMain, &msg)) 
			{
				if (!IsDialogMessage (g_hwndCurrentDlg, &msg))
				{ 
					TranslateMessage (&msg); 
					DispatchMessage (&msg); 
				}
			}
		} 
	}
 
	// clean out caches
	PGPclCloseLibrary ();

	// close down SDK
	PGPFreeContext (g_context);
	
	// close Winsock
	if (g_bWinsock)
		WSACleanup ();

	// required by OLE
	OleUninitialize ();

	return msg.wParam; 
}


