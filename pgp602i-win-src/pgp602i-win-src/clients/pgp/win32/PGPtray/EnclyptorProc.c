/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: EnclyptorProc.c,v 1.35.2.2.2.1 1998/11/12 03:13:49 heller Exp $
____________________________________________________________________________*/
#include "precomp.h"

BOOL PopupTaskbarMenu(HWND hwndTarget, BOOL InPGPOperation);
BOOL bUseCurrent=FALSE;
UINT nLaunchKeysTimer=0;
BOOL PGPDiskExists=FALSE;

#define REGPATH "Software\\Network Associates\\PGP60\\PGPtray"

void RememberUseCurrent(BOOL bUseCurrent)
{
	HKEY	hKey;
	LONG	lResult;
	DWORD	dw = 0;
	
	dw = bUseCurrent;

	lResult = RegOpenKeyEx(	HKEY_CURRENT_USER,
							REGPATH, 
							0, 
							KEY_ALL_ACCESS, 
							&hKey);

	if (lResult == ERROR_SUCCESS) 
	{
		RegSetValueEx (	hKey, 
						"UseCurrent", 
						0, 
						REG_DWORD, 
						(LPBYTE)&dw, 
						sizeof(dw));

		RegCloseKey (hKey);
	}
}

BOOL GetUseCurrent()
{
	HKEY	hKey;
	LONG	lResult;
	DWORD	dw;

	lResult = RegOpenKeyEx(	HKEY_CURRENT_USER,
							REGPATH, 
							0, 
							KEY_ALL_ACCESS, 
							&hKey);

	if (lResult == ERROR_SUCCESS) 
	{
		DWORD  size = sizeof(dw);
		DWORD  type = 0;

		RegQueryValueEx(hKey, 
						"UseCurrent", 
						0, 
						&type, 
						(LPBYTE)&dw, 
						&size);
	}
	else // Init Values
	{
		lResult = RegCreateKeyEx (	HKEY_CURRENT_USER, 
									REGPATH, 
									0, 
									NULL,
									REG_OPTION_NON_VOLATILE, 
									KEY_ALL_ACCESS, 
									NULL, 
									&hKey, 
									&dw);

		if (lResult == ERROR_SUCCESS) 
		{
			dw = 0;

			RegSetValueEx (	hKey, 
							"UseCurrent", 
							0, 
							REG_DWORD, 
							(LPBYTE)&dw, 
							sizeof(dw));

			RegCloseKey (hKey);
		}
	}

	return (BOOL)dw;
}

LRESULT CALLBACK EnclyptorProc(HWND hwnd, UINT msg, 
							   WPARAM wParam, LPARAM lParam)
{
	static char HelpFile[MAX_PATH + 1] = "\0";
	static BOOL InPGPOperation = FALSE;
	static HWND hwndFocus;
	
	// See if user deselects caching via prefs
	CheckForPurge(msg,wParam);

	switch(msg)
	{
		case WM_CREATE:
		{
			char szPGPDisk[MAX_PATH+1];
			FILE *fPGPDisk;

			bUseCurrent=GetUseCurrent();

			// See if we have to grey PGPDisk Menu Item
			PGPDiskExists=FALSE;
			PGPpath(szPGPDisk);
			strcat(szPGPDisk,"PGPdisk.exe");

			fPGPDisk=fopen(szPGPDisk,"rb");
			if(fPGPDisk!=NULL)
			{
				PGPDiskExists=TRUE;
				fclose(fPGPDisk);
			}

			StartUpdateTimer(hwnd, &nLaunchKeysTimer);
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
						PopupTaskbarMenu(hwnd, InPGPOperation);
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
					if(DoCopy(hwnd,PGPsc,bUseCurrent,&hwndFocus))
					{
						EncryptClipboard(hwnd, PGPsc, PGPtls, TRUE, FALSE);
						DoPaste(bUseCurrent,hwndFocus);
					}
					InPGPOperation = FALSE;
					break;
				}

				case ID_SIGN:
				{
					InPGPOperation = TRUE;
					if(DoCopy(hwnd,PGPsc,bUseCurrent,&hwndFocus))
					{
						EncryptClipboard(hwnd, PGPsc,PGPtls, FALSE, TRUE);
						DoPaste(bUseCurrent,hwndFocus);
					}
					InPGPOperation = FALSE;
					break;
				}

				case ID_ENCRYPTSIGN:
				{
					InPGPOperation = TRUE;
					if(DoCopy(hwnd,PGPsc,bUseCurrent,&hwndFocus))
					{
						EncryptClipboard(hwnd, PGPsc,PGPtls, TRUE, TRUE);
						DoPaste(bUseCurrent,hwndFocus);
					}
					InPGPOperation = FALSE;
					break;
				}

				case ID_DECRYPTVERIFY:
				{
					InPGPOperation = TRUE;
					if(DoCopy(hwnd,PGPsc,bUseCurrent,&hwndFocus))
					{
						DecryptClipboard(hwnd, PGPsc,PGPtls);
						DoFocus(bUseCurrent,hwndFocus);
					}
					InPGPOperation = FALSE;
					break;
				}

				
				case ID_ADDKEY:
				{
					InPGPOperation = TRUE;
					if(DoCopy(hwnd,PGPsc,bUseCurrent,&hwndFocus))
					{
						AddKeyClipboard(hwnd, PGPsc,PGPtls);
						DoFocus(bUseCurrent,hwndFocus);
					}
					InPGPOperation = FALSE;
					break;
				}

				case ID_WIPECLIP:
				{
					InPGPOperation = TRUE;
					ClipboardWipe(hwnd,PGPsc);
					InPGPOperation = FALSE;
					break;
				}
				
				case ID_VIEWCLIPBOARD:
				{
					InPGPOperation = TRUE;
					LaunchInternalViewer(hwnd);
					InPGPOperation = FALSE;
					break;
				}

				case ID_USECURRENT:
				{
					bUseCurrent = !bUseCurrent;
					RememberUseCurrent(bUseCurrent);
					break;
				}
					
				case ID_LAUNCHPGPKEYS:
				{
					InPGPOperation = TRUE;
					DoLaunchKeys(hwnd);
					InPGPOperation = FALSE;
					break;
				}

				case ID_LAUNCHPGPTOOLS:
				{
					InPGPOperation = TRUE;
					DoLaunchTools(hwnd);
					InPGPOperation = FALSE;
					break;
				}

				case ID_LAUNCHPGPDISK:
				{
					InPGPOperation = TRUE;
					DoLaunchDisk(hwnd);
					InPGPOperation = FALSE;
					break;
				}

				case ID_PROPERTIES:
				{
					InPGPOperation = TRUE;
					PGPscPreferences(hwnd, PGPsc,PGPtls);
					InPGPOperation = FALSE;
					break;
				}

				case ID_QUIT_ENCLYPTOR:
				{
					RememberUseCurrent(bUseCurrent);
					SetForegroundWindow(hwnd);
					PostMessage(hwnd, WM_CLOSE, 0, 0);
					break;
				}

				case ID_HELP_TOPICS:
				{
					InPGPOperation = TRUE;
					PGPpath(HelpFile);
					strcat(HelpFile,"PGP60.hlp");
					WinHelp(hwnd, HelpFile, HELP_FINDER, 0);
					InPGPOperation = FALSE;
					break;
				}
								
			}
			break;
		}
		
		case WM_ENDSESSION :
		{
			break;
		}

		case WM_CLOSE:
		{	
			KillTimer(hwnd, nLaunchKeysTimer);
			PostQuitMessage(0);	
			return 0;
		}
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}


