/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: RDlstsrt.h,v 1.1 1997/08/11 20:41:56 pbj Exp $
____________________________________________________________________________*/
#ifndef Included_RDLSTSRT_h	/* [ */
#define Included_RDLSTSRT_h

/*
 * ListSort.h  List View sorting routine
 *
 * Apparently the sorting algorithm used in PGPlib is somehow different than
 * Microsoft employs in their standard listview control. This makes things 
 * sometimes confusing, since PGPkeys follows the PGPlib convention. Albeit
 * bucking PC GUI standards SortEm changes the listview sort to comply with
 * PGPlib and PGPkeys.
 *
 * Copyright (C) 1996 Pretty Good Privacy, Inc.
 * All rights reserved.
 */

void SortEm(HWND hwndLView);

int CALLBACK ListViewCompareProc(LPARAM lParam1,
                                 LPARAM lParam2,LPARAM lParamSort);
#define HEADUSERID 0
#define HEADVALIDITY 1
#define HEADTRUST 2
#define HEADSIZE 3


#endif /* ] Included_RDLSTSRT_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
