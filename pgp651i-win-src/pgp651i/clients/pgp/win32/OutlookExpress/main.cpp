/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: main.cpp,v 1.6 1999/04/09 15:26:05 dgal Exp $
____________________________________________________________________________*/

#include <windows.h>
#include "PluginInfo.h"
#include "HookProcs.h"
#include "UIutils.h"

extern "C" {
__declspec(dllexport) void AttachOutlookExpressPlugin(HWND hwnd);
};


BOOL WINAPI DllMain(HINSTANCE hinstDll, DWORD fdwReason, LPVOID)
{
 	if (DLL_PROCESS_ATTACH == fdwReason)
		UISetInstance(hinstDll);

	return TRUE;
}


__declspec(dllexport) void AttachOutlookExpressPlugin(HWND hwnd)
{
	PluginInfo *plugin;
	char szWndClass[1024];
	
	GetClassName(hwnd, szWndClass, 1023);

	plugin = CreatePluginInfo(hwnd);
	
	if (plugin == NULL)
		return;
	
	// Save away old proc
	SetProp(hwnd, "oldproc", 
		(HANDLE) GetWindowLong(hwnd, GWL_WNDPROC)); 
	
	// Subclass Outlook Express 4.x main window
	if (!strcmp(szWndClass, "ThorBrowserWndClass")) 
		SetWindowLong(hwnd, GWL_WNDPROC, (DWORD) MainWndProc);

	// Subclass Outlook Express 5.x main window
	else if (!strcmp(szWndClass, "Outlook Express Browser Class"))
	{
		plugin->bOE5 = TRUE;
		SetWindowLong(hwnd, GWL_WNDPROC, (DWORD) MainWndProc);
	}

	// Subclass Outlook Express message window	
	else if (!strcmp(szWndClass, "ATH_Note"))
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
