/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PKTool.c - toolbar routines
	

	$Id: PKTool.c,v 1.11 1999/01/14 23:08:04 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"	/* or pgpConfig.h in the CDK */

// project header files
#include "pgpkeysx.h"

// constant definitions
#define BITMAP_CX			16
#define BITMAP_CY			16

#define IDX_TBNEWKEY		0
#define IDX_SEPARATOR1		1
#define IDX_TBREVOKKEY		2
#define IDX_TBSIGNKEY		3
#define IDX_TBDELETE		4
#define IDX_SEPARATOR2		5
#define IDX_TBSEARCH		6
#define IDX_TBSENDKEY		7
#define IDX_TBUPDATEKEY		8
#define IDX_TBPROPERTIES	9
#define IDX_SEPARATOR3		10
#define IDX_TBIMPORT		11
#define IDX_TBEXPORT		12
#define NUM_TBBUTTONS		13

// local globals
static BOOL bIsSeparator[NUM_TBBUTTONS] = {0,1,0,0,0,1,0,0,0,0,1,0,0};

// external global variables
extern HINSTANCE	g_hInst;
extern HWND			g_hWndMain;
extern BOOL			g_bReadOnly;
extern BOOL			g_bKeyGenEnabled;

//	____________________________________
//
//	Create new toolbar for main window

HWND
PKCreateToolbar (HWND hwndParent)
{
	HWND		hwndToolbar;
	HIMAGELIST	himl;
	TBBUTTON	tbArray[NUM_TBBUTTONS];
	HDC			hdc;
	INT			i, iNumBits, iBitmap;
	HBITMAP		hbmp;

	// create the toolbar control.
	hwndToolbar = CreateWindowEx(
					WS_EX_TOOLWINDOW,
					TOOLBARCLASSNAME,
					NULL,
					WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | 
					WS_CLIPSIBLINGS | CCS_NODIVIDER |
					CCS_NOPARENTALIGN | CCS_NORESIZE | 
					TBSTYLE_FLAT | TBSTYLE_TOOLTIPS, 
					TOOLBARXOFFSET,TOOLBARYOFFSET,0,0, 
					hwndParent, (HMENU)IDC_TOOLBAR,
					g_hInst, NULL); 


	// sets the size of the TBBUTTON structure.
	SendMessage (hwndToolbar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);

	// set the bitmap size.
	SendMessage	(hwndToolbar, TB_SETBITMAPSIZE,
			   0, (LPARAM)MAKELONG (BITMAP_CX, BITMAP_CY));

	// set the button size.
	SendMessage	(hwndToolbar, TB_SETBUTTONSIZE,
			   0, (LPARAM)MAKELONG (BITMAP_CX+8, BITMAP_CY+2));

	// get imagelist on basis of display capabilities
	hdc = GetDC (NULL);		// DC for desktop
	iNumBits = GetDeviceCaps (hdc, BITSPIXEL) * GetDeviceCaps (hdc, PLANES);
	ReleaseDC (NULL, hdc);

	if (iNumBits <= 8) {
		himl = ImageList_Create (BITMAP_CX, BITMAP_CY, ILC_COLOR|ILC_MASK, 
										NUM_TBBUTTONS, 0); 
		hbmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_TOOLBAR4BIT));
		ImageList_AddMasked (himl, hbmp, RGB(255, 0, 255));
		DeleteObject (hbmp);
	}
	else {
		himl = ImageList_Create (BITMAP_CX, BITMAP_CY, ILC_COLOR24|ILC_MASK, 
										NUM_TBBUTTONS, 0); 
		hbmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_TOOLBAR24BIT));
		ImageList_AddMasked (himl, hbmp, RGB(255, 0, 255));
		DeleteObject (hbmp);
	}
	SendMessage (hwndToolbar, TB_SETIMAGELIST, 0, (LPARAM)himl);
   
	// Loop to fill the array of TBBUTTON structures.
	iBitmap = 0;
	for (i=0; i<NUM_TBBUTTONS; i++) {
		tbArray[i].iBitmap   = iBitmap;
		tbArray[i].idCommand = 0;
		tbArray[i].fsStyle   = TBSTYLE_BUTTON;
		tbArray[i].dwData    = 0;
		tbArray[i].iString   = iBitmap;
		if (bIsSeparator[i]) {
			tbArray[i].fsState = 0;
			tbArray[i].fsStyle = TBSTYLE_SEP;
		}
		else {
			tbArray[i].fsState = TBSTATE_ENABLED;
			tbArray[i].fsStyle = TBSTYLE_BUTTON;
			iBitmap++;
		}
	}

	tbArray[IDX_TBNEWKEY].idCommand		= IDM_CREATEKEY;
	tbArray[IDX_TBREVOKKEY].idCommand	= IDM_REVOKEKEY;
	tbArray[IDX_TBSIGNKEY].idCommand	= IDM_CERTIFYKEY;
	tbArray[IDX_TBDELETE].idCommand		= IDM_DELETEKEY;
	tbArray[IDX_TBSEARCH].idCommand		= IDM_SEARCH;
	tbArray[IDX_TBUPDATEKEY].idCommand	= IDM_GETFROMSERVER;
	tbArray[IDX_TBSENDKEY].idCommand	= IDM_TOOLBARSENDTOSERVER;
	tbArray[IDX_TBPROPERTIES].idCommand	= IDM_PROPERTIES;
	tbArray[IDX_TBIMPORT].idCommand		= IDM_IMPORTKEYS;
	tbArray[IDX_TBEXPORT].idCommand		= IDM_EXPORTKEYS;

	// add the buttons
	SendMessage (hwndToolbar,
			   TB_ADDBUTTONS, (UINT)NUM_TBBUTTONS, (LPARAM)tbArray);

	return hwndToolbar;
}

