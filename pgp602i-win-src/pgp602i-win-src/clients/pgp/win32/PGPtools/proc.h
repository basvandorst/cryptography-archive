/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: proc.h,v 1.2.16.1 1998/11/12 03:13:47 heller Exp $
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
