/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: CBTProc.cpp,v 1.1.12.1 1998/11/12 03:12:42 heller Exp $
____________________________________________________________________________*/

#include <windows.h>
#include "PluginInfo.h"
#include "HookProcs.h"

extern HHOOK g_hHook[10];
extern int g_nNumHooks;
extern DWORD g_dwThreadID[10];

void AttachPluginToWindow(HWND hwnd);


LRESULT WINAPI CBTProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	int nIndex;
	DWORD dwThreadID;

	switch (nCode)
	{
	case HCBT_CREATEWND:
	case HCBT_ACTIVATE:
		{
			AttachPluginToWindow((HWND) wParam);
			break;
		}	
	}			
	
	dwThreadID = GetCurrentThreadId();

	for (nIndex=0; nIndex<g_nNumHooks; nIndex++)
	{
		if (g_dwThreadID[nIndex] == dwThreadID)
			break;
	}

	return CallNextHookEx(g_hHook[nIndex], nCode, wParam, lParam);
}


void AttachPluginToWindow(HWND hwnd)
{
	DLGPROC lpOldProc;
	PluginInfo *plugin;
	char szWndClass[1024];
	
	GetClassName(hwnd, szWndClass, 1023);

	if (strcmp(szWndClass, "ThorBrowserWndClass") && 
		strcmp(szWndClass, "ATH_Note"))
		return;
	
	lpOldProc = (DLGPROC)GetProp(hwnd, "oldproc");
	if (lpOldProc != NULL)
		return;

	plugin = CreatePluginInfo(hwnd);
	
	if (plugin == NULL)
		return;
	
	// Save away old proc
	SetProp(hwnd, "oldproc", 
		(HANDLE) GetWindowLong(hwnd, GWL_WNDPROC)); 
	
	// Subclass new Outlook Express main window
	if(!strcmp(szWndClass, "ThorBrowserWndClass")) 
		SetWindowLong(hwnd, GWL_WNDPROC, (DWORD) MainWndProc);
	
	// Subclass new Outlook Express message window	
	else
		SetWindowLong(hwnd, GWL_WNDPROC, (DWORD) UnknownWndProc);
	
	// Store the pointer to the plugin information
	SavePluginInfo(hwnd, plugin); 
	return;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
