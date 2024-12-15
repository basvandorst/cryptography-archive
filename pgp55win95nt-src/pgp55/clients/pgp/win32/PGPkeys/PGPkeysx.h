/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	PGPKeysx.h - internal include file for PGPkeys project	

	$Id: PGPkeysx.h,v 1.41 1997/11/05 16:16:45 pbj Exp $
____________________________________________________________________________*/
#ifndef Included_PGPkeysx_h	/* [ */
#define Included_PGPkeysx_h

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

#include "pgpBuildFlags.h"
#include "pgpMem.h"
#include "pgpErrors.h"
#include "pgpUtilities.h"
#include "pgpKeys.h"
#include "pgpAdminPrefs.h"
#include "pgpClientPrefs.h"

#include "..\include\help\PGPpkHlp.h"

#include "..\include\pgpWerr.h"
#include "..\include\pgpkm.h"
#include "..\include\pgpgm.h"
#include "..\include\treelist.h"
#include "..\include\pgpcmdlg.h"

#define MAXSHEETS	16		// maximum number of simultaneous dialogs

#define IDC_STATUSBAR			0x3333
#define IDM_DEFAULTKEYSERVER	41000

// constant definitions

#define KCD_NEWKEY			0x0001
#define KCD_NEWDEFKEY		0x0002

#define DEFAULTWINDOWWIDTH	520
#define DEFAULTWINDOWHEIGHT 300
#define DEFAULTWINDOWX      85
#define DEFAULTWINDOWY		90
#define	DEFAULTGROUPSHOW	FALSE
#define DEFAULTGROUPPERCENT	30

#define KM_M_CREATEDONE				WM_APP
#define KM_M_RELOADKEYRINGS			WM_APP+1
#define KM_M_GENERATION_COMPLETE	WM_APP+2
#define KM_M_CHANGEPHASE			WM_APP+3

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
	PGPContextRef	ContextRef;

	HMENU			hMenuKeyMan;
	HMENU			hMenuKeyserver;
	INT				iToolHeight;
	INT				iKeysHeight;

	PGPcdGROUPFILE*	pGroupFile;
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
} PGPKEYSSTRUCT;


// PGPkeys.c
VOID PKAddSearchDlgToList (HWND hwndSearchDlg);
VOID PKRemoveSearchDlgFromList (HWND hwndSearchDlg);
VOID PKEnableSearchDlgList (BOOL bEnabled);
VOID PKReloadSearchDlgList (VOID);

// PKMenu.c
VOID PGPkeysSetMainMenu (PGPKEYSSTRUCT*	ppks);
VOID PGPkeysContextMenu (PGPKEYSSTRUCT*	ppks, HWND hWnd, 
						 INT x, INT y);
VOID PKGetServerFromID (UINT uID, LPSTR szServer, UINT uLen);

// PKMisc.c
VOID* pkAlloc (LONG size);
VOID pkFree (VOID* p);
BOOL PGPkeysGetWindowPosFromRegistry (DWORD* show, INT* X, INT* Y, 
			INT* Width, INT* Height, BOOL* bShowGroups, INT* iGroupsPercent);
BOOL PGPkeysGetColWidthsFromRegistry (WORD * widths, INT nWidths);
BOOL PGPkeysGetSortOrderFromRegistry (LONG * field);
BOOL PGPkeysPGPPreferences (PGPKEYSSTRUCT* ppks, HWND hwnd, INT iPage);
VOID PGPkeysSetWindowRegistryData (HWND hWnd, BOOL bGroupShow, 
								   INT iGroupPercent);
VOID PGPkeysSetPathRegistryData (VOID);
INT PGPkeysCommitKeyRingChanges (PGPKeySetRef KeySet, BOOL bBroadcast);
VOID PGPkeysRegister (VOID);

// PKMsgPrc.c
LONG APIENTRY KeyManagerWndProc(HWND hWndDlg, UINT msg, WPARAM wParam, 
								LPARAM lParam);
VOID SetMainMenu (VOID);

// PKUser.c
LRESULT PGPkeysMessageBox (HWND hwnd, INT iCaption, 
						   INT iMessage, ULONG flags);
BOOL PGPkeysBackupWarn (HWND hwnd, BOOL* pbBackup);
VOID PGPkeysBackup (HWND hWnd);
BOOL PGPkeysUseBadPassPhrase (HWND hwnd);
VOID PGPkeysHelpAbout (HWND hWnd);
BOOL PGPkeysUserNameOK (HWND hWnd, LPSTR szUserName);
VOID PGPkeysReadOnlyWarning (HWND hWnd);
VOID PGPkeysWipeEditBox (HWND hDlg, UINT uID);

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