//	____________________________________
//
//	destroy toolbar

VOID
PKDestroyToolbar (HWND hwndToolbar) {

	HIMAGELIST	himl;

	himl = (HIMAGELIST)SendMessage (hwndToolbar, TB_GETIMAGELIST, 0, 0);
	if (himl)
		ImageList_Destroy (himl);

}

//	____________________________________
//
//	get rect of send to server button

VOID
PKGetSendToServerButtonRect (HWND hwndToolbar, LPRECT lprect) {

	SendMessage (hwndToolbar, 
			TB_GETITEMRECT, IDX_TBSENDKEY, (LPARAM)lprect);
}

//	____________________________________
//
//	get toolbar tooltip text

VOID
PKGetToolbarTooltipText (LPTOOLTIPTEXT lpttt) {

	lpttt->hinst = g_hInst; 

	// Specify the resource identifier of the descriptive 
	// text for the given button. 
	switch (lpttt->hdr.idFrom) { 
		case IDM_CREATEKEY: 
			lpttt->lpszText = MAKEINTRESOURCE(IDS_TOOLTIPNEWKEY); 
			break; 
		case IDM_REVOKEKEY: 
			lpttt->lpszText = MAKEINTRESOURCE(IDS_TOOLTIPREVOKE); 
			break; 
		case IDM_CERTIFYKEY: 
			lpttt->lpszText = MAKEINTRESOURCE(IDS_TOOLTIPSIGN); 
			break; 
		case IDM_DELETEKEY: 
			lpttt->lpszText = MAKEINTRESOURCE(IDS_TOOLTIPDELETE); 
			break; 
		case IDM_SEARCH: 
			lpttt->lpszText = MAKEINTRESOURCE(IDS_TOOLTIPSEARCH); 
			break; 
		case IDM_TOOLBARSENDTOSERVER: 
			lpttt->lpszText = MAKEINTRESOURCE(IDS_TOOLTIPSENDTOSERVER); 
			break; 
		case IDM_GETFROMSERVER: 
			lpttt->lpszText = MAKEINTRESOURCE(IDS_TOOLTIPUPDATE); 
			break; 
		case IDM_PROPERTIES: 
			lpttt->lpszText = MAKEINTRESOURCE(IDS_TOOLTIPPROPERTIES); 
			break; 
		case IDM_IMPORTKEYS: 
			lpttt->lpszText = MAKEINTRESOURCE(IDS_TOOLTIPIMPORT); 
			break; 
		case IDM_EXPORTKEYS: 
			lpttt->lpszText = MAKEINTRESOURCE(IDS_TOOLTIPEXPORT); 
			break; 
	} 

}

//	____________________________________
//
//	set toolbar buttons states (enabled/grayed)

