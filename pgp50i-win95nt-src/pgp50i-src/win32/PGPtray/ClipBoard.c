/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
/*4514 is "unreferenced inline func"*/
#pragma warning (disable : 4214 4201 4115 4514)
#include <windows.h>
#pragma warning (default : 4214 4201 4115)
#include <assert.h>

#include "..\include\config.h"
#include "..\include\pgpmem.h"

void* RetrieveClipboardData(HWND hwnd, UINT* pClipboardFormat, DWORD* pDataSize)
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
 	 	 	 	 pLocalDataBuffer = pgpAlloc(dwBufferSize + 1);
 	 	 	 	 memset(pLocalDataBuffer, '\0', (dwBufferSize + 1)
 	 	 	 	 	 	 	 	 	 	 	 	 * sizeof(BYTE));

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
		else
		{
			if(!hClipboardData) /*There's nothing on the clipboard.*/
			{
				ReturnValue = pgpAlloc(1);
				memset(ReturnValue, '\0', 1);
			}
			else
			{
				/*UINT LastFormat = 0;
				char text[1500];
				char format[100];

				strcpy(text, "Available Formats:\n\n");

				while(LastFormat = EnumClipboardFormats(LastFormat))
				{
					switch(LastFormat)
					{
						case CF_TEXT:
							wsprintf(format, "CF_TEXT\n");
							break;

						case CF_BITMAP:
							wsprintf(format, "CF_BITMAP\n");
							break;

						case CF_METAFILEPICT:
							wsprintf(format, "CF_METAFILEPICT\n");
							break;

						case CF_SYLK:
							wsprintf(format, "CF_SYLK\n");
							break;

						case CF_DIF:
							wsprintf(format, "CF_DIF\n");
							break;

						case CF_TIFF:
							wsprintf(format, "CF_TIFF\n");
							break;

						case CF_OEMTEXT:
							wsprintf(format, "CF_OEMTEXT\n");
							break;

						case CF_DIB:
							wsprintf(format, "CF_DIB\n");
							break;

						case CF_PALETTE:
							wsprintf(format, "CF_PALETTE\n");
							break;

						case CF_PENDATA:
 	 	 	 	 	 	 	 wsprintf(format, "CF_PENDATA\n");
 	 	 	 	 	 	 	 break;

 	 	 	 	 	 	 case CF_RIFF:
 	 	 	 	 	 	 	 wsprintf(format, "CF_RIFF\n");
 	 	 	 	 	 	 	 break;

 	 	 	 	 	 	 case CF_WAVE:
 	 	 	 	 	 	 	 wsprintf(format, "CF_WAVE\n");
 	 	 	 	 	 	 	 break;

 	 	 	 	 	 	 case CF_UNICODETEXT:
 	 	 	 	 	 	 	 wsprintf(format, "CF_UNICODETEXT\n");
 	 	 	 	 	 	 	 break;

 	 	 	 	 	 	 case CF_ENHMETAFILE:
 	 	 	 	 	 	 	 wsprintf(format, "CF_ENHMETAFILE\n");
 	 	 	 	 	 	 	 break;

 	 	 	 	 	 	 case CF_HDROP:
 	 	 	 	 	 	 	 wsprintf(format, "CF_HDROP\n");
 	 	 	 	 	 	 	 break;


 	 	 	 	 	 	 default:
 	 	 	 	 	 	 	 wsprintf(format, "%lu (Private Format)\n",
 	 	 	 	 	 	 	 	 	 LastFormat);
 	 	 	 	 	 	 	 break;
 	 	 	 	 	 }

 	 	 	 	 	 strcat(text, format);
 	 	 	 	 }

 	 	 	 	 MessageBox(NULL, text, "Available Formats", MB_OK);

 	 	 	 	 *pClipboardFormat = LastFormat;
 	 	 	 	 */

 	 	 	 	 MessageBox(NULL,	 "This version of PGP only supports\n"
 	 	 	 	 	 	 	 	 	 "encrypting text on the clipboard.",
 	 	 	 	 	 	 	 	 	 0, MB_OK);
 	 	 	 }
 	 	 }
 	 	 CloseClipboard();
 	 }
 	 else
 	 {
 	 	 MessageBox(hwnd,"Another Application is busy with the\n"
 	 	 	 	 	 	 "clipboard.  Attempt this operation after\n"
 	 	 	 	 	 	 "this application completes its use of the\n"
 	 	 	 	 	 	 "clipboard.", "Clipboard Busy", MB_OK);
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
#ifdef SUPPORT_NON_TEXT
  	  	  	  	  else
  	  	  	  	  {
  	  	  	  	  	  hClipboardData = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE,
 	 	 	 	 	 	 	 	 	 	 	 	 dwDataSize);
  	  	  	  	  }
#endif

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
 	 	 else
 	 	 {
 	 	 	 MessageBox(hwnd,"Another Application is busy with the\n"
 	 	 	 	 	 	 	 "clipboard.  Attempt this operation after\n"
 	 	 	 	 	 	 	 "this application completes its use of the\n"
 	 	 	 	 	 	 	 "clipboard.", "Clipboard Busy", MB_OK);
 	 	 }
 	 }
 }
