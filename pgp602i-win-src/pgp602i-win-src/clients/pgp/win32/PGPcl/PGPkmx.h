/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PGPkmx.h - internal header file for KeyManager DLL
	

	$Id: PGPkmx.h,v 1.50 1998/08/12 18:31:25 pbj Exp $
____________________________________________________________________________*/
#ifndef Included_PGPkmx_h	/* [ */
#define Included_PGPkmx_h

#define _PGPKMDLL

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0400

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

// PGP client header files
#include "..\include\help\pgpclhlp.h"
#include "..\include\pgpcl.h"
#include "..\include\pgpkm.h"
#include "..\include\pgpImage.h"
#include "..\include\treelist.h"

// local header files
#include "resource.h"

// macro definitions
#define CKERR		if (IsPGPError (err)) goto done

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

#define KM_PROPACTION_UPDATEKEY		1
#define KM_PROPACTION_SPLITKEY		2

struct _KEYMAN;

// split key structures
typedef struct _SHAREHOLDERSTRUCT {
	BOOL						bPublicKey;
	PGPKeyID					keyid;
	PGPPublicKeyAlgorithm		keyalg;
	CHAR						szUserID[kPGPMaxUserIDSize];
	LPSTR						pszPassphrase;
	UINT						uShares;
} SHAREHOLDERSTRUCT, *PSHAREHOLDERSTRUCT;

typedef struct _SPLITKEYSTRUCT {
	struct _SPLITKEYSTRUCT*		next;
	struct _SPLITKEYSTRUCT**	pHeadOfList;
	struct _KEYMAN*				pKM;
	HWND						hwndDlg;
	HWND						hwndList;
	HIMAGELIST					hIml;
	LPDROPTARGET				pDropTarget;
	PGPKeyID					keyidToSplit;
	PGPPublicKeyAlgorithm		keyalgToSplit;
	CHAR						szUserIDToSplit[kPGPMaxUserIDSize];
	UINT						uTotalShares;
	UINT						uThreshold;
	INT							iIndexCurrent;
	PSHAREHOLDERSTRUCT			pshsCurrent;
} SPLITKEYSTRUCT, *PSPLITKEYSTRUCT;

// Manager global variables
typedef struct _KEYMAN {
	PGPContextRef		Context;			
	PGPtlsContextRef	tlsContext;

	HWND				hWndParent;
	HWND				hWndTree;
	HWNDLISTPROC		lpfnHwndListFunc;
	INT					iID;
	HWND				hWndStatusBar;
	HANDLE				hRequestMutex;
	HANDLE				hAccessMutex;
	HIMAGELIST			hIml;
	LPDROPTARGET		pDropTarget;		//pointer to DropTarget object
	CHAR				szHelpFile[MAX_PATH];	//name of help file

	PGPKeySetRef		KeySetMain;			//pointer to main keyset
	PGPKeySetRef		KeySetDisp;			//pointer to display keyset
	LONG				lKeyListSortField;	//keylist sort order
	BOOL				bMainKeySet;	
	ULONG				ulOptionFlags;
	ULONG				ulDisableActions;
	ULONG				ulShowColumns;		//currently unused
	ULONG				ulHideColumns;		//currently unused

	BOOL				bMultipleSelected;	
	UINT				uSelectedFlags;
	INT					iFocusedItemType;
	INT					iFocusedObjectType;
	HTLITEM				hFocusedItem;
	VOID*				pFocusedObject;

	INT					iValidityThreshold;	//for setting icons to green

	INT					iNumberSheets;			//number of open prop sheets
	HWND				hWndTable[MAXSHEETS];
	PGPKeyRef			KeyTable[MAXSHEETS];

	WORD				wColumnField[NUMBERFIELDS];
	WORD				wFieldWidth[NUMBERFIELDS];

	PGPKeyServerEntry	keyserver;
	PSPLITKEYSTRUCT		pSplitKeyDialogList;

} KEYMAN, *PKEYMAN;


// KMAddUser.c
BOOL KMAddUserToKey (PKEYMAN pKM);
BOOL KMAddPhotoToKey (PKEYMAN pKM);

// KMChange.c	
BOOL KMChangePhrase (HWND hwndParent, PKEYMAN pKM, 
					 PGPContextRef context, PGPtlsContextRef tlsContext,
					 PGPKeySetRef keyset, PGPKeyRef key); 

