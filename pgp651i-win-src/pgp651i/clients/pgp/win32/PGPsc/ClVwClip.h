/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: ClVwClip.h,v 1.4 1999/03/10 02:34:39 heller Exp $
____________________________________________________________________________*/
#ifndef Included_CLVWCLIP_h	/* [ */
#define Included_CLVWCLIP_h

UINT EnclyptorViewClipboard(HWND hwnd,void* pInput,DWORD dwInSize,
							void** ppOutput,DWORD* pOutSize);

void TextViewer(HWND hwnd,char *pClipboardData,DWORD dwDataSize);

#endif /* ] Included_CLVWCLIP_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

