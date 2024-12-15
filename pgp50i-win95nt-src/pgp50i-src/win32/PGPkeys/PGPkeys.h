/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */
//: PGPkeys.h
//. 06dec96 pbj
//
//	$Id: PGPkeys.h,v 1.5 1997/05/27 18:39:29 pbj Exp $
//

#include "..\include\config.h"
#include "..\include\pgpkeydb.h"
#include "..\include\pgpmem.h"
#include "..\include\treelist.h"
#include "pgpkeyshelp.h"
#include "..\include\pgpcomdlg.h"

// constant definitions

#define IDX_NONE			-1
#define IDX_RSAPUBKEY		0
#define IDX_RSAPUBDISKEY	1
#define IDX_RSAPUBREVKEY	2
#define IDX_RSAPUBEXPKEY	3
#define IDX_RSASECKEY		4
#define IDX_RSASECDISKEY	5
#define IDX_RSASECREVKEY	6
#define IDX_RSASECEXPKEY	7
#define IDX_DSAPUBKEY		8
#define IDX_DSAPUBDISKEY	9
#define IDX_DSAPUBREVKEY	10
#define IDX_DSAPUBEXPKEY	11
#define IDX_DSASECKEY		12
#define IDX_DSASECDISKEY	13
#define IDX_DSASECREVKEY	14
#define IDX_DSASECEXPKEY	15
#define IDX_RSAUSERID		16
#define IDX_DSAUSERID		17
#define IDX_CERT			18
#define IDX_REVCERT			19
#define IDX_BADCERT			20
#define NUM_BITMAPS			21

#define OBJECT_NONE		0
#define OBJECT_KEY		1
#define OBJECT_USERID	2
#define OBJECT_CERT		3

#define NUMBERCOLUMNS	5

//#define DRAW_LOGO		1
#ifdef DRAW_LOGO
#define LOGO_WIDTH		124
#define LOGO_HEIGHT		28
#else
#define LOGO_WIDTH		0
#define LOGO_HEIGHT		0
#endif // DRAW_LOGO

#define KCD_NEWKEY			0x0001
#define KCD_NEWDEFKEY		0x0002

#define G_BUFLEN		512

#define DEFAULTWINDOWWIDTH	520
#define DEFAULTWINDOWHEIGHT 300
#define DEFAULTWINDOWX      85
#define DEFAULTWINDOWY		90

#define DEFAULTCOLWIDTHNAME  240
#define DEFAULTCOLWIDTHVALID  54
#define DEFAULTCOLWIDTHTRUST  54
#define DEFAULTCOLWIDTHDATE   70
#define DEFAULTCOLWIDTHSIZE   70

#define KM_M_CREATEDONE				WM_USER
#define KM_M_RELOADKEYRINGS			WM_USER+1
#define KM_M_GENERATION_COMPLETE	WM_USER+2
#define KM_M_CHANGEPHASE			WM_USER+3
#define KM_M_TICKCLOCK				WM_USER+4

// Key Types
#define KEYTYPE_RSA_OLDSTYLE    1
#define KEYTYPE_RSA_ENCRYPTION  2
#define KEYTYPE_RSA_SIGNATURE   3
#define KEYTYPE_LAST          4


// KMAddUser.c
BOOL KMAddUserToKey (HWND hparent);

// KMChange.c	
VOID KMChangePhrase (HWND hparent, PGPKey* pKey);

// KMConvert.c
void KMConvertStringKeyID (LPSTR sz);
VOID KMConvertStringFingerprint (UINT uAlgorithm, LPSTR sz);
VOID KMConvertTimeToString (PGPTime* tm, LPSTR sz, INT ilen);
UINT KMConvertFromPGPTrust (UINT uPGPTrust);
UINT KMConvertToPGPTrust (UINT uTrust);
UINT KMConvertFromPGPValidity (UINT uPGPValidity);

// KMFocus.c
VOID KMSetFocus (HTLITEM hFocused, BOOL bMultiple);
INT KMFocusedItemType (VOID);
INT KMFocusedObjectType (VOID);
HTLITEM KMFocusedItem (VOID);
VOID* KMFocusedObject (VOID);
BOOL KMMultipleSelected (VOID);
BOOL KMPromiscuousSelected (VOID);
BOOL KMSigningAllowed (VOID);
VOID KMGetSelectedObjects (PGPKeySet** ppKeySet, LPSTR* pszKeyIDs);
VOID KMFreeSelectedObjects (PGPKeySet* pKeySet, LPSTR szKeyIDs);

// KMIDataObject.cpp
LPDATAOBJECT KMCreateDataObject (LPSTR szName, LPSTR szExport);

// KMIDropSource.cpp
LPDROPSOURCE KMCreateDropSource (VOID);

