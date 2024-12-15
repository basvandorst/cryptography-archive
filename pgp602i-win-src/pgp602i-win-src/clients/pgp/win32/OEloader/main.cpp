/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: main.cpp,v 1.7.8.1 1998/11/12 03:12:41 heller Exp $
____________________________________________________________________________*/

#include <windows.h>
#include "resource.h"

#define OE_REGKEY	"SOFTWARE\\Microsoft\\Outlook Express"

HINSTANCE g_hInstance = NULL;
UINT WM_PGP_KILL_OELOADER = 0;

/*
extern "C" {
__declspec(dllimport) HookOutlookExpressPlugin(DWORD dwThreadId);
};
*/

void (*HookOutlookExpressPlugin)(DWORD);

LRESULT CALLBACK MainWndProc(HWND hwnd, 
							 UINT uMsg, 
							 WPARAM wParam, 
							 LPARAM lParam);

BOOL GetOELocation(char *szOutlookExpress);


int WINAPI WinMain(
    HINSTANCE hInstance,	// handle to current instance
    HINSTANCE hPrevInstance,	// handle to previous instance
    LPSTR lpCmdLine,	// pointer to command line
    int nCmdShow 	// show state of window
   )
{
    MSG msg; 
    WNDCLASS wc; 
	HWND hwndMain;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	HANDLE hOutlookExpress;
	HANDLE hPGP60oe;
	HWND hwndOE;
	DWORD dwProcessId;
	DWORD dwThreadId;
	DWORD dwWait;
	BOOL bMainWindow = TRUE;
	char szOutlookExpress[MAX_PATH*2];
 
    g_hInstance = hInstance;  /* save instance handle */ 

	/* Attempt to load the Outlook Express plug-in DLL */

	hPGP60oe = LoadLibrary("PGP60oe.dll");
	if (hPGP60oe == NULL)
	{
		char szMsg[255];
		char szTitle[255];

		HookOutlookExpressPlugin = NULL;
		LoadString(hInstance, IDS_NOPLUGIN, szMsg, 254);
		LoadString(hInstance, IDS_TITLE, szTitle, 254);
		MessageBox(NULL, szMsg, szTitle, MB_ICONEXCLAMATION);
	}
	else
		HookOutlookExpressPlugin = (void (*)(DWORD)) 
									GetProcAddress(hPGP60oe, 
										"HookOutlookExpressPlugin");

	/* Launch the real Outlook Express process */

	if (!GetOELocation(szOutlookExpress))
		return FALSE;

	si.cb = sizeof(STARTUPINFO);
	si.lpReserved = NULL;
	si.lpDesktop = NULL;
	si.lpTitle = NULL;
	si.dwFlags = 0;
	si.cbReserved2 = NULL;
	si.lpReserved2 = NULL;

	strcat(szOutlookExpress, " ");
	strcat(szOutlookExpress, lpCmdLine);

	CreateProcess(NULL, szOutlookExpress, NULL, NULL, FALSE, 
		NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi);

	if (HookOutlookExpressPlugin == NULL)
		return FALSE;

	if (strstr(lpCmdLine, "/reg") || strstr(lpCmdLine, "/unreg"))
		return FALSE;

	WM_PGP_KILL_OELOADER = RegisterWindowMessage("PGP Kill OE Loader");

	hOutlookExpress = OpenProcess(STANDARD_RIGHTS_REQUIRED, FALSE, 
						pi.dwProcessId);
	
	WaitForInputIdle(hOutlookExpress, INFINITE);
	CloseHandle(hOutlookExpress);

	if (strstr(lpCmdLine, "/mailurl") || strstr(lpCmdLine, "/eml") ||
		strstr(lpCmdLine, "/nws"))
		bMainWindow = FALSE;

	if (bMainWindow)
	{
		hwndOE = NULL;
		dwWait = GetTickCount();
		do
		{
			hwndOE = FindWindowEx(NULL, hwndOE, "ThorBrowserWndClass", NULL);
			dwThreadId = GetWindowThreadProcessId(hwndOE, &dwProcessId);
		}
		while (	(dwProcessId != pi.dwProcessId) && 
				((GetTickCount() - dwWait) < 30000));
		
		HookOutlookExpressPlugin(dwThreadId);
	}

	if ((GetTickCount() - dwWait) >= 30000)
		return FALSE;
	
	hwndOE = NULL;
	dwWait = GetTickCount();
	do
	{
		hwndOE = FindWindowEx(NULL, hwndOE, "AthenaInitWindow", NULL);
		dwThreadId = GetWindowThreadProcessId(hwndOE, &dwProcessId);
	}
	while (	(dwProcessId != pi.dwProcessId) && 
			((GetTickCount() - dwWait) < 30000));
	
	HookOutlookExpressPlugin(dwThreadId);
	
    /* Register the window class for the main window. */ 
 
    if (!hPrevInstance) { 
        wc.style = 0; 
        wc.lpfnWndProc = (WNDPROC) MainWndProc; 
        wc.cbClsExtra = 0; 
        wc.cbWndExtra = 0; 
        wc.hInstance = hInstance; 
        wc.hIcon = LoadIcon((HINSTANCE) NULL, IDI_APPLICATION); 
        wc.hCursor = LoadCursor((HINSTANCE) NULL, IDC_ARROW); 
        wc.hbrBackground = GetStockObject(WHITE_BRUSH); 
        wc.lpszMenuName =  NULL; 
        wc.lpszClassName = "PGPOEloaderClass"; 
 
        if (!RegisterClass(&wc)) 
            return FALSE; 
    } 

    /* Create the main window. */ 
 
    hwndMain = CreateWindow("PGPOEloaderClass", "", 
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 
        CW_USEDEFAULT, CW_USEDEFAULT, (HWND) NULL, 
        (HMENU) NULL, hInstance, (LPVOID) NULL); 
 
    /* 
     * If the main window cannot be created, terminate 
     * the application. 
     */ 
 
    if (!hwndMain) 
        return FALSE; 
 
    /* Hide the window and paint its contents. */ 
 
    ShowWindow(hwndMain, SW_HIDE); 
    UpdateWindow(hwndMain); 
 
    /* Start the message loop. */ 
 
    while (GetMessage(&msg, (HWND) NULL, 0, 0)) 
	{
		TranslateMessage(&msg); 
		DispatchMessage(&msg); 
    } 
 
    /* Return the exit code to Windows. */ 
 
    return msg.wParam; 
}


