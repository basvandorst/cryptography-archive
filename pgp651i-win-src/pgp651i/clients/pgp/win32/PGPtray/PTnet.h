/*____________________________________________________________________________
	Copyright (C) 1999 Network Associates, Inc.
	All rights reserved.
	
	PThotkey.h - PGPtray hotkey functions
	

	$Id: PTnet.h,v 1.3 1999/02/25 23:15:14 pbj Exp $
____________________________________________________________________________*/
#ifndef Included_PTnet_h	/* [ */
#define Included_PTnet_h

#include "pgpPubTypes.h"

#define QUERYSERVICE		0
#define LOGON				1
#define LOGOFF				2
#define QUERYLOGON			3


PGP_BEGIN_C_DECLARATIONS

VOID 
PTNetInit (void);

VOID
PTNetLaunch (
		HWND	hwnd, 
		UINT	uStartPage);

BOOL
PTSendLogOnOffMessage (
		HWND	hwnd,
		UINT	uMessage);

BOOL 
PTUpdateTrayIconAndText (
		HWND	hwnd);

VOID
PTCheckForNetMsg (
		HWND	hwnd,
		UINT	msg,
		WPARAM	wParam,
		LPARAM	lParam);

BOOL
PTNetIsGUIDisabled (void);

BOOL
PTNetIsLogonDisabled (void);

BOOL
PTNetIsLogoffDisabled (void);

PGP_END_C_DECLARATIONS

#endif /* ] Included_PTnet_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
