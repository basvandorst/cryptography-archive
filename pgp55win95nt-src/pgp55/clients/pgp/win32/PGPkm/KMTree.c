//:KMTree.c - handle creating and filling TreeList control
//
//	$Id: KMTree.c,v 1.50.4.2 1997/11/20 21:54:35 pbj Exp $
//

#include "pgpkmx.h"
#include <process.h>
#include "resource.h"

#define BITMAP_WIDTH 16
#define BITMAP_HEIGHT 16

typedef struct {
	FARPROC lpfnCallback;
	HWND hWndTree;
} EXPANDCOLLAPSESTRUCT;

typedef struct _USERVALSTRUCT {
	struct _USERVALSTRUCT* next;
	PKEYMAN pKM;
	LONG lValue;
} USERVALSTRUCT;


// external globals
extern HINSTANCE g_hInst;


//----------------------------------------------------|
//  get key user value list item

PGPError KMGetKeyUserVal (PKEYMAN pKM, PGPKeyRef Key, LONG* lValue) {
	USERVALSTRUCT* puvs;
	PGPError err;

	err = PGPGetKeyUserVal (Key, &puvs);
	if (err) return err;

	// there is an existing linked list
	if (puvs) {
		// search for the element inserted by this KM
		do {
			if (puvs->pKM == pKM) {
				*lValue = puvs->lValue;
				return kPGPError_NoErr;
			}
			puvs = puvs->next;
		} while (puvs);

		// no element in list inserted by this KM
		*lValue = 0;
	}

	// no user value
	else 
		*lValue = 0;

	return kPGPError_NoErr;
}


//----------------------------------------------------|
//  get userid user value list item

PGPError KMGetUserIDUserVal (PKEYMAN pKM, PGPUserIDRef UID, LONG* lValue) {
	USERVALSTRUCT* puvs;
	PGPError err;

	err = PGPGetUserIDUserVal (UID, &puvs);
	if (err) return err;

	// there is an existing linked list
	if (puvs) {
		// search for the element inserted by this KM
		do {
			if (puvs->pKM == pKM) {
				*lValue = puvs->lValue;
				return kPGPError_NoErr;
			}
			puvs = puvs->next;
		} while (puvs);

		// no element in list inserted by this KM
		*lValue = 0;
	}

	// no user value
	else 
		*lValue = 0;

	return kPGPError_NoErr;
}


//----------------------------------------------------|
//  get cert user value list item

PGPError KMGetCertUserVal (PKEYMAN pKM, PGPSigRef Cert, LONG* lValue) {
	USERVALSTRUCT* puvs;
	PGPError err;

	err = PGPGetSigUserVal (Cert, &puvs);
	if (err) return err;

	// there is an existing linked list
	if (puvs) {
		// search for the element inserted by this KM
		do {
			if (puvs->pKM == pKM) {
				*lValue = puvs->lValue;
				return kPGPError_NoErr;
			}
			puvs = puvs->next;
		} while (puvs);

		// no element in list inserted by this KM
		*lValue = 0;
	}

	// no user value
	else 
		*lValue = 0;

	return kPGPError_NoErr;
}


//----------------------------------------------------|
//  set key user value list item

PGPError KMSetKeyUserVal (PKEYMAN pKM, PGPKeyRef Key, LONG lValue) {
	USERVALSTRUCT* puvs;
	USERVALSTRUCT* puvsPrev;
	PGPError err;

	err = PGPGetKeyUserVal (Key, &puvs);
	if (err) return err;

	// there is an existing linked list
	if (puvs) {
		puvsPrev = NULL;
		// search for the element inserted by this KM
		do {
			if (puvs->pKM == pKM) {
				// if value is zero, remove item from list
				if (!lValue) {
					if (!puvsPrev) 
						PGPSetKeyUserVal (Key, puvs->next);
					else
						puvsPrev->next = puvs->next;
					KMFree (puvs);
				}
				// otherwise set the list element to the desired value
				else 
					puvs->lValue = lValue;
				return kPGPError_NoErr;
			}
			puvsPrev = puvs;
			puvs = puvs->next;
		} while (puvs);

		// no element in list inserted by this KM, create and append one
		if (!lValue) return kPGPError_NoErr;
		puvs = KMAlloc (sizeof(USERVALSTRUCT));
		puvsPrev->next = puvs;
	}
	// no user value, create one
	else {
		if (!lValue) return kPGPError_NoErr;
		puvs = KMAlloc (sizeof(USERVALSTRUCT));
		PGPSetKeyUserVal (Key, puvs);
	}

	// set contents of linked list element
	puvs->pKM = pKM;
	puvs->next = NULL;
	puvs->lValue = lValue;

	return kPGPError_NoErr;
}


//----------------------------------------------------|
//  set userid user value list item

PGPError KMSetUserIDUserVal (PKEYMAN pKM, PGPUserIDRef UID, LONG lValue) {
	USERVALSTRUCT* puvs;
	USERVALSTRUCT* puvsPrev;
	PGPError err;

	err = PGPGetUserIDUserVal (UID, &puvs);
	if (err) return err;

	// there is an existing linked list
	if (puvs) {
		puvsPrev = NULL;
		// search for the element inserted by this KM
		do {
			if (puvs->pKM == pKM) {
				// if value is zero, remove item from list
				if (!lValue) {
					if (!puvsPrev) 
						PGPSetUserIDUserVal (UID, puvs->next);
					else
						puvsPrev->next = puvs->next;
					KMFree (puvs);
				}
				// otherwise set the list element to the desired value
				else 
					puvs->lValue = lValue;
				return kPGPError_NoErr;
			}
			puvsPrev = puvs;
			puvs = puvs->next;
		} while (puvs);

		// no element in list inserted by this KM, create and append one
		if (!lValue) return kPGPError_NoErr;
		puvs = KMAlloc (sizeof(USERVALSTRUCT));
		puvsPrev->next = puvs;
	}
	// no user value, create one
	else {
		if (!lValue) return kPGPError_NoErr;
		puvs = KMAlloc (sizeof(USERVALSTRUCT));
		PGPSetUserIDUserVal (UID, puvs);
	}

	// set contents of linked list element
	puvs->pKM = pKM;
	puvs->next = NULL;
	puvs->lValue = lValue;

	return kPGPError_NoErr;
}


//----------------------------------------------------|
//  set cert user value list item

