/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	PGPclx.h - internal header for PGPcl project
	

	$Id: PGPui32.h,v 1.13.2.1.2.1 1998/11/12 03:24:21 heller Exp $
____________________________________________________________________________*/
#ifndef Included_PGPUI32_h	/* [ */
#define Included_PGPUI32_h

#define UNFINISHED_CODE_ALLOWED		1
#define WIN32_LEAN_AND_MEAN

// Win32 header files
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <stddef.h>

#include "pgpUserInterface.h"

// PGP SDK header files
#include "pgpBase.h"
#include "pgpMem.h"
#include "pgpKeys.h"
#include "pgpErrors.h"
#include "pgpUtilities.h"
#include "pgpFeatures.h"
#include "pgpSDKPrefs.h"
#include "pflPrefs.h"
#include "pflPrefTypes.h"
#include "pgpFileSpec.h"
#include "pgpMemoryIO.h"
#include "pgpWordWrap.h"
#include "pgpRandomPool.h"
#include "pgpKeyServer.h"

// Our resource includes
#include "UIhelp.h"
#include "resource.h"

#ifdef __cplusplus
extern "C" {
#endif

void PGPsdkUICreateImageList(HIMAGELIST *hIml);

PGPError PGPsdkUIErrorBox (HWND hWnd,PGPError error);

void GetHelpDir(char *szHelp);

void InitRandomKeyHook(HHOOK* phhookKeyboard, HHOOK* phhookMouse);

void UninitRandomKeyHook(HHOOK hhookKeyboard, HHOOK hhookMouse);

void MoveOptionsControl(HWND hDlg,HWND *hwndChild,
					   PGPOptionListRef options,
					   int x,int y,
					   int dx,int dy);

#ifdef __cplusplus
}
#endif

#endif /* ] Included_PGPUI32_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

