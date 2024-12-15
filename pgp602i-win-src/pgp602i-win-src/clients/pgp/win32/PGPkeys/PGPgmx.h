/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PGPgmx.h - internal header file for Group Manager code
	

	$Id: PGPgmx.h,v 1.23 1998/08/11 15:20:06 pbj Exp $
____________________________________________________________________________*/
#ifndef Included_PGPgmx_h	/* [ */
#define Included_PGPgmx_h

#define WIN32_LEAN_AND_MEAN

// Win32 header files
#include <windows.h>
#include <commctrl.h>
#include <ole2.h>
#include <shellapi.h>

// PGP build flags
#include "pgpBuildFlags.h"

// PGP SDK header files
#include "pgpMem.h"
#include "pgpErrors.h"
#include "pgpUtilities.h"
#include "pgpKeys.h"
#include "pgpGroups.h"

// PGP client header files
#include "resource.h"
#include "..\include\pgpcl.h"
#include "..\include\treelist.h"
#include "..\include\pgpImage.h"
#include "..\include\help\PGPpkHlp.h"

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
	PGPContextRef		context;
	PGPtlsContextRef	tlsContext;

	HWND				hWndParent;
	HWND				hWndTree;
	INT					iID;
	HWND				hWndStatusBar;
	HKEYMAN				hKM;
	HIMAGELIST			hIml;

	PGPKeySetRef		keysetMain;
	PGPclGROUPFILE*		pGroupFile;
	PGPGroupSetRef		groupsetMain;
	LPDROPTARGET		pDropTarget;	//pointer to DropTarget object
	CHAR				szHelpFile[MAX_PATH];	

	LONG				lKeyListSortField;	//keylist sort order
	ULONG				ulOptionFlags;
	ULONG				ulDisableActions;
	ULONG				ulShowColumns;		// not used
	ULONG				ulHideColumns;		// not used

	BOOL				bMultipleSelected;
	BOOL				bLocatingKeys;
	UINT				uSelectedFlags;
	INT					iFocusedItemType;
	INT					iFocusedObjectType;
	HTLITEM				hFocusedItem;
	VOID*				pFocusedObject;

	HTLITEM				hFocusedItemDragging;

	INT					iValidityThreshold;

	INT					iXCursorOffset;
	INT					iYCursorOffset;

	WORD				wColumnField[NUMBERFIELDS];
	WORD				wFieldWidth[NUMBERFIELDS];

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
LPDROPTARGET GMCreateDropTarget (HWND hwnd, VOID* pGroupMan);
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
UINT GMConvertFromPGPValidity (UINT uPGPValidity);
VOID GMGetColumnPreferences (PGROUPMAN pGM);
PGPError GMSortGroupSet (PGROUPMAN pGM);
BOOL GMGetFromServer (PGROUPMAN pGM);
PGPError GMCommitGroupChanges (PGROUPMAN pGM, BOOL bBroadcast);
VOID GMGetColumnPreferences (PGROUPMAN pGM);
VOID GMSetColumnPreferences (PGROUPMAN pGM);
VOID GMGetGroupsFromServer (PGROUPMAN pGM);
VOID GMSendGroupsToServer (PGROUPMAN pGM);

// GMNewGrp.c
BOOL GMGroupProperties (PGROUPMAN pGM);

// GMTree.c
BOOL GMLoadGroupsIntoTree (PGROUPMAN pGM, BOOL bReInsert, 
							BOOL bExpandNew, BOOL bForceRealloc);
BOOL GMAddColumns (PGROUPMAN pGM); 



#endif /* ] Included_PGPgmx_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