PGPError KMSetCertUserVal (PKEYMAN pKM, PGPSigRef Cert, LONG lValue) {
	USERVALSTRUCT* puvs;
	USERVALSTRUCT* puvsPrev;
	PGPError err;

	err = PGPGetSigUserVal (Cert, &puvs);
	if (err) return err;

	// there is an existing linked list
	if (puvs) {
		puvsPrev = NULL;
		// search for the element inserted by this KM
		do {
			if (puvs->pKM == pKM) {
				// if value is zero, remove item from list
				if (!lValue) {
					if (!puvsPrev) 
						PGPSetSigUserVal (Cert, puvs->next);
					else
						puvsPrev->next = puvs->next;
					KMFree (puvs);
				}
				// otherwise set the list element to the desired value
				else 
					puvs->lValue = lValue;
				return kPGPError_NoErr;
			}
			puvsPrev = puvs;
			puvs = puvs->next;
		} while (puvs);

		// no element in list inserted by this KM, create and append one
		if (!lValue) return kPGPError_NoErr;
		puvs = KMAlloc (sizeof(USERVALSTRUCT));
		puvsPrev->next = puvs;
	}
	// no user value, create one
	else {
		if (!lValue) return kPGPError_NoErr;
		puvs = KMAlloc (sizeof(USERVALSTRUCT));
		PGPSetSigUserVal (Cert, puvs);
	}

	// set contents of linked list element
	puvs->pKM = pKM;
	puvs->next = NULL;
	puvs->lValue = lValue;

	return kPGPError_NoErr;
}


//----------------------------------------------------|
//  Create TreeList Window

HKEYMAN PGPkmExport PGPkmCreateKeyManager (PGPContextRef Context, 
										   HWND hWndParent, INT Id, 
										   INT x, INT y,
										   INT nWidth, INT nHeight) {

	HBITMAP hBmp;      // handle to a bitmap
	PKEYMAN pKM;
	HDC		hDC;
	INT		iNumBits;

	// Ensure that the common control DLL is loaded.
	InitCommonControls ();

	// Ensure that the custom control DLL is loaded.
	InitTreeListControl ();

	pKM = KMAlloc (sizeof (_KEYMAN));
	if (!pKM) return NULL;

	pKM->hWndParent = hWndParent;
	pKM->hWndTree = NULL;
	pKM->hRequestMutex = CreateMutex (NULL, FALSE, NULL);
	pKM->hAccessMutex = CreateMutex (NULL, TRUE, NULL);
	pKM->iId = Id;
	pKM->hIml = NULL;
	pKM->pDropTarget = NULL;			//pointer to DropTarget object
	lstrcpy (pKM->szHelpFile, "");		//string containing name of help file
	lstrcpy (pKM->szPutKeyserver, "");	//name of "send keys" server

	pKM->Context = Context;			//PGP context
	pKM->KeySetDisp = NULL;			//displayed keyset
	pKM->KeySetMain = NULL;			//main keyset
	pKM->bMainKeySet = FALSE;
	pKM->ulOptionFlags = 0;
	pKM->ulDisableActions = 0;
	pKM->ulHideColumns = 0;
	pKM->ulShowColumns = 0;

	pKM->bMultipleSelected = FALSE;
	pKM->uSelectedFlags = 0;
	pKM->iFocusedItemType = 0;
	pKM->iFocusedObjectType = 0;
	pKM->hFocusedItem = NULL;
	pKM->pFocusedObject = NULL;

	pKM->iValidityThreshold = KM_VALIDITY_MARGINAL;

	pKM->iNumberSheets = 0;

	KMGetColumnPreferences (pKM);

	// Create the tree view window.
	pKM->hWndTree = CreateWindowEx (WS_EX_CLIENTEDGE, WC_TREELIST, "",
			WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP | WS_GROUP |
			TLS_HASBUTTONS | TLS_HASLINES | TLS_AUTOSCROLL | TLS_PROMISCUOUS,
			x, y, nWidth, nHeight,
			hWndParent, (HMENU)Id, g_hInst, NULL);

	if (pKM->hWndTree == NULL) return NULL;

	// Initialize the tree view window.
	// First create imagelist and load the appropriate bitmaps 
	// based on current display capabilities.
	
	hDC = GetDC (NULL);		// DC for desktop
	iNumBits = GetDeviceCaps (hDC, BITSPIXEL) * GetDeviceCaps (hDC, PLANES);
	ReleaseDC (NULL, hDC);

	if (iNumBits <= 8) {
		pKM->hIml =	ImageList_Create (16, 16, ILC_COLOR|ILC_MASK, 
										NUM_BITMAPS, 0); 
		hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_IMAGES4BIT));
		ImageList_AddMasked (pKM->hIml, hBmp, RGB(255, 0, 255));
		DeleteObject (hBmp);
	}
	else {
		pKM->hIml =	ImageList_Create (16, 16, ILC_COLOR24|ILC_MASK, 
										NUM_BITMAPS, 0); 
		hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_IMAGES24BIT));
		ImageList_AddMasked (pKM->hIml, hBmp, RGB(255, 0, 255));
		DeleteObject (hBmp);
	}

	// Associate the image list with the tree view control.
	TreeList_SetImageList (pKM->hWndTree, pKM->hIml);

	KMSetFocus (pKM, NULL, FALSE);
	pKM->pDropTarget = KMCreateDropTarget ((VOID*)pKM); 
	CoLockObjectExternal ((IUnknown*)pKM->pDropTarget, TRUE, TRUE);
	RegisterDragDrop (pKM->hWndTree, pKM->pDropTarget);
	KMEnableDropTarget (pKM->pDropTarget, FALSE);

 	return (HKEYMAN)pKM;
}


//----------------------------------------------------|
//  Insert column information into control

