/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: proc.h,v 1.3 1999/03/10 03:00:36 heller Exp $
____________________________________________________________________________*/
#ifndef Included_PROC_h	/* [ */
#define Included_PROC_h

LRESULT CALLBACK HiddenWndProc (HWND hwnd, UINT iMsg, 
					WPARAM wParam, LPARAM lParam);

LRESULT WINAPI MyDropFilesProc(HWND hwnd, UINT msg, 
					WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK PGPtoolsWndProc (HWND hwnd, UINT iMsg, 
					WPARAM wParam, LPARAM lParam);

#endif /* ] Included_PROC_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
