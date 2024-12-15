/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	PGPcdlgx.h - internal header for PGPcomdlg project
	

	$Id: PGPcdlgx.h,v 1.16 1997/10/07 20:38:13 pbj Exp $
____________________________________________________________________________*/
#ifndef Included_PGPcdlgx_h	/* [ */
#define Included_PGPcdlgx_h

#define UNFINISHED_CODE_ALLOWED 1

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

#include "stddef.h"

#include "resource.h"

#include "pgpBuildFlags.h"
#include "pgpBase.h"
#include "pgpMem.h"
#include "pgpKeys.h"
#include "pgpErrors.h"
#include "pgpUtilities.h"
#include "pgpFeatures.h"
#include "pgpSDKPrefs.h"
#include "pflPrefs.h"
#include "pflPrefTypes.h"
#include "pgpAdminPrefs.h"
#include "pgpClientPrefs.h"
#include "pgpFileSpec.h"

// wjb
#include "pgpMemoryIO.h"
#include "pgpWordWrap.h"
// end wjb

#include "..\include\help\PGPcdHlp.h"	// file containing constants
//							               passed to WINHELP

#include "..\include\PGPWerr.h"
#include "..\include\PGPcmdlg.h"

#define LINCENSEEX	150
#define LINCENSEEY	8

#define IDC_KSTREELIST		5555

// prototypes

// PGPcmdlg.c
VOID* pcAlloc (UINT uBytes);
VOID pcFree (VOID* p);
VOID PaintUserInfo (HDC hDC, INT iXpos, INT iYpos);
LRESULT PGPcomdlgMessageBox (HWND hWnd, INT iCaption, INT iMessage, 
							 ULONG ulFlags);
HBITMAP LoadResourceBitmap (HINSTANCE hInstance, LPSTR lpString,
                            HPALETTE FAR* lphPalette);

// CDksPref.c
HWND CDCreateKeyserverTreeList (PGPPrefRef PrefRef, HWND hWndParent);
INT CDDestroyKeyserverTreeList (HWND hWndTree);
INT CDNewKeyserver (HWND hWndParent, HWND hWndTree);
INT CDRemoveKeyserver (HWND hWndParent, HWND hWndTree);
INT CDSetDefaultKeyserver (HWND hWndParent, HWND hWndTree);
INT CDProcessKeyserverTreeList (HWND hWndParent, HWND hWndTree, 
							WPARAM wParam, LPARAM lParam);
INT CDSaveKeyserverPrefs (PGPPrefRef PrefRef, HWND hWndParent, HWND hWndTree);
VOID CDEditKeyserver (HWND hWndParent, HWND hWndTree);

// PGPphras.c
VOID PGPFreePhrase (LPSTR szPhrase);
INT PGPGetSignCachedPhrase (PGPContextRef		Context, 
							HWND				hWnd,
							LPSTR				szPrompt,
							BOOL				bForceUserInput, 
							LPSTR*				pszBuffer, 
							PGPKeySetRef		KeySet,
							PGPKeyRef*			pKey,
							PGPHashAlgorithm*	pulHashAlg,
							UINT*				puOptions, 
							UINT				uFlags); 


#endif /* ] Included_PGPcdlgx_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
