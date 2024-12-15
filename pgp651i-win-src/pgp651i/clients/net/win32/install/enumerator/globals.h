/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	

	$Id: globals.h,v 1.9 1999/03/30 23:12:37 philipn Exp $
____________________________________________________________________________*/
#pragma once

#ifndef GLOBALS_H
#define GLOBALS_H

#include <windows.h>
#include <commctrl.h>


/*Globals*/
HINSTANCE	g_hInstance;
HIMAGELIST	g_hImages;
HICON		ghIcon;	

LPBYTE		lpbActualServiceName;
static char szAppName      []				= "Set Adapter";
static char szShortAppName []				= "Set Adapter";
BOOL		bWan; 
BOOL		bSilentInstall;
BOOL		bIsNT;
BOOL		bSansCmdLine;
BOOL		bDelReg;
BOOL		bRunOnce;

#endif