// KMColumn.c
VOID KMGetColumnPreferences (PKEYMAN pKM); 
VOID KMSetColumnPreferences (PKEYMAN pKM);
VOID KMGetSelectedColumns (PKEYMAN pKM, ULONG* pulColumnFlags); 
VOID KMSelectColumns (PKEYMAN pKM, ULONG ulColumnFlags); 

// KMConvert.c
VOID KMConvertStringFingerprint (UINT uAlgorithm, LPSTR sz);
VOID KMConvertTimeToDays (PGPTime tm, INT* piDays);
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
PGPError KMGetSelectedKeys (PKEYMAN pKM, PGPKeySetRef* pKeySet, INT* piCount);

// KMIDataObject.cpp
LPDATAOBJECT KMCreateDataObject (PKEYMAN pKM, LPSTR szName);
BOOL KMOKToDeleteDataObject (LPDATAOBJECT pDataObject);

// KMIDropSource.cpp
LPDROPSOURCE KMCreateDropSource (PKEYMAN pKM, HWND hwnd, HWND hwndTree);

// KMIDropTarget.cpp
LPDROPTARGET KMCreateDropTarget (
					HWND hwnd, VOID* pKeyMan, VOID* pSplitStruct);
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
BOOL KMSetDefaultKey (PKEYMAN pKM);
BOOL KMSetPrimaryUserID (PKEYMAN pKM);
BOOL KMAddSelectedToMain (PKEYMAN pKM);
BOOL KMReverifySigs (PKEYMAN pKM);

// KMMenu.c
VOID KMContextMenu (PKEYMAN pKM, INT x, INT y);

// KMMisc.c
VOID* KMAlloc (LONG size);
VOID KMFree (VOID* p);
PGPKeyRef KMGetKeyFromUserID (PKEYMAN pKM, PGPUserIDRef UserID);
BOOL KMCheckForSecretKeys (PGPKeySetRef KeySet);
BOOL KMIsThisTheOnlyUserID (PKEYMAN pKM, PGPUserIDRef UID);
BOOL KMIsThisThePrimaryUserID (PKEYMAN pKM, PGPUserIDRef UID);
BOOL KMExistingPhotoID (PKEYMAN pKM, PGPKeyRef key);
BOOL KMGetKeyName (PGPKeyRef Key, LPSTR sz, UINT uLen);
BOOL KMGetUserIDName (PGPUserIDRef UserID, LPSTR sz, UINT uLen);
INT KMDetermineKeyIcon (PKEYMAN pKM, PGPKeyRef Key, BOOL* lpbItalics);
INT KMDetermineUserIDIcon (PGPKeyRef Key, PGPUserIDRef UserID, 
						BOOL* pbItalics);
INT KMDetermineCertIcon (PGPSigRef Cert, BOOL* pbItalics);
INT KMCommitKeyRingChanges (PKEYMAN pKM);
BOOL KMGetKeyIDFromKey (PGPKeyRef Key, LPSTR sz, UINT u);
BOOL KMGetKeyIDFromCert (PGPSigRef Cert, LPSTR sz, UINT u);
PGPKeyRef KMGetKeyFromCert (PKEYMAN pKM, PGPSigRef Cert);
VOID KMFindWindowFromPoint (PKEYMAN pKM, POINT* ppt, HWND phwnd);

// KMPhoto.c
INT KMGetDIBSize (LPBITMAPINFO lpbi, INT* piWidth, INT* piHeight);
HBITMAP KMDDBfromDIB (LPBITMAPINFO lpbi, HPALETTE* lphPalette);
PGPError KMDIBfromPhoto (LPBYTE buf, INT isize, BOOL bForDisplay,
						 LPBITMAPINFO* plpbmi);
PGPError KMCopyPhotoToClipboard (HWND hWnd, PBYTE buf, INT isize);
PGPError KMPastePhotoFromClipboard (HWND hWnd, LPBYTE* pbuf, INT* pisize);
PGPError KMReadPhotoFromFile (LPSTR pszFile, LPBYTE* pbuf, INT* pisize);

