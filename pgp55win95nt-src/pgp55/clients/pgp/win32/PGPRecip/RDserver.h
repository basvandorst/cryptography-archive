/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: RDserver.h,v 1.4 1997/10/16 15:08:53 wjb Exp $
____________________________________________________________________________*/
#ifndef Included_RDSERVER_h	/* [ */
#define Included_RDSERVER_h

/*
 * KeyServe.h  Routines needed to support the keyserver context menu
 *
 * All of these routines are used within the listview subclass handler, but
 * since they deal with the keyserver, they are isolated in this file for
 * clarity.
 *
 * Copyright (C) 1996 Pretty Good Privacy, Inc.
 * All rights reserved.
 */

int	LookUpSelectedKeys(HWND hWndParent,
			   PGPContextRef context,
			   PGPKeySetRef keysetToAddTo);

int	LookUpNotFoundKeys(HWND hWndParent,
			   PGPContextRef context,
			   PGPKeySetRef keysetToAddTo);

BOOL GetADKsFromServer(PGPContextRef context,
							 HWND hwnd,
							 PUSERKEYINFO pui,
							 PGPKeySetRef keysetToAddTo);

int	LookUpSingleUserID(HWND hWndParent,
			   PGPContextRef context,
			   PUSERKEYINFO pui,
			   PGPKeySetRef keysetToAddTo);

#endif /* ] Included_RDSERVER_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
