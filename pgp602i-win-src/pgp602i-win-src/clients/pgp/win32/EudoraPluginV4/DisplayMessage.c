/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	DisplayMessage.c,v 1.3 1997/07/21 06:03:26 elrod Exp
____________________________________________________________________________*/

// System Headers
#include <windows.h>

// Project Headers
#include "resource.h"

int DisplayMessage(HWND hwnd, long StringId, long TitleId, UINT buttons)
{
	char szMessageBuffer[1024];
	char szTitleBuffer[256];
	HINSTANCE hInst = NULL;

	hInst = GetModuleHandle("PGPplugin.dll");

	LoadString(hInst, StringId, szMessageBuffer, sizeof(szMessageBuffer));
	LoadString(hInst, TitleId, szTitleBuffer, sizeof(szTitleBuffer));

	return MessageBox(hwnd, szMessageBuffer, szTitleBuffer, buttons);
}
