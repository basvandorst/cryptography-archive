/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: Taskbar.c,v 1.6 1999/03/10 03:03:33 heller Exp $
____________________________________________________________________________*/
#include "precomp.h"

BOOL TaskbarAddIcon(HWND hwnd, UINT uID, HICON hicon, 
					LPSTR lpszTip, UINT cbMsg)
{
	BOOL result;
	NOTIFYICONDATA tnid;

	memset(&tnid,0x00,sizeof(NOTIFYICONDATA));

	tnid.cbSize				= sizeof(NOTIFYICONDATA);
	tnid.hWnd				= hwnd;
	tnid.uID				= uID;
	tnid.uFlags				= NIF_MESSAGE | NIF_ICON | NIF_TIP;
	tnid.uCallbackMessage	= cbMsg;
	tnid.hIcon				= hicon;

	if(lpszTip)
	{
		strcpy(tnid.szTip, lpszTip);
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

	memset(&tnid,0x00,sizeof(NOTIFYICONDATA));

	tnid.cbSize				= sizeof(NOTIFYICONDATA);
	tnid.hWnd				= hwnd;
	tnid.uID				= uID;

	result = Shell_NotifyIcon(NIM_DELETE, &tnid);

	return result;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
