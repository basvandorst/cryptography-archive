/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: RDmove.h,v 1.9 1997/10/16 15:08:52 wjb Exp $
____________________________________________________________________________*/
#ifndef Included_RDMOVE_h	/* [ */
#define Included_RDMOVE_h

/*
 * MoveItem.h  List View movement routines that support ADK
 *
 * These routines use the algorithm developed by Mark W. to support ADK
 *
 * Copyright (C) 1996 Pretty Good Privacy, Inc.
 * All rights reserved.
 */

PADKINFO GetADKList(PUSERKEYINFO pui,
					PGPKeyRef basekey, 
					PGPKeySetRef allkeys,
					PADKINFO HeadADK);
void FreeADKLists(PADKINFO ListHead);
int MoveListViewItems(HWND left, HWND right, BOOL RecToUser);
int MoveMarkedItems(HWND src, HWND dest);
int InitialMoveListViewItems(HWND left, HWND right,PUSERKEYINFO pui);

void DragUserX(HWND hdlg,
			   PUSERKEYINFO headpui,
			   PUSERKEYINFO X,
			   BOOL RecToUser);
BOOL VetoSelectedKeys(HWND hdlg,
			   PPREFS Prefs,
			   PUSERKEYINFO headpui,
			   BOOL RecToUser);

#endif /* ] Included_RDMOVE_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