BOOL KMAddColumns (PKEYMAN pKM) {
	TL_COLUMN tlc;
	CHAR sz[64];
	INT iField, iCol, ids;

	TreeList_DeleteAllColumns (pKM->hWndTree);

	tlc.mask = TLCF_FMT | TLCF_WIDTH | TLCF_TEXT | 
				TLCF_SUBITEM | TLCF_DATATYPE | TLCF_DATAMAX |
				TLCF_MOUSENOTIFY;
	tlc.pszText = sz;

	tlc.iSubItem = 0;
	tlc.fmt = TLCFMT_LEFT;
	tlc.iDataType = TLC_DATASTRING;
	tlc.cx = pKM->wFieldWidth[0];
	tlc.bMouseNotify = FALSE;
	LoadString (g_hInst, IDS_NAMEFIELD, sz, sizeof(sz));
	TreeList_InsertColumn (pKM->hWndTree, 0, &tlc);

	for (iCol=1; iCol<NUMBERFIELDS; iCol++) {
		iField = pKM->wColumnField[iCol];
		if (iField) {
			switch (iField) {
			case KMI_VALIDITY :
				ids = IDS_VALIDITYFIELD;
				if (pKM->ulOptionFlags & KMF_NOVICEMODE) 
					tlc.fmt = TLCFMT_IMAGE;
				else 
					tlc.fmt = TLCFMT_LINBAR;
				tlc.cchTextMax = 
						KMConvertFromPGPValidity (kPGPValidity_Complete);
				tlc.iDataType = TLC_DATALONG;
				tlc.bMouseNotify = TRUE;
				break;

			case KMI_SIZE :
				ids = IDS_SIZEFIELD;
				tlc.fmt = TLCFMT_LEFT;
				tlc.iDataType = TLC_DATASTRING;
				tlc.bMouseNotify = FALSE;
				break;

			case KMI_DESCRIPTION :
				ids = IDS_DESCRIPTIONFIELD;
				tlc.fmt = TLCFMT_LEFT;
				tlc.iDataType = TLC_DATASTRING;
				tlc.bMouseNotify = FALSE;
				break;

			case KMI_KEYID :
				ids = IDS_KEYIDFIELD;
				tlc.fmt = TLCFMT_LEFT;
				tlc.iDataType = TLC_DATASTRING;
				tlc.bMouseNotify = FALSE;
				break;
				
			case KMI_TRUST :
				ids = IDS_TRUSTFIELD;
				tlc.fmt = TLCFMT_LINBAR;
				tlc.cchTextMax = 
						KMConvertFromPGPTrust (kPGPKeyTrust_Complete);
				tlc.iDataType = TLC_DATALONG;
				tlc.bMouseNotify = FALSE;
				break;

			case KMI_CREATION :
				ids = IDS_CREATIONFIELD;
				tlc.fmt = TLCFMT_LEFT;
				tlc.iDataType = TLC_DATASTRING;
				tlc.bMouseNotify = FALSE;
				break;
				
			case KMI_EXPIRATION :
				ids = IDS_EXPIRATIONFIELD;
				tlc.fmt = TLCFMT_LEFT;
				tlc.iDataType = TLC_DATASTRING;
				tlc.bMouseNotify = FALSE;
				break;
				
			case KMI_ADK :
				ids = IDS_ADKFIELD;
				tlc.fmt = TLCFMT_IMAGE;
				tlc.iDataType = TLC_DATALONG;
				tlc.bMouseNotify = FALSE;
				break;
			}
			LoadString (g_hInst, ids, sz, sizeof(sz));
			tlc.cx = pKM->wFieldWidth[iField];
			TreeList_InsertColumn (pKM->hWndTree, iCol, &tlc);
		}
	}

	return TRUE;
}


//----------------------------------------------------|
//  Set (or add) a tree item to the tree

HTLITEM SetOneItem (PKEYMAN pKM, BOOL bReInsert, HTLITEM hItem, 
					HTLITEM hParent, LPSTR szText, HTLITEM hInsAfter, 
					INT iImage, UINT uState, LPARAM lParam) {
	TL_TREEITEM tlI;
	TL_INSERTSTRUCT tlIns;

	tlI.hItem = hItem;
	tlI.mask = TLIF_TEXT | TLIF_IMAGE | TLIF_STATE | TLIF_PARAM;
	tlI.stateMask = TLIS_BOLD | TLIS_ITALICS;
	tlI.stateMask |= uState;
	tlI.state = uState;
	tlI.pszText = szText;
	tlI.cchTextMax = lstrlen (szText);
	tlI.iImage = iImage;
	tlI.iSelectedImage = iImage;
	tlI.lParam = lParam;

	// Insert the data into the tree.
	if (bReInsert || !hItem) {
		tlIns.item = tlI;
		tlIns.hInsertAfter = hInsAfter;
		tlIns.hParent = hParent;
		return (TreeList_InsertItem (pKM->hWndTree, &tlIns));
	}
	else {
		TreeList_SetItem (pKM->hWndTree, &tlI);
		return hItem;
	}
}


//----------------------------------------------------|
//  Construct string representation of number of key bits

VOID 
KMGetKeyBitsString (PGPKeySetRef KeySet, PGPKeyRef Key, LPSTR sz, UINT u) 
{
	UINT uAlg;
	UINT uKeyBits, uSubKeyBits;
	PGPSubKeyRef SubKey;
	PGPKeyListRef KeyList;
	PGPKeyIterRef KeyIter;
	INT iError;
	CHAR szbuf[32];

	PGPGetKeyNumber (Key, kPGPKeyPropAlgID, &uAlg);
	switch (uAlg) {
	case kPGPPublicKeyAlgorithm_RSA :
		PGPGetKeyNumber (Key, kPGPKeyPropBits, &uKeyBits);
		wsprintf (szbuf, "%i", uKeyBits);
		lstrcpyn (sz, szbuf, u);
		break;

	case kPGPPublicKeyAlgorithm_DSA :
		iError = PGPGetKeyNumber (Key, kPGPKeyPropBits, &uKeyBits);
		PGPOrderKeySet (KeySet, kPGPAnyOrdering, &KeyList);
		PGPNewKeyIter (KeyList, &KeyIter);
		PGPKeyIterSeek (KeyIter, Key);
		PGPKeyIterNextSubKey (KeyIter, &SubKey);
		if (SubKey) {
			PGPGetSubKeyNumber (SubKey, kPGPKeyPropBits, &uSubKeyBits);
			wsprintf (szbuf, "%i/%i", uSubKeyBits, uKeyBits);
		}
		else wsprintf (szbuf, "%i", uKeyBits);
		PGPFreeKeyIter (KeyIter);
		PGPFreeKeyList (KeyList);
		lstrcpyn (sz, szbuf, u);
		break;

	default :
		LoadString (g_hInst, IDS_UNKNOWN, sz, u);
		break;
	}
}


//----------------------------------------------------|
//  Set list data for a key

VOID FillDescription (INT idx, LPSTR sz, INT iLen) {
	INT ids;

	switch (idx) {
	case IDX_RSAPUBKEY :	ids = IDS_RSAPUBKEY;		break;
	case IDX_DSAPUBKEY :	ids = IDS_DSAPUBKEY;		break;
	case IDX_RSASECKEY :	ids = IDS_RSASECKEY;		break;
	case IDX_DSASECKEY :	ids = IDS_DSASECKEY;		break;

	case IDX_RSAPUBDISKEY : ids = IDS_RSAPUBDISKEY;		break;
	case IDX_RSAPUBREVKEY : ids = IDS_RSAPUBREVKEY;		break;
	case IDX_RSAPUBEXPKEY : ids = IDS_RSAPUBEXPKEY;		break;

	case IDX_RSASECDISKEY : ids = IDS_RSASECDISKEY;		break;
	case IDX_RSASECREVKEY : ids = IDS_RSASECREVKEY;		break;
	case IDX_RSASECEXPKEY : ids = IDS_RSASECEXPKEY;		break;

	case IDX_DSAPUBDISKEY : ids = IDS_DSAPUBDISKEY;		break;
	case IDX_DSAPUBREVKEY : ids = IDS_DSAPUBREVKEY;		break;
	case IDX_DSAPUBEXPKEY : ids = IDS_DSAPUBEXPKEY;		break;

	case IDX_DSASECDISKEY : ids = IDS_DSASECDISKEY;		break;
	case IDX_DSASECREVKEY : ids = IDS_DSASECREVKEY;		break;
	case IDX_DSASECEXPKEY : ids = IDS_DSASECEXPKEY;		break;

	case IDX_RSAUSERID :	ids = IDS_RSAUSERID;		break;
	case IDX_DSAUSERID :	ids = IDS_DSAUSERID;		break;

	case IDX_CERT :			ids = IDS_CERT;				break;
	case IDX_REVCERT :		ids = IDS_REVCERT;			break;
	case IDX_BADCERT :		ids = IDS_BADCERT;			break;
	case IDX_EXPORTCERT :	ids = IDS_EXPORTCERT;		break;
	case IDX_METACERT :		ids = IDS_METACERT;			break;
	case IDX_EXPORTMETACERT:ids = IDS_EXPORTMETACERT;	break;

	default :				ids = IDS_UNKNOWN;			break;
	}

	LoadString (g_hInst, ids, sz, iLen);
}


