/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: RDdrag.h,v 1.1 1997/08/11 20:41:51 pbj Exp $
____________________________________________________________________________*/
#ifndef Included_RDDRAG_h	/* [ */
#define Included_RDDRAG_h


/*
 * DragItem.h  Routines needed to create images for the ListView drag
 *
 * Since we're using ownerdrawn listboxes, we have to do all this ourselves.
 * These routines build the ImageList used with drag operations.
 *
 * Copyright (C) 1996 Pretty Good Privacy, Inc.
 * All rights reserved.
 */

HIMAGELIST MakeDragImage(HWND hwndDragFrom,int *left,int *bottom);          
 

#endif /* ] Included_RDDRAG_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
