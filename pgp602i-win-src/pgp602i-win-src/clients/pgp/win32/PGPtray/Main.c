/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: Main.c,v 1.17.2.1.2.1 1998/11/12 03:13:50 heller Exp $
____________________________________________________________________________*/
#include "precomp.h"
#include "..\include\PGPhk.h"

static char szAppName[]="PGPtray_Hidden_Window";

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
	wc.lpszClassName	= szAppName;

	RegisterClass(&wc);

	hwnd = CreateWindow(
		szAppName, szAppName, WS_OVERLAPPEDWINDOW, 
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

	g_hinst = hInstance;

	if(*lpszCmdLine)
	{
		FILELIST *ListHead;

		if(!InitPGPsc(NULL,&PGPsc,&PGPtls))
			return TRUE;

		hwnd=TrayWindow();

		ListHead=CmdLineToFileList(lpszCmdLine);

		DecryptFileList(hwnd, PGPsc,PGPtls, ListHead);

		UninitPGPsc(hwnd,PGPsc,PGPtls);

		return 0;
	}

	if(FindWindow(szAppName,szAppName)) 
	{
		return(0);
	}

	if(!InitPGPsc(NULL,&PGPsc,&PGPtls))
			return TRUE;

	hwndTray=TrayWindow();

	hIconKey = LoadIcon(g_hinst, 
			MAKEINTRESOURCE(IDI_TASKBAR));
	TaskbarAddIcon(hwndTray,1, hIconKey, "PGPtray", 
			WM_TASKAREA_MESSAGE);

	SetHook(); // Our CBT hook for current focus

	while( GetMessage( &msg, NULL, 0, 0 ) )            
	{
		if( IsDialogMessage( hwndTray, &msg ) )     
					//  Process Tab messages and such
			continue;

		TranslateMessage( &msg );                      
		DispatchMessage( &msg );
	}

	DeleteHook(); // Our CBT hook for current focus

	TaskbarDeleteIcon(hwndTray, 1);

	UninitPGPsc(NULL,PGPsc,PGPtls);

	{
		HWND test;

		test=ReadCurrentFocus();
	}

	return 0;
}

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