//----------------------------------------------------|
//  Set list data for a key

HTLITEM SetKeyData (PKEYMAN pKM, HTLITEM hItem, PGPKeyRef Key, INT idx) { 
	TL_LISTITEM tlL;
	INT iField, iCol;
	Boolean bAxiomatic;

	CHAR szText [128];
	INT iValue;
	PGPTime time;
	UINT u;

	tlL.pszText = szText;
	tlL.hItem = hItem;
	tlL.stateMask = TLIS_VISIBLE;

	for (iCol=1; iCol<NUMBERFIELDS; iCol++) {
		iField = pKM->wColumnField[iCol];
		if (iField) {
			switch (iField) {
			case KMI_VALIDITY :
				PGPGetPrimaryUserIDValidity (Key, &iValue);
				iValue = KMConvertFromPGPValidity (iValue);
				PGPGetKeyBoolean (Key, kPGPKeyPropIsAxiomatic, &bAxiomatic);
				if (bAxiomatic) iValue = 
						KMConvertFromPGPValidity (kPGPValidity_Complete)+1;
				tlL.state = TLIS_VISIBLE;
				tlL.mask = TLIF_DATAVALUE | TLIF_STATE;
				if (pKM->ulOptionFlags & KMF_NOVICEMODE) {
					if (iValue > KM_VALIDITY_COMPLETE)
						tlL.lDataValue = IDX_AXIOMATIC;
					else if (iValue >= pKM->iValidityThreshold) 
						tlL.lDataValue = IDX_VALID;
					else tlL.lDataValue = IDX_INVALID;
				}
				else tlL.lDataValue = iValue;
				break;

			case KMI_SIZE :
				tlL.state = TLIS_VISIBLE;
				tlL.mask = TLIF_TEXT | TLIF_STATE;
				KMGetKeyBitsString (pKM->KeySetDisp, 
										Key, szText, sizeof(szText));
				break;

			case KMI_DESCRIPTION :
				tlL.state = TLIS_VISIBLE;
				tlL.mask = TLIF_TEXT | TLIF_STATE;
				FillDescription (idx, szText, sizeof(szText));
				break;

			case KMI_KEYID :
				tlL.state = TLIS_VISIBLE;
				tlL.mask = TLIF_TEXT | TLIF_STATE;
				KMGetKeyIDFromKey (Key, szText, sizeof(szText));
				break;
				
			case KMI_TRUST :
				PGPGetKeyNumber (Key, kPGPKeyPropTrust, &iValue);
				iValue = KMConvertFromPGPTrust (iValue);
				PGPGetKeyBoolean (Key, kPGPKeyPropIsAxiomatic, &bAxiomatic);
				if (bAxiomatic) 
					iValue = KMConvertFromPGPTrust (kPGPKeyTrust_Ultimate)+1;		
				tlL.state = TLIS_VISIBLE;
				tlL.mask = TLIF_DATAVALUE | TLIF_STATE;
				tlL.lDataValue = iValue;
				break;

			case KMI_CREATION :
				tlL.state = TLIS_VISIBLE;
				tlL.mask = TLIF_TEXT | TLIF_STATE;
				PGPGetKeyTime (Key, kPGPKeyPropCreation, &time);
				KMConvertTimeToString (time, szText, sizeof(szText));
				break;
				
			case KMI_EXPIRATION :
				tlL.state = TLIS_VISIBLE;
				tlL.mask = TLIF_TEXT | TLIF_STATE;
				PGPGetKeyTime (Key, kPGPKeyPropExpiration, &time);
				if (time != kPGPExpirationTime_Never) 
					KMConvertTimeToString (time, szText, sizeof (szText));
				else 
					LoadString (g_hInst, IDS_NEVER, szText, sizeof (szText));
				break;
				
			case KMI_ADK :
				tlL.state = TLIS_VISIBLE;
				tlL.mask = TLIF_DATAVALUE | TLIF_STATE;
				PGPCountAdditionalRecipientRequests (Key, &u);
				if (u > 0) 
					tlL.lDataValue = IDX_ADK;
				else
					tlL.lDataValue = IDX_NOADK;
				break;
			}
			tlL.iSubItem = iCol;
			hItem = (HTLITEM) TreeList_SetListItem (pKM->hWndTree, 
													&tlL, FALSE);
		}
	}

	return (hItem);
}


//----------------------------------------------------|
//  Set treelist list data for a userID

HTLITEM SetIDData (PKEYMAN pKM, HTLITEM hItem, PGPUserIDRef UserID, INT idx) {
	TL_LISTITEM tlL;
	CHAR szText [128];
	INT iField, iCol;
	INT iValue;

	tlL.pszText = szText;
	tlL.hItem = hItem;
	tlL.stateMask = TLIS_VISIBLE;

	for (iCol=1; iCol<NUMBERFIELDS; iCol++) {
		iField = pKM->wColumnField[iCol];
		if (iField) {
			switch (iField) {
			case KMI_VALIDITY :
				PGPGetUserIDNumber (UserID, kPGPUserIDPropValidity, &iValue);
				iValue = KMConvertFromPGPValidity (iValue);
				tlL.state = TLIS_VISIBLE;
				tlL.mask = TLIF_DATAVALUE | TLIF_STATE;
				if (pKM->ulOptionFlags & KMF_NOVICEMODE) {
					if (iValue >= pKM->iValidityThreshold) 
						tlL.lDataValue = IDX_VALID;
					else tlL.lDataValue = IDX_INVALID;
				}
				else tlL.lDataValue = iValue;
				break;

			case KMI_SIZE :
				tlL.state = 0;
				tlL.mask = TLIF_STATE;
				break;

			case KMI_DESCRIPTION :
				tlL.state = TLIS_VISIBLE;
				tlL.mask = TLIF_TEXT | TLIF_STATE;
				FillDescription (idx, szText, sizeof(szText));
				break;

			case KMI_KEYID :
				tlL.state = 0;
				tlL.mask = TLIF_STATE;
				break;
				
			case KMI_TRUST :
				tlL.state = 0;
				tlL.mask = TLIF_STATE;
				break;

			case KMI_CREATION :
				tlL.state = 0;
				tlL.mask = TLIF_STATE;
				break;
				
			case KMI_EXPIRATION :
				tlL.state = 0;
				tlL.mask = TLIF_STATE;
				break;
				
			case KMI_ADK :
				tlL.state = 0;
				tlL.mask = TLIF_STATE;
				break;
			}
			tlL.iSubItem = iCol;
			hItem = (HTLITEM) TreeList_SetListItem (pKM->hWndTree, 
													&tlL, FALSE);
		}
	}
	return (hItem);
}


