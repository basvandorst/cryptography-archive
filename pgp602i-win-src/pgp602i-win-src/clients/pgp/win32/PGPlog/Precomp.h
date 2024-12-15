/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: Precomp.h,v 1.8.2.1.2.1 1998/11/12 03:13:21 heller Exp $
____________________________________________________________________________*/
#ifndef Included_PRECOMP_h	/* [ */
#define Included_PRECOMP_h

/*++

  File: precomp.h
  Author: Bill Blanke (wjb@pgp.com)
  Date: September 25, 1997
  Revisions:

--*/

#define WIN32_LEAN_AND_MEAN
#define PGPExport __declspec( dllimport )

#ifndef WIN32

#define BST_CHECKED			1
#define ULONG unsigned long
#define USHORT unsigned short    
#define LPTSTR  char* 

#define MAX_PATH 256

#define UCHAR unsigned char
#define CHAR char
#define INT int  
#define APIENTRY WINAPI   
#define BST_CHECKED 1
#define BST_UNCHECKED 0                 

#define LPCTSTR char *

#endif

#define STATUS_SUCCESS			0x0000000
#define STATUS_FAILURE			0x0000001
#define STATUS_CANCEL			0x0000002

#define NOERROR	0

// system include files
#include <windows.h>
#include <windowsx.h>
#include <assert.h>
#include <commdlg.h>
#include <stdio.h>
#include <stdlib.h>
#include <DDEML.h>
#include <commctrl.h>

// project include files
#include "..\include\PGPsc.h"
#include "..\include\PGPImage.h"

#include "ListMng.h"
#include "OwnDraw.h"
#include "resource.h"

#define DISMISSY 25
#define IDC_LOGLIST 101   

HFONT hFont;
LISTSTRUCT logList;
DRAWSTRUCT logDraw;

HINSTANCE g_hinst;

void *PGPsc;
void *PGPtls;

#endif /* ] Included_PRECOMP_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
