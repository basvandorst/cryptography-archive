/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: Precomp.h,v 1.7 1997/09/19 20:16:24 wjb Exp $
____________________________________________________________________________*/
#ifndef Included_PRECOMP_h	/* [ */
#define Included_PRECOMP_h

/*++

Module Name:

    precomp.h

Abstract:

    This file includes all the headers required to build the dll
    to ease the process of building a precompiled header.

Author:


Revision History:


--*/

#define WIN32_LEAN_AND_MEAN
#define PGPExport __declspec( dllimport )

// System headers
#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h> 
#include <string.h>
#include <ShellAPI.h>

#define UNFINISHED_CODE_ALLOWED 1

// PGPlib headers
#include "..\include\PGPsc.h"

// PGPtray headers

#include "enclyptorproc.h"
#include "resource.h"
#include "taskbar.h"

HINSTANCE g_hinst;
HICON hIconKey;
HWND hwndTray;

void *PGPsc;
#define WM_TASKAREA_MESSAGE		WM_USER + 1

#endif /* ] Included_PRECOMP_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

