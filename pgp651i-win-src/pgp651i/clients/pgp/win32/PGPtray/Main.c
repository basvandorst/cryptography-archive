/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: Main.c,v 1.25 1999/05/11 02:41:55 wjb Exp $
____________________________________________________________________________*/
#include "precomp.h"
#include "pgpTrayIPC.h"
#include "PThotkey.h"
#include "..\include\PGPhk.h"

HWND TrayWindow(void)
{
	HWND hwnd;
	WNDCLASS wc;

	// Register the Server Window Class

	wc.style			= 0;
	wc.lpfnWndProc		= EnclyptorProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= g_hinst;
	wc.hIcon			= 0;
	wc.hCursor			= 0;
	wc.hbrBackground	= 0;
	wc.lpszMenuName		= 0;
	wc.lpszClassName	= PGPTRAY_WINDOWNAME;

	RegisterClass(&wc);

	hwnd = CreateWindow(
		PGPTRAY_WINDOWNAME, PGPTRAY_WINDOWNAME, WS_OVERLAPPEDWINDOW, 
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, NULL, NULL, g_hinst, NULL );

	ShowWindow(hwnd, SW_HIDE);
	UpdateWindow(hwnd);

	return hwnd;
}

int WINAPI WinMain (HINSTANCE hInstance,HINSTANCE hinstPrev, 
					LPSTR lpszCmdLine, int nCmdShow)
{
	HWND hwnd;
	MSG msg;
	OSVERSIONINFO osid;
	BOOL bIsNT;

	osid.dwOSVersionInfoSize = sizeof (osid);
	GetVersionEx (&osid);   
	bIsNT=(osid.dwPlatformId == VER_PLATFORM_WIN32_NT);

	g_hinst = hInstance;

	LoadString (g_hinst, IDS_SZAPP, szApp, sizeof(szApp));

	if(*lpszCmdLine)
	{
		FILELIST *ListHead;

		if(!InitPGPsc(NULL,&PGPsc,&PGPtls))
			return TRUE;

		hwnd=TrayWindow();

		ListHead=CmdLineToFileList(lpszCmdLine);

		DecryptFileList(hwnd, szApp, PGPsc,PGPtls, ListHead);

		UninitPGPsc(hwnd,PGPsc,PGPtls);

		return 0;
	}

	if(FindWindow(PGPTRAY_WINDOWNAME,PGPTRAY_WINDOWNAME)) 
	{
		return(0);
	}

	if(!InitPGPsc(NULL,&PGPsc,&PGPtls))
			return TRUE;

	hwndTray=TrayWindow();

	hIconKey = LoadIcon(g_hinst, 
			MAKEINTRESOURCE(IDI_TRAYICON));
	TaskbarAddIcon(hwndTray, TASKBAR_TRAY_ID, hIconKey, "PGPtray", 
			WM_TASKAREA_MESSAGE);

	PTHotKeyInit ();

	SetHook(); // Our CBT hook for current focus

	bIsNT=FALSE;

	if(!bIsNT)
	{
		g_hwnd=hwndTray;
		InitializeWipeOnDelete(hwndTray);
	}

	while( GetMessage( &msg, NULL, 0, 0 ) )            
	{
		if( IsDialogMessage( hwndTray, &msg ) )     
					//  Process Tab messages and such
			continue;

		TranslateMessage( &msg );                      
		DispatchMessage( &msg );
	}

	if(!bIsNT)
	{
		UninitializeWipeOnDelete();
	}

	DeleteHook(); // Our CBT hook for current focus

	TaskbarDeleteIcon(hwndTray, TASKBAR_TRAY_ID);

	UninitPGPsc(NULL,PGPsc,PGPtls);

	return 0;
}

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
