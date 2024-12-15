/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PGPKeysx.h - internal include file for PGPkeys project	

	$Id: PGPkeysx.h,v 1.72 1999/04/01 03:52:28 pbj Exp $
____________________________________________________________________________*/
#ifndef Included_PGPkeysx_h	/* [ */
#define Included_PGPkeysx_h

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0400

// Win32 header files
#include <windows.h>
#include <commctrl.h>

// PGP build flags
#include "pgpBuildFlags.h"

// PGP SDK header files
#include "pgpMem.h"
#include "pgpErrors.h"
#include "pgpUtilities.h"
#include "pgpKeys.h"
#include "pgpAdminPrefs.h"
#include "pgpClientPrefs.h"
#include "pgpKeyServerPrefs.h"

// PGP client header files
#include "..\include\help\PGPpkHlp.h"
#include "..\include\pgpWerr.h"
#include "..\include\pgpkm.h"
#include "..\include\pgpgm.h"
#include "..\include\pgpcl.h"
#include "..\include\treelist.h"
#include "resource.h"

#define MAXSHEETS	16		// maximum number of simultaneous dialogs

#define IDC_STATUSBAR			0x3333
#define IDC_TOOLBAR				0x3334
#define IDM_DOMAINKEYSERVERX	41000
#define IDM_TOOLBARSENDTOSERVER	42000

// macro definitions
#define CKERR			if (IsPGPError (err)) goto done	

// constant definitions
#define KCD_NEWKEY			0x0001
#define KCD_NEWDEFKEY		0x0002

#define DEFAULTWINDOWWIDTH	520
#define DEFAULTWINDOWHEIGHT 300
#define DEFAULTWINDOWX      85
#define DEFAULTWINDOWY		90
#define	DEFAULTGROUPSHOW	FALSE
#define DEFAULTGROUPPERCENT	30
#define	DEFAULTTOOLHEIGHT	28
#define TOOLBARXOFFSET		4
#define TOOLBARYOFFSET		4

#define KM_M_CREATEDONE				WM_APP+99
#define KM_M_RELOADKEYRINGS			WM_APP+98
#define KM_M_GENERATION_COMPLETE	WM_APP+97
#define KM_M_CHANGEPHASE			WM_APP+96
#define KM_M_RESIZE					WM_APP+95

// typedefs
typedef struct {
	HWND			hWndMain;
	BOOL			bMainWindow;
	BOOL			bMemoryKeySet;
	BOOL			bLocalKeySet;
	UINT			uKeySelectionFlags;
	HKEYMAN			hKM;
	KMCONFIG		kmConfig;
	HWND			hWndTreeList;	
	HWND			hWndToolBar;
	HWND			hWndSearchControl;
	PGPKeySetRef	KeySetDisp;
	PGPKeySetRef	KeySetMain;

	HMENU			hMenuKeyMan;
	HMENU			hMenuKeyserver;
	INT				iToolHeight;
	INT				iKeysHeight;

	PGPclGROUPFILE*	pGroupFile;
	BOOL			bGroupsVisible;
	BOOL			bGroupsFocused;
	HGROUPMAN		hGM;
	GMCONFIG		gmConfig;
	UINT			uGroupSelectionFlags;
	HWND			hWndTreeListGroups;
	INT				iGroupsHeight;
	INT				iGroupsPercent;
	BOOL			bGrabEnabled;
	BOOL			bGrabbed;
	HCURSOR			hCursorOld;

	BOOL			bSearchInProgress;
	INT				iStatusValue;
	INT				iStatusPrevValue;
	INT				iStatusDirection;

	INT				iTLSstatus;
	CHAR			szTLSserver[256];
	PGPKeySetRef	keysetAuth;
	PGPKeyRef		keyAuth;
	PGPtlsCipherSuiteNum	tlsCipher;

} PGPKEYSSTRUCT, *PPGPKEYSSTRUCT;


// PGPkeys.c
VOID PKDialogListFunc (HWND hwndDlg, BOOL bAdd, POINT* ppt, HWND* phwnd);
VOID PKEnableDlgList (BOOL bEnabled);
VOID PKReloadDlgList (VOID);
VOID PKPostMessageDlgList (UINT uMsg, WPARAM wParam, LPARAM lParam);

// PKgenwiz.c
VOID PKCreateKey (HWND hParent, PGPKeySetRef keyset);

// PKMenu.c
VOID PKInitMenuKeyMan (HMENU hmenu);
VOID PKSetMainMenu (PGPKEYSSTRUCT*	ppks);
VOID PKContextMenu (PGPKEYSSTRUCT*	ppks, HWND hWnd, INT x, INT y);
VOID PKGetServerFromID (UINT uID, PGPKeyServerEntry* pkeyserver);
VOID PKToolbarKeyserverMenu (HWND hWnd, LPRECT lprect); 

// PKMisc.c
VOID* pkAlloc (LONG size);
VOID pkFree (VOID* p);
BOOL PKGetPrivatePrefData (DWORD* show, INT* X, INT* Y, 
			INT* Width, INT* Height, BOOL* bShowGroups, INT* iGroupsPercent,
			INT* iToolHeight);
BOOL PKGetColWidthsFromRegistry (WORD * widths, INT nWidths);
BOOL PKGetSortOrderFromRegistry (LONG * field);
VOID PKReloadPGPPreferences (PGPKEYSSTRUCT* ppks);
BOOL PKPGPPreferences (PGPKEYSSTRUCT* ppks, HWND hwnd, INT iPage);
VOID PKSetPrivatePrefData (HWND hWnd, BOOL bGroupShow, 
						   INT iGroupPercent, INT iToolHeight);
VOID PKSetPathRegistryData (VOID);
INT  PKCommitKeyRingChanges (PGPKeySetRef KeySet, BOOL bBroadcast);
BOOL PKAutoUpdateAllKeys (HWND hwnd, PGPKeySetRef keyset, BOOL bForce);
BOOL PKAutoUpdateIntroducers (HWND hwnd, PGPKeySetRef keyset, BOOL bForce);
BOOL PKUpdateCARevocations (HWND hwnd, HKEYMAN kKM, PGPKeySetRef keysetMain);
BOOL PKScheduleNextCRLUpdate (PGPContextRef context, PGPKeySetRef keyset);

// PKMsgPrc.c
LONG APIENTRY KeyManagerWndProc(HWND hWndDlg, UINT msg, WPARAM wParam, 
								LPARAM lParam);

// PKSend.c
INT PKSendShares (PGPContextRef context, HWND hwnd, PGPKeySetRef keyset); 

// PKTools.c
HWND PKCreateToolbar (HWND hwndParent);
VOID PKDestroyToolbar (HWND hwndToolbar);
VOID PKGetToolbarTooltipText (LPTOOLTIPTEXT lpttt);
VOID PKSetToolbarButtonStates (PPGPKEYSSTRUCT ppks);
VOID PKGetSendToServerButtonRect (HWND hwndToolbar, LPRECT lprect);

// PKUser.c
LRESULT PKMessageBox (HWND hwnd, INT iCaption, 
						   INT iMessage, ULONG flags);
BOOL PKBackupWarn (HWND hwnd, BOOL* pbBackup);
VOID PKBackup (HWND hWnd);
VOID PKHelpAbout (HWND hWnd);
VOID PKReadOnlyWarning (HWND hWnd);
VOID PKWipeEditBox (HWND hDlg, UINT uID);
BOOL PKSplitKeyWarn (HWND hWnd);

// PKgenwiz.c
VOID PGPkeysCreateKey (HWND hwnd, PGPKeySetRef KeySet);

#endif /* ] Included_PGPkeysx_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
