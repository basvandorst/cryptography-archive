/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: HookProcs.h,v 1.7 1999/04/09 15:26:26 dgal Exp $
____________________________________________________________________________*/
#ifndef Included_HookProcs_h	/* [ */
#define Included_HookProcs_h

LRESULT CALLBACK MainWndProc(HWND hDlg, 
							 UINT msg,
							 WPARAM wParam, 
							 LPARAM lParam);

LRESULT CALLBACK SendMsgWndProc(HWND hDlg, 
								UINT msg,
								WPARAM wParam, 
								LPARAM lParam);

LRESULT CALLBACK ReadMsgWndProc(HWND hDlg, 
								UINT msg,
								WPARAM wParam, 
								LPARAM lParam);

LRESULT CALLBACK UnknownWndProc(HWND hDlg, 
								UINT msg,
								WPARAM wParam, 
								LPARAM lParam);

LRESULT CALLBACK CommonWndProc(HWND hDlg, 
							   UINT msg,
							   WPARAM wParam, 
							   LPARAM lParam);

LRESULT CALLBACK SendToolbarParentWndProc(HWND hDlg, 
										  UINT msg,
										  WPARAM wParam, 
										  LPARAM lParam);

LRESULT CALLBACK ReadToolbarParentWndProc(HWND hDlg, 
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
