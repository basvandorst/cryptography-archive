/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	PGPkmx.h - internal header file for KeyManager DLL
	

	$Id: PGPkmx.h,v 1.30.4.2 1997/11/20 21:54:36 pbj Exp $
____________________________________________________________________________*/
#ifndef Included_PGPkmx_h	/* [ */
#define Included_PGPkmx_h

#define _PGPKMDLL

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

#include "..\include\treelist.h"
#include "..\include\pgpcmdlg.h"
#include "..\include\pgpkm.h"
#include "..\include\pgpImage.h"
#include "..\include\help\pgpkmhlp.h"

// constant definitions

#define OBJECT_NONE		0
#define OBJECT_KEY		1
#define OBJECT_USERID	2
#define OBJECT_CERT		3

#define NUMBERFIELDS	9
#define MAXSHEETS		16		// maximum number of simultaneous dialogs

#define DEFAULTWINDOWWIDTH	520
#define DEFAULTWINDOWHEIGHT 300
#define DEFAULTWINDOWX      85
#define DEFAULTWINDOWY		90

#define KMI_NAME			0
#define KMI_VALIDITY		1
#define KMI_SIZE			2
#define KMI_DESCRIPTION		3
#define KMI_KEYID			4
#define KMI_TRUST			5
#define KMI_CREATION		6
#define KMI_EXPIRATION		7
#define KMI_ADK				8
#define NUMBERFIELDS		9

#define KM_VALIDITY_INVALID		0
#define KM_VALIDITY_MARGINAL	1
#define KM_VALIDITY_COMPLETE	2

// Manager global variables
typedef struct {
	HWND hWndParent;
	HWND hWndTree;
	HWND hWndStatusBar;
	HANDLE hRequestMutex;
	HANDLE hAccessMutex;
	INT iId;
	HIMAGELIST hIml;
	LPDROPTARGET pDropTarget;			//pointer to DropTarget object
	CHAR szHelpFile[MAX_PATH];			//name of help file
	CHAR szPutKeyserver[256];			//keyserver to upload to

	PGPContextRef Context;			
	PGPKeySetRef KeySetDisp;		//pointer to main keyset
	PGPKeySetRef KeySetMain;		//pointer to main keyset
	LONG lKeyListSortField;				//keylist sort order
	BOOL bMainKeySet;
	ULONG ulOptionFlags;
	ULONG ulDisableActions;
	ULONG ulShowColumns;
	ULONG ulHideColumns;

	BOOL bMultipleSelected;
	UINT uSelectedFlags;
	INT iFocusedItemType;
	INT iFocusedObjectType;
	HTLITEM hFocusedItem;
	VOID* pFocusedObject;

	INT iValidityThreshold;

	INT iNumberSheets;
	HWND hWndTable[MAXSHEETS];
	PGPKeyRef KeyTable[MAXSHEETS];

	HMENU hMenuKeys;
	HMENU hMenuUserIDs;
	HMENU hMenuCerts;
	HMENU hMenuMixed;
	HMENU hMenuNone;
	HMENU hKeyServer;
	PDWORD pdwMenuIds;

	WORD wColumnField[NUMBERFIELDS];
	WORD wFieldWidth[NUMBERFIELDS];

} _KEYMAN;
typedef _KEYMAN* PKEYMAN;


// KMAddUser.c
BOOL KMAddUserToKey (PKEYMAN pKM);

// KMChange.c	
VOID KMChangePhrase (PKEYMAN pKM, HWND hWndParent, PGPKeyRef Key);

// KMColumn.c
VOID KMGetColumnPreferences (PKEYMAN pKM); 
VOID KMSetColumnPreferences (PKEYMAN pKM);
VOID KMSelectColumns (PKEYMAN pKM, BOOL bDialog);

