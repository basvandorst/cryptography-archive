/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */
//:TreeList.c - handle "TreeList" window class
//
//	$Id: TreeList.c,v 1.4 1997/05/19 14:36:26 pbj Exp $
//

#include <windows.h>
#include <commctrl.h>
#include "TreeListVer.h"
#include "TLintern.h"


//----------------------------------------------------|
//  Load and initialize control

void WINAPI InitTreeListControl (void) {

	WNDCLASS  wc;
	
	InitCommonControls ();

	// register new window classes for menus
	wc.style = CS_DBLCLKS | CS_GLOBALCLASS;	    // Class style(s).
	wc.lpfnWndProc = (WNDPROC) TreeListMsgProc;
	wc.cbClsExtra = 0;	         // No per-class extra data.
	wc.cbWndExtra = sizeof (TLWndData*);		// pointer to extra data
												//		structure
	wc.hInstance = 0;	
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor (NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW+1); // Background color
	wc.lpszMenuName = NULL;						// No menu
	wc.lpszClassName = "TreeListCtrl32";		// Name used in CreateWindow
	RegisterClass (&wc);

}