//----------------------------------------------------|
//  Set treelist list data for a certification

HTLITEM 
SetCertData (
			 PKEYMAN	pKM, 
			 HTLITEM	hItem, 
			 PGPSigRef	Cert, 
			 INT		idx) 
{
	TL_LISTITEM tlL;
	char szText [128];
	INT iField, iCol;
	PGPTime time;

	tlL.pszText = szText;
	tlL.hItem = hItem;
	tlL.stateMask = TLIS_VISIBLE;

	for (iCol=1; iCol<NUMBERFIELDS; iCol++) {
		iField = pKM->wColumnField[iCol];
		if (iField) {
			switch (iField) {
			case KMI_VALIDITY :
				tlL.state = 0;
				tlL.mask = TLIF_STATE;
				break;

			case KMI_SIZE :
				tlL.state = 0;
				tlL.mask = TLIF_STATE;
				break;

			case KMI_DESCRIPTION :
				tlL.state = TLIS_VISIBLE;
				tlL.mask = TLIF_TEXT | TLIF_STATE;
				FillDescription (idx, szText, sizeof(szText));
				break;

			case KMI_KEYID :
				tlL.state = TLIS_VISIBLE;
				tlL.mask = TLIF_TEXT | TLIF_STATE;
				KMGetKeyIDFromCert (Cert, szText, sizeof(szText));
				break;
				
			case KMI_TRUST :
				tlL.state = 0;
				tlL.mask = TLIF_STATE;
				break;

			case KMI_CREATION :
				tlL.state = TLIS_VISIBLE;
				tlL.mask = TLIF_TEXT | TLIF_STATE;
				PGPGetSigTime (Cert, kPGPSigPropCreation, &time);
				KMConvertTimeToString (time, szText, sizeof(szText));
				break;
				
			case KMI_EXPIRATION :
				tlL.state = 0;
				tlL.mask = TLIF_STATE;
				break;
				
			case KMI_ADK :
				tlL.state = 0;
				tlL.mask = TLIF_STATE;
				break;
			}
			tlL.iSubItem = iCol;
			hItem = (HTLITEM) TreeList_SetListItem (pKM->hWndTree, 
													&tlL, FALSE);
		}
	}
	return (hItem);
}


//----------------------------------------------------|
//  Reload a single key

HTLITEM ReloadKey (PKEYMAN pKM, PGPKeyIterRef KeyIter, PGPKeyRef Key, 
				     BOOL bReInsert, BOOL bForceNewAlloc, BOOL bExpandNew,
					 BOOL bFirstCall, HTLITEM hTPrev) {

	HTLITEM hTKey, hTUID, hTCert;
	TL_TREEITEM tli;
	INT idx;
	CHAR sz[kPGPMaxUserIDSize];
	CHAR sz2[64];
	CHAR szID[32];
	UINT uState;
	BOOL bItalics, bNew;
	PGPUserIDRef UserID;
	PGPSigRef Cert;
	PGPKeyRef CertKey;
	PGPError err;
	static BOOL bNewKeyExpanded;

	uState = 0;
	bNew = FALSE;
	if (bFirstCall) bNewKeyExpanded = FALSE;

	KMGetKeyUserVal (pKM, Key, (long*)&hTKey);
	if (!hTKey) {
//		PGPIncKeyRefCount (Key);
		bNew = TRUE;
	}

	// determine icon and italics
	idx = KMDetermineKeyIcon (pKM, Key, &bItalics);
	if (bItalics) uState |= TLIS_ITALICS;

	// get primary userid name string
	KMGetKeyName (Key, sz, sizeof(sz));

	// insert key item into tree and save pointer to tree item
	if (!hTKey && bExpandNew) uState |= TLIS_SELECTED;
	if (bForceNewAlloc) hTKey = NULL;
	hTKey = SetOneItem (pKM, bReInsert, hTKey, NULL, sz, hTPrev, idx, 
						uState, (LPARAM)Key);
	KMSetKeyUserVal (pKM, Key, (long)hTKey);

	// if a reinsertion, then we're done
	if (bReInsert) return hTKey;

	SetKeyData (pKM, hTKey, Key, idx);

	// iterate through userids
	PGPKeyIterNextUserID (KeyIter, &UserID);
	while (UserID) {
		uState = 0;
		KMGetUserIDUserVal (pKM, UserID, (long*)&hTUID);
		if (!hTUID) bNew = TRUE;

		// get and set treelist tree data for this userid
		KMGetUserIDName (UserID, sz, sizeof(sz));
		if (bForceNewAlloc) hTUID = NULL;
		idx = KMDetermineUserIDIcon (Key);
		hTUID = SetOneItem (pKM, bReInsert, hTUID, hTKey, sz, 
							(HTLITEM)TLI_LAST, idx, uState, (LPARAM)UserID);
		KMSetUserIDUserVal (pKM, UserID, (long)hTUID);

		// get and set treelist list data for this userid
		SetIDData (pKM, hTUID, UserID, idx);
		
		// iterate through certifications
		PGPKeyIterNextUIDSig (KeyIter, &Cert);
		while (Cert) {
			uState = 0;
			KMGetCertUserVal (pKM, Cert, (long*)&hTCert);
			if (!hTCert) bNew = TRUE;

			// get and set treelist tree data for this cert
			err = PGPGetSigCertifierKey (Cert, pKM->KeySetDisp, &CertKey);
			if (err == kPGPError_ItemNotFound) {
				err = kPGPError_NoErr;
				CertKey = NULL;
			}

			if (!CertKey && (pKM->KeySetDisp != pKM->KeySetMain)) {
				err = PGPGetSigCertifierKey (Cert, pKM->KeySetMain, &CertKey);
			}
			if (err == kPGPError_ItemNotFound) {
				err = kPGPError_NoErr;
				CertKey = NULL;
			}

			PGPcomdlgErrorMessage (err);
			bItalics = FALSE;
			if (CertKey) {
				KMGetKeyName (CertKey, sz, sizeof(sz));
			}
			else {
				bItalics = TRUE;
				KMGetKeyIDFromCert (Cert, szID, sizeof(szID));
				LoadString (g_hInst, IDS_UNAVAILABLECERT, 
									sz2, sizeof(sz2));
				wsprintf (sz, sz2, szID);
			}
			idx = KMDetermineCertIcon (Cert, &bItalics);
			if (bItalics) uState |= TLIS_ITALICS;
			if (bForceNewAlloc) hTCert = NULL;
			hTCert = SetOneItem (pKM, bReInsert, hTCert, hTUID, sz, 
								(HTLITEM)TLI_LAST, idx, 
								uState, (LPARAM)Cert);
			KMSetCertUserVal (pKM, Cert, (long)hTCert);

			// get and set treelist list data for this cert
			SetCertData (pKM, hTCert, Cert, idx);

			PGPKeyIterNextUIDSig (KeyIter, &Cert);
		} 
		PGPKeyIterNextUserID (KeyIter, &UserID);
	}

	// select and expand key, if appropriate 
	if (bExpandNew && bNew) {
		tli.hItem = hTKey;
		if (!bNewKeyExpanded) {
			TreeList_Select (pKM->hWndTree, &tli, TRUE);
			TreeList_Expand (pKM->hWndTree, &tli, TLE_EXPANDALL);
			bNewKeyExpanded = TRUE;
		}
		else {
			KMSetFocus (pKM, KMFocusedItem (pKM), TRUE);
			TreeList_Expand (pKM->hWndTree, &tli, TLE_EXPANDALL);
		}
	}

	return hTKey;
}


