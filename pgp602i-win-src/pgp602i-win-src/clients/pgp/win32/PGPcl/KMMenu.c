/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	KMMenu.c - handle menu enabling/disabling chores
	

	$Id: KMMenu.c,v 1.24 1998/08/11 14:43:33 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

// project header files
#include "pgpkmx.h"

// External globals
extern HINSTANCE g_hInst;

//	___________________________________________________
//
//	determine if each action is enabled or not

//	copy 
static BOOL 
sIsCopyEnabled (PKEYMAN pKM) {
	if (KMPromiscuousSelected (pKM)) return FALSE;
	if (KMFocusedObjectType (pKM) != OBJECT_KEY) return FALSE;
	return TRUE;
}

//	paste 
static BOOL 
sIsPasteEnabled (PKEYMAN pKM) {
	if (pKM->ulOptionFlags & KMF_READONLY) return FALSE;
	if (!KMDataToPaste ()) return FALSE;
	return TRUE;
}

//	delete 
BOOL 
IsDeleteEnabled (PKEYMAN pKM) {
	if (pKM->ulOptionFlags & KMF_READONLY) return FALSE;
	if (KMMultipleSelected (pKM)) return TRUE;
	switch (KMFocusedItemType (pKM)) {
		case IDX_NONE : 
			return FALSE;
		case IDX_RSAUSERID :
		case IDX_DSAUSERID :
			if (KMIsThisTheOnlyUserID (pKM, 
					(PGPUserIDRef)KMFocusedObject (pKM))) 
				return FALSE;
			else
				return TRUE;
		default :
			return TRUE;
	}
}

//	delete from server
static BOOL 
sIsDeleteFromServerEnabled (PKEYMAN pKM) {
	if (KMPromiscuousSelected (pKM)) return FALSE;
	if (KMFocusedObjectType (pKM) != OBJECT_KEY) return FALSE;
	if ((pKM->keyserver.protocol != kPGPKeyServerType_LDAP) &&
		(pKM->keyserver.protocol != kPGPKeyServerType_LDAPS)) 
		return FALSE;
	return TRUE;
}

//	select all 
static BOOL 
sIsSelectAllEnabled (PKEYMAN pKM) {
	return TRUE;
}

//	collapse all 
static BOOL 
sIsCollapseAllEnabled (PKEYMAN pKM) {
	if (KMFocusedItemType (pKM) == IDX_NONE) return TRUE;
	return FALSE;
}

//	expand all 
static BOOL 
sIsExpandAllEnabled (PKEYMAN pKM) {
	if (KMFocusedItemType (pKM) == IDX_NONE) return TRUE;
	return FALSE;
}

//	collapse selected 
static BOOL 
sIsCollapseSelEnabled (PKEYMAN pKM) {
	if (KMFocusedItemType (pKM) == IDX_NONE) return FALSE;
	return TRUE;
}

//	expand selected 
static BOOL 
sIsExpandSelEnabled (PKEYMAN pKM) {
	if (KMFocusedItemType (pKM) == IDX_NONE) return FALSE;
	return TRUE;
}

//	certify 
static BOOL 
sIsCertifyEnabled (PKEYMAN pKM) {
	if (pKM->ulOptionFlags & KMF_READONLY) return FALSE;
	if (KMPromiscuousSelected (pKM)) return KMSigningAllowed (pKM);
	switch (KMFocusedItemType (pKM)) {
		case IDX_NONE : 
			return FALSE;
		case IDX_CERT :
		case IDX_REVCERT :
		case IDX_EXPCERT :
		case IDX_BADCERT :
		case IDX_EXPORTCERT :
		case IDX_TRUSTEDCERT :
		case IDX_METACERT :
			return FALSE;
		case IDX_RSASECEXPKEY :
		case IDX_DSASECEXPKEY :
		case IDX_RSASECREVKEY :
		case IDX_DSASECREVKEY :
		case IDX_RSAPUBEXPKEY :
		case IDX_DSAPUBEXPKEY :
		case IDX_RSAPUBREVKEY :
		case IDX_DSAPUBREVKEY :
			if (!KMMultipleSelected (pKM)) return FALSE;
			else return TRUE;
		default :
			return TRUE;
	}
}

