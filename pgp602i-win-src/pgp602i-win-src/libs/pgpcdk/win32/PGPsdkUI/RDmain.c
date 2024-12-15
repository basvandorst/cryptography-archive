/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: RDmain.c,v 1.11.2.1.2.1 1998/11/12 03:24:30 heller Exp $
____________________________________________________________________________*/

#include "RDprecmp.h"

// newPGPRecipientDialog
//
// Set up our custom dialog class and hook routines

UINT newPGPRecipientDialog(PRECGBL prg)
{
	BOOL ReturnValue = FALSE;
	WNDCLASS wndclass;

	g_hinst=prg->g_hInst;

	memset(&wndclass,0x00,sizeof(WNDCLASS));

    wndclass.style = CS_HREDRAW | CS_VREDRAW;                     
    wndclass.lpfnWndProc = (WNDPROC)DefDlgProc; 
    wndclass.cbClsExtra = 0;              
    wndclass.cbWndExtra = DLGWINDOWEXTRA;              
    wndclass.hInstance = g_hinst;       
    wndclass.hIcon = LoadIcon (g_hinst, MAKEINTRESOURCE(IDI_DLGICON));
    wndclass.hCursor = LoadCursor (NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1); 
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = "PGPrecipClass";

	RegisterClass (&wndclass);

	ReturnValue = DialogBoxParam(g_hinst, 
		MAKEINTRESOURCE( IDD_RECIPIENTDLG ),
		prg->mHwndParent, 
		(DLGPROC) RecipientDlgProc, 
		(LPARAM) prg);
 
    FreeLinkedLists(prg->gUserLinkedList);

    return ReturnValue;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
