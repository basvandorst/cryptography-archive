/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
/*4514 is "unreferenced inline func"*/
#pragma warning (disable : 4214 4201 4115 4514)
#include <windows.h>
#pragma warning (default : 4214 4201 4115)

#include "resource.h"

BOOL TaskbarAddIcon(HWND hwnd, UINT uID, HICON hicon, LPSTR lpszTip, UINT cbMsg)
{
	BOOL result;
	NOTIFYICONDATA tnid;

	tnid.cbSize				= sizeof(NOTIFYICONDATA);
	tnid.hWnd				= hwnd;
	tnid.uID				= uID;
	tnid.uFlags				= NIF_MESSAGE | NIF_ICON | NIF_TIP;
	tnid.uCallbackMessage	= cbMsg;
	tnid.hIcon				= hicon;

	if(lpszTip)
	{
		lstrcpyn(tnid.szTip, lpszTip, sizeof(tnid.szTip));
	}
	else
	{
		tnid.szTip[0] = '\0';
	}

	result = Shell_NotifyIcon(NIM_ADD, &tnid);

	if(hicon)
	{
		DestroyIcon(hicon);
	}

	return result;
}


BOOL TaskbarDeleteIcon(HWND hwnd, UINT uID)
{
	BOOL result;
	NOTIFYICONDATA tnid;

	tnid.cbSize				= sizeof(NOTIFYICONDATA);
	tnid.hWnd				= hwnd;
	tnid.uID				= uID;

	result = Shell_NotifyIcon(NIM_DELETE, &tnid);

	return result;
}
