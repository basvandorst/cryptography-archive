/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: openfile.h,v 1.4.4.1 1997/11/07 17:53:05 wjb Exp $
____________________________________________________________________________*/
#ifndef Included_OPENFILE_h	/* [ */
#define Included_OPENFILE_h

BOOL GetFileName(HWND hwnd,
				 char *lpszMessage,
				 FILELIST **FileList,
				 BOOL Clipboard,
				 char *FilterString);

#endif /* ] Included_OPENFILE_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
