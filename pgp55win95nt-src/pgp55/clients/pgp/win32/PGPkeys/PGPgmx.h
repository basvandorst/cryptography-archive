/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	PGPgmx.h - internal header file for Group Manager code
	

	$Id: PGPgmx.h,v 1.12 1997/10/20 15:23:53 pbj Exp $
____________________________________________________________________________*/
#ifndef Included_PGPgmx_h	/* [ */
#define Included_PGPgmx_h

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <ole2.h>
#include <shellapi.h>

#include "pgpBuildFlags.h"
#include "pgpMem.h"
#include "pgpErrors.h"
#include "pgpUtilities.h"
#include "pgpKeys.h"
#include "pgpGroups.h"

#include "..\include\treelist.h"
#include "..\include\pgpcmdlg.h"
#include "..\include\pgpImage.h"

#define _PGPGMDLL
#include "..\include\pgpkm.h"
#include "..\include\pgpgm.h"

// constant definitions

#define OBJECT_NONE		0
#define OBJECT_GROUP	1
#define OBJECT_KEY		2

#define GMI_NAME			0
#define GMI_VALIDITY		1
#define GMI_DESCRIPTION		2
#define NUMBERFIELDS		3

#define GM_VALIDITY_INVALID		0
#define GM_VALIDITY_MARGINAL	1
#define GM_VALIDITY_COMPLETE	2

// Manager global variables
typedef struct {
	HWND hWndParent;
	HWND hWndTree;
	HWND hWndStatusBar;
	HKEYMAN	hKM;
	INT iId;
	HIMAGELIST hIml;
	PGPKeySetRef keysetMain;
	PGPcdGROUPFILE* pGroupFile;
	PGPGroupSetRef groupsetMain;
	LPDROPTARGET pDropTarget;			//pointer to DropTarget object
	CHAR szHelpFile[MAX_PATH];			//name of help file

	PGPContextRef context;
	PFLContextRef pflcontext;			
	LONG lKeyListSortField;				//keylist sort order
	ULONG ulOptionFlags;
	ULONG ulDisableActions;
	ULONG ulShowColumns;
	ULONG ulHideColumns;

	BOOL bMultipleSelected;
	BOOL bLocatingKeys;
	UINT uSelectedFlags;
	INT iFocusedItemType;
	INT iFocusedObjectType;
	HTLITEM hFocusedItem;
	VOID* pFocusedObject;

	INT iValidityThreshold;

	HMENU hMenuGroups;
	HMENU hMenuKeys;
	HMENU hMenuMixed;
	HMENU hMenuNone;

	WORD wColumnField[NUMBERFIELDS];
	WORD wFieldWidth[NUMBERFIELDS];

} _GROUPMAN;
typedef _GROUPMAN* PGROUPMAN;

typedef struct _GM FAR* HGROUPMAN;

// GMFocus.c
VOID GMSetFocus (PGROUPMAN pGM, HTLITEM hFocused, BOOL bMultiple);
INT GMFocusedItemType (PGROUPMAN pGM);
INT GMFocusedObjectType (PGROUPMAN pGM);
HTLITEM GMFocusedItem (PGROUPMAN pGM);
VOID* GMFocusedObject (PGROUPMAN pGM);
BOOL GMMultipleSelected (PGROUPMAN pGM);
BOOL GMPromiscuousSelected (PGROUPMAN pGM);
UINT GMSelectedFlags (PGROUPMAN pGM);
BOOL GMSigningAllowed (PGROUPMAN pGM);
VOID GMGetSelectedObjects (PGROUPMAN ppGM, PGPKeySetRef* pKeySet, 
						   LPSTR* pszKeyIDs);
VOID GMFreeSelectedObjects (PGPKeySetRef KeySet, LPSTR szKeyIDs);

// GMIDropTarget.cpp
LPDROPTARGET GMCreateDropTarget (VOID* pGroupMan);
VOID GMReleaseDropTarget (LPDROPTARGET pDropTarget);
VOID GMEnableDropTarget (LPDROPTARGET pDropTarget, BOOL bEnable);

// GMOps.c
BOOL GMPasteKeys (PGROUPMAN pGM);
BOOL GMLocateKeys (PGROUPMAN pGM);
BOOL GMDeleteObject (PGROUPMAN pGM);
BOOL GMAddSelectedToFocused (PGROUPMAN pGM);
BOOL GMImportGroups (PGROUPMAN pGM, HDROP hDrop);

// GMMisc.c
VOID* gmAlloc (LONG size);
VOID gmFree (VOID* p);
LRESULT PGPgmMessageBox (HWND hWnd, INT iCaption, INT iMess, ULONG ulFlags);
UINT ConvertFromPGPValidity (UINT uPGPValidity);
VOID GetColumnPreferences (PGROUPMAN pGM);
PGPError GMSortGroupSet (PGROUPMAN pGM);
BOOL GMGetFromServer (PGROUPMAN pGM);
PGPError GMCommitGroupChanges (PGROUPMAN pGM, BOOL bBroadcast);
VOID GMGetColumnPreferences (PGROUPMAN pGM);
VOID GMSetColumnPreferences (PGROUPMAN pGM);

// GMNewGrp.c
BOOL GMGroupProperties (PGROUPMAN pGM);

// GMTree.c
BOOL GMLoadGroupsIntoTree (PGROUPMAN pGM, BOOL bReInsert, 
							BOOL bExpandNew, BOOL bForceRealloc);



#endif /* ] Included_PGPgmx_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
