/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.	

	$Id: Search.c,v 1.9 1998/08/11 15:20:15 pbj Exp $
____________________________________________________________________________*/

// System Headers 
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>


// PGPsdk Headers
#include "pgpKeys.h"
#include "pgpErrors.h"
#include "pgpKeyServer.h"
#include "pgpKeyServerPrefs.h"
#include "pgpTLS.h"
#include "pgpBuildFlags.h"
#include "pgpUtilities.h"

// Project Headers
#include "Search.h"
#include "SearchProc.h"
#include "resource.h"

BOOL 
InitSearch()
{
	BOOL ReturnValue = TRUE;

	InitCommonControls();

	return ReturnValue;
}

void 
CloseSearch()
{
	return;
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
