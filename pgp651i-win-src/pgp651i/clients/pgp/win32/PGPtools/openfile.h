/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	


	$Id: openfile.h,v 1.7 1999/03/10 02:46:04 heller Exp $



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
