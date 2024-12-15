/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PGPclx.h - internal header for PGPcl project
	

	$Id: PGPclx.h,v 1.29 1999/04/12 21:41:50 pbj Exp $
____________________________________________________________________________*/
#ifndef Included_PGPclx_h	/* [ */
#define Included_PGPclx_h

#define WIN32_LEAN_AND_MEAN

// system header files
#include <windows.h>
#include <commctrl.h>
#include <stddef.h>

// PGP build flags
#include "pgpBuildFlags.h"

// PGP SDK header files
#include "pgpUserInterface.h"
#include "pgpBase.h"
#include "pgpMem.h"
#include "pgpKeys.h"
#include "pgpErrors.h"
#include "pgpUtilities.h"
#include "pgpGroups.h"
#include "pgpFeatures.h"
#include "pgpSDKPrefs.h"
#include "pflPrefs.h"
#include "pflPrefTypes.h"
#include "pgpAdminPrefs.h"
#include "pgpClientPrefs.h"
#include "pgpFileSpec.h"
#include "pgpMemoryIO.h"
#include "pgpWordWrap.h"

// PGP client header files
#include "..\include\help\PGPclHlp.h"
#include "..\include\PGPWerr.h"
#include "..\include\PGPcl.h"

// local header files
#include "resource.h"

// macro definitions
#define CKERR		if (IsPGPError (err)) goto done

// common definitions
#define IDC_KSTREELIST		5555
#define IDC_ADKTREELIST		5556
#define IDC_REVOKERTREELIST	5557

// typedefs
typedef struct {
	HWND				hwnd;
	PGPKeySetRef		keysetMain;
	PGPKeyServerEntry*	pentry;
} CLIENTSERVERSTRUCT, *PCLIENTSERVERSTRUCT;

// prototypes
// CLcache.c
VOID PGPInitPassphraseCaches (PGPContextRef context);
VOID PGPClosePassphraseCaches (VOID);

// CLMisc.c
VOID* clAlloc (UINT uBytes);
VOID clFree (VOID* p);
VOID CLPaintUserInfo (
		PGPMemoryMgrRef	memMgr, 
		HWND			hwnd,
		HDC				hDC, 
		LPSTR			pszVersion);
LRESULT PGPclMessageBox (
		HWND			hWnd, 
		INT				iCaption, 
		INT				iMessage, 
		ULONG			ulFlags);
HBITMAP CLLoadResourceBitmap (
		HINSTANCE		hInstance, 
		LPSTR			lpString,
		HPALETTE*		lphPalette);
BOOL CLAddKeysToMain (
		PGPContextRef	context, 
		HWND			hwnd,
		PGPKeySetRef	keysetToAdd,
		PGPKeySetRef	keysetMain);

// CLpref.c
PGPError CLGetCAServerPrefs (
		PGPContextRef		context,
		PGPPrefRef			prefref, 
		PGPKeyServerEntry**	ppentry);
PGPError CLSetCAServerPrefs (
		PGPContextRef		context,
		PGPPrefRef			prefref, 
		PGPKeyServerEntry*	pentry);
PGPError CLSetCARevocationServerPrefs (
		PGPContextRef		context,
		PGPPrefRef			prefref, 
		PGPKeyServerEntry*	pentry);

// CLksPref.c
HWND CLCreateKeyserverTreeList (
		PGPPrefRef		PrefRef, 
		PGPContextRef	context, 
		HWND			hWndParent,
		PGPKeySetRef	keysetMain);
INT CLDestroyKeyserverTreeList (HWND hWndTree);
INT CLNewKeyserver (HWND hWndParent, HWND hWndTree);
VOID CLRemoveKeyserver (HWND hWndParent, HWND hWndTree);
INT CLSetDefaultKeyserver (HWND hWndParent, HWND hWndTree);
INT CLProcessKeyserverTreeList (
		HWND			hWndParent, 
		HWND			hWndTree, 
		WPARAM			wParam, 
		LPARAM			lParam);
INT CLSaveKeyserverPrefs (
		PGPPrefRef		PrefRef, 
		HWND			hWndParent, 
		HWND			hWndTree);
VOID CLEditKeyserver (HWND hWndParent, HWND hWndTree);
VOID CLSetKeyserverAsRoot (HWND hWndParent, HWND hWndTree);
VOID CLMoveKeyserverUp (HWND hWndParent, HWND hWndTree);
VOID CLMoveKeyserverDown (HWND hWndParent, HWND hWndTree);

// CLsdksrv.c
PGPError CLHandleTLSEvent (
		PGPContextRef		context,
		PGPEvent*			pevent,
		HWND				hwnd,
		PGPKeySetRef		keyset,
		PGPKeyServerEntry*	pentry,
		BOOL*				pbSecure);

// CLsrvprf.c
PGPError CLInitKeyServerPrefs(
		PGPUInt32			uServer,
		PGPKeyServerEntry*	pkeyserver,
		HWND				hwndParent,
		PGPContextRef		context,
		PGPKeySetRef		keysetMain,
		PGPPrefRef			clientPrefsRef,
		CHAR				*pszUserID,
		PCLIENTSERVERSTRUCT	*ppcss,
		PGPKeyServerEntry	**ksEntries,
		PGPKeyServerSpec	**serverList,
		PGPUInt32			*numKSEntries);

PGPError CLUninitKeyServerPrefs(
		PGPUInt32			uServer,
		PCLIENTSERVERSTRUCT	pcss,
		PGPKeyServerEntry	*ksEntries,
		PGPKeyServerSpec	*serverList,
		PGPUInt32			numKSEntries);

#endif /* ] Included_PGPclx_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
