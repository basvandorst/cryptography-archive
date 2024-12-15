/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	KMNotProc - notification processing and associated routines
	

	$Id: KMNotPrc.c,v 1.12 1997/09/12 17:41:37 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

#include "pgpkmx.h"
#include <stdio.h>
#include "resource.h"


// External globals
extern HINSTANCE g_hInst;

//	___________________________________________________
//
//  Key manager notification processing procedure

PGPError PGPkmExport 
PGPkmDefaultNotificationProc (
		HKEYMAN hKM, 
		LPARAM	lParam) 
{
	HTLITEM hFocused;
	BOOL	bMultiple;
	INT		i, iField;
	PKEYMAN pKM = (PKEYMAN)hKM;

	if (!hKM) return kPGPError_BadParams;

	switch (((LPNM_TREELIST)lParam)->hdr.code) {

	case TLN_SELCHANGED :
		if (pKM->ulOptionFlags & KMF_ONLYSELECTKEYS) {
			TL_TREEITEM tli;
			HTLITEM hItem, hInitialItem;

			hInitialItem = ((LPNM_TREELIST)lParam)->itemNew.hItem;
			tli.hItem = hInitialItem;
			do {
				hItem = tli.hItem;
				tli.mask = TLIF_PARENTHANDLE;
				TreeList_GetItem (pKM->hWndTree, &tli);
			} while (tli.hItem);

			if (hItem != hInitialItem) {
				tli.hItem = hItem;
				TreeList_Select (pKM->hWndTree, &tli, TRUE);
			}
			else {
				bMultiple = (((LPNM_TREELIST)lParam)->flags & TLC_MULTIPLE);
				hFocused = ((LPNM_TREELIST)lParam)->itemNew.hItem;
				KMSetFocus (pKM, hFocused, bMultiple);
			}
		}
		else {
			bMultiple = (((LPNM_TREELIST)lParam)->flags & TLC_MULTIPLE);
			hFocused = ((LPNM_TREELIST)lParam)->itemNew.hItem;
			KMSetFocus (pKM, hFocused, bMultiple);
		}
		((LPNM_TREELIST)lParam)->flags = KMSelectedFlags (pKM);
		break;

	case TLN_KEYDOWN :
		switch (((TL_KEYDOWN*)lParam)->wVKey) {
		case VK_DELETE :
			KMDeleteObject (pKM);
			break;
		}
		break;

	case TLN_CONTEXTMENU :
		((LPNM_TREELIST)lParam)->flags = KMSelectedFlags (pKM);
		break;

	case TLN_BEGINDRAG :
		if (pKM->ulOptionFlags & KMF_ENABLEDRAGOUT) {
			KMEnableDropTarget (pKM->pDropTarget, FALSE);
			KMDragAndDrop (pKM);
			KMEnableDropTarget (pKM->pDropTarget, 
							!(pKM->ulOptionFlags & KMF_READONLY) &&
							(pKM->ulOptionFlags & KMF_ENABLEDROPIN));
		}
		break;

	case TLN_LISTITEMCLICKED : 
		if (PGPkmIsActionEnabled (hKM, KM_CERTIFY))
			KMCertifyKeyOrUserID (pKM);
		break;

	case TLN_HEADERCLICKED :
		i = pKM->lKeyListSortField;
		iField = pKM->wColumnField[((LPNM_TREELIST)lParam)->index];
		switch (iField) {
			case KMI_NAME : 
				if (pKM->lKeyListSortField == kPGPUserIDOrdering)
					i = kPGPReverseUserIDOrdering; 
				else 
					i = kPGPUserIDOrdering; 
				break;

			case KMI_VALIDITY :
				if (pKM->lKeyListSortField == kPGPValidityOrdering)
					i = kPGPReverseValidityOrdering;
				else
					i = kPGPValidityOrdering;
				break;

			case KMI_TRUST :
				if (pKM->lKeyListSortField == kPGPTrustOrdering)
					i = kPGPReverseTrustOrdering; 
				else
					i = kPGPTrustOrdering; 
				break;

			case KMI_CREATION : 
				if (pKM->lKeyListSortField == kPGPCreationOrdering)
					i = kPGPReverseCreationOrdering; 
				else
					i = kPGPCreationOrdering; 
				break;

			case KMI_EXPIRATION : 
				if (pKM->lKeyListSortField == kPGPExpirationOrdering)
					i = kPGPReverseExpirationOrdering; 
				else
					i = kPGPExpirationOrdering; 
				break;

			case KMI_SIZE : 
				if (pKM->lKeyListSortField == kPGPEncryptKeySizeOrdering)
					i = kPGPReverseEncryptKeySizeOrdering;
				else
					i = kPGPEncryptKeySizeOrdering; 
				break;

			case KMI_KEYID : 
				if (pKM->lKeyListSortField == kPGPKeyIDOrdering)
					i = kPGPReverseKeyIDOrdering;
				else
					i = kPGPKeyIDOrdering; 
				break;

			default : break;
		}
		if (i != pKM->lKeyListSortField) {
			pKM->lKeyListSortField = i;
			TreeList_DeleteTree (pKM->hWndTree, FALSE);
			InvalidateRect (pKM->hWndTree, NULL, TRUE);
			UpdateWindow (pKM->hWndTree);
			KMLoadKeyRingIntoTree (pKM, TRUE, FALSE, FALSE);
			InvalidateRect (pKM->hWndTree, NULL, TRUE);
		}
		break;
	}
	return 0;
}

