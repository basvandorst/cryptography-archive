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
#include <stdio.h>
#include <stdlib.h>
#include <direct.h>

#include "..\include\config.h"
#include "..\include\pgpmem.h"

#include "clipboard.h"
#include "define.h"
#include "enclyptorviewclipboard.h"

/*This code isn't going to work for non-text clipboard data types.  When/If
 *this is supported, some other method will have to be used.  The basic
 *problem is that ShellExecute() depends on the file extension to find the
 *appropriate viewer; since we know the type but not the extension (?? is
 *this true??) then we will have to do some other thing.
 */

void GetTempDirectory(char *TempDir, DWORD TempDirSize);
void GetTempFile(char *TempFile, char *TempDir);

// DeleteTempFileThread wjb@pgp.com 5/20/97
//
// This thread hangs around until the external viewer has been closed.
// It then deletes the temporary file.

DWORD DeleteTempFileThread(SHELLEXECUTEINFO *sei)
{
	WaitForSingleObject(sei->hProcess, INFINITE );

	remove(sei->lpFile);

	CloseHandle(sei->hInstApp); // No memory leaks!

	free((char *)sei->lpFile); // Free the filename
	free(sei);                 // Free our structure

	return TRUE;
}

void LaunchExternalViewer(HWND hwnd)
{
	HINSTANCE hApplication = NULL;
	char TempFile[_MAX_PATH], TempDir[_MAX_PATH];
	BOOL AllOK = TRUE;
	void* pClipboardData = NULL;
	DWORD dwDataSize = 0;
	UINT ClipboardFormat = 0;

	GetTempDirectory(TempDir, sizeof(TempDir));
	/*First of all, we'll need a temporary file name.*/
	GetTempFile(TempFile, TempDir);

	if(TempFile)
	{
		pClipboardData = RetrieveClipboardData(hwnd, &ClipboardFormat,
												&dwDataSize);

		if(pClipboardData)
		{
			FILE *ouf;

			ouf = fopen(TempFile, "wb");
			if(ouf)
			{
				fwrite(pClipboardData, sizeof(char), dwDataSize, ouf);
				fclose(ouf);
			}
			else
			{
				char TempMessage[_MAX_PATH + 100];

				sprintf(TempMessage, "Unable to create file\n%s", TempMessage);

				MessageBox(hwnd,
						   TempMessage,
						   "Error",
						   MB_ICONERROR | MB_OK);

				AllOK = FALSE;
			}
		}

		if(AllOK)
		{
			HANDLE deletethread;
			SHELLEXECUTEINFO *sei;
			DWORD ThreadID;
			char *TempFileMem;

			sei=(SHELLEXECUTEINFO *)malloc(sizeof(SHELLEXECUTEINFO));
			memset(sei,0x00,sizeof(SHELLEXECUTEINFO));

			TempFileMem=(char *)malloc(_MAX_PATH);
			memset(TempFileMem,0x00,_MAX_PATH);

			strcpy(TempFileMem,TempFile);

			sei->cbSize=sizeof(SHELLEXECUTEINFO);
			sei->fMask = SEE_MASK_NOCLOSEPROCESS;
			sei->hwnd=hwnd;
			sei->lpVerb=NULL;
			sei->lpFile=TempFileMem;
			sei->lpParameters=NULL;
			sei->lpDirectory=NULL;
			sei->nShow=SW_SHOWNORMAL;
			
			ShellExecuteEx(sei);

			if(sei->hInstApp <= (HINSTANCE) SE_ERR_DLLNOTFOUND)
			{
				if(sei->hInstApp == (HINSTANCE) SE_ERR_NOASSOC ||
				   sei->hInstApp == (HINSTANCE) SE_ERR_ASSOCINCOMPLETE)
				{
					MessageBox(sei->hwnd,
						"Unable to find type associated with text files.",
						"Error",
						MB_ICONERROR | MB_OK);
				}

				remove(sei->lpFile); // Delete temp file
				free((char *)sei->lpFile);
				free(sei);
			}
			else  // Ext viewer launched. Start thread to clean up.
			{
			deletethread=CreateThread(NULL,0,
				(LPTHREAD_START_ROUTINE)DeleteTempFileThread,
				sei,0,&ThreadID);
			
			if(deletethread==0)
				MessageBox(hwnd,
				"Unable to start delete thread for temporary file.\n"
				"Temporary file will be deleted upon exit of PGPtray.",
				"Error",MB_OK);
			}
		}
	}
	else
	{
		char buffer[_MAX_PATH + 128];

		sprintf(buffer, "Unable to create temp file\n%s", TempFile);
		MessageBox(hwnd,
				   buffer,
				   "Error",
				   MB_ICONERROR | MB_OK);

	}
}

