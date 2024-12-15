/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: TextIO.h,v 1.2.12.1 1998/11/12 03:12:50 heller Exp $
____________________________________________________________________________*/
#ifndef Included_TextIO_h	/* [ */
#define Included_TextIO_h

#include <windows.h>

BOOL GetRecipientText(HWND hwnd, char **szRecipients);
BOOL GetMessageText(HWND hwnd, BOOL bCut, char **szMessage);
BOOL SetMessageText(HWND hwnd, char *szMessage);
BOOL FindReplyHeader(char *szReplyHeader);
void AddReply(char *szHeader, char *szReply);
void FixBadSpaces(char *szInput);

#endif /* ] Included_TextIO_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
