/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: hook.c,v 1.1.2.2.2.1 1998/11/12 03:13:17 heller Exp $
____________________________________________________________________________*/

#include "windows.h"
#include "..\include\PGPhk.h"

HINSTANCE g_hinst;

#pragma data_seg(".SHARDAT")
static HHOOK  g_hCBTHook   = NULL ;
static DWORD  TrayProcessID = 0   ;
static HWND   CurrentFocus = NULL ; // SetFocus window
#pragma data_seg()

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
