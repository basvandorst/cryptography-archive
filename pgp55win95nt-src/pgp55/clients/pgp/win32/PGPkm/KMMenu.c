/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	KMMenu.c - handle menu enabling/disabling chores
	

	$Id: KMMenu.c,v 1.16 1997/10/07 20:38:18 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

#include "pgpkmx.h"
#include "resource.h"

#include "pgpBuildFlags.h"

// External globals
extern HINSTANCE g_hInst;

//	___________________________________________________
//
//	determine if each action is enabled or not

//	copy 
BOOL IsCopyEnabled (PKEYMAN pKM) {
	if (KMPromiscuousSelected (pKM)) return FALSE;
	if (KMFocusedObjectType (pKM) != OBJECT_KEY) return FALSE;
	return TRUE;
}

//	paste 
BOOL IsPasteEnabled (PKEYMAN pKM) {
	if (pKM->ulOptionFlags & KMF_READONLY) return FALSE;
	if (!KMDataToPaste ()) return FALSE;
	return TRUE;
}

//	delete 
BOOL IsDeleteEnabled (PKEYMAN pKM) {
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
BOOL IsDeleteFromServerEnabled (PKEYMAN pKM) {
	if (KMPromiscuousSelected (pKM)) return FALSE;
	if (KMFocusedObjectType (pKM) != OBJECT_KEY) return FALSE;
	if (strnicmp (pKM->szPutKeyserver, "ldap", 4)) return FALSE;
	return TRUE;
}

//	select all 
BOOL IsSelectAllEnabled (PKEYMAN pKM) {
	return TRUE;
}

//	collapse all 
BOOL IsCollapseAllEnabled (PKEYMAN pKM) {
	if (KMFocusedItemType (pKM) == IDX_NONE) return TRUE;
	return FALSE;
}

//	expand all 
BOOL IsExpandAllEnabled (PKEYMAN pKM) {
	if (KMFocusedItemType (pKM) == IDX_NONE) return TRUE;
	return FALSE;
}

//	collapse selected 
BOOL IsCollapseSelEnabled (PKEYMAN pKM) {
	if (KMFocusedItemType (pKM) == IDX_NONE) return FALSE;
	return TRUE;
}

//	expand selected 
BOOL IsExpandSelEnabled (PKEYMAN pKM) {
	if (KMFocusedItemType (pKM) == IDX_NONE) return FALSE;
	return TRUE;
}

//	certify 
BOOL IsCertifyEnabled (PKEYMAN pKM) {
	if (pKM->ulOptionFlags & KMF_READONLY) return FALSE;
	if (KMPromiscuousSelected (pKM)) return KMSigningAllowed (pKM);
	switch (KMFocusedItemType (pKM)) {
		case IDX_NONE : 
			return FALSE;
		case IDX_CERT :
		case IDX_REVCERT :
		case IDX_BADCERT :
		case IDX_EXPORTCERT :
		case IDX_METACERT :
		case IDX_EXPORTMETACERT :
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
BOOL IsEnableEnabled (PKEYMAN pKM) {
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
BOOL IsDisableEnabled (PKEYMAN pKM) {
	Boolean b;

	if (pKM->ulOptionFlags & KMF_READONLY) return FALSE;
	if (KMPromiscuousSelected (pKM)) return FALSE;
	if (KMMultipleSelected (pKM)) return FALSE;
	switch (KMFocusedItemType (pKM)) {
		case IDX_RSAPUBKEY :
		case IDX_DSAPUBKEY :
			return TRUE;
		case IDX_RSASECKEY :
		case IDX_DSASECKEY :
			PGPGetKeyBoolean ((PGPKeyRef)KMFocusedObject (pKM),
				kPGPKeyPropIsAxiomatic, &b);
			if (b) return FALSE;
			else return TRUE;
		default :
			return FALSE;
	}
}

//	disable on server
BOOL IsDisableOnServerEnabled (PKEYMAN pKM) {
	if (KMPromiscuousSelected (pKM)) return FALSE;
	if (KMFocusedObjectType (pKM) != OBJECT_KEY) return FALSE;
	if (strnicmp (pKM->szPutKeyserver, "ldap", 4)) return FALSE;
	return TRUE;
}

//	add user id 
BOOL IsAddUserEnabled (PKEYMAN pKM) {
	if (pKM->ulOptionFlags & KMF_READONLY) return FALSE;
	if (KMPromiscuousSelected (pKM)) return FALSE;
	if (KMMultipleSelected (pKM)) return FALSE;
	switch (KMFocusedItemType (pKM)) {
		case IDX_RSASECKEY :
		case IDX_DSASECKEY :
		case IDX_RSASECDISKEY :
		case IDX_DSASECDISKEY :
			return TRUE;
		default :
			return FALSE;
	}
}

//	revoke 
BOOL IsRevokeEnabled (PKEYMAN pKM) {
	Boolean b;

	if (pKM->ulOptionFlags & KMF_READONLY) return FALSE;
	if (KMPromiscuousSelected (pKM)) return FALSE;
	if (KMMultipleSelected (pKM)) return FALSE;
	switch (KMFocusedItemType (pKM)) {
		case IDX_RSASECKEY :
		case IDX_DSASECKEY :
		case IDX_RSASECDISKEY :
		case IDX_DSASECDISKEY :
			return TRUE;
		case IDX_CERT :
		case IDX_BADCERT :
		case IDX_EXPORTCERT :
		case IDX_METACERT :
		case IDX_EXPORTMETACERT :
			PGPGetSigBoolean ((PGPSigRef)KMFocusedObject (pKM), 
				kPGPSigPropIsMySig, &b);
			if (b) return TRUE;
			else return FALSE;
		default :
			return FALSE;
	}
}

//	set as default
BOOL IsSetAsDefaultEnabled (PKEYMAN pKM) {
	if (KMPromiscuousSelected (pKM)) return FALSE;
	if (KMMultipleSelected (pKM)) return FALSE;
	switch (KMFocusedItemType (pKM)) {
		case IDX_RSASECKEY :
		case IDX_RSASECDISKEY :
			if (pKM->ulOptionFlags & KMF_DISABLERSAOPS) return FALSE;
			else return TRUE;
		case IDX_DSASECKEY :
		case IDX_DSASECDISKEY :
			return TRUE;
		default :
			return FALSE;
	}
}

//	set as primary
BOOL IsSetAsPrimaryEnabled (PKEYMAN pKM) {
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
BOOL IsImportEnabled (PKEYMAN pKM) {
	if (pKM->ulOptionFlags & KMF_READONLY) return FALSE;
	return TRUE;
}

//	export
BOOL IsExportEnabled (PKEYMAN pKM) {
	if (KMPromiscuousSelected (pKM)) return FALSE;
	if (KMFocusedObjectType (pKM) != OBJECT_KEY) return FALSE;
	return TRUE;
}

//	properties
BOOL IsPropertiesEnabled (PKEYMAN pKM) {
	Boolean b;

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
BOOL IsSendToServerEnabled (PKEYMAN pKM) {
	if (KMPromiscuousSelected (pKM)) return FALSE;
	if (KMFocusedObjectType (pKM) != OBJECT_KEY) return FALSE;
	return TRUE;
}

//	get from keyserver
BOOL IsGetFromServerEnabled (PKEYMAN pKM) {
	if (KMPromiscuousSelected (pKM)) return FALSE;
	if (KMFocusedObjectType (pKM) == OBJECT_NONE) return FALSE;
	if (KMFocusedObjectType (pKM) == OBJECT_USERID) return FALSE;
	return TRUE;
}

//	unselect all 
BOOL IsUnselectAllEnabled (PKEYMAN pKM) {
	return TRUE;
}

//	add to main 
BOOL IsAddToMainEnabled (PKEYMAN pKM) {
	if (KMPromiscuousSelected (pKM)) return FALSE;
	if (KMFocusedObjectType (pKM) != OBJECT_KEY) return FALSE;
	return TRUE;
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

	case KM_COPY :				return IsCopyEnabled (pKM);
	case KM_PASTE :				return IsPasteEnabled (pKM);
	case KM_DELETEFROMSERVER :	return IsDeleteFromServerEnabled (pKM);
	case KM_DELETE :			return IsDeleteEnabled (pKM);
	case KM_SELECTALL :			return IsSelectAllEnabled (pKM);
	case KM_COLLAPSEALL :		return IsCollapseAllEnabled (pKM);
	case KM_COLLAPSESEL :		return IsCollapseSelEnabled (pKM);
	case KM_EXPANDALL :			return IsExpandAllEnabled (pKM);
	case KM_EXPANDSEL :			return IsExpandSelEnabled (pKM);
	case KM_CERTIFY :			return IsCertifyEnabled (pKM);
	case KM_ENABLE :			return IsEnableEnabled (pKM);
	case KM_DISABLEONSERVER :	return IsDisableOnServerEnabled (pKM);
	case KM_DISABLE :			return IsDisableEnabled (pKM);
	case KM_ADDUSERID :			return IsAddUserEnabled (pKM);
	case KM_REVOKE :			return IsRevokeEnabled (pKM);
	case KM_SETASDEFAULT :		return IsSetAsDefaultEnabled (pKM);
	case KM_SETASPRIMARY :		return IsSetAsPrimaryEnabled (pKM);
	case KM_IMPORT :			return IsImportEnabled (pKM);
	case KM_EXPORT :			return IsExportEnabled (pKM);
	case KM_PROPERTIES :		return IsPropertiesEnabled (pKM);
	case KM_SENDTOSERVER :		return IsSendToServerEnabled (pKM);
	case KM_GETFROMSERVER :		return IsGetFromServerEnabled (pKM);
	case KM_UNSELECTALL:		return IsUnselectAllEnabled (pKM);
	case KM_ADDTOMAIN :			return IsAddToMainEnabled (pKM);
	default :					return FALSE;
	}
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

	HTLITEM			hFirst;
	TL_TREEITEM		tlI;

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
		KMSendToServer (pKM);
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

	}

	return kPGPError_NoErr;
}

