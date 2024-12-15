/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: HookProcs.h,v 1.3.10.1 1998/11/12 03:12:43 heller Exp $
____________________________________________________________________________*/
#ifndef Included_HookProcs_h	/* [ */
#define Included_HookProcs_h

LRESULT CALLBACK CBTProc(int nCode, WPARAM wParam, LPARAM lParam);

BOOL CALLBACK MainWndProc(HWND hDlg, 
						  UINT msg,
						  WPARAM wParam, 
						  LPARAM lParam);

BOOL CALLBACK ToolbarParentWndProc(HWND hDlg, 
								   UINT msg,
								   WPARAM wParam, 
								   LPARAM lParam);

BOOL CALLBACK SendMsgWndProc(HWND hDlg, 
							 UINT msg,
							 WPARAM wParam, 
							 LPARAM lParam);

BOOL CALLBACK ReadMsgWndProc(HWND hDlg, 
							 UINT msg,
							 WPARAM wParam, 
							 LPARAM lParam);

BOOL CALLBACK UnknownWndProc(HWND hDlg, 
							 UINT msg,
							 WPARAM wParam, 
							 LPARAM lParam);

BOOL CALLBACK CommonWndProc(HWND hDlg, 
							UINT msg,
							WPARAM wParam, 
							LPARAM lParam);

#endif /* ] Included_HookProcs_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
