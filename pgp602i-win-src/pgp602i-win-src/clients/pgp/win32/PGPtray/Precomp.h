/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: Precomp.h,v 1.9.8.1 1998/11/12 03:13:51 heller Exp $
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
#include "..\..\shared\pgpBuildFlags.h"

// PGPtray headers

#include "enclyptorproc.h"
#include "resource.h"
#include "taskbar.h"

HINSTANCE g_hinst;
HICON hIconKey;
HWND hwndTray;

void *PGPsc;
void *PGPtls;

#define WM_TASKAREA_MESSAGE		WM_USER + 1

#endif /* ] Included_PRECOMP_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

