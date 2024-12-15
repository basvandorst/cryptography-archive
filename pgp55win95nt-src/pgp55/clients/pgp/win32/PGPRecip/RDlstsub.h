/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: RDlstsub.h,v 1.3 1997/09/08 18:24:23 wjb Exp $
____________________________________________________________________________*/
#ifndef Included_RDLSTSUB_h	/* [ */
#define Included_RDLSTSUB_h

/*
 * ListSub.h  List View helper routines
 *
 * These routines handle the listview subclass operations and also
 * do some of the needed operations on the linked list of keys.
 *
 * Copyright (C) 1996 Pretty Good Privacy, Inc.
 * All rights reserved.
 */

LRESULT WINAPI MyListviewWndProc(HWND hwnd, UINT msg, 
                                 WPARAM wParam, LPARAM lParam);
void FreeLinkedLists(PUSERKEYINFO ListHead);
int LengthOfList(PUSERKEYINFO ListHead);
void MakeReloadArray(HWND hwnd,PRECIPIENTDIALOGSTRUCT prds);
void FreeReloadArray(PRECIPIENTDIALOGSTRUCT prds);

int RecipientSetFromGUI(HWND hwnd,
				   PRECIPIENTDIALOGSTRUCT prds,
				   PUSERKEYINFO RecipientLL,
				   PGPUInt32 *pNumKeys);

int RecipientSetFromList(PRECIPIENTDIALOGSTRUCT prds,
						PUSERKEYINFO RecipientLL,
						PGPUInt32 *pNumKeys);

#endif /* ] Included_RDLSTSUB_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