BOOL PopupTaskbarMenu(HWND hwndTarget, BOOL InPGPOperation)
{
	HMENU hMenu = NULL;
	HMENU hSubMenu = NULL;
	POINT pt;
	UINT flags;

	GetCursorPos( &pt );

	// If the cursor is at the top, we need to reverse the menu
	if(pt.y<400)
		hMenu = LoadMenu(g_hinst, MAKEINTRESOURCE(IDM_TASKBARMENUREV));
	else
		hMenu = LoadMenu(g_hinst, MAKEINTRESOURCE(IDM_TASKBARMENU));

	flags = (bUseCurrent ?	MF_BYCOMMAND|MF_CHECKED : 
							MF_BYCOMMAND|MF_UNCHECKED);

	CheckMenuItem(hMenu, ID_USECURRENT, flags);

	// PGPdisk only in biz and pp versions
#if !(PGP_BUSINESS_SECURITY || PGP_PERSONAL_PRIVACY)
		EnableMenuItem(hMenu, 
					   ID_LAUNCHPGPDISK, 
					   MF_BYCOMMAND | MF_GRAYED);
#endif

	// Its not installed anyway so forget it...
	if (!PGPDiskExists)
		EnableMenuItem(hMenu, 
					   ID_LAUNCHPGPDISK, 
					   MF_BYCOMMAND | MF_GRAYED);

	if(InPGPOperation)
	{
		EnableMenuItem(hMenu, 
					   ID_ENCRYPT, 
					   MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, 
					   ID_SIGN, 
					   MF_BYCOMMAND | MF_GRAYED);
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
		EnableMenuItem(hMenu, 
					   ID_VIEWCLIPBOARD, 
					   MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, 
					   ID_USECURRENT, 
					   MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, 
					   ID_HELP_TOPICS, 
					   MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, 
					   ID_LAUNCHPGPKEYS, 
					   MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, 
					   ID_LAUNCHPGPDISK, 
					   MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, 
					   ID_WIPECLIP, 
					   MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, 
					   ID_LAUNCHPGPTOOLS, 
					   MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, 
					   ID_QUIT_ENCLYPTOR, 
					   MF_BYCOMMAND | MF_GRAYED);
	}

	hSubMenu = GetSubMenu(hMenu,0);

     //  Calls to SetForegroundWindow and PostMessage to fix a bug
     //  documented in PSS ID Number: Q135788
	SetForegroundWindow(hwndTarget); 
	TrackPopupMenu( hSubMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON, 
					pt.x, pt.y, 0, hwndTarget, NULL);
	PostMessage( hwndTarget, WM_NULL, 0, 0 ) ;
// Above fixes "three clicks" problem  -wjb

	DestroyMenu(hMenu);

	return TRUE;
}

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
