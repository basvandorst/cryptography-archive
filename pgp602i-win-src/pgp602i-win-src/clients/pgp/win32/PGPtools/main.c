/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: main.c,v 1.18.2.1.2.1 1998/11/12 03:13:44 heller Exp $
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

		ListHead=CmdLineToFileList(szCmdLine);

		DecryptFileList(hwnd,PGPsc,PGPtls,ListHead);

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
			TranslateMessage (&msg) ;
			DispatchMessage (&msg) ;
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