//----------------------------------------------------|
//  Scan entire keyring loading in any new data

BOOL KMLoadKeyRingIntoTree (PKEYMAN pKM, BOOL bReInsert, 
							BOOL bExpandNew, BOOL bForceRealloc) {

	HCURSOR			hCursorOld;
	PGPKeyListRef	KeyList;
	PGPKeyIterRef	KeyIter;
	PGPKeyRef		Key;
	PGPKeyRef		DefaultKey;
	TL_TREEITEM		tli;
	HTLITEM			hTPrevKey;
	BOOL			bFirst;
	PGPBoolean		bSecret, bRevoked, bExpired, bCanSign;

	hTPrevKey = (HTLITEM)TLI_FIRST;
	bFirst = TRUE;

	if (pKM->KeySetDisp) {
		hCursorOld = SetCursor (LoadCursor (NULL, IDC_WAIT));
		PGPOrderKeySet (pKM->KeySetDisp, pKM->lKeyListSortField, &KeyList);
		PGPNewKeyIter (KeyList, &KeyIter);

		PGPKeyIterNext (KeyIter, &Key);

		while (Key) {
			hTPrevKey = ReloadKey (pKM, KeyIter, Key, bReInsert, 
						bForceRealloc, bExpandNew, bFirst, hTPrevKey);
			PGPKeyIterNext (KeyIter, &Key);
			bFirst = FALSE;
		}
		PGPFreeKeyIter (KeyIter);
		PGPFreeKeyList (KeyList);

		PGPGetDefaultPrivateKey (pKM->KeySetMain, &DefaultKey);
		if (DefaultKey) {
			PGPGetKeyBoolean (DefaultKey, kPGPKeyPropIsSecret, &bSecret);
			PGPGetKeyBoolean (DefaultKey, kPGPKeyPropIsRevoked, &bRevoked);
			PGPGetKeyBoolean (DefaultKey, kPGPKeyPropIsExpired, &bExpired);
			PGPGetKeyBoolean (DefaultKey, kPGPKeyPropCanSign, &bCanSign);

			if (bSecret && bCanSign && !bRevoked && !bExpired) {
				KMGetKeyUserVal (pKM, DefaultKey, (long*)&(tli.hItem));
				if (tli.hItem) {
					tli.state = TLIS_BOLD;
					tli.stateMask = TLIS_BOLD;
					tli.mask = TLIF_STATE;
					TreeList_SetItem (pKM->hWndTree, &tli);
				}
			}
		}

		SetCursor (hCursorOld);
		if (bExpandNew) {
			tli.hItem = KMFocusedItem (pKM);
			if (tli.hItem) TreeList_EnsureVisible (pKM->hWndTree, &tli);
		}
		return TRUE;
	}

}


//----------------------------------------------------|
//  iterate entire keyset, deleting user value linked-list elements

BOOL KMDeleteAllUserValues (PKEYMAN pKM) {

	PGPKeyListRef KeyList;
	PGPKeyIterRef KeyIter;
	PGPKeyRef Key;
	PGPUserIDRef UserID;
	PGPSigRef Cert;

	if (!pKM) return FALSE;
	if (!pKM->KeySetDisp) return FALSE;

	PGPOrderKeySet (pKM->KeySetDisp, kPGPAnyOrdering, &KeyList);
	PGPNewKeyIter (KeyList, &KeyIter);

	PGPKeyIterNext (KeyIter, &Key);

	while (Key) {
		PGPKeyIterNextUserID (KeyIter, &UserID);
		while (UserID) {
			PGPKeyIterNextUIDSig (KeyIter, &Cert);
			while (Cert) {
				KMSetCertUserVal (pKM, Cert, 0);
				PGPKeyIterNextUIDSig (KeyIter, &Cert);
			}
			KMSetUserIDUserVal (pKM, UserID, 0);
			PGPKeyIterNextUserID (KeyIter, &UserID);
		}
		KMSetKeyUserVal (pKM, Key, 0);
		PGPKeyIterNext (KeyIter, &Key);
	}
	PGPFreeKeyIter (KeyIter);
	PGPFreeKeyList (KeyList);

	return TRUE;
}


//----------------------------------------------------|
//  repaint single key 

BOOL KMRepaintKeyInTree (PKEYMAN pKM, PGPKeyRef Key) {
	BOOL bItalics;
	INT idx;
	PGPKeyListRef KeyList;
	PGPKeyIterRef KeyIter;
	PGPUserIDRef UserID;
	PGPSigRef Cert;
	HTLITEM hTKey, hTUID, hTCert;

	KMGetKeyUserVal (pKM, Key, (long*)&hTKey);

	if (hTKey) {
		idx = KMDetermineKeyIcon (pKM, Key, &bItalics);
		SetKeyData (pKM, hTKey, Key, idx);

		PGPOrderKeySet (pKM->KeySetDisp, pKM->lKeyListSortField, &KeyList);
		PGPNewKeyIter (KeyList, &KeyIter);
		PGPKeyIterSeek (KeyIter, Key);


		// iterate through user IDs
		PGPKeyIterNextUserID (KeyIter, &UserID);
		while (UserID) {
			KMGetUserIDUserVal (pKM, UserID, (long*)&hTUID);
			idx = KMDetermineUserIDIcon (Key);
			SetIDData (pKM, hTUID, UserID, idx);
			
			// iterate through certifications
			PGPKeyIterNextUIDSig (KeyIter, &Cert);
			while (Cert) {
				KMGetCertUserVal (pKM, Cert, (long*)&hTCert);
				idx = KMDetermineCertIcon (Cert, &bItalics);
				SetCertData (pKM, hTCert, Cert, idx);
				PGPKeyIterNextUIDSig (KeyIter, &Cert);
			}

			PGPKeyIterNextUserID (KeyIter, &UserID);
		}

		PGPFreeKeyIter (KeyIter);
		PGPFreeKeyList (KeyList);
	}

	return TRUE;
}


