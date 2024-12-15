/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: RDproc.h,v 1.2 1997/09/19 21:50:24 wjb Exp $
____________________________________________________________________________*/
#ifndef Included_RDPROC_h	/* [ */
#define Included_RDPROC_h

/*
 * RecProc.c  Main message loop for the recipient dialog
 *
 * This message loops handles all the operations of the recipient
 * dialog, except those that are done in the listview subclass.
 *
 * Copyright (C) 1996 Pretty Good Privacy, Inc.
 * All rights reserved.
 */

BOOL CALLBACK RecipientDlgProc(HWND hdlg,UINT uMsg,    
                               WPARAM wParam,LPARAM lParam);
void GetHelpFile(char *Path); 
PRECGBL AllocRecGbl(void);
int InitRecGbl(HWND hwnd,PRECGBL pRecGbl);
PRECGBL RecGbl(HWND hwnd);
void ReloadRings(HWND hdlg);

#endif /* ] Included_RDPROC_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