// KMConvert.c
void KMConvertStringKeyID (LPSTR sz);
VOID KMConvertStringFingerprint (UINT uAlgorithm, LPSTR sz);
VOID KMConvertTimeToString (PGPTime tm, LPSTR sz, INT ilen);
UINT KMConvertFromPGPTrust (UINT uPGPTrust);
UINT KMConvertToPGPTrust (UINT uTrust);
UINT KMConvertFromPGPValidity (UINT uPGPValidity);

// KMFocus.c
VOID KMSetFocus (PKEYMAN pKM, HTLITEM hFocused, BOOL bMultiple);
INT KMFocusedItemType (PKEYMAN pKM);
INT KMFocusedObjectType (PKEYMAN pKM);
HTLITEM KMFocusedItem (PKEYMAN pKM);
VOID* KMFocusedObject (PKEYMAN pKM);
BOOL KMMultipleSelected (PKEYMAN pKM);
BOOL KMPromiscuousSelected (PKEYMAN pKM);
UINT KMSelectedFlags (PKEYMAN pKM);
BOOL KMSigningAllowed (PKEYMAN pKM);
VOID KMGetSelectedObjects (PKEYMAN pKM, PGPKeySetRef* pKeySet, 
						   LPSTR* pszKeyIDs);
VOID KMFreeSelectedObjects (PGPKeySetRef KeySet, LPSTR szKeyIDs);

// KMIDataObject.cpp
LPDATAOBJECT KMCreateDataObject (LPSTR szName, LPSTR szExport);
BOOL KMOKToDeleteDataObject (LPDATAOBJECT pDataObject);

// KMIDropSource.cpp
LPDROPSOURCE KMCreateDropSource (VOID);

// KMIDropTarget.cpp
LPDROPTARGET KMCreateDropTarget (VOID* pKeyMan);
VOID KMReleaseDropTarget (LPDROPTARGET pDropTarget);
VOID KMEnableDropTarget (LPDROPTARGET pDropTarget, BOOL bEnable);

// KMKeyIO.c
BOOL KMImportKey (PKEYMAN pKM, HDROP hDrop);
BOOL KMExportKeys (PKEYMAN pKM, LPSTR szFile);
BOOL KMCopyKeys (PKEYMAN pKM, HANDLE* phMem);
BOOL KMDataToPaste (VOID);
BOOL KMPasteKeys (PKEYMAN pKM);
BOOL KMDragAndDrop (PKEYMAN pKM);

// KMKeyOps.c
BOOL KMDeleteObject (PKEYMAN pKM);
BOOL KMCertifyKeyOrUserID (PKEYMAN pKM);
BOOL KMDisableKey (PKEYMAN pKM, PGPKeyRef Key);
BOOL KMEnableKey (PKEYMAN pKM, PGPKeyRef Key);
BOOL KMRevokeKey (PKEYMAN pKM);
BOOL KMRevokeCert (PKEYMAN pKM);
BOOL KMSetDefaultKey (PKEYMAN pKM);
BOOL KMSetPrimaryUserID (PKEYMAN pKM);
BOOL KMAddSelectedToMain (PKEYMAN pKM);

// KMMenu.c
VOID KMContextMenu (PKEYMAN pKM, INT x, INT y);

// KMMisc.c
VOID* KMAlloc (LONG size);
VOID KMFree (VOID* p);
PGPKeyRef KMGetKeyFromUserID (PKEYMAN pKM, PGPUserIDRef UserID);
BOOL KMCheckForSecretKeys (PGPKeySetRef KeySet);
BOOL KMIsThisTheOnlyUserID (PKEYMAN pKM, PGPUserIDRef UID);
BOOL KMIsThisThePrimaryUserID (PKEYMAN pKM, PGPUserIDRef UID);
BOOL KMGetKeyName (PGPKeyRef Key, LPSTR sz, UINT uLen);
BOOL KMGetUserIDName (PGPUserIDRef UserID, LPSTR sz, UINT uLen);
INT KMDetermineKeyIcon (PKEYMAN pKM, PGPKeyRef Key, BOOL* lpbItalics);
INT KMDetermineUserIDIcon (PGPKeyRef Key);
INT KMDetermineCertIcon (PGPSigRef Cert, BOOL* pbItalics);
INT KMCommitKeyRingChanges (PKEYMAN pKM);
BOOL KMGetKeyIDFromKey (PGPKeyRef Key, LPSTR sz, UINT u);
BOOL KMGetKeyIDFromCert (PGPSigRef Cert, LPSTR sz, UINT u);
PGPKeyRef KMGetKeyFromCert (PKEYMAN pKM, PGPSigRef Cert);
BOOL KMPGPPreferences (HWND hwnd, INT iPage);

