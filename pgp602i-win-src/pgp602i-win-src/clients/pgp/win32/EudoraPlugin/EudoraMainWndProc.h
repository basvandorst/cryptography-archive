/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	EudoraMainWndProc.h,v 1.4 1997/10/22 23:05:49 elrod Exp
____________________________________________________________________________*/

#ifndef Included_EudoraMainWndProc_h	/* [ */
#define Included_EudoraMainWndProc_h

LRESULT 
WINAPI 
EudoraMainWndProc(	HWND hwnd, 
					UINT msg, 
					WPARAM wParam, 
					LPARAM lParam);

LRESULT 
WINAPI 
EudoraStatusbarWndProc(	HWND hwnd, 
						UINT msg, 
						WPARAM wParam, 
						LPARAM lParam);

#endif /* ] Included_EudoraMainWndProc_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