//----------------------------------------------------|
//  Insert single key into treelist

BOOL KMUpdateKeyInTree (PKEYMAN pKM, PGPKeyRef Key) {

	PGPKeyListRef	KeyList;
	PGPKeyIterRef	KeyIter;
	PGPKeyRef		PrevKey;
	PGPKeyRef		DefaultKey;
	TL_TREEITEM		tli;
	HTLITEM			hTPrevKey;
	PGPBoolean		bSecret, bRevoked, bExpired, bCanSign;

	if (pKM->KeySetDisp) {	
		if (!PGPcomdlgErrorMessage (KMGetKeyUserVal (pKM, Key, 
					(long*)&(tli.hItem)))) {
			if (tli.hItem) {
				TreeList_DeleteItem (pKM->hWndTree, &tli);
				KMSetKeyUserVal (pKM, Key, 0L);
//				PGPFreeKey (Key);
			}

			PGPOrderKeySet (pKM->KeySetDisp, 
										pKM->lKeyListSortField, &KeyList);
			PGPNewKeyIter (KeyList, &KeyIter);
			PGPKeyIterSeek (KeyIter, Key);
			PGPKeyIterPrev (KeyIter, &PrevKey);
			PGPKeyIterSeek (KeyIter, Key);

			hTPrevKey = (HTLITEM)TLI_FIRST;
			if (PrevKey) {
				if (!PGPcomdlgErrorMessage (KMGetKeyUserVal (pKM, PrevKey, 
						(long*)&(tli.hItem)))) {
					hTPrevKey = tli.hItem;
				}
			}	

			ReloadKey (pKM, KeyIter, Key, FALSE, TRUE, FALSE, 
						TRUE, hTPrevKey);

			PGPFreeKeyIter (KeyIter);
			PGPFreeKeyList (KeyList);

			PGPGetDefaultPrivateKey (pKM->KeySetMain, &DefaultKey);
			if (DefaultKey == Key) {

				PGPGetKeyBoolean (Key, kPGPKeyPropIsSecret, &bSecret);
				PGPGetKeyBoolean (Key, kPGPKeyPropIsRevoked, &bRevoked);
				PGPGetKeyBoolean (Key, kPGPKeyPropIsExpired, &bExpired);
				PGPGetKeyBoolean (Key, kPGPKeyPropCanSign, &bCanSign);

				if (bSecret && bCanSign && !bRevoked && !bExpired) {
					KMGetKeyUserVal (pKM, DefaultKey, (long*)&(tli.hItem));
					if (tli.hItem) {
						tli.state = TLIS_BOLD;
						tli.stateMask = TLIS_BOLD;
						tli.mask = TLIF_STATE;
						TreeList_SetItem (pKM->hWndTree, &tli);
					}
				}
			}

			KMGetKeyUserVal (pKM, Key, (long*)&(tli.hItem));
			TreeList_Select (pKM->hWndTree, &tli, TRUE);

			return TRUE;
		}
	}
	return FALSE;
}


//----------------------------------------------------|
//  Set validity for a treelist item

HTLITEM SetItemValidity (PKEYMAN pKM, HTLITEM hItem, int iValidity) {
	TL_LISTITEM tlL;

	tlL.hItem = hItem;

	tlL.iSubItem = 1;
	tlL.mask = TLIF_DATAVALUE;
	if (pKM->ulOptionFlags & KMF_NOVICEMODE) {
		if (iValidity > KM_VALIDITY_COMPLETE)
			tlL.lDataValue = IDX_AXIOMATIC;
		else if (iValidity >= pKM->iValidityThreshold) 
			tlL.lDataValue = IDX_VALID;
		else tlL.lDataValue = IDX_INVALID;
	}
	else tlL.lDataValue = iValidity;
	hItem = (HTLITEM) TreeList_SetListItem (pKM->hWndTree, &tlL, FALSE);

	return (hItem);
}


//----------------------------------------------------|
//  Scan entire keyring loading in any new data

BOOL KMUpdateAllValidities (PKEYMAN pKM) {

	HCURSOR hCursorOld;
	HTLITEM hTKey, hTUID;
	PGPKeyListRef KeyList;
	PGPKeyIterRef KeyIter;
	PGPKeyRef Key;
	PGPUserIDRef UserID;
	Boolean bAxiomatic;
	UINT u;

	if (pKM->KeySetDisp) {
		hCursorOld = SetCursor (LoadCursor (NULL, IDC_WAIT));
		PGPOrderKeySet (pKM->KeySetDisp, pKM->lKeyListSortField, &KeyList);
		PGPNewKeyIter (KeyList, &KeyIter);

		PGPKeyIterNext (KeyIter, &Key);

		// iterate through keys
		while (Key) {
			KMGetKeyUserVal (pKM, Key, (long*)&hTKey);
			if (hTKey) {
				// if axiomatic set trust and validity to out-of-range values
				//  in order to flag different graphical representation
				PGPGetKeyBoolean (Key, kPGPKeyPropIsAxiomatic, &bAxiomatic);
				if (bAxiomatic) 
					u = KMConvertFromPGPValidity (kPGPValidity_Complete) + 1;
				else {
					if (PGPcomdlgErrorMessage (PGPGetPrimaryUserIDValidity (
						Key, &u))) u = 0;
					else 
						u = KMConvertFromPGPValidity (u);
				}
				SetItemValidity (pKM, hTKey, u);
			}

			// iterate through userids
			PGPKeyIterNextUserID (KeyIter, &UserID);
			while (UserID) {
				KMGetUserIDUserVal (pKM, UserID, (long*)&hTUID);
				if (hTUID) {
					PGPGetUserIDNumber (UserID, kPGPUserIDPropValidity, &u);
					u = KMConvertFromPGPValidity (u);
					SetItemValidity (pKM, hTUID, u);
				}
				PGPKeyIterNextUserID (KeyIter, &UserID);
			}
			PGPKeyIterNext (KeyIter, &Key);
		}
		PGPFreeKeyIter (KeyIter);
		PGPFreeKeyList (KeyList);
		SetCursor (hCursorOld);

		return TRUE;
	}

	// error on open key rings
	else return FALSE;
}