// KMProps.c
BOOL KMKeyProperties (PKEYMAN pKM);
VOID KMUpdateKeyProperties (PKEYMAN pKM);
VOID KMUpdateKeyPropertiesThread (PKEYMAN pKM);
VOID KMDeletePropertiesKey (PKEYMAN pKM, PGPKeyRef Key);
VOID KMDeleteAllKeyProperties (PKEYMAN pKM, BOOL bCloseWindows);
VOID KMEnableAllKeyProperties (PKEYMAN pKM, BOOL bEnable);

// KMRevoke.c
BOOL KMRevokeKey (PKEYMAN pKM);
BOOL KMRevokeCert (PKEYMAN pKM);
BOOL KMAddRevoker (PKEYMAN pKM);

// KMServer.c
BOOL KMSendToServer (PKEYMAN pKM, UINT uServerFlags);
BOOL KMGetFromServer (PKEYMAN pKM);
BOOL KMGetFromServerInternal (PKEYMAN pKM, 
						BOOL bQueryAdd, BOOL bWarn, BOOL bGetSigners); 
BOOL KMDeleteFromServer (PKEYMAN pKM);
BOOL KMDisableOnServer (PKEYMAN pKM);

// KMShare.c
BOOL KMSplitKey (PKEYMAN pKM, PGPKeyRef key);
BOOL KMExistSplitKeyDialog (PKEYMAN pKM);
BOOL KMSplitDropKeys (PSPLITKEYSTRUCT psks, HANDLE hMem);

// KMTree.c
BOOL KMAddColumns (PKEYMAN pKM);
BOOL KMLoadKeyRingIntoTree (PKEYMAN pKM, BOOL bReInsert, 
							BOOL bExpandNew, BOOL bForceRealloc);
BOOL KMUpdateKeyInTree (PKEYMAN pKM, PGPKeyRef Key, BOOL bForceNew);
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
VOID KMSelectKey (PKEYMAN pKM, PGPKeyRef key, BOOL bDeselect);
VOID KMGetKeyBitsString (PGPKeySetRef KeySet, 
						 PGPKeyRef Key, LPSTR sz, UINT u);

// KMUser.c
LRESULT KMMessageBox (HWND hwnd, INT iCaption, INT iMessage, ULONG flags);
BOOL KMUseBadPassPhrase (HWND hwnd);
BOOL KMConstructUserID (HWND hDlg, UINT uNameIDC, UINT uAddrIDC, 
						LPSTR* pszUserID);
BOOL KMSignConfirm (PKEYMAN pKM, BOOL* pbSendToServer);
VOID KMWipeEditBox (HWND hDlg, UINT uID);
PGPError KMGetKeyPhrase (
		PGPContextRef		context,
		PGPtlsContextRef	tlsContext,
		HWND				hwnd, 
		LPSTR				szPrompt,
		PGPKeySetRef		keyset,
		PGPKeyRef			key,
		LPSTR*				ppszPhrase,
		PGPByte**			ppPasskeyBuffer,
		PGPUInt32*			piPasskeyLength);
PGPError KMGetSigningKeyPhrase (
		PGPContextRef		context,
		PGPtlsContextRef	tlsContext,
		HWND				hwnd, 
		LPSTR				szPrompt,
		PGPKeySetRef		keyset,
		BOOL				bRejectSplitKeys,
		PGPKeyRef*			pkey,
		LPSTR*				ppszPhrase,
		PGPByte**			ppPasskeyBuffer,
		PGPUInt32*			piPasskeyLength);
PGPError KMGetConfirmationPhrase (
		PGPContextRef		context,
		HWND				hwnd, 
		LPSTR				szPrompt,
		PGPKeySetRef		keyset,
		INT					iMinPhraseLength,
		INT					iMinPhraseQuality,
		LPSTR*				ppszPhrase);
PGPError KMGetDecryptionPhrase (
		PGPContextRef		context,
		PGPtlsContextRef	tlsContext,
		HWND				hwnd, 
		LPSTR				szPrompt,
		PGPKeySetRef		keysetMain,
		PGPKeyRef*			pkey,
		PGPKeySetRef		keysetDecryption,
		PGPUInt32			iKeyIDCount,
		PGPKeyID*			keyidsDecryption,
		PGPKeySetRef*		pkeysetToAdd,
		LPSTR*				ppszPhrase,
		PGPByte**			ppPasskeyBuffer,
		PGPUInt32*			piPasskeyLength); 
VOID KMFreePhrase (LPSTR pszPhrase);
VOID KMFreePasskey (PGPByte* pbyte, PGPSize size);

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
