/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: EudoraMainWndProc.c,v 1.22.8.1 1998/11/12 03:11:25 heller Exp $
____________________________________________________________________________*/

// System Headers
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <richedit.h>

// PGPsdk Headers
#include "pgpConfig.h"
#include "pgpKeys.h"
#include "pgpErrors.h"
#include "pgpUtilities.h"
#include "pgpTLS.h"

// Shared Headers
#include "PGPcl.h"

// Project Headers
#include "AddKey.h"
#include "PGPDefinedMessages.h"
#include "resource.h"

// Global Variables
HWND					g_hwndEudoraStatusbar = NULL;
HWND					g_hwndEudoraMainWindow = NULL;

extern HINSTANCE		g_hinst;
extern PGPContextRef	g_pgpContext;
extern PGPtlsContextRef	g_tlsContext;
extern HHOOK			g_hCBTHook;
extern UINT				g_nPurgeCacheMsg;


LRESULT 
WINAPI 
EudoraMainWndProc(HWND hwnd, 
				  UINT msg, 
				  WPARAM wParam, 
				  LPARAM lParam)
{
	WNDPROC lpOldProc;
	static BOOL bShutdown = FALSE;

	lpOldProc = (WNDPROC)GetProp( hwnd, "oldproc" );
   
	if (msg == g_nPurgeCacheMsg)
	{
		PGPclPurgeCachedPassphrase(wParam);
		return TRUE;
	}

	switch(msg)
	{ 

		case WM_DESTROY:   
		{
			//  Put back old window proc and
			SetWindowLong( hwnd, GWL_WNDPROC, (DWORD)lpOldProc );

			if(g_hCBTHook)
			{
				UnhookWindowsHookEx(g_hCBTHook);
			}

			PGPclPurgeCachedPassphrase (
				PGPCL_DECRYPTIONCACHE|PGPCL_SIGNINGCACHE);

			if( PGPRefIsValid(g_pgpContext) )
			{
				PGPclCloseLibrary();
				PGPFreeTLSContext(g_tlsContext);
				PGPFreeContext(g_pgpContext);
			}


			RemoveProp(hwnd, "oldproc");
			break;
		}

		case WM_ENDSESSION:
		{
			bShutdown = (BOOL)wParam;

			SendMessage(hwnd, WM_CLOSE, 0,0);
			return 0;
			break;
		}

		case WM_MENUSELECT:
		{
			char szStatus[256];

			strcpy(szStatus, "");

			switch(LOWORD(wParam))
			{
				case ID_PREFERENCES:
				{
					LoadString(g_hinst, IDS_PGPPREFSTATUS, szStatus,
						sizeof(szStatus));
					break;
				}
								

				case ID_ABOUTPGP:
				{
					LoadString(g_hinst, IDS_PGPABOUTSTATUS, szStatus,
						sizeof(szStatus));
					break;
				}
				

				case ID_PGPKEYS:
				{
					LoadString(g_hinst, IDS_PGPKEYSSTATUS, szStatus,
						sizeof(szStatus));
					break;
				}
				

				case ID_PGPHELP:
				{
					LoadString(g_hinst, IDS_PGPHELPSTATUS, szStatus,
						sizeof(szStatus));
					break;
				}

				case ID_PGPDECRYPTVERIFY:
				{
					LoadString(g_hinst, IDS_PGPDECRYPTSTATUS, szStatus,
						sizeof(szStatus));
					break;
				}

				case ID_PGPENCRYPTSIGN:
				{
					LoadString(g_hinst, IDS_PGPENCRYPTSIGNSTATUS, szStatus,
						sizeof(szStatus));
					break;
				}
				case ID_PGPSIGN:
				{
					LoadString(g_hinst, IDS_PGPSIGNSTATUS, szStatus,
						sizeof(szStatus));
					break;
				}
				case ID_PGPENCRYPT:
				{
					LoadString(g_hinst, IDS_PGPENCRYPTSTATUS, szStatus,
						sizeof(szStatus));
					break;
				}

			}	
			
			if (strlen(szStatus) > 0)
				SendMessage(g_hwndEudoraStatusbar,
					SB_SETTEXT,
					0,
					(LPARAM) szStatus);

			break;
		}

		case WM_INITMENUPOPUP:
		{
			HMENU hmenuPopup = (HMENU) wParam;    // handle of submenu
			UINT uPos = (UINT) LOWORD(lParam);    // submenu item position 
			BOOL fSystemMenu = (BOOL) HIWORD(lParam); // window menu flag 
			LRESULT ReturnValue = 0;

			ReturnValue = CallWindowProc(	lpOldProc, 
											hwnd, 
											msg, 
											wParam, 
											lParam ) ;

			if(hmenuPopup && !fSystemMenu)
			{
				char szMenuString[256] = {0x00};
				char szEncryptMenu[256];
				char szPGPkeysMenu[256];
				char szDecryptMenu[256];

				GetMenuString(	hmenuPopup, 
								0, 
								szMenuString, 
								sizeof(szMenuString), 
								MF_BYPOSITION);

				LoadString(g_hinst, IDS_ENCRYPTMENU, szEncryptMenu,
					sizeof(szEncryptMenu));
				LoadString(g_hinst, IDS_PGPKEYSMENU, szPGPkeysMenu,
					sizeof(szPGPkeysMenu));
				LoadString(g_hinst, IDS_DECRYPTMENU, szDecryptMenu,
					sizeof(szDecryptMenu));

				if( !strcmp(szMenuString, szEncryptMenu) || 
					!strcmp(szMenuString, szPGPkeysMenu) || 
					!strcmp(szMenuString, szDecryptMenu) )
				{
					EnableMenuItem( hmenuPopup, 0, MF_BYPOSITION|MF_ENABLED); 
					EnableMenuItem( hmenuPopup, 1, MF_BYPOSITION|MF_ENABLED); 
					EnableMenuItem( hmenuPopup, 2, MF_BYPOSITION|MF_ENABLED); 
					EnableMenuItem( hmenuPopup, 3, MF_BYPOSITION|MF_ENABLED); 
					EnableMenuItem( hmenuPopup, 4, MF_BYPOSITION|MF_ENABLED); 
				}
			}

			return ReturnValue;
			break;
		}


		case WM_INITMENU:
		{
			HMENU hmenu = (HMENU) wParam;
			LRESULT ReturnValue = 0;

			ReturnValue = CallWindowProc(	lpOldProc, 
											hwnd, 
											msg, 
											wParam, 
											lParam ) ;

			if(hmenu)
			{
				HMENU submenu = NULL;
				HMENU PGPMenu = NULL;
				char szMenuString[256] = {0x00};
				char szCutMenu[256];
				char szCopyMenu[256];

				GetMenuString(	hmenu, 
								0, 
								szMenuString, 
								sizeof(szMenuString), 
								MF_BYPOSITION);

				LoadString(g_hinst, IDS_CUTMENU, szCutMenu,
					sizeof(szCutMenu));
				LoadString(g_hinst, IDS_COPYMENU, szCopyMenu,
					sizeof(szCopyMenu));

				if(!strcmp(szMenuString, szCutMenu))
				{
					PGPMenu = LoadMenu(	g_hinst, 
										MAKEINTRESOURCE(IDM_PGPENCRYPTMENU));

					AppendMenu(hmenu, MF_ENABLED|MF_SEPARATOR , 0, "");
					AppendMenu(hmenu, MF_ENABLED|MF_STRING|MF_POPUP , 
												(UINT)PGPMenu, "P&GP");
				}
				else if(!strcmp(szMenuString, szCopyMenu))
				{
					PGPMenu = LoadMenu(	g_hinst, 
										MAKEINTRESOURCE(IDM_PGPDECRYPTMENU));

					AppendMenu(hmenu, MF_ENABLED|MF_SEPARATOR , 0, "");
					AppendMenu(hmenu, MF_ENABLED|MF_STRING|MF_POPUP , 
												(UINT)PGPMenu, "P&GP");
				}
			}

			return ReturnValue;
			break;
		}

		case WM_COMMAND:
		{
			switch( LOWORD(wParam) )
			{
				case IDC_SENDMAIL:
				{
					HWND hwndMDIClient = NULL;
					HWND hwndActive = NULL;

					hwndMDIClient = FindWindowEx(	hwnd, 
													NULL, 
													"MDIClient", 
													NULL);

					if(hwndMDIClient)
					{
						hwndActive = (HWND)SendMessage(	hwndMDIClient, 
														WM_MDIGETACTIVE, 
														0, 
														0);

						if(hwndActive)
						{
							HWND hwndToolbar = NULL;

							hwndToolbar =  FindWindowEx(hwndActive, 
														NULL, 
														"ToolbarWindow32", 
														NULL);

							if(hwndToolbar)
							{
								SendMessage(hwndToolbar, 
											WM_COMMAND,
											(WPARAM)IDC_SENDMAIL, 
											0);
								return 0;
							}
						}
					}
					break;
				}	

				case ID_ABOUTPGP:
				{
					PGPclHelpAbout (g_pgpContext, hwnd, NULL, NULL, NULL);
					break;
				}

				case ID_PGPHELP:
				{
					CHAR szHelpFile[MAX_PATH] = {0x00};
					char szKey[] =	"Software\\Microsoft\\Windows\\"
									"CurrentVersion\\App Paths\\PGPKeys.exe";
					HKEY hKey;
					LONG lResult;
					CHAR* p;
					DWORD dwValueType, dwSize;
					char szHelpName[256];

					LoadString(g_hinst, IDS_HELPFILE, szHelpName,
						sizeof(szHelpName));

					lResult = RegOpenKeyEx (HKEY_LOCAL_MACHINE, 
											szKey,
											0, 
											KEY_READ, 
											&hKey);

					if (lResult == ERROR_SUCCESS) 
					{
						dwSize = MAX_PATH;
						lResult = RegQueryValueEx (	hKey, 
													NULL, 
													NULL, 
													&dwValueType, 
													(LPBYTE)szHelpFile, 
													&dwSize);
						RegCloseKey (hKey);
						p = strrchr (szHelpFile, '\\');
						if (p) 
						{
							p++;
							*p = '\0';
							strcat(szHelpFile, szHelpName);
						}
						else 
						{	
							lstrcpy (szHelpFile, "");
						}
					}

					WinHelp (hwnd, szHelpFile, HELP_FINDER, 0);

					break;
				}

				case ID_PGPKEYS:
				{
					char szPath[MAX_PATH];
					char szPGPkeys[256];
					PGPError error = kPGPError_NoErr;

					LoadString(g_hinst, IDS_PGPKEYSEXE, szPGPkeys,
						sizeof(szPGPkeys));

					error = PGPclGetPGPPath (szPath, sizeof(szPath));

					if( IsntPGPError(error) )
					{
						// '/s' keeps it from showing that 
						// damn splash screen
						strcat(szPath, szPGPkeys);
						// run it...
						WinExec(szPath, SW_SHOW);
					}
					else
					{
						char szError[256];

						LoadString(g_hinst, IDS_E_LAUNCHPGPKEYS, szError,
							sizeof(szError));

						MessageBox(NULL, 
							szError, 
							0, 
							MB_OK);
					}
					
					break;
				}	

				case ID_PREFERENCES:
				{
					PGPclPreferences (g_pgpContext, hwnd, 2, NULL);
					break;
				}	

				case ID_PGPDECRYPTVERIFY:
				{
					HWND hwndMDIClient = NULL;
					HWND hwndActive = NULL;

					hwndMDIClient = FindWindowEx	(hwnd, 
													NULL, 
													"MDIClient", 
													NULL);

					if(hwndMDIClient)
					{
						hwndActive = (HWND)SendMessage(	hwndMDIClient, 
														WM_MDIGETACTIVE, 
														0, 
														0);

						if(hwndActive)
						{
							HWND hwndToolbar = NULL;

							hwndToolbar =  FindWindowEx(hwndActive, 
														NULL, 
														"ToolbarWindow32", 
														NULL);

							if(hwndToolbar)
							{
								switch(wParam)
								{
									case ID_PGPDECRYPTVERIFY:
									{
										SendMessage(hwndToolbar, 
													WM_COMMAND,
													(WPARAM)IDC_DECRYPT,
													0);	
										break;
									}
								}
							}
						}	
					}
					break;
				}

				case ID_PGPENCRYPTSIGN:
				case ID_PGPSIGN:
				case ID_PGPENCRYPT:
				{
					HWND hwndMDIClient = NULL;
					HWND hwndActive = NULL;

					
					hwndMDIClient = FindWindowEx(	hwnd, 
													NULL, 
													"MDIClient", 
													NULL);

					if(hwndMDIClient)
					{
						hwndActive = (HWND)SendMessage(hwndMDIClient, 
														WM_MDIGETACTIVE, 
														0, 
														0);

						if(hwndActive)
						{
							HWND hwndToolbar = NULL;
							HWND hwndPGPToolbar = NULL;

							hwndToolbar =  FindWindowEx(hwndActive, 
														NULL, 
														"ToolbarWindow32", 
														NULL);

							hwndPGPToolbar = FindWindowEx(	hwndToolbar, 
															NULL, 
															"ToolbarWindow32",
															NULL);

							if(hwndPGPToolbar)
							{
								switch(wParam)
								{
									case ID_PGPENCRYPT:
									{
										SendMessage(hwndPGPToolbar,
													TB_CHECKBUTTON,
													(WPARAM)IDC_ENCRYPT, 
													MAKELPARAM(TRUE, 0));
										break;
									}

									case ID_PGPSIGN:
									{
										SendMessage(hwndPGPToolbar,
													TB_CHECKBUTTON,
													(WPARAM)IDC_SIGN, 
													MAKELPARAM(TRUE, 0));
										break;
									}

									case ID_PGPENCRYPTSIGN:
									{
										SendMessage(hwndPGPToolbar,
											TB_CHECKBUTTON,
											(WPARAM)IDC_ENCRYPT, 
											MAKELPARAM(TRUE, 0));

										SendMessage(hwndPGPToolbar,
											TB_CHECKBUTTON,
											(WPARAM)IDC_SIGN, 
											MAKELPARAM(TRUE, 0));
										break;
									}
								}
								
								SendMessage(hwndToolbar, 
											WM_COMMAND,
											(WPARAM)IDC_JUSTDOIT, 
											(LPARAM)SENT_FROM_CONTEXT_MENU);

							}
						}	
					}
					
					break;
				}
			}

			break;
		}
	} 
	
	//  Pass all non-custom messages to old window proc
	return CallWindowProc(lpOldProc, hwnd, msg, wParam, lParam ) ;
}


LRESULT WINAPI EudoraStatusbarWndProc(HWND hwnd, 
									  UINT msg, 
									  WPARAM wParam, 
									  LPARAM lParam)
{
	WNDPROC lpOldProc;

	lpOldProc = (WNDPROC)GetProp( hwnd, "oldproc" );
   
	switch(msg)
	{
		case WM_DESTROY:   
		{
			//  Put back old window proc and
			SetWindowLong( hwnd, GWL_WNDPROC, (DWORD)lpOldProc );

			//  remove window property
			RemoveProp( hwnd, "oldproc" ); 
			break;
		}

	} 
	
	//  Pass all non-custom messages to old window proc
	return CallWindowProc(lpOldProc, hwnd, msg, wParam, lParam ) ;
}
