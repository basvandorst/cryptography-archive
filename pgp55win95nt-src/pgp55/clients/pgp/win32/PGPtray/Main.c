/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: Main.c,v 1.14 1997/09/21 14:19:05 wjb Exp $
____________________________________________________________________________*/
#include "precomp.h"

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

		if(!InitPGPsc(NULL,&PGPsc))
			return TRUE;

		hwnd=TrayWindow();

		ListHead=0;

		AddToFileList(&ListHead,lpszCmdLine);
		DecryptFileList(hwnd, PGPsc, ListHead);

		UninitPGPsc(hwnd,PGPsc);

		return 0;
	}

	if(FindWindow(szAppName,szAppName)) 
	{
		return(0);
	}

	if(!InitPGPsc(NULL,&PGPsc))
			return TRUE;

	hwndTray=TrayWindow();

	hIconKey = LoadIcon(g_hinst, 
			MAKEINTRESOURCE(IDI_TASKBAR));
	TaskbarAddIcon(hwndTray,1, hIconKey, "PGPtray", 
			WM_TASKAREA_MESSAGE);

	while( GetMessage( &msg, NULL, 0, 0 ) )            
	{
		if( IsDialogMessage( hwndTray, &msg ) )     
					//  Process Tab messages and such
			continue;

		TranslateMessage( &msg );                      
		DispatchMessage( &msg );
	}

	TaskbarDeleteIcon(hwndTray, 1);

	UninitPGPsc(NULL,PGPsc);

	return 0;
}

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