LRESULT CALLBACK MainWndProc(HWND hwnd, 
							 UINT uMsg, 
							 WPARAM wParam, 
							 LPARAM lParam)
{
	if (uMsg == WM_PGP_KILL_OELOADER)
	{
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


BOOL GetOELocation(char *szOutlookExpress)
{
	HKEY		hKey;
	LONG		lResult;
	DWORD		dwValueType, dwSize;
	BOOL		bSuccess = FALSE;
	char		szMsg[255];
	char		szTitle[255];

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, OE_REGKEY, 0, KEY_READ, &hKey);

	if (lResult == ERROR_SUCCESS) 
	{
		dwSize = MAX_PATH;
		lResult = RegQueryValueEx(hKey, "InstallRoot", 0, &dwValueType, 
					(LPBYTE)szOutlookExpress, &dwSize);
		RegCloseKey (hKey);
		if (lResult == ERROR_SUCCESS) 
		{
			strcat(szOutlookExpress, "\\PGPmsimn.exe");
			if (GetFileAttributes(szOutlookExpress) == 0xFFFFFFFF)
			{
				LoadString(g_hInstance, IDS_BADINSTALL, szMsg, 254);
				LoadString(g_hInstance, IDS_TITLE, szTitle, 254);
				MessageBox(NULL, szMsg, szTitle, MB_ICONEXCLAMATION);
			}
			else
				bSuccess = TRUE;
		}
		else
		{
			LoadString(g_hInstance, IDS_NOMSIMN, szMsg, 254);
			LoadString(g_hInstance, IDS_TITLE, szTitle, 254);
			MessageBox(NULL, szMsg, szTitle, MB_ICONEXCLAMATION);
		}
	}
	else
	{
		LoadString(g_hInstance, IDS_NOMSIMN, szMsg, 254);
		LoadString(g_hInstance, IDS_TITLE, szTitle, 254);
		MessageBox(NULL, szMsg, szTitle, MB_ICONEXCLAMATION);
	}

	return bSuccess;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
