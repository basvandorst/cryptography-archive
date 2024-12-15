/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: UIutils.h,v 1.2.6.1 1997/11/11 00:46:16 dgal Exp $
____________________________________________________________________________*/
#ifndef Included_UIutils_h	/* [ */
#define Included_UIutils_h

#include <windows.h>

PGP_BEGIN_C_DECLARATIONS

HINSTANCE UIGetInstance(void);
void UISetInstance(HINSTANCE hInstance);
void UIGetString(char *szBuffer, size_t nLength, UINT nID);
void UIFormatString(char *szBuffer, size_t nLength, UINT nID, char *szData);
void UIDisplayString(HWND hwnd, char *szBuffer);
void UIDisplayStringID(HWND hwnd, UINT nID);
BOOL UIAskYesNoStringID(HWND hwnd, UINT nID);
void UIHexDump(HWND hwnd, char *szMessage, int nLength);
void UIDisplayErrorCode(char *szFile, 
						int nLine, 
						char *szModule, 
						int nCode);
BOOL UIWarnUser(HWND hwnd, UINT nID, char *szRegValue);
void PGPDebugMessage(const char *szMessage);

PGP_END_C_DECLARATIONS

#endif /* ] Included_utils_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
