/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: EudoraMainWndProc.c,v 1.12 1997/10/22 23:05:49 elrod Exp $
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

// Shared Headers
#include "PGPcmdlg.h"
#include "PGPphras.h"
#include "PGPkm.h"

// Project Headers
#include "AddKey.h"
#include "PGPDefinedMessages.h"
#include "resource.h"

// Global Variables
HWND					g_hwndEudoraStatusbar = NULL;
HWND					g_hwndEudoraMainWindow = NULL;

extern HINSTANCE		g_hinst;
extern PGPContextRef	g_pgpContext;
extern HHOOK			g_hCBTHook;


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

			PGPPurgeCachedPhrase();
			PGPPurgeSignCachedPhrase();

			if( PGPRefIsValid(g_pgpContext) )
			{
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
			switch(LOWORD(wParam))
			{
				case ID_PREFERENCES:
				{
					SendMessage(g_hwndEudoraStatusbar,
								SB_SETTEXT,
								0,
								(LPARAM) "Edit your PGP preferences");
					break;
				}
								

				case ID_ABOUTPGP:
				{
					SendMessage(g_hwndEudoraStatusbar,
								SB_SETTEXT,
								0,
								(LPARAM)"Display PGP product and "
										"version information");
					break;
				}
				

				case ID_PGPKEYS:
				{
					SendMessage(g_hwndEudoraStatusbar,
								SB_SETTEXT,
								0,
								(LPARAM)"Launch the PGPkeys Application");
					break;
				}
				

				case ID_PGPHELP:
				{
					SendMessage(g_hwndEudoraStatusbar,
								SB_SETTEXT,
								0,
								(LPARAM)"Need help with PGP? You\'ll find it "
										"here");
					break;
				}

				case ID_PGPGETKEY:
				{
					SendMessage(g_hwndEudoraStatusbar,
								SB_SETTEXT,
								0,
								(LPARAM)"Extract PGP Key(s) from Email "
										"Message or Selected Text");
					break;
				}

				case ID_PGPDECRYPTVERIFY:
				{
					SendMessage(g_hwndEudoraStatusbar,
								SB_SETTEXT,
								0,
								(LPARAM)"Decrypt PGP Encrypted Email "
										"Message or Selected Text");
					break;
				}

				case ID_PGPENCRYPTSIGN:
				{
					SendMessage(g_hwndEudoraStatusbar,
								SB_SETTEXT,
								0,
								(LPARAM)"PGP Encrypt and Sign Email "
										"Message or Selected Text");
					break;
				}
				case ID_PGPSIGN:
				{
					SendMessage(g_hwndEudoraStatusbar,
								SB_SETTEXT,
								0,
								(LPARAM)"PGP Sign Email Message or Selected "
										"Text");
					break;
				}
				case ID_PGPENCRYPT:
				{
					SendMessage(g_hwndEudoraStatusbar,
								SB_SETTEXT,
								0,
								(LPARAM)"PGP Encrypt Email Message or "
										"Selected Text");
					break;
				}

			}	
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

				GetMenuString(	hmenuPopup, 
								0, 
								szMenuString, 
								sizeof(szMenuString), 
								MF_BYPOSITION);

				if( !strcmp(szMenuString, "&Encrypt") || 
					!strcmp(szMenuString, "&Launch PGPkeys...") || 
					!strcmp(szMenuString, "&Decrypt and Verify") )
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

				GetMenuString(	hmenu, 
								0, 
								szMenuString, 
								sizeof(szMenuString), 
								MF_BYPOSITION);

				if(!strcmp(szMenuString, "Cu&t"))
				{
					PGPMenu = LoadMenu(	g_hinst, 
										MAKEINTRESOURCE(IDM_PGPENCRYPTMENU));

					AppendMenu(hmenu, MF_ENABLED|MF_SEPARATOR , 0, "");
					AppendMenu(hmenu, MF_ENABLED|MF_STRING|MF_POPUP , 
												(UINT)PGPMenu, "P&GP");
				}
				else if(!strcmp(szMenuString, "&Copy"))
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
					PGPcomdlgHelpAbout (hwnd, NULL, NULL, NULL);
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
							strcat(szHelpFile, "PGP55.hlp");
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
					PGPError error = kPGPError_NoErr;

					error = PGPcomdlgGetPGPPath (szPath, sizeof(szPath));

					if( IsntPGPError(error) )
					{
						// '/s' keeps it from showing that 
						// damn splash screen
						strcat(szPath, "PGPkeys.exe /s");
						// run it...
						WinExec(szPath, SW_SHOW);
					}
					else
					{
						MessageBox(NULL, 
							"Unable to locate the PGPkeys Application", 
							0, 
							MB_OK);
					}
					
					break;
				}	

				case ID_PREFERENCES:
				{
					PGPcomdlgPreferences (g_pgpContext, hwnd, 2);
					break;
				}	

				case ID_PGPGETKEY:
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

									case ID_PGPGETKEY:
									{
										SendMessage(hwndToolbar, 
													WM_COMMAND,
													(WPARAM)IDC_GETKEY,
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
