/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	KMTree.c - handle creating and filling TreeList control
	

	$Id: KMTree.c,v 1.31 1998/08/11 14:43:40 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

// project header files
#include "pgpkmx.h"

// constant defitions
#define BITMAP_WIDTH 16
#define BITMAP_HEIGHT 16

typedef struct {
	FARPROC			lpfnCallback;
	HWND			hWndTree;
} EXPANDCOLLAPSESTRUCT;

typedef struct {
	FARPROC			lpfnCallback;
	HWND			hWndTree;
	PGPKeySetRef	keysetSelected;
} SELECTEDSTRUCT;

typedef struct _USERVALSTRUCT {
	struct _USERVALSTRUCT* next;
	PKEYMAN			pKM;
	LONG			lValue;
} USERVALSTRUCT;

// external globals
extern HINSTANCE g_hInst;

//	_______________________________________________
//
//  get key user value list item

PGPError 
KMGetKeyUserVal (PKEYMAN pKM, PGPKeyRef Key, LONG* lValue) 
{
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


//	_______________________________________________
//
//  get userid user value list item

PGPError 
KMGetUserIDUserVal (PKEYMAN pKM, PGPUserIDRef UID, LONG* lValue) 
{
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


//	_______________________________________________
//
//  get cert user value list item

PGPError 
KMGetCertUserVal (PKEYMAN pKM, PGPSigRef Cert, LONG* lValue) 
{
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


//	_______________________________________________
//
//  set key user value list item

PGPError 
KMSetKeyUserVal (PKEYMAN pKM, PGPKeyRef Key, LONG lValue) 
{
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


//	_______________________________________________
//
//  set userid user value list item

PGPError 
KMSetUserIDUserVal (PKEYMAN pKM, PGPUserIDRef UID, LONG lValue) 
{
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


//	_______________________________________________
//
//  set cert user value list item

PGPError 
KMSetCertUserVal (PKEYMAN pKM, PGPSigRef Cert, LONG lValue) 
{
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


//	_______________________________________________
//
//  Create TreeList Window

HKEYMAN PGPkmExport 
PGPkmCreateKeyManager (
		PGPContextRef		Context, 
		PGPtlsContextRef	tlsContext,
		HWND				hWndParent,
		INT					iID, 
		HWNDLISTPROC		lpfnHwndListFunc,
		INT					x, 
		INT					y,
		INT					nWidth, 
		INT					nHeight) 
{

	HBITMAP					hBmp;      // handle to a bitmap
	PKEYMAN					pKM;
	HDC						hDC;
	INT						iNumBits;

	pKM = KMAlloc (sizeof (KEYMAN));
	if (!pKM) return NULL;
	memset (pKM, 0x00, sizeof (KEYMAN));

	pKM->hWndParent = hWndParent;
	pKM->hWndTree = NULL;
	pKM->lpfnHwndListFunc = lpfnHwndListFunc;

	pKM->iID = iID;
	pKM->hRequestMutex = CreateMutex (NULL, FALSE, NULL);
	pKM->hAccessMutex = CreateMutex (NULL, TRUE, NULL);
	pKM->hIml = NULL;
	pKM->pDropTarget = NULL;			//pointer to DropTarget object
	lstrcpy (pKM->szHelpFile, "");		//string containing name of help file
	ZeroMemory (&pKM->keyserver, sizeof(PGPKeyServerEntry));	

	pKM->Context = Context;			//PGP context
	pKM->tlsContext = tlsContext;	//TLS context
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
	pKM->pSplitKeyDialogList = NULL;

	KMGetColumnPreferences (pKM);

	// Create the tree view window.
	pKM->hWndTree = CreateWindowEx (WS_EX_CLIENTEDGE, WC_TREELIST, "",
			WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP | WS_GROUP |
			TLS_HASBUTTONS | TLS_HASLINES | TLS_AUTOSCROLL | 
			TLS_PROMISCUOUS | TLS_DRAGABLEHEADERS,
			x, y, nWidth, nHeight,
			hWndParent, (HMENU)iID, g_hInst, NULL);

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
	pKM->pDropTarget = KMCreateDropTarget (pKM->hWndTree, (VOID*)pKM, FALSE); 
	CoLockObjectExternal ((IUnknown*)pKM->pDropTarget, TRUE, TRUE);
	RegisterDragDrop (pKM->hWndTree, pKM->pDropTarget);
	KMEnableDropTarget (pKM->pDropTarget, FALSE);

 	return (HKEYMAN)pKM;
}


//	_______________________________________________
//
//  Insert column information into control

BOOL 
KMAddColumns (PKEYMAN pKM) 
{
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


//	_______________________________________________
//
//  Set (or add) a tree item to the tree

static HTLITEM 
sSetOneItem (
		PKEYMAN		pKM, 
		BOOL		bReInsertExisting, 
		HTLITEM		hItem, 
		HTLITEM		hParent, 
		LPSTR		szText, 
		HTLITEM		hInsAfter, 
		INT			iImage, 
		UINT		uState, 
		LPARAM		lParam) 
{
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
	if (bReInsertExisting || !hItem) {
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


//	_______________________________________________
//
//  Construct string representation of number of key bits

VOID 
KMGetKeyBitsString (
		PGPKeySetRef	KeySet, 
		PGPKeyRef		Key, 
		LPSTR			sz, 
		UINT			u) 
{
	UINT uAlg;
	UINT uKeyBits, uSubKeyBits;
	PGPSubKeyRef SubKey;
	PGPKeyListRef KeyList;
	PGPKeyIterRef KeyIter;
	PGPError err;
	CHAR szbuf[32];

	PGPGetKeyNumber (Key, kPGPKeyPropAlgID, &uAlg);
	switch (uAlg) {
	case kPGPPublicKeyAlgorithm_RSA :
		PGPGetKeyNumber (Key, kPGPKeyPropBits, &uKeyBits);
		wsprintf (szbuf, "%i", uKeyBits);
		lstrcpyn (sz, szbuf, u);
		break;

	case kPGPPublicKeyAlgorithm_DSA :
		// key key bits
		err = PGPGetKeyNumber (Key, kPGPKeyPropBits, &uKeyBits);
		wsprintf (szbuf, "%i", uKeyBits);

		// now try to get subkey bits
		PGPOrderKeySet (KeySet, kPGPAnyOrdering, &KeyList);
		PGPNewKeyIter (KeyList, &KeyIter);
		PGPKeyIterSeek (KeyIter, Key);
		PGPKeyIterNextSubKey (KeyIter, &SubKey);
		if (SubKey) {
			PGPGetSubKeyNumber (SubKey, kPGPKeyPropBits, &uSubKeyBits);
			wsprintf (szbuf, "%i/%i", uSubKeyBits, uKeyBits);
		}
		PGPFreeKeyIter (KeyIter);
		PGPFreeKeyList (KeyList);

		lstrcpyn (sz, szbuf, u);
		break;

	default :
		LoadString (g_hInst, IDS_UNKNOWN, sz, u);
		break;
	}
}


//	_______________________________________________
//
//  Set list data for a key

static VOID 
sFillDescription (INT idx, INT iVal, LPSTR sz, INT iLen) 
{

	INT		ids				= 0;
	BOOL	bSig			= FALSE;
	CHAR	szTemp[128];

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
	case IDX_RSASECSHRKEY : ids = IDS_RSASECSHRKEY;		break;

	case IDX_DSAPUBDISKEY : ids = IDS_DSAPUBDISKEY;		break;
	case IDX_DSAPUBREVKEY : ids = IDS_DSAPUBREVKEY;		break;
	case IDX_DSAPUBEXPKEY : ids = IDS_DSAPUBEXPKEY;		break;

	case IDX_DSASECDISKEY : ids = IDS_DSASECDISKEY;		break;
	case IDX_DSASECREVKEY : ids = IDS_DSASECREVKEY;		break;
	case IDX_DSASECEXPKEY : ids = IDS_DSASECEXPKEY;		break;
	case IDX_DSASECSHRKEY : ids = IDS_DSASECSHRKEY;		break;

	case IDX_RSAUSERID :	ids = IDS_RSAUSERID;		break;
	case IDX_DSAUSERID :	ids = IDS_DSAUSERID;		break;
	case IDX_PHOTOUSERID :	ids = IDS_PHOTOUSERID;		break;
	case IDX_INVALIDUSERID:	ids = IDS_UNKNOWNUSERID;	break;

	case IDX_CERT :		
		ids = IDS_CERT;	
		bSig = TRUE;
		break;

	case IDX_REVCERT :		
		ids = IDS_REVCERT;			
		bSig = TRUE;
		break;

	case IDX_EXPCERT :		
		ids = IDS_EXPCERT;			
		bSig = TRUE;
		break;

	case IDX_BADCERT :		
		ids = IDS_BADCERT;			
		bSig = TRUE;
		break;

	case IDX_EXPORTCERT :	
		ids = IDS_EXPORTCERT;		
		bSig = TRUE;
		break;

	case IDX_TRUSTEDCERT :		
		ids = IDS_EXPORTMETACERT;	
		bSig = TRUE;
		break;

	case IDX_METACERT:
		ids = IDS_METACERT;			
		bSig = TRUE;
		break;

	default :				
		ids = IDS_UNKNOWN;			
		break;
	}

	sz[0] = '\0';

	if (bSig) {
		switch (iVal) {
		case kPGPPublicKeyAlgorithm_RSA :
			LoadString (g_hInst, IDS_RSA, sz, iLen);
			break;

		case kPGPPublicKeyAlgorithm_DSA :
			LoadString (g_hInst, IDS_DSS, sz, iLen);
			break;

		default :
			LoadString (g_hInst, IDS_UNKNOWN, sz, iLen);
			break;
		}
		lstrcat (sz, " ");
	}

	LoadString (g_hInst, ids, szTemp, sizeof(szTemp));
	lstrcat (sz, szTemp);
}


//	_______________________________________________
//
//  Set list data for a key

static HTLITEM 
sSetKeyData (
		PKEYMAN		pKM, 
		HTLITEM		hItem, 
		PGPKeyRef	Key, 
		INT			idx) 
{ 
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
				sFillDescription (idx, 0, szText, sizeof(szText));
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


//	_______________________________________________
//
//  Set treelist list data for a userID

static HTLITEM 
sSetIDData (
		PKEYMAN			pKM, 
		HTLITEM			hItem, 
		PGPUserIDRef	UserID, 
		INT				idx) 
{
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
				sFillDescription (idx, 0, szText, sizeof(szText));
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


//	_______________________________________________
//
//  Set treelist list data for a certification

static HTLITEM 
sSetCertData (
			 PKEYMAN	pKM, 
			 HTLITEM	hItem, 
			 PGPSigRef	Cert, 
			 INT		idx) 
{
	TL_LISTITEM tlL;
	CHAR		szText [128];
	INT			iField, iCol;
	PGPTime		time;
	INT			iAlg;

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
				PGPGetSigNumber (Cert, kPGPSigPropAlgID, &iAlg);
				sFillDescription (idx, iAlg, szText, sizeof(szText));
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
				tlL.state = TLIS_VISIBLE;
				tlL.mask = TLIF_TEXT | TLIF_STATE;
				PGPGetSigTime (Cert, kPGPSigPropExpiration, &time);
				if (time != kPGPExpirationTime_Never) 
					KMConvertTimeToString (time, szText, sizeof (szText));
				else 
					LoadString (g_hInst, IDS_NEVER, szText, sizeof (szText));
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


//	_______________________________________________
//
//  Reload a single key

static HTLITEM 
sReloadKey (
		PKEYMAN			pKM, 
		PGPKeyIterRef	KeyIter, 
		PGPKeyRef		Key, 
		BOOL			bReInsertExisting, 
		BOOL			bForceNewAlloc, 
		BOOL			bExpandNew, 
		BOOL			bFirstCall, 
		HTLITEM			hTPrev) 
{
	HTLITEM hTKey, hTUID, hTCert;
	TL_TREEITEM tli;
	INT idx;
	CHAR sz[kPGPMaxUserIDSize +1];
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
	hTKey = sSetOneItem (pKM, bReInsertExisting, hTKey, NULL, sz, hTPrev, 
						idx, uState, (LPARAM)Key);
	KMSetKeyUserVal (pKM, Key, (long)hTKey);

	// if a reinsertion, then we're done
	if (bReInsertExisting) 
		return hTKey;

	sSetKeyData (pKM, hTKey, Key, idx);

	// iterate through userids
	PGPKeyIterNextUserID (KeyIter, &UserID);
	while (UserID) {
		uState = 0;
		KMGetUserIDUserVal (pKM, UserID, (long*)&hTUID);
		if (!hTUID) bNew = TRUE;

		// get and set treelist tree data for this userid
		KMGetUserIDName (UserID, sz, sizeof(sz));
		if (bForceNewAlloc) hTUID = NULL;
		idx = KMDetermineUserIDIcon (Key, UserID, &bItalics);
		if (bItalics) uState |= TLIS_ITALICS;

		hTUID = sSetOneItem (pKM, FALSE, hTUID, hTKey, sz, 
							(HTLITEM)TLI_LAST, idx, uState, (LPARAM)UserID);
		KMSetUserIDUserVal (pKM, UserID, (long)hTUID);

		// get and set treelist list data for this userid
		sSetIDData (pKM, hTUID, UserID, idx);
		
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

			PGPclErrorBox (NULL, err);
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
			hTCert = sSetOneItem (pKM, FALSE, hTCert, hTUID, sz, 
								(HTLITEM)TLI_LAST, idx, 
								uState, (LPARAM)Cert);
			KMSetCertUserVal (pKM, Cert, (long)hTCert);

			// get and set treelist list data for this cert
			sSetCertData (pKM, hTCert, Cert, idx);

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


//	_______________________________________________
//
//  Scan entire keyring loading in any new data
//
//	bReInsertExisting	- is only used when reordering keys in window.
//						  This is set TRUE if the caller has already
//						  deleted the treelist but not deallocated the items.
//	bExpandNew			- causes new keys to be expanded
//	bForceRealloc		- forces reallocation of treelist items.  Used after
//						  the caller has deleted the treelist and deallocated
//						  all the items. 

BOOL 
KMLoadKeyRingIntoTree (
		PKEYMAN	pKM, 
		BOOL	bReInsertExisting, 
		BOOL	bExpandNew, 
		BOOL	bForceRealloc) 
{

	HCURSOR			hCursorOld;
	PGPKeyListRef	KeyList;
	PGPKeyIterRef	KeyIter;
	PGPKeyRef		Key;
	PGPKeyRef		keyDef;
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
			hTPrevKey = sReloadKey (pKM, KeyIter, Key, bReInsertExisting, 
						bForceRealloc, bExpandNew, bFirst, hTPrevKey);
			PGPKeyIterNext (KeyIter, &Key);
			bFirst = FALSE;
		}
		PGPFreeKeyIter (KeyIter);
		PGPFreeKeyList (KeyList);

		if (pKM->bMainKeySet) {
			PGPGetDefaultPrivateKey (pKM->KeySetMain, &keyDef);
			if (keyDef) {
				PGPGetKeyBoolean (keyDef, kPGPKeyPropIsSecret, &bSecret);
				PGPGetKeyBoolean (keyDef, kPGPKeyPropIsRevoked, &bRevoked);
				PGPGetKeyBoolean (keyDef, kPGPKeyPropIsExpired, &bExpired);
				PGPGetKeyBoolean (keyDef, kPGPKeyPropCanSign, &bCanSign);

				if (bSecret && bCanSign && !bRevoked && !bExpired) {
					KMGetKeyUserVal (pKM, keyDef, (long*)&(tli.hItem));
					if (tli.hItem) {
						tli.state = TLIS_BOLD;
						tli.stateMask = TLIS_BOLD;
						tli.mask = TLIF_STATE;
						TreeList_SetItem (pKM->hWndTree, &tli);
					}
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


//	_______________________________________________
//
//  iterate entire keyset, deleting user value linked-list elements

BOOL 
KMDeleteAllUserValues (PKEYMAN pKM) 
{

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


//	_______________________________________________
//
//  Insert single key into treelist

BOOL 
KMUpdateKeyInTree (
		PKEYMAN		pKM, 
		PGPKeyRef	Key, 
		BOOL		bForceNew) 
{

	PGPKeyListRef	KeyList;
	PGPKeyIterRef	KeyIter;
	PGPKeyRef		PrevKey;
	PGPKeyRef		DefaultKey;
	TL_TREEITEM		tli;
	HTLITEM			hTPrevKey;
	HTLITEM			hTFocused;
	PGPBoolean		bSecret, bRevoked, bExpired, bCanSign;

	if (pKM->KeySetDisp) {	
		hTFocused = KMFocusedItem (pKM);
		if (IsntPGPError (PGPclErrorBox (NULL, KMGetKeyUserVal (pKM, Key, 
					(long*)&(tli.hItem))))) {
			if (bForceNew && tli.hItem) {
				TreeList_DeleteItem (pKM->hWndTree, &tli);
				KMSetKeyUserVal (pKM, Key, 0L);
			}

			PGPOrderKeySet (pKM->KeySetDisp, 
										pKM->lKeyListSortField, &KeyList);
			PGPNewKeyIter (KeyList, &KeyIter);
			PGPKeyIterSeek (KeyIter, Key);
			PGPKeyIterPrev (KeyIter, &PrevKey);
			PGPKeyIterSeek (KeyIter, Key);

			hTPrevKey = (HTLITEM)TLI_FIRST;
			if (PrevKey) {
				if (IsntPGPError (PGPclErrorBox (NULL, KMGetKeyUserVal (pKM, 
						PrevKey, (long*)&(tli.hItem))))) {
					hTPrevKey = tli.hItem;
				}
			}	

			sReloadKey (pKM, KeyIter, Key, FALSE, bForceNew, FALSE, 
						TRUE, hTPrevKey);

			PGPFreeKeyIter (KeyIter);
			PGPFreeKeyList (KeyList);

			// only set default key if this is the main keyset
			if (pKM->bMainKeySet) {
				PGPGetDefaultPrivateKey (pKM->KeySetMain, &DefaultKey);
				if (DefaultKey == Key) {

					PGPGetKeyBoolean (Key, kPGPKeyPropIsSecret, &bSecret);
					PGPGetKeyBoolean (Key, kPGPKeyPropIsRevoked, &bRevoked);
					PGPGetKeyBoolean (Key, kPGPKeyPropIsExpired, &bExpired);
					PGPGetKeyBoolean (Key, kPGPKeyPropCanSign, &bCanSign);

					if (bSecret && bCanSign && !bRevoked && !bExpired) {
						KMGetKeyUserVal (pKM, 
									DefaultKey, (long*)&(tli.hItem));
						if (tli.hItem) {
							tli.state = TLIS_BOLD;
							tli.stateMask = TLIS_BOLD;
							tli.mask = TLIF_STATE;
							TreeList_SetItem (pKM->hWndTree, &tli);
						}
					}
				}
			}

			// set selection appropriately
			if (bForceNew) 
				KMGetKeyUserVal (pKM, Key, (long*)&(tli.hItem));
			else
				tli.hItem = hTFocused;
			TreeList_Select (pKM->hWndTree, &tli, TRUE);

			return TRUE;
		}
	}
	return FALSE;
}


//	_______________________________________________
//
//  Set validity for a treelist item

static HTLITEM 
sSetItemValidity (
		PKEYMAN	pKM, 
		HTLITEM	hItem, 
		INT		iValidity) 
{
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


//	_______________________________________________
//
//  update validity values for all keys/userids

BOOL 
KMUpdateAllValidities (PKEYMAN pKM) 
{

	HCURSOR			hCursorOld;
	HTLITEM			hTKey, hTUID;
	PGPKeyListRef	keylist;
	PGPKeyIterRef	keyiter;
	PGPKeyRef		key;
	PGPUserIDRef	userid;
	Boolean			bAxiomatic;
	UINT			u;

	if (pKM->KeySetDisp) {
		hCursorOld = SetCursor (LoadCursor (NULL, IDC_WAIT));
		PGPOrderKeySet (pKM->KeySetDisp, pKM->lKeyListSortField, &keylist);
		PGPNewKeyIter (keylist, &keyiter);

		PGPKeyIterNext (keyiter, &key);

		// iterate through keys
		while (key) {
			KMGetKeyUserVal (pKM, key, (long*)&hTKey);
			if (hTKey) {
				// if axiomatic set trust and validity to out-of-range values
				//  in order to flag different graphical representation
				PGPGetKeyBoolean (key, kPGPKeyPropIsAxiomatic, &bAxiomatic);
				if (bAxiomatic) 
					u = KMConvertFromPGPValidity (kPGPValidity_Complete) + 1;
				else {
					if (IsPGPError (PGPclErrorBox (NULL,
							PGPGetPrimaryUserIDValidity (key, &u)))) u = 0;
					else 
						u = KMConvertFromPGPValidity (u);
				}
				sSetItemValidity (pKM, hTKey, u);
			}

			// iterate through userids
			PGPKeyIterNextUserID (keyiter, &userid);
			while (userid) {
				KMGetUserIDUserVal (pKM, userid, (long*)&hTUID);
				if (hTUID) {
					PGPGetUserIDNumber (userid, kPGPUserIDPropValidity, &u);
					u = KMConvertFromPGPValidity (u);
					sSetItemValidity (pKM, hTUID, u);
				}

				PGPKeyIterNextUserID (keyiter, &userid);
			}
			PGPKeyIterNext (keyiter, &key);
		}
		PGPFreeKeyIter (keyiter);
		PGPFreeKeyList (keylist);
		SetCursor (hCursorOld);

		return TRUE;
	}

	// error on open key rings
	else return FALSE;
}


//	_______________________________________________
//
//  Expand a single item
//	routine called as a
//	callback function from the TreeList control to 
//	expand a single item.
//
//	lptli	= pointer to TreeList item to expand

static BOOL CALLBACK 
sExpandSingleItem (TL_TREEITEM* lptli, LPARAM lParam) 
{
	EXPANDCOLLAPSESTRUCT* pecs = (EXPANDCOLLAPSESTRUCT*)lParam;
	TreeList_Expand (pecs->hWndTree, lptli, TLE_EXPANDALL);
	return TRUE;
}


//	_______________________________________________
//
//  Expand the selected items

BOOL KMExpandSelected (PKEYMAN pKM) {
	EXPANDCOLLAPSESTRUCT ecs;

	ecs.lpfnCallback = sExpandSingleItem;
	ecs.hWndTree = pKM->hWndTree;
	TreeList_IterateSelected (pKM->hWndTree, &ecs);
	InvalidateRect (pKM->hWndTree, NULL, TRUE);

	return TRUE;
}


//	_______________________________________________
//
//  Collapse a single item
//	routine called as a
//	callback function from the TreeList control to 
//	collapse a single item.
// 
//	lptli	= pointer to TreeList item to collapse

static BOOL CALLBACK 
sCollapseSingleItem (TL_TREEITEM* lptli, LPARAM lParam) 
{
	EXPANDCOLLAPSESTRUCT* pecs = (EXPANDCOLLAPSESTRUCT*)lParam;
	TreeList_Expand (pecs->hWndTree, lptli, TLE_COLLAPSEALL);
	return TRUE;
}


//	_______________________________________________
//
//  Collapse the selected items

BOOL KMCollapseSelected (PKEYMAN pKM) {
	EXPANDCOLLAPSESTRUCT ecs;

	ecs.lpfnCallback = sCollapseSingleItem;
	ecs.hWndTree = pKM->hWndTree;
	TreeList_IterateSelected (pKM->hWndTree, &ecs);
	InvalidateRect (pKM->hWndTree, NULL, TRUE);

	return TRUE;
}


//	_______________________________________________
//
//  Cleanup treelist

PGPError PGPkmExport 
PGPkmDestroyKeyManager (
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


//	_______________________________________________
//
//  Load keyset into treelist

PGPError PGPkmExport 
PGPkmLoadKeySet (
		HKEYMAN			hKeyMan, 
		PGPKeySetRef	KeySetDisp,
		PGPKeySetRef	KeySetMain) 
{

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


//	_______________________________________________
//
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


//	_______________________________________________
//
//  Select specified key

VOID 
KMSelectKey (PKEYMAN pKM, PGPKeyRef key, BOOL bDeselect) 
{
	TL_TREEITEM		tli;

	KMGetKeyUserVal (pKM, key, (long*)&(tli.hItem));
	if (tli.hItem) {
		TreeList_Select (pKM->hWndTree, &tli, bDeselect);
	}
}

//	_______________________________________________
//
//  Select specified key

PGPError PGPkmExport 
PGPkmSelectKey (HKEYMAN hKeyMan, PGPKeyRef key, BOOL bDeselect) 
{
	PKEYMAN			pKM = (PKEYMAN)hKeyMan;

	KMSelectKey (pKM, key, bDeselect);

	return kPGPError_NoErr;
}

//	_______________________________________________
//
//  Get a keyset of all selected keys

PGPError PGPkmExport 
PGPkmGetSelectedKeys (HKEYMAN hKeyMan, PGPKeySetRef* pkeysetSelected) 
{
	PKEYMAN			pKM = (PKEYMAN)hKeyMan;

	return (KMGetSelectedKeys (pKM, pkeysetSelected, NULL));
}



