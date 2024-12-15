/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: hook.c,v 1.6 1999/03/16 18:20:01 dgal Exp $
____________________________________________________________________________*/

#include "windows.h"
#include "resource.h"
#include "..\include\PGPhk.h"

HINSTANCE g_hinst;

#define OE_REG_KEY	"Software\\Network Associates\\PGP\\OE"

#pragma data_seg(".SHARDAT")
static HHOOK  g_hCBTHook   = NULL ;
static DWORD  TrayProcessID = 0   ;
static HWND   CurrentFocus = NULL ; // SetFocus window
#pragma data_seg()

void (*AttachOutlookExpressPlugin)(HWND);

LRESULT WINAPI CBTProc (int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0)
	{
		switch (nCode)
        {
			case HCBT_SETFOCUS:
			{
				HWND hwndCurrent;
				DWORD ThreadID;
				DWORD ProcessID;

				hwndCurrent=(HWND)LOWORD(wParam);

				ThreadID=GetWindowThreadProcessId(hwndCurrent,&ProcessID);
				if((ProcessID!=TrayProcessID)&&(CurrentFocus!=hwndCurrent)&&
					(hwndCurrent!=NULL))
				{
					CurrentFocus=hwndCurrent;
				}
				break;
			}	
			
			case HCBT_CREATEWND:
			case HCBT_ACTIVATE:
			{
				char szWndClass[1024];
				HANDLE hPGPoe;
				DLGPROC lpOldProc;
				HKEY hkey;

				/* Don't try loading Outlook Express plug-in if
					it wasn't installed */

				if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, OE_REG_KEY, 0, 
						KEY_ALL_ACCESS, &hkey) != ERROR_SUCCESS)
					break;

				GetClassName((HWND) wParam, szWndClass, 1023);

				if (!strcmp(szWndClass, "ThorBrowserWndClass") || 
					!strcmp(szWndClass, "ATH_Note") ||
					!strcmp(szWndClass, "Outlook Express Browser Class"))
				{
					/* Check to make sure we haven't already attached */

					lpOldProc = (DLGPROC)GetProp((HWND) wParam, "oldproc");
					if (lpOldProc != NULL)
					{
						RegCloseKey(hkey);
						break;
					}

					/* Attempt to load the Outlook Express plug-in DLL */

					hPGPoe = LoadLibrary("PGPoe.dll");
					if (hPGPoe == NULL)
					{
						char szMsg[255];
						char szTitle[255];

						RegDeleteKey(hkey, "OutlookExpress");
						RegCloseKey(hkey);
						RegDeleteKey(HKEY_LOCAL_MACHINE, OE_REG_KEY);

						LoadString(g_hinst, IDS_NOPLUGIN, szMsg, 254);
						LoadString(g_hinst, IDS_TITLE, szTitle, 254);
						MessageBox(NULL, szMsg, szTitle, MB_ICONEXCLAMATION);
					}
					else
					{
						AttachOutlookExpressPlugin = (void (*)(HWND)) 
							GetProcAddress(hPGPoe, 
								"AttachOutlookExpressPlugin");

						AttachOutlookExpressPlugin((HWND) wParam);
						RegCloseKey(hkey);
					}
				}
				else
					RegCloseKey(hkey);
				break;
			}	
		}			
	}

	return CallNextHookEx (g_hCBTHook, nCode, wParam, lParam);
}


HWND ReadCurrentFocus(void)
{
	return CurrentFocus;
}

void SetHook(void)
{
	TrayProcessID=GetCurrentProcessId();

	g_hCBTHook=SetWindowsHookEx(WH_CBT, 
		(HOOKPROC)CBTProc,
		g_hinst,
		0);
}

void DeleteHook(void)
{
	UnhookWindowsHookEx(g_hCBTHook);
}
		
BOOL WINAPI DllMain (HINSTANCE hinstDll, DWORD fdwReason,LPVOID lpvReserved)
{
   switch (fdwReason) {
      case DLL_PROCESS_ATTACH:
         g_hinst = hinstDll;
		 //vErrorOut(fg_white,"DLL_PROCESS_ATTACH\n");
         break;
      case DLL_THREAD_ATTACH:
		  //vErrorOut(fg_white,"DLL_THREAD_ATTACH\n");
         break;
      case DLL_THREAD_DETACH:
		  //vErrorOut(fg_red,"DLL_THREAD_DETACH\n");
		break;
      case DLL_PROCESS_DETACH:
		  //vErrorOut(fg_red,"DLL_PROCESS_DETACH\n");
         break;
   }
   return(TRUE);
}

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
