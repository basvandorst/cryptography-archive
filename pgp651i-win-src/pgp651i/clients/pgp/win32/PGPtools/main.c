/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: main.c,v 1.24 1999/04/13 17:29:55 wjb Exp $
____________________________________________________________________________*/
#include "precomp.h"

#ifdef _WIN32 
static char szAppName[] = "PGPtools" ;
#else
static char szAppName[] = "PGPtls16" ;
#endif

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
		    LPSTR szCmdLine, int iCmdShow)
{
	MSG         msg ;
	HWND        hwnd ;
	WNDCLASS    wndclass ; 
	int WinHeight;
	HACCEL hAccel;

	g_hinst = hInstance ;

	LoadString (g_hinst, IDS_SZAPP, szApp, sizeof(szApp));

	if(*szCmdLine)
	{
		FILELIST *ListHead;

		if(!InitPGPsc(NULL,&PGPsc,&PGPtls))
			return TRUE;

		wndclass.style			= 0;
		wndclass.lpfnWndProc	= (WNDPROC)HiddenWndProc;
		wndclass.cbClsExtra		= 0;
		wndclass.cbWndExtra		= 0;
		wndclass.hInstance		= hInstance;
		wndclass.hIcon			= 0;
		wndclass.hCursor		= 0;
		wndclass.hbrBackground	= 0;
		wndclass.lpszMenuName	= 0;
		wndclass.lpszClassName	= "PGPtools_Hidden_Window";

		RegisterClass(&wndclass);

		hwnd = CreateWindow("PGPtools_Hidden_Window", 
					"PGPtools_Hidden_Window", WS_OVERLAPPEDWINDOW, 
					CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
					CW_USEDEFAULT, NULL, NULL, hInstance, NULL );

		ShowWindow(hwnd, SW_HIDE);
		UpdateWindow(hwnd);

		// Free space wipe from command line
		if(!strncmp (szCmdLine, "/w",2))
		{
			BOOL RetVal;

			RetVal=FreeSpaceWipeCmdLine(hwnd,PGPsc,&szCmdLine[3]);
			UninitPGPsc(hwnd,PGPsc,PGPtls);

			if(RetVal)
				return 0; // SAGE 0 Completed Successfully

			return 6; // SAGE 6 Unsuccessful; error writing to drive
		}

		ListHead=CmdLineToFileList(szCmdLine);

		DecryptFileList(hwnd,szApp,PGPsc,PGPtls,ListHead);

		UninitPGPsc(hwnd,PGPsc,PGPtls);
		return TRUE;
	}
 
	if(WindowExists(szAppName,szAppName)) 
		return TRUE;
    
	if(!InitPGPsc(NULL,&PGPsc,&PGPtls))
		return TRUE;

	wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
	wndclass.lpfnWndProc   = (WNDPROC) PGPtoolsWndProc ;
	wndclass.cbClsExtra    = 0 ;
	wndclass.cbWndExtra    = 0 ;
	wndclass.hInstance     = hInstance ;
	wndclass.hIcon         = LoadIcon (g_hinst,
								MAKEINTRESOURCE(IDI_TOOLSICO)) ;
	wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
	wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wndclass.lpszMenuName  = NULL;//MAKEINTRESOURCE(IDR_PGPTOOLSMENU);
	wndclass.lpszClassName = szAppName ;

	RegisterClass(&wndclass) ;
	
	hAccel=LoadAccelerators(hInstance,MAKEINTRESOURCE(IDR_ACCELERATOR1));

	WinHeight=120;

	hwnd = CreateWindow(  szAppName, szAppName,
                          WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | 
						  WS_SIZEBOX| WS_SYSMENU,
	                      CW_USEDEFAULT, CW_USEDEFAULT,
	                      450,       
						  WinHeight,
                          NULL,NULL, hInstance, NULL) ;

	ShowWindow (hwnd, iCmdShow) ;
	UpdateWindow (hwnd); 

	while (GetMessage (&msg, NULL, 0, 0))
	{
		if(!TranslateAccelerator(hwnd,hAccel,&msg))
		{
			if(!IsDialogMessage( hwnd, &msg ))
			{
				TranslateMessage (&msg) ;
				DispatchMessage (&msg) ;
			}
		}
	}
     
	UninitPGPsc(hwnd,PGPsc,PGPtls);

	return msg.wParam ;
}     

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

