/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: TextIO.h,v 1.4 1999/04/09 15:26:26 dgal Exp $
____________________________________________________________________________*/
#ifndef Included_TextIO_h	/* [ */
#define Included_TextIO_h

#include <windows.h>

BOOL SaveClipboardText(HWND hwnd);
BOOL RestoreClipboardText(HWND hwnd);
BOOL GetRecipientText(HWND hwnd, char **szRecipients);
BOOL GetMessageText(HWND hwnd, BOOL bCut, char **szMessage);
BOOL SetMessageText(HWND hwnd, char *szMessage);
BOOL FindReplyHeader(char *szReplyHeader);
void AddReply(char *szHeader, char *szReply);
void FixBadSpaces(char *szInput);
void DoCopyThread(void *args);
void DoPasteThread(void *args);

#endif /* ] Included_TextIO_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
