/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: Taskbar.h,v 1.3 1999/03/10 03:03:35 heller Exp $
____________________________________________________________________________*/
#ifndef Included_TASKBAR_h	/* [ */
#define Included_TASKBAR_h

BOOL TaskbarAddIcon(HWND hwnd, UINT uID, HICON hicon, 
					LPSTR lpszTip, UINT cbMsg);

BOOL TaskbarDeleteIcon(HWND hwnd, UINT uID);

#endif /* ] Included_TASKBAR_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
