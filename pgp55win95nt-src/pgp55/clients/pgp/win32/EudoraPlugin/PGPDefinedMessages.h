/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	PGPDefinedMessages.h,v 1.7 1997/10/22 23:05:54 elrod Exp
____________________________________________________________________________*/

#ifndef Included_PGPDefinedMessages_h	/* [ */
#define Included_PGPDefinedMessages_h

#define WM_PGP_CREATE_INTERFACE WM_USER + 1000

#define IDC_DECRYPT			WM_USER + 1010
#define IDC_GETKEY			WM_USER + 1011
#define IDC_KEYMGR			WM_USER + 1012
#define IDC_JUSTDOIT		WM_USER + 1013
#define IDC_ENCRYPT			WM_USER + 1014
#define IDC_SIGN			WM_USER + 1015
#define IDC_ENCRYPT_SIGN	WM_USER + 1016
#define IDC_MIME			WM_USER + 1017

// Eudora Messages
#define IDC_SENDMAIL	32802
#define IDC_PLUGINMENU	32987


// Private Flags
#define SENT_FROM_CONTEXT_MENU 666

#endif /* ] Included_PGPDefinedMessages_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