// enable 
static BOOL 
sIsEnableEnabled (PKEYMAN pKM) {
	if (pKM->ulOptionFlags & KMF_READONLY) return FALSE;
	if (KMPromiscuousSelected (pKM)) return FALSE;
	if (KMMultipleSelected (pKM)) return FALSE;
	switch (KMFocusedItemType (pKM)) {
		case IDX_RSASECDISKEY :
		case IDX_DSASECDISKEY :
		case IDX_RSAPUBDISKEY :
		case IDX_DSAPUBDISKEY :
			return TRUE;
		default :
			return FALSE;
	}
}

//	disable 
static BOOL 
sIsDisableEnabled (PKEYMAN pKM) {
	PGPBoolean b;

	if (pKM->ulOptionFlags & KMF_READONLY) return FALSE;
	if (KMPromiscuousSelected (pKM)) return FALSE;
	if (KMMultipleSelected (pKM)) return FALSE;
	switch (KMFocusedItemType (pKM)) {
		case IDX_RSAPUBKEY :
		case IDX_DSAPUBKEY :
			return TRUE;
		case IDX_RSASECKEY :
		case IDX_RSASECSHRKEY :
		case IDX_DSASECKEY :
		case IDX_DSASECSHRKEY :
			PGPGetKeyBoolean ((PGPKeyRef)KMFocusedObject (pKM),
				kPGPKeyPropIsAxiomatic, &b);
			if (b) return FALSE;
			else return TRUE;
		default :
			return FALSE;
	}
}

//	disable on server
static BOOL 
sIsDisableOnServerEnabled (PKEYMAN pKM) {
	if (KMPromiscuousSelected (pKM)) return FALSE;
	if (KMFocusedObjectType (pKM) != OBJECT_KEY) return FALSE;
	if ((pKM->keyserver.protocol != kPGPKeyServerType_LDAP) &&
		(pKM->keyserver.protocol != kPGPKeyServerType_LDAPS)) 
		return FALSE;
	return TRUE;
}

//	add user id 
static BOOL 
sIsAddUserEnabled (PKEYMAN pKM) {
	if (pKM->ulOptionFlags & KMF_READONLY) return FALSE;
	if (KMPromiscuousSelected (pKM)) return FALSE;
	if (KMMultipleSelected (pKM)) return FALSE;
	switch (KMFocusedItemType (pKM)) {
		case IDX_RSASECKEY :
		case IDX_DSASECKEY :
		case IDX_RSASECDISKEY :
		case IDX_DSASECDISKEY :
		case IDX_RSASECSHRKEY :
		case IDX_DSASECSHRKEY :
			return TRUE;
		default :
			return FALSE;
	}
}

//	add photo id 
static BOOL 
sIsAddPhotoEnabled (PKEYMAN pKM) {
	if (pKM->ulOptionFlags & KMF_READONLY) return FALSE;
	if (KMPromiscuousSelected (pKM)) return FALSE;
	if (KMMultipleSelected (pKM)) return FALSE;
	switch (KMFocusedItemType (pKM)) {
		case IDX_DSASECKEY :
		case IDX_DSASECDISKEY :
		case IDX_DSASECSHRKEY :
			return TRUE;
		default :
			return FALSE;
	}
}

//	add revoker 
static BOOL 
sIsAddRevokerEnabled (PKEYMAN pKM) {
	if (pKM->ulOptionFlags & KMF_READONLY) return FALSE;
	if (KMPromiscuousSelected (pKM)) return FALSE;
	if (KMMultipleSelected (pKM)) return FALSE;
	switch (KMFocusedItemType (pKM)) {
		case IDX_DSASECKEY :
		case IDX_DSASECDISKEY :
		case IDX_DSASECSHRKEY :
			return TRUE;
		default :
			return FALSE;
	}
}

