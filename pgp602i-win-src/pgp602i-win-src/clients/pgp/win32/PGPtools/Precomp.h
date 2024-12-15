/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: Precomp.h,v 1.8.2.1.2.1 1998/11/12 03:13:43 heller Exp $
____________________________________________________________________________*/
#ifndef Included_PRECOMP_h	/* [ */
#define Included_PRECOMP_h

/*++

  File: precomp.h
  Author: Bill Blanke (wjb@pgp.com)
  Date: September 14, 1997
  Revisions:

--*/

#define WIN32_LEAN_AND_MEAN
#define PGPLIB 

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

#define PGPExport __declspec( dllimport )


// system include files
#include <windows.h>
#include <windowsx.h>
#include <assert.h>
#include <commdlg.h>
#include <stdio.h>
#include <stdlib.h>
#include <ShellAPI.h>

#define UNFINISHED_CODE_ALLOWED 1

// PGPlib headers

#include "..\include\PGPsc.h"

#include "openfile.h"
#include "proc.h"
#include "resource.h"

HINSTANCE g_hinst;
FARPROC origButtonProc;

void *PGPsc;
void *PGPtls;

#define IDM_STAYONTOP 1

#endif /* ] Included_PRECOMP_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
