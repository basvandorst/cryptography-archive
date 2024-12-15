/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: Working.h,v 1.2 1997/10/07 23:36:55 dgal Exp $
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