//	revoke 
static BOOL 
sIsRevokeEnabled (PKEYMAN pKM) {
	PGPBoolean b1, b2;

	if (pKM->ulOptionFlags & KMF_READONLY) return FALSE;
	if (KMPromiscuousSelected (pKM)) return FALSE;
	if (KMMultipleSelected (pKM)) return FALSE;

	switch (KMFocusedItemType (pKM)) {
		case IDX_RSASECKEY :
		case IDX_DSASECKEY :
		case IDX_RSASECDISKEY :
		case IDX_DSASECDISKEY :
		case IDX_RSASECSHRKEY :
		case IDX_DSASECSHRKEY :
			PGPGetKeyBoolean ((PGPKeyRef)KMFocusedObject (pKM),
				kPGPKeyPropCanSign, &b2);
 			if (b2) return TRUE;
			else return FALSE;

		case IDX_RSAPUBKEY :
		case IDX_DSAPUBKEY :
		case IDX_RSAPUBDISKEY :
		case IDX_DSAPUBDISKEY :
			PGPGetKeyBoolean ((PGPKeyRef)KMFocusedObject (pKM),
				kPGPKeyPropIsRevocable, &b1);
 			if (b1) return TRUE;
			else return FALSE;

		case IDX_CERT :
		case IDX_BADCERT :
		case IDX_EXPORTCERT :
		case IDX_TRUSTEDCERT :
		case IDX_METACERT :
			PGPGetSigBoolean ((PGPSigRef)KMFocusedObject (pKM), 
				kPGPSigPropIsMySig, &b1);
			if (b1) return TRUE;
			else return FALSE;

		default :
			return FALSE;
	}
}

//	set as default
static BOOL 
sIsSetAsDefaultEnabled (PKEYMAN pKM) {
	PGPBoolean b;

	if (KMPromiscuousSelected (pKM)) return FALSE;
	if (KMMultipleSelected (pKM)) return FALSE;
	switch (KMFocusedItemType (pKM)) {
		case IDX_RSASECKEY :
		case IDX_RSASECDISKEY :
		case IDX_RSASECSHRKEY :
			PGPGetKeyBoolean ((PGPKeyRef)KMFocusedObject (pKM),
				kPGPKeyPropCanSign, &b);
			if (b) return TRUE;
			else return FALSE;
		case IDX_DSASECKEY :
		case IDX_DSASECDISKEY :
		case IDX_DSASECSHRKEY :
			return TRUE;
		default :
			return FALSE;
	}
}

//	set as primary
static BOOL 
sIsSetAsPrimaryEnabled (PKEYMAN pKM) {
	if (pKM->ulOptionFlags & KMF_READONLY) return FALSE;
	if (KMPromiscuousSelected (pKM)) return FALSE;
	if (KMMultipleSelected (pKM)) return FALSE;
	switch (KMFocusedItemType (pKM)) {
		case IDX_RSAUSERID :
		case IDX_DSAUSERID :
			return (!KMIsThisThePrimaryUserID (pKM, 
				(PGPUserIDRef)KMFocusedObject (pKM)));
		default :
			return FALSE;
	}
}

//	import 
static BOOL 
sIsImportEnabled (PKEYMAN pKM) {
	if (pKM->ulOptionFlags & KMF_READONLY) return FALSE;
	return TRUE;
}

//	export
static BOOL 
sIsExportEnabled (PKEYMAN pKM) {
	if (KMPromiscuousSelected (pKM)) return FALSE;
	if (KMFocusedObjectType (pKM) != OBJECT_KEY) return FALSE;
	return TRUE;
}

//	properties
static BOOL 
sIsPropertiesEnabled (PKEYMAN pKM) {
	PGPBoolean b;

	if (pKM->ulOptionFlags & KMF_MODALPROPERTIES) {
		if (KMMultipleSelected (pKM)) return FALSE;
	}
	if (KMFocusedObjectType (pKM) == OBJECT_NONE) return FALSE;
	if (KMFocusedObjectType (pKM) == OBJECT_CERT) {
		if (KMMultipleSelected (pKM)) return FALSE;
		PGPGetSigBoolean ((PGPSigRef)KMFocusedObject (pKM), 
							kPGPSigPropIsVerified, &b);
		if (!b) return FALSE;
	}
	return TRUE;
}

//	send to keyserver
static BOOL 
sIsSendToServerEnabled (PKEYMAN pKM) {
	if (KMPromiscuousSelected (pKM)) return FALSE;
	if (KMFocusedObjectType (pKM) != OBJECT_KEY) return FALSE;
	return TRUE;
}

//	get from keyserver
static BOOL 
sIsGetFromServerEnabled (PKEYMAN pKM) {
	if (KMPromiscuousSelected (pKM)) return FALSE;
	if (KMFocusedObjectType (pKM) == OBJECT_NONE) return FALSE;
	if (KMFocusedObjectType (pKM) == OBJECT_USERID) return FALSE;
	return TRUE;
}

