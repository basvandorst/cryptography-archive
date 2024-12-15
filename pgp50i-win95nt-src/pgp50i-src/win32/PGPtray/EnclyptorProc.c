/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
/*4514 is "unreferenced inline func"*/
#pragma warning (disable : 4214 4201 4115 4514)
#include <windows.h>
#pragma warning (default : 4214 4201 4115)
#include <stdio.h>
#include <assert.h>

#include "..\include\config.h"
#include "..\include\pgpkeydb.h"
#include "..\include\pgpcomdlg.h"

#include "define.h"
#include "resource.h"
#include "clipboard.h"
#include "enclyptorkeydb.h"
#include "wordwrap.h"
#include "EnclyptorEncryptSign.h"
#include "EnclyptorDecryptVerify.h"
#include "EnclyptorAddKey.h"
#include "EnclyptorViewClipboard.h"
#include "viewers.h"
#include "actions.h"

extern HINSTANCE g_hinst;
extern HICON hIconKey;
extern BOOL IsTed;

static void PopupTaskbarMenu(HWND hwndTarget, BOOL InPGPOperation);
static void GetHelpFilePath (char *HelpFile, DWORD HelpFileSize);

LRESULT CALLBACK EnclyptorProc(HWND hwnd, UINT msg, WPARAM wParam,
								LPARAM lParam)
{
	static char HelpFile[MAX_PATH + 1] = "\0";
	static BOOL InPGPOperation = FALSE;
	
	switch(msg)
	{
		case WM_CREATE:
		{
			return 0;
		}

		case WM_QUERYOPEN:
		{
			return 0;
		}

		case WM_TASKAREA_MESSAGE:
		{
			UINT uID = (UINT) wParam;
			UINT uMouseMsg = (UINT) lParam;

			if(uMouseMsg == WM_RBUTTONDOWN)
			{
				switch(uID)
				{
					case 1:
					{
						SHORT AltKeyState = 0;
						SHORT CtrlKeyState = 0;

						CtrlKeyState = GetAsyncKeyState(VK_CONTROL);
						AltKeyState = GetAsyncKeyState(VK_MENU);

						if((CtrlKeyState & 0x8000) &&
							(AltKeyState & 0x8000) && IsTed)
						{
							PlaySound((LPCSTR) WAV_TED3, g_hinst, SND_RESOURCE);
						}
						else
						{
							PopupTaskbarMenu(hwnd, InPGPOperation);
						}
						return 0;
					}
				}
			}
			else if(uMouseMsg == WM_LBUTTONDOWN)
			{
				switch(uID)
				{
					case 1:
					{
						PopupTaskbarMenu(hwnd, InPGPOperation);
//						PostMessage(hwnd, WM_COMMAND,
//									(WPARAM)ID_LAUNCHPGPKEYS, 0);
						return 0;
					}
				}
			}
			break;
		}

		case WM_COMMAND:
		{
			switch(wParam)
			{
				case ID_ENCRYPT:
				{
					InPGPOperation = TRUE;
					DoEncrypt(hwnd, TRUE, FALSE);
					InPGPOperation = FALSE;
					break;
				}

				case ID_SIGN:
				{
					InPGPOperation = TRUE;
					DoEncrypt(hwnd, FALSE, TRUE);
					InPGPOperation = FALSE;
					break;
				}

				case ID_ENCRYPTSIGN:
				{
					InPGPOperation = TRUE;
					DoEncrypt(hwnd, TRUE, TRUE);
					InPGPOperation = FALSE;
					break;
				}

				case ID_DECRYPTVERIFY:
				{
					InPGPOperation = TRUE;
					DoDecrypt(hwnd);
					InPGPOperation = FALSE;
					break;
				}

				
				case ID_ADDKEY:
				{
					InPGPOperation = TRUE;
					DoAddKey(hwnd);
					InPGPOperation = FALSE;
					break;
				}

				case ID_LAUNCHVIEWER:
				{
					LaunchExternalViewer(hwnd);
					break;
				}
				
				case ID_VIEWCLIPBOARD:
				{
					LaunchInternalViewer(hwnd);
					break;
				}

				case ID_WHATIS_TED1:
					PlaySound((LPCSTR) WAV_TED1, g_hinst, SND_RESOURCE);
					break;

				case ID_WHATIS_TED2:
					PlaySound((LPCSTR) WAV_TED2, g_hinst, SND_RESOURCE);
					break;

				case ID_LAUNCHPGPKEYS:
				{
					DoLaunchKeys(hwnd);
					break;
				}

				case ID_PROPERTIES:
				{
					InPGPOperation = TRUE;
					PGPcomdlgPreferences(hwnd, 0);
					InPGPOperation = FALSE;
					break;
				}

				case ID_QUIT_ENCLYPTOR:
				{
					CleanupTempFiles();
					SetForegroundWindow(hwnd);
					PostMessage(hwnd, WM_CLOSE, 0, 0);
					break;
				}

				case ID_HELP_TOPICS:
				{
					if(!(*HelpFile))
					{
						GetHelpFilePath(HelpFile, sizeof(HelpFile));
					}
					WinHelp(hwnd, HelpFile, HELP_FINDER, 0);
				}
								
			}
			break;
		}
		
		case WM_ENDSESSION :
		{
			CleanupTempFiles();
			break;
		}

		case WM_CLOSE:
		{	
			PostQuitMessage(0);
	
			return 0;
		}
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}


static void PopupTaskbarMenu(HWND hwndTarget, BOOL InPGPOperation)
{
	HMENU hMenu = NULL;
	POINT pt;

	if(!IsTed)
		hMenu = LoadMenu(g_hinst, MAKEINTRESOURCE(IDM_TASKBARMENU));
	else
		hMenu = LoadMenu(g_hinst, MAKEINTRESOURCE(IDM_TEDBARMENU));

	GetCursorPos( &pt );

	if(InPGPOperation)
	{
		EnableMenuItem(hMenu,
					    ID_ENCRYPTSIGN,
					   MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu,
					   ID_DECRYPTVERIFY,
					   MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu,
					   ID_ADDKEY,
					   MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu,
					   ID_PROPERTIES,
					   MF_BYCOMMAND | MF_GRAYED);
	}

	hMenu = GetSubMenu(hMenu,0);

	SetForegroundWindow(hwndTarget);

	TrackPopupMenu( hMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON,
					pt.x, pt.y, 0, hwndTarget, NULL);

	DestroyMenu(hMenu);
}

//----------------------------------------------------|
// get PGPkeys path from registry and substitute Help file name

static void GetHelpFilePath (char *HelpFile, DWORD HelpFileSize)
{
	HKEY hKey;
	LONG lResult;
	CHAR sz[MAX_PATH];
	CHAR* p;
	DWORD dwValueType, dwSize;

	assert(HelpFile);

	*HelpFile = '\0';

	lResult = RegOpenKeyEx (HKEY_CURRENT_USER,
							"Software\\PGP\\PGP\\PGPkeys",
							0,
							KEY_READ,
							&hKey);

	if (lResult == ERROR_SUCCESS)
	{
		dwSize = HelpFileSize;
		lResult = RegQueryValueEx (hKey, "ExePath", NULL, &dwValueType,
									(LPBYTE)HelpFile, &dwSize);
		RegCloseKey (hKey);
		p = strrchr (HelpFile, '\\');
		if (p)
		{
			p++;
			*p = '\0';
		}
		else lstrcpy (HelpFile, "");
	}

	LoadString (g_hinst, IDS_HELPFILENAME, sz, sizeof(sz));
	lstrcat (HelpFile, sz);
}
