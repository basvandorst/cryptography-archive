/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	TimeDateWndProc.h - header for timedate control procs
	

	$Id: TimeDateWndProc.h,v 1.2 1997/08/28 10:07:22 elrod Exp $
____________________________________________________________________________*/
#ifndef Included_TimeDateWndProc_h	/* [ */
#define Included_TimeDateWndProc_h


#ifdef __cplusplus
extern "C" {
#endif

LRESULT 
CALLBACK 
TimeDateWndProc(HWND hwnd, 
				UINT msg, 
				WPARAM wParam, 
				LPARAM lParam);

LRESULT 
CALLBACK 
TimeDateFrameWndProc(HWND hwnd, 
					UINT msg, 
					WPARAM wParam, 
					LPARAM lParam);

#ifdef __cplusplus
}
#endif


#endif /* ] Included_TimeDateWndProc_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