//	unselect all 
static BOOL 
sIsUnselectAllEnabled (PKEYMAN pKM) {
	return TRUE;
}

//	add to main 
static BOOL 
sIsAddToMainEnabled (PKEYMAN pKM) {
	if (KMPromiscuousSelected (pKM)) return FALSE;
	if (KMFocusedObjectType (pKM) != OBJECT_KEY) return FALSE;
	return TRUE;
}

//	reverify signatures 
static BOOL 
sIsReverifySigsEnabled (PKEYMAN pKM) {
	if (KMPromiscuousSelected (pKM)) return FALSE;
	if (KMFocusedObjectType (pKM) != OBJECT_KEY) return FALSE;
	return TRUE;
}

//	split key for sharing 
static BOOL 
sIsSplitKeyEnabled (PKEYMAN pKM) {
	PGPBoolean b;

	if (pKM->ulOptionFlags & KMF_READONLY) return FALSE;
	if (KMPromiscuousSelected (pKM)) return FALSE;
	if (KMMultipleSelected (pKM)) return FALSE;
	switch (KMFocusedItemType (pKM)) {
		case IDX_RSASECKEY :
		case IDX_RSASECDISKEY :
			PGPGetKeyBoolean ((PGPKeyRef)KMFocusedObject (pKM),
				kPGPKeyPropCanSign, &b);
			if (b) return TRUE;
			else return FALSE;
		case IDX_DSASECKEY :
		case IDX_DSASECDISKEY :
			return TRUE;
		default :
			return FALSE;
	}
}

//	___________________________________________________
//
//	determine if action is enabled or not, based on
//	flags passed in and current selection

BOOL PGPkmExport 
PGPkmIsActionEnabled (
		HKEYMAN hKM, 
		ULONG	ulAction) 
{

	PKEYMAN pKM = (PKEYMAN)hKM;

	if (!hKM) return FALSE;
	if (ulAction & pKM->ulDisableActions) return FALSE;

	switch (ulAction) {

	case KM_COPY :				return sIsCopyEnabled (pKM);
	case KM_PASTE :				return sIsPasteEnabled (pKM);
	case KM_DELETEFROMSERVER :	return sIsDeleteFromServerEnabled (pKM);
	case KM_DELETE :			return IsDeleteEnabled (pKM);
	case KM_SELECTALL :			return sIsSelectAllEnabled (pKM);
	case KM_COLLAPSEALL :		return sIsCollapseAllEnabled (pKM);
	case KM_COLLAPSESEL :		return sIsCollapseSelEnabled (pKM);
	case KM_EXPANDALL :			return sIsExpandAllEnabled (pKM);
	case KM_EXPANDSEL :			return sIsExpandSelEnabled (pKM);
	case KM_CERTIFY :			return sIsCertifyEnabled (pKM);
	case KM_ENABLE :			return sIsEnableEnabled (pKM);
	case KM_DISABLEONSERVER :	return sIsDisableOnServerEnabled (pKM);
	case KM_DISABLE :			return sIsDisableEnabled (pKM);
	case KM_ADDUSERID :			return sIsAddUserEnabled (pKM);
	case KM_ADDPHOTOID :		return sIsAddPhotoEnabled (pKM);
	case KM_ADDREVOKER :		return sIsAddRevokerEnabled (pKM);
	case KM_REVOKE :			return sIsRevokeEnabled (pKM);
	case KM_SETASDEFAULT :		return sIsSetAsDefaultEnabled (pKM);
	case KM_SETASPRIMARY :		return sIsSetAsPrimaryEnabled (pKM);
	case KM_IMPORT :			return sIsImportEnabled (pKM);
	case KM_EXPORT :			return sIsExportEnabled (pKM);
	case KM_PROPERTIES :		return sIsPropertiesEnabled (pKM);
	case KM_SENDTOSERVER :		return sIsSendToServerEnabled (pKM);
	case KM_GETFROMSERVER :		return sIsGetFromServerEnabled (pKM);
	case KM_UNSELECTALL:		return sIsUnselectAllEnabled (pKM);
	case KM_ADDTOMAIN :			return sIsAddToMainEnabled (pKM);
	case KM_REVERIFYSIGS :		return sIsReverifySigsEnabled (pKM);
	case KM_SPLITKEY	 :		return sIsSplitKeyEnabled (pKM);
	default :					return FALSE;
	}
}

