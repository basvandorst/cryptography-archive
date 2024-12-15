/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
/*4514 is "unreferenced inline func"*/
#pragma warning (disable : 4214 4201 4115 4514)
#include <windows.h>
#pragma warning (default : 4214 4201 4115)

static int Ticket = 0;

#include "..\include\errs.h"

#include "..\include\config.h"
#include "..\include\pgpmem.h"

#include "define.h"
#include "clipboard.h"
#include "enclyptorkeydb.h"
#include "wordwrap.h"
#include "enclyptorencryptsign.h"
#include "enclyptordecryptverify.h"
#include "enclyptoraddkey.h"

unsigned long DoEncrypt(HWND hwnd, BOOL Encrypt, BOOL Sign)
{
 	 UINT ReturnValue = 0;
 	 UINT ClipboardFormat = 0;
 	 void* pClipboardData = NULL;
 	 DWORD dwDataSize = 0;
 	 DWORD dwOutputSize = 0;
 	 void* pOutputBuffer = NULL;
 	 unsigned long MyReturnCode = SUCCESS;

 	 if(!Ticket)
 	 {
 	 	 Ticket = 1;

 	 	 pClipboardData = RetrieveClipboardData(hwnd, &ClipboardFormat,
 	 	 	 	 	 	 	 	 	 	 	 	 &dwDataSize);

		if(pClipboardData)
 	 	{
			if(ClipboardFormat == CF_TEXT)
			{
 	 	 	 	 unsigned short WordWrapThreshold = 80;

 	 	 	 	 if(WeNeedToWordWrap(&WordWrapThreshold))
 	 	 	 	 {
 	 	 	 	 	 char* szWrappedBuffer = NULL;

 	 	 	 	 	 if(STATUS_SUCCESS == WrapBuffer(&szWrappedBuffer,
 	 	 	 	 	 	 	 	 	 	 	 	 	 pClipboardData,
 	 	 	 	 	 	 	 	 	 	 	 	 	 WordWrapThreshold))
 	 	 	 	 	 {
 	 	 	 	 	 	 DWORD dwWrappedSize = strlen(szWrappedBuffer);

 	 	 	 	 	 	 if(dwDataSize < dwWrappedSize)
 	 	 	 	 	 	 {
 	 	 	 	 	 	 	 pgpRealloc(&pClipboardData, dwWrappedSize + 1);

 	 	 	 	 	 	 	 if(pClipboardData)
 	 	 	 	 	 	 	 {
 	 	 	 	 	 	 	 	 strcpy(pClipboardData, szWrappedBuffer);
 	 	 	 	 	 	 	 	 dwDataSize = dwWrappedSize;
 	 	 	 	 	 	 	 }
 	 	 	 	 	 	 }

 	 	 	 	 	 	 pgpFree(szWrappedBuffer);
 	 	 	 	 	 }
 	 	 	 	 }

 	 	 	 	 ReturnValue = EnclyptorEncryptSign(hwnd, pClipboardData,
 	 	 	 	 	 	 	 	 	 	 dwDataSize, &pOutputBuffer,
 	 	 	 	 	 	 	 	 	 	 &dwOutputSize, Encrypt, Sign);	
 	 	 	 	
 	 	 	 	 if(ReturnValue == STATUS_SUCCESS)
 	 	 	 	 {
 	 	 	 	 	 //vErrorOut(fg_pink, "%s\n",(char*)pOutputBuffer);

 	 	 	 	 	 StoreClipboardData(hwnd, pOutputBuffer, dwOutputSize);

 	 	 	 	 	 pgpFree(pOutputBuffer);
 	 	 	 	 }
 	 	 	 }
 	 	 	 pgpFree(pClipboardData);
 	 	 }

 	 	 Ticket = 0;
 	 }
 	 return(MyReturnCode);
 }

 unsigned long DoDecrypt(HWND hwnd)
  {
 	 UINT ReturnValue = 0;
 	 UINT ClipboardFormat = 0;
 	 void* pClipboardData = NULL;
 	 DWORD dwDataSize = 0;
 	 DWORD dwOutputSize = 0;
  	  void* pOutputBuffer = NULL;
  	  unsigned long MyReturnCode = SUCCESS;

  	  if(!Ticket)
  	  {
  	  	  Ticket = 1;

  	  	  pClipboardData = RetrieveClipboardData(hwnd, &ClipboardFormat,
  	  	  	  	  	  	  	  	  	  	  	  	  &dwDataSize);

  	  	  if(pClipboardData)
  	  	  {
  	  	  	  if(ClipboardFormat == CF_TEXT)
  	  	  	  {

 	 	 	 	 ReturnValue = EnclyptorDecryptVerify(hwnd, pClipboardData,
 	 	 	 	 	 	 	 	 	 	 	 	 dwDataSize, &pOutputBuffer,
 	 	 	 	 	 	 	 	 	 	 	 	 &dwOutputSize);	
 	 	 	 	
 	 	 	 	 if(ReturnValue == STATUS_SUCCESS)
 	 	 	 	 {
//					StoreClipboardData(hwnd, pOutputBuffer, dwDataSize);
 	 	 	 	 	 pgpFree(pOutputBuffer);
 	 	 	 	  }
			}
			pgpFree(pClipboardData);
		}

		Ticket = 0;
	}
	return(MyReturnCode);
}

unsigned long DoAddKey(HWND hwnd)
{
	UINT ReturnValue = 0;
	UINT ClipboardFormat = 0;
	void* pClipboardData = NULL;
	DWORD dwDataSize = 0;
	DWORD dwOutputSize = 0;
	void* pOutputBuffer = NULL;
	unsigned long MyReturnCode = SUCCESS;

	if(!Ticket)
	{
		Ticket = 1;

		pClipboardData = RetrieveClipboardData(hwnd, &ClipboardFormat,
												&dwDataSize);

		if(pClipboardData)
		{
			if(ClipboardFormat == CF_TEXT)
			{
				ReturnValue = EnclyptorAddKey(hwnd, pClipboardData, dwDataSize);
				
				if(ReturnValue == STATUS_SUCCESS)
				{
					MessageBox(hwnd,
						"PGP successfully added the key(s) to your keyring.",
						"PGP - Add Key", MB_OK);
				}
				else
				{
					MessageBox(hwnd,
						"PGP failed to add the key(s) to your keyring.",
						"PGP - Add Key", MB_OK|MB_ICONWARNING);
				}
			}
			pgpFree(pClipboardData);
		}
		Ticket = 0;
	}
	return(MyReturnCode);
 }

unsigned long DoLaunchKeys(HWND hwnd)
{
	HKEY hkey;
	UINT ReturnValue = 0;
	UINT ClipboardFormat = 0;
	void* pClipboardData = NULL;
	DWORD dwDataSize = 0;
	DWORD dwOutputSize = 0;
	void* pOutputBuffer = NULL;
	unsigned long MyReturnCode = SUCCESS;

	if(hwnd)
		; /*Avoid W4 warning*/
														
	if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER,
									"Software\\PGP\\PGP\\PGPkeys",
									0, KEY_ALL_ACCESS,&hkey))
	{
		DWORD Size,Type;
		char szPath[MAX_PATH];

		Size = sizeof(szPath);
		if(RegQueryValueEx(hkey,
						   "ExePath",
						   NULL,
						   &Type,
						   (BYTE*)szPath,
						   &Size) == ERROR_SUCCESS)
		{
			strcat(szPath, " /s");
			WinExec(szPath, SW_SHOW);
		}
		else
		{
			/*Some kind of error handling*/
		}
	 }

	return(MyReturnCode);
}
