/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
/*4514 is "unreferenced inline func"*/
#pragma warning (disable : 4214 4201 4115 4514)
#include <windows.h>
#pragma warning (default : 4214 4201 4115)

#include "..\include\config.h"
#include "..\include\PGPkeydb.h"
#include "..\include\PGPcomdlg.h"
#include "..\include\PGPphrase.h"

#include "..\pgpwctx\shelldecryptverify.h"

#include "enclyptorproc.h"
#include "resource.h"
#include "taskbar.h"
#include "define.h"
#include "actions.h"

HINSTANCE g_hinst = NULL;

HICON hIconKey;

BOOL IsTed = FALSE;

int WINAPI WinMain (HINSTANCE hInstance,HINSTANCE hinstPrev,
					LPSTR lpszCmdLine, int nCmdShow)
{
	HWND hwnd = NULL;
	WNDCLASSEX wc;
	MSG msg;
	HANDLE hMutex = NULL;
	BOOL GotOneShot = FALSE;
	char *p;

	if(nCmdShow || hinstPrev)
		; /*Avoid W4 Warning*/

	pgpLibInit();

	if(PGPcomdlgIsExpired(NULL))
		return(0);

	if(strnicmp(lpszCmdLine, "-TED", strlen("-TED")) == 0)
		IsTed = TRUE;

	p = strstr(lpszCmdLine, "HWND=");
	if(p)
	{
		p += 5;
		hwnd = (HWND) atol(p);
	}

	if(strnicmp(lpszCmdLine, "-AutoSign", strlen("-AutoSign")) == 0)
	{
		GotOneShot = TRUE;
		DoEncrypt(hwnd, FALSE, TRUE);
	}

	if(strnicmp(lpszCmdLine, "-AutoEncrypt", strlen("-AutoEncrypt")) == 0)
	{
		GotOneShot = TRUE;
		DoEncrypt(hwnd, TRUE, FALSE);
	}

	if(strnicmp(lpszCmdLine, "-AutoEncryptSign",
				strlen("-AutoEncryptSign")) == 0)
	{
		GotOneShot = TRUE;
		DoEncrypt(hwnd, TRUE, TRUE);
	}

	if(strnicmp(lpszCmdLine, "-AutoDecryptVerify",
				strlen("-AutoEncryptVerify")) == 0)
	{
		GotOneShot = TRUE;
		DoDecrypt(hwnd);
	}

	if(strnicmp(lpszCmdLine, "-AddKey", strlen("-AddKey")) == 0)
	{
		GotOneShot = TRUE;
		DoAddKey(hwnd);
	}

	if(strnicmp(lpszCmdLine, "-LaunchKeys", strlen("-LaunchKeys")) == 0)
	{
		GotOneShot = TRUE;
		DoLaunchKeys(hwnd);
	}

	if(strnicmp(lpszCmdLine, "-Properties", strlen("-Properties")) == 0)
	{
		GotOneShot = TRUE;
		PGPcomdlgPreferences(hwnd, 0);
	}

	if(!GotOneShot)
	{
		if(!*lpszCmdLine)
		{
			hMutex = CreateMutex(NULL,TRUE,"ENCLYPTOR_MUTEX");

			if(ERROR_ALREADY_EXISTS == GetLastError())
			{
				CloseHandle(hMutex);
				return(0);
			}

			g_hinst = hInstance;

			// Register the Server Window Class
			wc.cbSize			= sizeof(wc);
			wc.style			= 0;
			wc.lpfnWndProc		= EnclyptorProc;
			wc.cbClsExtra		= 0;
			wc.cbWndExtra		= 0;
			wc.hInstance		= hInstance;
			wc.hIcon			= 0;
			wc.hCursor			= 0;
			wc.hbrBackground	= 0;
			wc.lpszMenuName		= 0;
			wc.lpszClassName	= "PGPtray_Hidden_Window";
			wc.hIconSm			= 0;

			RegisterClassEx(&wc);

			hwnd = CreateWindow("PGPtray_Hidden_Window", "",
								WS_OVERLAPPEDWINDOW,
								CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
								CW_USEDEFAULT, NULL, NULL, hInstance, NULL );

			ShowWindow(hwnd, SW_HIDE);
			UpdateWindow(hwnd);



			if(!IsTed)
			{
				hIconKey = LoadIcon(g_hinst, MAKEINTRESOURCE(IDI_TASKBAR));
				TaskbarAddIcon(hwnd, 1, hIconKey, "PGPtray",
								WM_TASKAREA_MESSAGE);
			}
			else
			{
				hIconKey = LoadIcon(g_hinst, MAKEINTRESOURCE(IDI_TED));
				TaskbarAddIcon(hwnd, 1, hIconKey, "PGPted",
								WM_TASKAREA_MESSAGE);
			}

			while( GetMessage( &msg, NULL, 0, 0 ) )  // Do message loop thang
			{
				// Process Tab messages and such
				if( IsDialogMessage( hwnd, &msg ) )
					continue;

				TranslateMessage( &msg );          // Process other messages
				DispatchMessage( &msg );
			}
		}
		else
		{
			char *Passphrase = NULL;

			ShellDecryptVerify(GetForegroundWindow(), lpszCmdLine, FALSE,
								FALSE, &Passphrase, TRUE);
			if(Passphrase)
			{
				PGPFreePhrase(Passphrase);
			}
		}
	}
	else
	{
		if(hwnd)
		{
			PostMessage(hwnd, WM_PASTE, 0, 0);
		}
	}

	pgpLibCleanup();

	TaskbarDeleteIcon(hwnd, 1);

	return 0;
}