//	___________________________________________________
//
//	select all keys

static VOID
sSelectAll (PKEYMAN pKM) 
{
	HTLITEM			hFirst;
	TL_TREEITEM		tlI;

	// get first item in list
	TreeList_SelectChildren (pKM->hWndTree, NULL);
	hFirst = TreeList_GetFirstItem (pKM->hWndTree);

	// try to get second item
	tlI.hItem = hFirst;
	if (tlI.hItem) {
		tlI.mask = TLIF_NEXTHANDLE;
		TreeList_GetItem (pKM->hWndTree, &tlI);
	}

	// if second item exists, then multiple select
	if (tlI.hItem)
		KMSetFocus (pKM, hFirst, TRUE);
	else 
		KMSetFocus (pKM, hFirst, FALSE);

	SetFocus (pKM->hWndTree);
}

//	___________________________________________________
//
//	perform the specified action

PGPError PGPkmExport 
PGPkmPerformAction (
		HKEYMAN hKM, 
		ULONG	ulAction) 
{
	PKEYMAN			pKM = (PKEYMAN)hKM;

	if (!hKM) return kPGPError_BadParams;
	if (!PGPkmIsActionEnabled (hKM, ulAction)) 
		return kPGPError_UnknownRequest;

	switch (ulAction) {

	case KM_COPY :
		KMCopyKeys (pKM, NULL);
		break;

	case KM_PASTE :
		KMPasteKeys (pKM);
		break;

	case KM_DELETE :
		KMDeleteObject (pKM);
		break;

	case KM_SELECTALL :
		sSelectAll (pKM);
		break;

	case KM_COLLAPSEALL :
		KMCollapseSelected (pKM);
		break;

	case KM_COLLAPSESEL :
		KMCollapseSelected (pKM);
		break;

	case KM_EXPANDALL :
		KMExpandSelected (pKM);
		break;

	case KM_EXPANDSEL :
		KMExpandSelected (pKM);
		break;

	case KM_CERTIFY :
		KMCertifyKeyOrUserID (pKM);
		break;

	case KM_ENABLE :
		KMEnableKey (pKM, (PGPKeyRef)KMFocusedObject (pKM));
		break;

	case KM_DISABLE :
		KMDisableKey (pKM, (PGPKeyRef)KMFocusedObject (pKM));
		break;

	case KM_ADDUSERID :
		KMAddUserToKey (pKM); 
		break;

	case KM_ADDPHOTOID :
		KMAddPhotoToKey (pKM);
		break;

	case KM_ADDREVOKER :
		KMAddRevoker (pKM);
		break;

	case KM_REVOKE :
		if (KMFocusedObjectType (pKM) == OBJECT_CERT) KMRevokeCert (pKM);
		else KMRevokeKey (pKM);
		break;

	case KM_SETASDEFAULT :
		KMSetDefaultKey (pKM);
		break;

	case KM_SETASPRIMARY :
		KMSetPrimaryUserID (pKM);
		break;

	case KM_IMPORT :
		KMImportKey (pKM, NULL);
		break;
		
	case KM_EXPORT :
		KMExportKeys (pKM, NULL);
		break;

	case KM_PROPERTIES :
		KMKeyProperties (pKM);
		break;

	case KM_SENDTOSERVER :
		KMSendToServer (pKM, PGPCL_SPECIFIEDSERVER);
		break;

	case KM_GETFROMSERVER :
		KMGetFromServer (pKM);
		break;

	case KM_DELETEFROMSERVER :
		KMDeleteFromServer (pKM);
		break;

	case KM_DISABLEONSERVER :
		KMDisableOnServer (pKM);
		break;

	case KM_UNSELECTALL :
		TreeList_Select (pKM->hWndTree, NULL, TRUE);
		KMSetFocus (pKM, NULL, FALSE);
		break;
		
	case KM_ADDTOMAIN :
		KMAddSelectedToMain (pKM);
		break;

	case KM_REVERIFYSIGS :
		KMReverifySigs (pKM);
		break;

	case KM_SPLITKEY :
		KMSplitKey (pKM, (PGPKeyRef)KMFocusedObject (pKM));
		break;

	}

	return kPGPError_NoErr;
}

