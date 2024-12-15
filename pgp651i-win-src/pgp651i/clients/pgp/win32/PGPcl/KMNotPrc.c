/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	KMNotProc - notification processing and associated routines
	

	$Id: KMNotPrc.c,v 1.7 1999/02/11 00:42:10 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

// project header files
#include "pgpkmx.h"

// External globals
extern HINSTANCE g_hInst;

//	___________________________________________________
//
//  reorder columns according to new header order

static VOID  
sReorderColumns (PKEYMAN pKM)
{
	INT		iOrderArray[NUMBERFIELDS];
	WORD	wColumnField[NUMBERFIELDS];
	INT		i, iNumCols;

	// save order
	KMSetColumnPreferences (pKM);
	iNumCols = 1;
	for (i=0; i<NUMBERFIELDS; i++) {
		wColumnField[i] = pKM->wColumnField[i];
		if (wColumnField[i]) iNumCols++;
	}

	TreeList_GetOrderArray (pKM->hWndTree, iNumCols, &iOrderArray);

	for (i=0; i<iNumCols; i++) {
		pKM->wColumnField[i] = wColumnField[iOrderArray[i]];
	}

	KMDeleteAllUserValues (pKM);
	KMSetFocus (pKM, NULL, FALSE);
	TreeList_DeleteTree (pKM->hWndTree, TRUE);
	TreeList_DeleteAllColumns (pKM->hWndTree);

	KMAddColumns (pKM);
	KMLoadKeyRingIntoTree (pKM, FALSE, FALSE, TRUE);
	InvalidateRect (pKM->hWndTree, NULL, TRUE);
	UpdateWindow (pKM->hWndTree);

	KMSetColumnPreferences (pKM);
}


//	___________________________________________________
//
//  Key manager notification processing procedure

PGPError PGPkmExport 
PGPkmDefaultNotificationProc (
		HKEYMAN hKM, 
		LPARAM	lParam) 
{
	PKEYMAN			pKM			= (PKEYMAN)hKM;
	LPNM_TREELIST	lpntl		= (LPNM_TREELIST)lParam;

	HTLITEM			hFocused;
	BOOL			bMultiple;
	INT				i, iField;

	if (!hKM) return kPGPError_BadParams;

	switch (lpntl->hdr.code) {

	case TLN_SELCHANGED :
		if (pKM->ulOptionFlags & KMF_ONLYSELECTKEYS) {
			TL_TREEITEM tli;
			HTLITEM hItem, hInitialItem;

			hInitialItem = lpntl->itemNew.hItem;
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
				bMultiple = lpntl->flags & TLC_MULTIPLE;
				hFocused = lpntl->itemNew.hItem;
				KMSetFocus (pKM, hFocused, bMultiple);
			}
		}
		else {
			bMultiple = lpntl->flags & TLC_MULTIPLE;
			hFocused = lpntl->itemNew.hItem;
			KMSetFocus (pKM, hFocused, bMultiple);
		}
		lpntl->flags = KMSelectedFlags (pKM);
		break;

	case TLN_KEYDOWN :
		switch (((TL_KEYDOWN*)lParam)->wVKey) {
		case VK_DELETE :
			KMDeleteObject (pKM);
			break;
		}
		break;

	case TLN_CONTEXTMENU :
		lpntl->flags = KMSelectedFlags (pKM);
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

	case TLN_ITEMDBLCLICKED :
		if (PGPkmIsActionEnabled (hKM, KM_PROPERTIES))
			KMKeyProperties (pKM);
		break;

	case TLN_HEADERREORDERED :
		sReorderColumns (pKM);
		break;

	case TLN_HEADERCLICKED :
		i = pKM->lKeyListSortField;
		iField = pKM->wColumnField[lpntl->index];
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

