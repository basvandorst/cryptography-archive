/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: EnclyptorProc.c,v 1.20 1997/09/20 18:47:56 wjb Exp $
____________________________________________________________________________*/
#include "precomp.h"

BOOL PopupTaskbarMenu(HWND hwndTarget, BOOL InPGPOperation);

LRESULT CALLBACK EnclyptorProc(HWND hwnd, UINT msg, 
							   WPARAM wParam, LPARAM lParam)
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
					DoEncrypt(hwnd, PGPsc, TRUE, FALSE);
					InPGPOperation = FALSE;
					break;
				}

				case ID_SIGN:
				{
					InPGPOperation = TRUE;
					DoEncrypt(hwnd, PGPsc, FALSE, TRUE);
					InPGPOperation = FALSE;
					break;
				}

				case ID_ENCRYPTSIGN:
				{
					InPGPOperation = TRUE;
					DoEncrypt(hwnd, PGPsc, TRUE, TRUE);
					InPGPOperation = FALSE;
					break;
				}

				case ID_DECRYPTVERIFY:
				{
					InPGPOperation = TRUE;
					DoDecrypt(hwnd, PGPsc);
					InPGPOperation = FALSE;
					break;
				}

				
				case ID_ADDKEY:
				{
					InPGPOperation = TRUE;
					DoAddKey(hwnd, PGPsc);
					InPGPOperation = FALSE;
					break;
				}

				case ID_WIPECLIP:
				{
					InPGPOperation = TRUE;
					ClipboardWipe(hwnd);
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

				case ID_PROPERTIES:
				{
					InPGPOperation = TRUE;
					PGPscPreferences(hwnd, PGPsc);
					InPGPOperation = FALSE;
					break;
				}

				case ID_QUIT_ENCLYPTOR:
				{
//					CleanupTempFiles();
					SetForegroundWindow(hwnd);
					PostMessage(hwnd, WM_CLOSE, 0, 0);
					break;
				}

				case ID_HELP_TOPICS:
				{
					InPGPOperation = TRUE;
					PGPpath(HelpFile);
					strcat(HelpFile,"PGP55.hlp");
					WinHelp(hwnd, HelpFile, HELP_FINDER, 0);
					InPGPOperation = FALSE;
					break;
				}
								
			}
			break;
		}
		
		case WM_ENDSESSION :
		{
//			CleanupTempFiles();
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


BOOL PopupTaskbarMenu(HWND hwndTarget, BOOL InPGPOperation)
{
	HMENU hMenu = NULL;
	HMENU hSubMenu = NULL;
	POINT pt;

	hMenu = LoadMenu(g_hinst, MAKEINTRESOURCE(IDM_TASKBARMENU));

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
					   ID_HELP_TOPICS, 
					   MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, 
					   ID_LAUNCHPGPKEYS, 
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

	GetCursorPos( &pt );

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
