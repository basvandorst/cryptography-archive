/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: EnclyptorProc.h,v 1.6 1999/03/10 02:42:25 heller Exp $
____________________________________________________________________________*/
#ifndef Included_ENCLYPTORPROC_h	/* [ */
#define Included_ENCLYPTORPROC_h

INT PTMessageBox (HWND hwnd, INT iCaption, INT iText, UINT uStyle);

LRESULT CALLBACK EnclyptorProc(HWND hwnd, UINT msg, 
							   WPARAM wParam, LPARAM lParam);

#endif /* ] Included_ENCLYPTORPROC_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
