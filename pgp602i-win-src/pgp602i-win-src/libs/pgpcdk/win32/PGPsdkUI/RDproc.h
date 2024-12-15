/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: RDproc.h,v 1.4.10.1 1998/11/12 03:24:33 heller Exp $
____________________________________________________________________________*/
#ifndef Included_RDPROC_h	/* [ */
#define Included_RDPROC_h

/*
 * RecProc.c  Main message loop for the recipient dialog
 *
 * This message loops handles all the operations of the recipient
 * dialog, except those that are done in the listview subclass.
 *
 * Copyright (C) 1996 Network Associates, Inc. and its affiliates.
 * All rights reserved.
 */

BOOL CALLBACK RecipientDlgProc(HWND hdlg,UINT uMsg,    
                               WPARAM wParam,LPARAM lParam);

#endif /* ] Included_RDPROC_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
