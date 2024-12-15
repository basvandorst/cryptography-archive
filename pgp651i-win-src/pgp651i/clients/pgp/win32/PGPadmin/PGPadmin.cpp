/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: PGPadmin.cpp,v 1.11 1999/04/01 01:20:14 dgal Exp $
____________________________________________________________________________*/

#include <windows.h>
#include "resource.h"
#include "CreateWiz.h"
#include "pgpKeys.h"
#include "PGPcl.h"

HINSTANCE g_hInstance = NULL;
HWND g_hCurrentDlgWnd = NULL;
BOOL g_bGotReloadMsg = FALSE;

static UINT g_ReloadKeyringMsg = 0;

LRESULT CALLBACK MainWndProc(HWND hwnd, 
							 UINT uMsg, 
							 WPARAM wParam, 
							 LPARAM lParam);

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
    UNREFERENCED_PARAMETER(lpCmdLine); 
 
    g_hInstance = hInstance;  /* save instance handle */ 

	/* Register "Reload Keyring" message */

	g_ReloadKeyringMsg = RegisterWindowMessage(RELOADKEYRINGMSG);

    /* Register the window class for the main window. */ 
 
    if (!hPrevInstance) { 
        wc.style = 0; 
        wc.lpfnWndProc = (WNDPROC) MainWndProc; 
        wc.cbClsExtra = 0; 
        wc.cbWndExtra = 0; 
        wc.hInstance = hInstance; 
        wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ADMIN)); 
        wc.hCursor = LoadCursor((HINSTANCE) NULL, 
            IDC_ARROW); 
        wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH); 
        wc.lpszMenuName =  MAKEINTRESOURCE(IDR_MENU); 
        wc.lpszClassName = "PGPAdminWizardClass"; 
 
        if (!RegisterClass(&wc)) 
            return FALSE; 
    } 
 
    /* Create the main window. */ 
 
    hwndMain = CreateWindow("PGPAdminWizardClass", "PGPAdminWizard", 
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 
        CW_USEDEFAULT, CW_USEDEFAULT, (HWND) NULL, 
        (HMENU) NULL, hInstance, (LPVOID) NULL); 
 
    /* 
     * If the main window cannot be created, terminate 
     * the application. 
     */ 
 
    if (!hwndMain) 
        return FALSE; 
 
    /* Show the window and paint its contents. */ 
 
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
//	SHORT vkHotKey;

	switch (uMsg)
	{
	case WM_CREATE:
		// Send the message to start the wizard as soon as possible
		PostMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDM_WIZARD, 0), 0);
		return 0;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDM_WIZARD:
			// Launch the wizard. When it ends, close the program
			// If the keyring reload message was received, restart
			// the wizard
			do
			{
				g_bGotReloadMsg = TRUE;
				CreateWiz(hwnd);
			}
			while (g_bGotReloadMsg == TRUE);
			PostMessage(hwnd, WM_CLOSE, 0, 0);
			return 0;

		case IDM_EXIT:
			PostMessage(hwnd, WM_CLOSE, 0, 0);
			return 0;
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	default:
		if (uMsg == g_ReloadKeyringMsg)
		{
			if (!g_bGotReloadMsg)
			{
				char szMsg[255];
				char szTitle[255];
				
				SendMessage(GetParent(g_hCurrentDlgWnd), PSM_SETFINISHTEXT, 
					0, (LPARAM) "Restart");
				SendMessage(GetParent(g_hCurrentDlgWnd), PSM_SETWIZBUTTONS, 
					0, PSWIZB_FINISH);
				
				g_bGotReloadMsg = TRUE;
				LoadString(g_hInstance, IDS_E_RELOADKEYRING, szMsg, 254);
				LoadString(g_hInstance, IDS_TITLE, szTitle, 254);
				MessageBox(hwnd, szMsg, szTitle, MB_ICONEXCLAMATION);
			}
				
			return 0;
		}
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