void LaunchInternalViewer(HWND hwnd)
{
	void *pClipboardData;
	UINT ClipboardFormat = 0;
	DWORD dwDataSize = 0;
	DWORD dwOutputSize = 0;
	void* pOutputBuffer = NULL;

	ShowWindow(hwnd, SW_HIDE);
	pClipboardData = RetrieveClipboardData(hwnd, &ClipboardFormat, &dwDataSize);

	if(pClipboardData)
	{
		if(ClipboardFormat == CF_TEXT)
		{
			OSVERSIONINFO osid;
			osid.dwOSVersionInfoSize = sizeof (osid);
			GetVersionEx (&osid);

			if ((osid.dwPlatformId == VER_PLATFORM_WIN32_NT) ||
				(dwDataSize < 65535)) {
				if(EnclyptorViewClipboard(hwnd, pClipboardData, dwDataSize,
							  		  &pOutputBuffer, &dwOutputSize)
							== STATUS_SUCCESS)
				{
					StoreClipboardData(hwnd, pOutputBuffer, dwOutputSize);

					pgpFree(pOutputBuffer);
				}
			}
			else
			{
				MessageBox (NULL, "Sorry, the clipboard contents exceed the\n"
					"limits of the PGP clipboard viewer", "PGP Warning",
					MB_OK|MB_ICONEXCLAMATION|MB_SETFOREGROUND);
					
			}
		}
		pgpFree(pClipboardData);
	}
}

void GetTempDirectory(char *TempDir, DWORD TempDirSize)
{
	HKEY hKey;
	LONG lResult;
	CHAR* p;
	DWORD dwValueType, dwSize;

	assert(TempDir);

	*TempDir = '\0';

	lResult = RegOpenKeyEx (HKEY_CURRENT_USER,
							"Software\\PGP\\PGP\\PGPkeys",
							0,
							KEY_READ,
							&hKey);

	if (lResult == ERROR_SUCCESS)
	{
		dwSize = TempDirSize;
		lResult = RegQueryValueEx (hKey, "ExePath", NULL, &dwValueType,
									(LPBYTE)TempDir, &dwSize);
		RegCloseKey (hKey);
		p = strrchr (TempDir, '\\');
		if (p)
		{
			p++;
			*p = '\0';
		}
		else lstrcpy (TempDir, "");
	}

	lstrcat (TempDir, "TEMP");

	_mkdir(TempDir);  /*If this fails, we just assume it was already there*/
}

void GetTempFile(char *TempFile, char *TempDir)
{
	static int TempIndex = 0;
	char FileSuffix[_MAX_PATH];
	BOOL GotGoodFile = FALSE;
	FILE *fh;

	assert(TempFile);
	assert(TempDir);

	while(!GotGoodFile)
	{
		ltoa(TempIndex, FileSuffix, 36);
		sprintf(TempFile, "%s\\TED%s.txt", TempDir, FileSuffix);
		fh = fopen(TempFile, "r");
		if(fh)
		{
			fclose(fh);
		}
		else
		{
			fh = fopen(TempFile, "w");
			if(fh)
			{
				fclose(fh);
				unlink(TempFile);
				GotGoodFile = TRUE;
			}
		}
		++TempIndex;
	}
}

void CleanupTempFiles(void)
{
	char TempDir[_MAX_PATH], FullFile[_MAX_PATH];
	HANDLE hDir;
	WIN32_FIND_DATA FindData;

	GetTempDirectory(TempDir, sizeof(TempDir));
	sprintf(FullFile, "%s\\TED*.txt", TempDir);

	hDir = FindFirstFile(FullFile, &FindData);
	while(hDir != INVALID_HANDLE_VALUE)
	{
		sprintf(FullFile, "%s\\%s", TempDir, FindData.cFileName);
		unlink(FullFile);

		if(!FindNextFile(hDir, &FindData))
			hDir = INVALID_HANDLE_VALUE;
	}
}
