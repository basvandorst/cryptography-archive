/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: RDlstsub.h,v 1.2.10.1 1998/11/12 03:24:29 heller Exp $
____________________________________________________________________________*/
#ifndef Included_RDLSTSUB_h	/* [ */
#define Included_RDLSTSUB_h

/*
 * ListSub.h  List View helper routines
 *
 * These routines handle the listview subclass operations and also
 * do some of the needed operations on the linked list of keys.
 *
 * Copyright (C) 1996 Network Associates, Inc. and its affiliates.
 * All rights reserved.
 */

LRESULT WINAPI MyListviewWndProc(HWND hwnd, UINT msg, 
                                 WPARAM wParam, LPARAM lParam);

void FreeLinkedLists(PUSERKEYINFO ListHead);
int LengthOfList(PUSERKEYINFO ListHead);

#endif /* ] Included_RDLSTSUB_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

