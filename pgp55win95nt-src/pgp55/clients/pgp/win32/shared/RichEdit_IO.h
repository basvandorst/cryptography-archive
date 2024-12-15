/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: RichEdit_IO.h,v 1.1 1997/07/16 06:40:40 elrod Exp $
____________________________________________________________________________*/
#ifndef Included_RichEdit_IO_h	/* [ */
#define Included_RichEdit_IO_h

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------|
//
//	Get contents of RichEdit control.
//		hWndRichEditCtrl= handle of control that is to be read
//		bRTF			= TRUE => return contents in RTF format
//						= FALSE => return contents in plain ASCII text format
//		lpcb			= pointer to variable to receive number of characters 
//						  tranferred into buffer (including trailing NULL character)
//
//	Function returns pointer to allocated buffer if successful, NULL 
//  otherwise.  When it is no longer needed, the calling routine must free 
//  the allocated buffer from the process heap by call HeapFree. E.g.:
//
//		HeapFree (GetProcessHeap(), 0, lpBuffer);
//
LPSTR GetRichEditContents (HWND hWndRichEditCtrl, LPLONG lpcb, BOOL bRTF, BOOL bSel);
//
//------------------------------------------------------------------|

//------------------------------------------------------------------|
//
//
//	Set contents of RichEdit control.
//		hWndRichEditCtrl= handle of control that is to be set.
//		bRTF			= TRUE => buffer contents are in RTF format
//						= FALSE => buffer contents are in plain ASCII text format
//		lpBuffer		= buffer containing text to load into control
//
//	Function returns 0 if no error.
//
LONG SetRichEditContents (HWND hWndRichEditCtrl, LPSTR lpBuffer, BOOL bRTF, BOOL bSel);
//
//------------------------------------------------------------------|
#ifdef __cplusplus
}
#endif
#endif /* ] Included_RichEdit_IO_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