//----------------------------------------------------|
//  Expand a single item
//	routine called as a
//	callback function from the TreeList control to 
//	expand a single item.
//
//	lptli	= pointer to TreeList item to expand

BOOL CALLBACK ExpandSingleItem (TL_TREEITEM* lptli, LPARAM lParam) {
	EXPANDCOLLAPSESTRUCT* pecs = (EXPANDCOLLAPSESTRUCT*)lParam;
	TreeList_Expand (pecs->hWndTree, lptli, TLE_EXPANDALL);
	return TRUE;
}


//----------------------------------------------------|
//  Expand the selected items

BOOL KMExpandSelected (PKEYMAN pKM) {
	TL_TREEITEM tli;
	EXPANDCOLLAPSESTRUCT ecs;

	if (KMMultipleSelected (pKM)) {
		ecs.lpfnCallback = ExpandSingleItem;
		ecs.hWndTree = pKM->hWndTree;
		TreeList_IterateSelected (pKM->hWndTree, &ecs);
		InvalidateRect (pKM->hWndTree, NULL, TRUE);
		return TRUE;
	}
	else {
		tli.hItem = KMFocusedItem (pKM);
		TreeList_Expand (pKM->hWndTree, &tli, TLE_EXPANDALL);
		InvalidateRect (pKM->hWndTree, NULL, TRUE);
		return TRUE;
	}
	return FALSE;
}


//----------------------------------------------------|
//  Collapse a single item
//	routine called as a
//	callback function from the TreeList control to 
//	collapse a single item.
// 
//	lptli	= pointer to TreeList item to collapse

BOOL CALLBACK CollapseSingleItem (TL_TREEITEM* lptli, LPARAM lParam) {
	EXPANDCOLLAPSESTRUCT* pecs = (EXPANDCOLLAPSESTRUCT*)lParam;
	TreeList_Expand (pecs->hWndTree, lptli, TLE_COLLAPSEALL);
	return TRUE;
}


//----------------------------------------------------|
//  Collapse the selected items

BOOL KMCollapseSelected (PKEYMAN pKM) {
	TL_TREEITEM tli;
	EXPANDCOLLAPSESTRUCT ecs;

	if (KMMultipleSelected (pKM)) {
		ecs.lpfnCallback = CollapseSingleItem;
		ecs.hWndTree = pKM->hWndTree;
		TreeList_IterateSelected (pKM->hWndTree, &ecs);
		InvalidateRect (pKM->hWndTree, NULL, TRUE);
		return TRUE;
	}
	else {
		tli.hItem = KMFocusedItem (pKM);
		TreeList_Expand (pKM->hWndTree, &tli, TLE_COLLAPSEALL);
		InvalidateRect (pKM->hWndTree, NULL, TRUE);
		return TRUE;
	}
	return FALSE;
}


//----------------------------------------------------|
//  Cleanup treelist

PGPError PGPkmExport PGPkmDestroyKeyManager (
		HKEYMAN	hKM,
		BOOL	bSaveColumnInfo) 
{

	PKEYMAN pKM = (PKEYMAN)hKM;
	if (!hKM) return kPGPError_BadParams;

	KMDeleteAllKeyProperties (pKM, TRUE);
	KMDeleteAllUserValues (pKM);

	CloseHandle (pKM->hRequestMutex);
	CloseHandle (pKM->hAccessMutex);

	RevokeDragDrop (pKM->hWndTree);
	KMReleaseDropTarget (pKM->pDropTarget);  
	CoLockObjectExternal ((IUnknown*)pKM->pDropTarget, FALSE, TRUE);
	DragAcceptFiles (pKM->hWndTree, FALSE);

	if (bSaveColumnInfo) KMSetColumnPreferences (pKM);

	SendMessage (pKM->hWndTree, WM_CLOSE, 0, 0);
	ImageList_Destroy (pKM->hIml);

	return kPGPError_NoErr;
}


//----------------------------------------------------|
//  Load keyset into treelist

PGPError PGPkmExport PGPkmLoadKeySet (HKEYMAN hKeyMan, 
									  PGPKeySetRef KeySetDisp,
									  PGPKeySetRef KeySetMain) {

	PKEYMAN pKM = (PKEYMAN)hKeyMan;
	if (!pKM) return kPGPError_BadParams;

	if (pKM->KeySetDisp) {
		KMDeleteAllKeyProperties (pKM, TRUE);
		KMDeleteAllUserValues (pKM);
		KMSetFocus (pKM, NULL, FALSE);
		pKM->KeySetDisp = NULL;
		pKM->KeySetMain = NULL;
		TreeList_DeleteTree (pKM->hWndTree, TRUE);
	}

	pKM->KeySetDisp = KeySetDisp;
	KMGetColumnPreferences (pKM);
	KMAddColumns (pKM);

	if (KeySetDisp) {
		if (KeySetMain) {
			pKM->KeySetMain = KeySetMain;
			pKM->bMainKeySet = TRUE;
		}
		else {
			pKM->KeySetMain = KeySetDisp;
			pKM->bMainKeySet = FALSE;
		}
		KMLoadKeyRingIntoTree (pKM, FALSE, FALSE, TRUE);
		KMEnableDropTarget (pKM->pDropTarget, 
							!(pKM->ulOptionFlags & KMF_READONLY) &&
							(pKM->ulOptionFlags & KMF_ENABLEDROPIN));
	}
	else {
		pKM->bMainKeySet = TRUE;
	}

	InvalidateRect (pKM->hWndTree, NULL, TRUE);
	UpdateWindow (pKM->hWndTree);

	return kPGPError_NoErr;
}


//----------------------------------------------------|
//  Load keyset into treelist

PGPError PGPkmExport 
PGPkmReLoadKeySet (HKEYMAN hKeyMan, BOOL bExpandNew) 
{
	PKEYMAN pKM = (PKEYMAN)hKeyMan;
	if (!pKM) return kPGPError_BadParams;
	if (!pKM->KeySetDisp) return kPGPError_BadParams;

	KMLoadKeyRingIntoTree (pKM, FALSE, bExpandNew, FALSE);

	InvalidateRect (pKM->hWndTree, NULL, TRUE);
	UpdateWindow (pKM->hWndTree);

	return kPGPError_NoErr;
}


//----------------------------------------------------|
//  Select specified key

PGPError PGPkmExport 
PGPkmSelectKey (HKEYMAN hKeyMan, PGPKeyRef key, BOOL bDeselect) 
{
	PKEYMAN			pKM = (PKEYMAN)hKeyMan;
	TL_TREEITEM		tli;
	BOOL			bMulti;

	KMGetKeyUserVal (pKM, key, (long*)&(tli.hItem));
	if (tli.hItem) {
		TreeList_Select (pKM->hWndTree, &tli, bDeselect);

		bMulti = !bDeselect;
	}

	return kPGPError_NoErr;
}


