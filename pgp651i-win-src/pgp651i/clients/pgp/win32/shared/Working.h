/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: Working.h,v 1.3 1999/03/10 03:04:55 heller Exp $
____________________________________________________________________________*/
#ifndef Included_Working_h	/* [ */
#define Included_Working_h

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

BOOL CALLBACK WorkingDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam,
							LPARAM lParam);
int WorkingCallback (void* arg, INT SoFar, INT Total) ;
HWND WorkingDlgProcThread(HMODULE hMod, 
						  HINSTANCE hInst,
						  HWND hParent,
						  char *Action,
						  char *FileName);

#ifdef __cplusplus
}
#endif

#endif /* ] Included_Working_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
