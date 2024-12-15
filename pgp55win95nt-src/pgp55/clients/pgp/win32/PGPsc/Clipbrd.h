/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: Clipbrd.h,v 1.2 1997/08/22 15:50:52 wjb Exp $
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
