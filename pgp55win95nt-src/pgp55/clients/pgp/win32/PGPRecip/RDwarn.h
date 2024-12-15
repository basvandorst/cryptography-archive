/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: RDwarn.h,v 1.1 1997/08/11 20:42:03 pbj Exp $
____________________________________________________________________________*/
#ifndef Included_RDWARN_h	/* [ */
#define Included_RDWARN_h

/*
 * WarnProc.h  Main message loop for the warning dialog
 *
 * This message loop handles all the operations of the warning
 * dialog. Its a bit trickey, since it includes some routines to tack
 * on our "global" variables to the USERDATA portion of the window
 * (for reentrancy)
 *
 * Copyright (C) 1996 Pretty Good Privacy, Inc.
 * All rights reserved.
 */

int RSADSARecWarning(HWND hwnd);


#endif /* ] Included_RDWARN_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
