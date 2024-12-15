/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	TimeDate.c - implements entrypoints for timedate control
	

	$Id: TimeDate.c,v 1.6 1997/09/10 08:28:49 elrod Exp $
____________________________________________________________________________*/
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <time.h>

#include "pgpKeys.h"
#include "PGPcmdlg.h"
#include "resource.h"

#include "TimeDateWndProc.h"

#define MAX_YEAR	2050
#define MIN_YEAR	1970


BOOL PGPcdExport
PGPcomdlgInitTimeDateControl(HINSTANCE hinst)
{
	WNDCLASS wc;
	static BOOL bRegistered = FALSE;

	memset(&wc, 0x00, sizeof(wc));

	wc.style = CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS;                     
	wc.lpfnWndProc = (WNDPROC)TimeDateWndProc; 
	wc.hInstance = hinst;       
	wc.hCursor = LoadCursor (NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszClassName = "TimeDateClass";

	if(RegisterClass(&wc))
	{
		wc.style = CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS;                     
		wc.lpfnWndProc = (WNDPROC)TimeDateFrameWndProc; 
		wc.hInstance = hinst;       
		wc.hCursor = LoadCursor (NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wc.lpszClassName = "TimeDateFrameClass";

		if(RegisterClass(&wc))
		{
			bRegistered = TRUE;
		}
	}
 
	return bRegistered;
}

HWND PGPcdExport
PGPcomdlgCreateTimeDateControl(	HWND hwndParent, 
							HINSTANCE hinst,
							LONG xPos,
							LONG yPos,
							LONG type)
{
	HWND hwnd = NULL;
	HWND hwndTime = NULL;

	hwnd = CreateWindowEx(	WS_EX_CONTROLPARENT,
							"TimeDateFrameClass", 
							"", 
							WS_CHILD|WS_VISIBLE|WS_TABSTOP |WS_GROUP,
							xPos, 
							yPos, 
							90,
							20, 
							hwndParent, 
							NULL, 
							hinst, 
							NULL);

	hwndTime = CreateWindowEx (	WS_EX_CLIENTEDGE|WS_EX_CONTROLPARENT, 
								"TimeDateClass", 
								"", 
								WS_CHILD|WS_VISIBLE|WS_TABSTOP |WS_GROUP,
								0,//xPos, 
								0,//yPos, 
								90,
								20, 
								hwnd, 
								NULL, 
								hinst, 
								(LPVOID)type);	

	SetProp(hwnd, "TimeDateWindow", hwndTime);

	return hwnd;
}
	


