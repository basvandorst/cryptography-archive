/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: Search.c,v 1.4 1997/07/17 02:02:55 elrod Exp $
____________________________________________________________________________*/
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "Search.h"
#include "SearchProc.h"
#include "pgpKeyServer.h"
#include "resource.h"



BOOL 
InitSearch()
{
	BOOL ReturnValue = TRUE;
	InitCommonControls();
	PGPKeyServerInit();

	return ReturnValue;
}

void 
CloseSearch()
{
	PGPKeyServerCleanup();
}

HWND 
CreateSearch(HINSTANCE hInstance, 
			 HWND hwndParent)
{
	HWND hwndSearch = NULL;
	RECT rectParent;

	GetClientRect(hwndParent, &rectParent);

	hwndSearch = CreateDialog(	hInstance, 
								MAKEINTRESOURCE(IDD_SEARCH), 
								hwndParent, 
								(DLGPROC)SearchProc);
	
	return hwndSearch;
}
