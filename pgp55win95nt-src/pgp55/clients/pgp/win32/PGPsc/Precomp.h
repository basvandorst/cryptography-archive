/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: Precomp.h,v 1.22 1997/09/30 20:35:57 wjb Exp $
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
#define PGPExport __declspec( dllexport )

// System headers
#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h> 
#include <string.h>
#include <ShellAPI.h>
#include <Direct.h>
#include <CommDlg.h>
#include <OLE2.h>

#include "resource.h"                

#define UNFINISHED_CODE_ALLOWED 1

// PGPlib headers

#include "..\..\shared\pgpVersionHeader.h"
#include "..\..\shared\pgpBuildFlags.h"
#include "pgpKeys.h"
#include "pgpConfig.h"
#include "pgpErrors.h"
#include "pgpEncode.h"
#include "pgpUtilities.h"
#include "pgpSDKPrefs.h"

#ifndef WIN32

#define BST_CHECKED			1
#define ULONG unsigned long
#define USHORT unsigned short    
#define LPTSTR  char* 

#define MAX_PATH 256

#define CHAR char
#define INT int  
#define APIENTRY WINAPI   
#define BST_CHECKED 1
#define BST_UNCHECKED 0                 
#define UCHAR unsigned char
#define LPCTSTR char *

#endif
 

#include "..\include\pgprecip.h"    
#include "..\include\PGPphras.h"
#include "..\include\PGPcmdlg.h"
#include "..\include\PGPkm.h"
#include "..\..\shared\pgpClientPrefs.h"
#include "..\..\shared\pgpAdminPrefs.h"

// PGPtray headers

#include "ClipBrd.h"
#include "ClVwClip.h"
#include "..\include\PGPsc.h"
#include "working.h"
#include "progval.h"
#include "recpass.h"
#include "events.h"
#include "saveopen.h"
#include "resource.h"
#include "..\shared\SigEvent.h"

#define MAX_BUFFER_SIZE 500000

#define OPENKEYRINGFLAGS 0

HINSTANCE g_hinst;


#endif /* ] Included_PRECOMP_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