// KMIDropTarget.cpp
LPDROPTARGET KMCreateDropTarget (VOID);
VOID KMReleaseDropTarget (LPDROPTARGET pDropTarget);
VOID KMEnableDropTarget (LPDROPTARGET pDropTarget, BOOL bEnable);

// KMKeyIO.c
BOOL KMImportKey (HWND hWnd, HDROP hDrop);
BOOL KMExportKeys (HWND hWnd, LPSTR szFile);
BOOL KMCopyKeys (HANDLE* phMem);
BOOL KMDataToPaste (VOID);
BOOL KMPasteKeys (HWND hWnd);

// KMKeyOps.c
BOOL KMDeleteObject (HWND hWnd);
BOOL KMCertifyKeyOrUserID (HWND hWnd);
BOOL KMDisableKey (HWND hWnd, PGPKey* pKey);
BOOL KMEnableKey (HWND hWnd, PGPKey* pKey);
BOOL KMRevokeKey (HWND hWnd);
BOOL KMRevokeCert (HWND hWnd);
BOOL KMDragAndDrop (HWND hWnd);
BOOL KMSetDefaultKey (HWND hwnd);
BOOL KMSetPrimaryUserID (HWND hwnd);

// KMMenu.c
VOID KMSetMainMenu (HMENU hMenu);
VOID KMContextMenu (HWND hWnd, INT x, INT y);
VOID KMCreateMenus (HMENU hMenuMain);
VOID KMDestroyMenus (void);

// KMMisc.c
VOID* KMAlloc (LONG size);
VOID KMFree (VOID* p);
PGPKey* KMGetKeyFromUserID (HWND hTree, PGPUserID* pUserID);
BOOL KMCheckForSecretKeys (PGPKeySet* pKeySet);
BOOL KMIsThisTheOnlyUserID (HWND hTree, PGPUserID* pUID);
BOOL KMIsThisThePrimaryUserID (HWND hTree, PGPUserID* pUID);
BOOL KMGetWindowPosFromRegistry (DWORD * show, INT * X, INT * Y,
								 INT * Width, INT * Height);
BOOL KMGetColWidthsFromRegistry (WORD * widths, INT nWidths);
BOOL KMGetSortOrderFromRegistry (LONG * field);
BOOL KMPGPPreferences (HWND hwnd, INT iPage);
VOID KMSetWindowRegistryData (HWND hwnd);
VOID KMSetPathRegistryData (VOID);
BOOL KMGetKeyName (PGPKey* pKey, LPSTR sz, UINT uLen);
BOOL KMGetUserIDName (PGPUserID* pUserID, LPSTR sz, UINT uLen);
INT KMDetermineKeyIcon (PGPKey* pKey, BOOL* lpbItalics);
INT KMCommitKeyRingChanges (PGPKeySet* pKeySet);

// KMMsgProc.c
LONG APIENTRY KeyManagerWndProc(HWND hWndDlg, UINT msg, WPARAM wParam,
								LPARAM lParam);
VOID SetMainMenu (VOID);

// KMProps.c
BOOL CALLBACK KeyPropDlgProc (HWND hDlg, UINT iMsg, WPARAM wParam,
							  LPARAM lParam);
BOOL KMKeyProperties (HWND hwnd);
VOID KMUpdateKeyProperties (void);
VOID KMDeletePropertiesKey (PGPKey* pKey);
VOID KMDeleteAllKeyProperties (void);

// KMTree.c
HWND KMCreateTreeList (HWND hWndParent);
BOOL KMAddColumns (HWND hwndTree);
BOOL KMLoadKeyRingIntoTree (HWND hParent, HWND hwndTree, BOOL bReInsert,
							BOOL bExpandNew);
BOOL KMUpdateKeyInTree (HWND hTree, PGPKey* pKey);
BOOL KMUpdateAllValidities (VOID);
BOOL KMDestroyTree (VOID);
BOOL KMExpandSelected (HWND hTree);
BOOL KMCollapseSelected (HWND hTree);

// KMUser.c
LRESULT KMMessageBox (HWND hwnd, INT iCaption, INT iMessage, ULONG flags);
BOOL KMBackupWarn (HWND hwnd);
BOOL KMUseBadPassPhrase (HWND hwnd);
BOOL KMSelectByKeyboard (HWND hwnd, LONG c, BOOL bStillTyping);
VOID KMHelpAbout (HWND hWnd);
BOOL KMUserNameOK (HWND hWnd, LPSTR szUserName);
BOOL KMConstructUserID (HWND hDlg, UINT uNameIDC, UINT uAddrIDC,
						LPSTR* pszUserID);
VOID KMReadOnlyWarning (HWND hWnd);
BOOL KMSignConfirm (HWND hWnd, BOOL* pbSendToServer);
VOID KMWipeEditBox (HWND hDlg, UINT uID);

// KMKeyWiz.c
VOID NewKeyWizThread (void *pArg);

// PGPkeygenwiz.c
VOID KMCreateKey (HWND hwnd);
