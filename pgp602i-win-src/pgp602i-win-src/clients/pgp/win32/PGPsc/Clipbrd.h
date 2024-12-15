/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: Clipbrd.h,v 1.2.18.1 1998/11/12 03:13:25 heller Exp $
____________________________________________________________________________*/
#ifndef Included_CLIPBRD_h	/* [ */
#define Included_CLIPBRD_h

void* RetrieveClipboardData(HWND hwnd, UINT* pClipboardFormat, 
							DWORD* pDataSize);
void StoreClipboardData(HWND hwnd, void* pData, DWORD dwDataSize);


#endif /* ] Included_CLIPBRD_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