// KMProps.c
BOOL KMKeyProperties (PKEYMAN pKM);
VOID KMUpdateKeyProperties (PKEYMAN pKM);
VOID KMDeletePropertiesKey (PKEYMAN pKM, PGPKeyRef Key);
VOID KMDeleteAllKeyProperties (PKEYMAN pKM, BOOL bCloseWindows);
VOID KMEnableAllKeyProperties (PKEYMAN pKM, BOOL bEnable);

// KMServer.c
BOOL KMSendToServer (PKEYMAN pKM);
BOOL KMGetFromServer (PKEYMAN pKM);
BOOL KMGetFromServerInternal (PKEYMAN pKM, BOOL bQueryAdd, BOOL bWarn); 
BOOL KMDeleteFromServer (PKEYMAN pKM);
BOOL KMDisableOnServer (PKEYMAN pKM);

// KMTree.c
BOOL KMAddColumns (PKEYMAN pKM);
BOOL KMLoadKeyRingIntoTree (PKEYMAN pKM, BOOL bReInsert, 
							BOOL bExpandNew, BOOL bForceRealloc);
BOOL KMUpdateKeyInTree (PKEYMAN pKM, PGPKeyRef Key);
BOOL KMUpdateAllValidities (PKEYMAN pKM);
BOOL KMExpandSelected (PKEYMAN pKM);
BOOL KMCollapseSelected (PKEYMAN pKM);
PGPError KMGetKeyUserVal (PKEYMAN pKM, PGPKeyRef Key, LONG* lValue);
PGPError KMGetUserIDUserVal (PKEYMAN pKM, PGPUserIDRef UID, LONG* lValue);
PGPError KMGetCertUserVal (PKEYMAN pKM, PGPSigRef Cert, LONG* lValue);
PGPError KMSetKeyUserVal (PKEYMAN pKM, PGPKeyRef Key, LONG lValue);
PGPError KMSetUserIDUserVal (PKEYMAN pKM, PGPUserIDRef UID, LONG lValue);
PGPError KMSetCertUserVal (PKEYMAN pKM, PGPSigRef Cert, LONG lValue);
BOOL KMDeleteAllUserValues (PKEYMAN pKM);
BOOL KMRepaintKeyInTree (PKEYMAN pKM, PGPKeyRef Key);
VOID KMGetKeyBitsString (PGPKeySetRef KeySet, 
						 PGPKeyRef Key, LPSTR sz, UINT u);

// KMUser.c
LRESULT KMMessageBox (HWND hwnd, INT iCaption, INT iMessage, ULONG flags);
BOOL KMUseBadPassPhrase (HWND hwnd);
BOOL KMConstructUserID (HWND hDlg, UINT uNameIDC, UINT uAddrIDC, 
						LPSTR* pszUserID);
BOOL KMSignConfirm (PKEYMAN pKM, BOOL* pbSendToServer);
VOID KMWipeEditBox (HWND hDlg, UINT uID);

// PGPkm.c
VOID KMRequestSDKAccess (PKEYMAN pKM);
VOID KMReleaseSDKAccess (PKEYMAN pKM);


#endif /* ] Included_PGPkmx_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
