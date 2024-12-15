/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	


	$Id: openfile.h,v 1.6.8.1 1998/11/12 03:13:45 heller Exp $



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
