/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: main.cpp,v 1.2.12.1 1998/11/12 03:12:52 heller Exp $
____________________________________________________________________________*/

#include <windows.h>
#include "HookProcs.h"
#include "UIutils.h"

HHOOK g_hHook[10];
int g_nNumHooks = 0;
DWORD g_dwThreadID[10];

UINT WM_PGP_KILL_OELOADER = 0;

extern "C" {
__declspec(dllexport) void HookOutlookExpressPlugin(DWORD ThreadId);
};


BOOL WINAPI DllMain(HINSTANCE hinstDll, DWORD fdwReason, LPVOID)
{
 	if (DLL_PROCESS_ATTACH == fdwReason)
 	{
		UISetInstance(hinstDll);
		WM_PGP_KILL_OELOADER = 
			RegisterWindowMessage("PGP Kill OE Loader");
 	}

 	if (DLL_PROCESS_DETACH == fdwReason)
 	{
		int nIndex;
		DWORD dwThreadID;

		dwThreadID = GetCurrentThreadId();
		
		for (nIndex=0; nIndex<g_nNumHooks; nIndex++)
		{
			if (g_dwThreadID[nIndex] == dwThreadID)
				break;
		}

		if (nIndex < g_nNumHooks)
		{
			UnhookWindowsHookEx(g_hHook[nIndex]);
			g_nNumHooks--;
			for (; nIndex<g_nNumHooks; nIndex++)
			{
				g_dwThreadID[nIndex] == g_dwThreadID[nIndex+1];
				g_hHook[nIndex] = g_hHook[nIndex+1];
			}
		}

		if (!g_nNumHooks)
			PostMessage(HWND_BROADCAST, WM_PGP_KILL_OELOADER, 0, 0);
 	}

	return TRUE;
}


__declspec(dllexport) void HookOutlookExpressPlugin(DWORD dwThreadId)
{
	g_hHook[g_nNumHooks] = SetWindowsHookEx(WH_CBT, (HOOKPROC) CBTProc, 
								UIGetInstance(), dwThreadId);
	g_nNumHooks++;
	return;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