VOID
PKSetToolbarButtonStates (PPGPKEYSSTRUCT ppks) {

	TBBUTTON	tbb;
	UINT		uState;
	INT			i;


	// groups window has focus
	if (ppks->bGroupsFocused) {
		for (i=0; i<NUM_TBBUTTONS; i++) {

			if (bIsSeparator[i]) 
				continue;

			SendMessage (ppks->hWndToolBar, TB_GETBUTTON, i, (LPARAM)&tbb);

			uState = TBSTATE_INDETERMINATE;

			switch (tbb.idCommand) {
			case IDM_CREATEKEY: 
				if (!g_bReadOnly && g_bKeyGenEnabled) 
					uState = TBSTATE_ENABLED;
				break; 

			case IDM_REVOKEKEY: 
				break; 

			case IDM_CERTIFYKEY: 
				break; 

			case IDM_DELETEKEY: 
				if (PGPgmIsActionEnabled (ppks->hGM, GM_DELETE))
					uState = TBSTATE_ENABLED;
				break; 

			case IDM_SEARCH: 
				uState = TBSTATE_ENABLED;
				break;
				
			case IDM_TOOLBARSENDTOSERVER:
				break; 

			case IDM_GETFROMSERVER: 
				break; 

			case IDM_PROPERTIES: 
				if (PGPgmIsActionEnabled (ppks->hGM, GM_LOCATEKEYS))
					uState = TBSTATE_ENABLED;
				break; 

			case IDM_IMPORTKEYS: 
				if (PGPkmIsActionEnabled (ppks->hKM, KM_IMPORT))
					uState = TBSTATE_ENABLED;
				break; 

			case IDM_EXPORTKEYS: 
				break; 
			}

			if (tbb.idCommand)
				SendMessage (ppks->hWndToolBar, TB_SETSTATE, tbb.idCommand, 
							MAKELPARAM (uState, 0));
		}

	}

	// main key window has focus
	else {

		for (i=0; i<NUM_TBBUTTONS; i++) {

			if (bIsSeparator[i]) 
				continue;

			SendMessage (ppks->hWndToolBar, TB_GETBUTTON, i, (LPARAM)&tbb);

			uState = TBSTATE_INDETERMINATE;

			switch (tbb.idCommand) {
			case IDM_CREATEKEY: 
				if (!g_bReadOnly && g_bKeyGenEnabled) 
					uState = TBSTATE_ENABLED;
				break; 

			case IDM_REVOKEKEY: 
				if (PGPkmIsActionEnabled (ppks->hKM, KM_REVOKE))
					uState = TBSTATE_ENABLED;
				break; 

			case IDM_CERTIFYKEY: 
				if (PGPkmIsActionEnabled (ppks->hKM, KM_CERTIFY))
					uState = TBSTATE_ENABLED;
				break; 

			case IDM_DELETEKEY: 
				if (PGPkmIsActionEnabled (ppks->hKM, KM_DELETE))
					uState = TBSTATE_ENABLED;
				break; 

			case IDM_SEARCH: 
				uState = TBSTATE_ENABLED;
				break;
				
			case IDM_TOOLBARSENDTOSERVER:
				if (PGPkmIsActionEnabled (ppks->hKM, KM_SENDTOSERVER))
					uState = TBSTATE_ENABLED;
				break; 

			case IDM_GETFROMSERVER: 
				if (PGPkmIsActionEnabled (ppks->hKM, KM_GETFROMSERVER))
					uState = TBSTATE_ENABLED;
				break; 

			case IDM_PROPERTIES: 
				if (PGPkmIsActionEnabled (ppks->hKM, KM_PROPERTIES))
					uState = TBSTATE_ENABLED;
				break; 

			case IDM_IMPORTKEYS: 
				if (PGPkmIsActionEnabled (ppks->hKM, KM_IMPORT))
					uState = TBSTATE_ENABLED;
				break; 

			case IDM_EXPORTKEYS: 
				if (PGPkmIsActionEnabled (ppks->hKM, KM_EXPORT))
					uState = TBSTATE_ENABLED;
				break; 
			}

			if (tbb.idCommand)
				SendMessage (ppks->hWndToolBar, TB_SETSTATE, tbb.idCommand, 
							MAKELPARAM (uState, 0));
		}
	}
}


