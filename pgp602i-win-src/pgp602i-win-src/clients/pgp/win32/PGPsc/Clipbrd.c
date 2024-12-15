/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: Clipbrd.c,v 1.5.18.1 1998/11/12 03:13:25 heller Exp $
____________________________________________________________________________*/
#include "precomp.h"

void* RetrieveClipboardData(HWND hwnd, UINT* pClipboardFormat, 
							DWORD* pDataSize)
{
	void* ReturnValue = 0;
	DWORD dwBufferSize = 0;
	void* pLocalDataBuffer = NULL;
	void* pClipboardBuffer = NULL;
	HANDLE hClipboardData = NULL;

	// See if we can gain access to the clipboard
	if(OpenClipboard(hwnd)) 
	{
		*pClipboardFormat = CF_TEXT;

		hClipboardData = GetClipboardData(*pClipboardFormat);

		if(hClipboardData && IsClipboardFormatAvailable(CF_TEXT))
		{
			dwBufferSize = GlobalSize(hClipboardData);

			// For Some Reason Windows is returning 1 bytes more than the data
			// and it is not a NULL Byte ...grrrrrr
			dwBufferSize--;

			//vErrorOut(fg_white, "BufferSize = %lu\n", dwBufferSize);

			pClipboardBuffer = GlobalLock(hClipboardData);

			if(pClipboardBuffer)
			{
				pLocalDataBuffer = malloc(dwBufferSize + 1);
				memset(pLocalDataBuffer, '\0', 
					(dwBufferSize + 1) * sizeof(BYTE));

				if(pLocalDataBuffer)
				{
					*pDataSize = dwBufferSize;
					memcpy(pLocalDataBuffer, pClipboardBuffer, dwBufferSize);
					if(strlen((char *) pLocalDataBuffer) < dwBufferSize)
						*pDataSize = strlen((char *) pLocalDataBuffer);

					ReturnValue = pLocalDataBuffer;
				}

				GlobalUnlock(hClipboardData);
			}
		}
		CloseClipboard();
	}
	return ReturnValue;
}


void StoreClipboardData(HWND hwnd, void* pData, DWORD dwDataSize)
{
	HANDLE hClipboardData = NULL;
	void* pClipboardBuffer = NULL;
	UINT ClipboardFormat = CF_TEXT;

	assert(pData);

	if(pData)
	{
		if(OpenClipboard(hwnd)) 
		{
			if(EmptyClipboard())
			{
				if(ClipboardFormat == CF_TEXT)
				{
					hClipboardData = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, 
						dwDataSize + 1);
				}

				if(hClipboardData)
				{
					pClipboardBuffer = 	GlobalLock(hClipboardData);

					if(pClipboardBuffer)
					{
						memcpy(pClipboardBuffer, pData, dwDataSize);
						if(ClipboardFormat == CF_TEXT)
						{
							*((char *) pClipboardBuffer + dwDataSize) = '\0';
						}
						GlobalUnlock(hClipboardData);

						SetClipboardData(ClipboardFormat, hClipboardData);
					}
				}
			}
			// Close the clipboard when we are done with it.
			CloseClipboard();
		}
	}
}
    

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
