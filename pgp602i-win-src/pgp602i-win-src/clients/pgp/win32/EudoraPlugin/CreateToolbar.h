/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	CreateToolbar.h,v 1.3 1997/10/22 23:05:47 elrod Exp
____________________________________________________________________________*/

#ifndef Included_CreateToolbar_h	/* [ */
#define Included_CreateToolbar_h

#define IDC_READTOOLBAR	WM_USER + 1020
#define IDC_SENDTOOLBAR	WM_USER + 1021


HWND CreateToolbarRead(HWND hwndParent);
HWND CreateToolbarSend(HWND hwndParent);
void CalculateToolbarRect(HWND hwndToolbar, RECT* rect);

#endif /* ] Included_CreateToolbar_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
