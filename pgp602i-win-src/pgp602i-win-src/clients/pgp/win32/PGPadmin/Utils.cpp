/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: Utils.cpp,v 1.31.8.1 1998/11/12 03:13:14 heller Exp $
____________________________________________________________________________*/

#include <windows.h>
#include "PGPadmin.h"
#include <commctrl.h>
#include "resource.h"
#include "pgpConfig.h"
#include "pgpErrors.h"
#include "pgpEncode.h"
#include "pgpPubTypes.h"
#include "pgpUtilities.h"
#include "pgpSDKPrefs.h"
#include "pgpKeys.h"
#include "pgpMem.h"
#include "PGPcl.h"
#include "pgpImage.h"
#include "pgpBuildFlags.h"
#include "pgpAdminPrefs.h"
#include "pgpClientPrefs.h"
#include "Utils.h"


void ShowKeyring(HWND hWnd, 
				 int nListCtrl, 
				 PGPContextRef pgpContext,
				 PGPKeySetRef onlyThisKeySet,
				 PGPBoolean bShowRSA, 
				 PGPBoolean bShowDH,
				 PGPBoolean bMustEncrypt,
				 PGPBoolean bMustSign)
{
    PGPKeyRef       newKey		= NULL;
	PGPKeyID		newKeyID;
	PGPSubKeyRef	newSubKey	= NULL;
    PGPError        err;
	PGPKeySetRef	userKeySet	= NULL;
    PGPKeyListRef	klist;
    PGPKeyIterRef	kiter;

	unsigned char	szUserID[kPGPMaxUserIDSize+1];
	char			szTempKeyID[kPGPMaxKeyIDStringSize+1];
	char			szSize[64];
	char			szCreation[64];
	char			szTitle[255];
	UINT			nLength;
	HWND			hList;
	HDC				hDC;
	int				iNumBits;
	LV_ITEM			lviKey;
	LV_COLUMN		lvcKey;
	UINT			nItem;
	HIMAGELIST		hImages;
	HBITMAP			hBmp;
	PGPInt32		nSignSize;
	PGPInt32		nEncryptSize;
	PGPTime			tCreation;
	PGPUInt16		year, month, day;
	PGPBoolean		bIsRevoked;
	PGPBoolean		bIsDisabled;
	PGPBoolean		bIsExpired;
	PGPBoolean		bIsEncryptKey;
	PGPBoolean		bIsSigningKey;

	PGPPublicKeyAlgorithm lAlg;

    if(IsPGPError(err = PGPOpenDefaultKeyRings(pgpContext, 
							(PGPKeyRingOpenFlags) 0, &userKeySet)))
	{
		MessageBox(NULL, "PGPOpenDefaultKeyRings error", "debug", MB_OK);
		return;
	}

	hList = GetDlgItem(hWnd, nListCtrl);
	ListView_DeleteAllItems(hList);
	ListView_DeleteColumn(hList, 2);
	ListView_DeleteColumn(hList, 1);
	ListView_DeleteColumn(hList, 0);
	
	hDC = GetDC (NULL);		// DC for desktop
	iNumBits = GetDeviceCaps (hDC, BITSPIXEL) * GetDeviceCaps (hDC, PLANES);
	ReleaseDC (NULL, hDC);

	if (iNumBits <= 8) 
	{
		hImages = ImageList_Create (16, 16, ILC_COLOR | ILC_MASK, 
					NUM_BITMAPS, 0); 
		hBmp = LoadBitmap (g_hInstance, MAKEINTRESOURCE (IDB_IMAGES4BIT));
		ImageList_AddMasked (hImages, hBmp, RGB(255, 0, 255));
		DeleteObject (hBmp);
	}
	else 
	{
		hImages = ImageList_Create (16, 16, ILC_COLOR24 | ILC_MASK, 
					NUM_BITMAPS, 0); 
		hBmp = LoadBitmap (g_hInstance, MAKEINTRESOURCE (IDB_IMAGES24BIT));
		ImageList_AddMasked (hImages, hBmp, RGB(255, 0, 255));
		DeleteObject (hBmp);
	}

	ListView_SetImageList(hList, hImages, LVSIL_SMALL);

	// Setup the list control columns
	LoadString(g_hInstance, IDS_USERID, szTitle, 254); 
	lvcKey.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lvcKey.fmt = LVCFMT_LEFT;
	lvcKey.cx = 175;
	lvcKey.pszText = szTitle;
	lvcKey.iSubItem = -1;
	ListView_InsertColumn(hList, 0, &lvcKey);
	
	LoadString(g_hInstance, IDS_CREATION, szTitle, 254); 
	lvcKey.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lvcKey.fmt = LVCFMT_CENTER;
	lvcKey.cx = 75;
	lvcKey.pszText = szTitle;
	lvcKey.iSubItem = 1;
	ListView_InsertColumn(hList, 1, &lvcKey);
	
	LoadString(g_hInstance, IDS_KEYSIZE, szTitle, 254); 
	lvcKey.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lvcKey.fmt = LVCFMT_CENTER;
	lvcKey.cx = 75;
	lvcKey.pszText = szTitle;
	lvcKey.iSubItem = 2;
	ListView_InsertColumn(hList, 2, &lvcKey);

    /* Initialize keyrings */

    PGPOrderKeySet(userKeySet, kPGPUserIDOrdering, &klist);
    PGPNewKeyIter(klist, &kiter);
	PGPKeyIterNext(kiter, &newKey);

	nItem = 0;
	while (newKey)
	{
		PGPGetKeyBoolean(newKey, kPGPKeyPropIsRevoked, &bIsRevoked);
		PGPGetKeyBoolean(newKey, kPGPKeyPropIsDisabled, &bIsDisabled);
		PGPGetKeyBoolean(newKey, kPGPKeyPropIsExpired, &bIsExpired);
		PGPGetKeyBoolean(newKey, kPGPKeyPropIsEncryptionKey, &bIsEncryptKey);
		PGPGetKeyBoolean(newKey, kPGPKeyPropIsSigningKey, &bIsSigningKey);

		if ((bIsRevoked || bIsDisabled || bIsExpired) ||
			(!bIsEncryptKey && bMustEncrypt) ||
			(!bIsSigningKey && bMustSign))
		{
			PGPKeyIterNext(kiter, &newKey);
			continue;
		}

		PGPGetKeyNumber(newKey, kPGPKeyPropAlgID, (PGPInt32 *) &lAlg);
		if ((bShowRSA && (lAlg == kPGPPublicKeyAlgorithm_RSA)) ||
			(bShowDH && (lAlg == kPGPPublicKeyAlgorithm_DSA)))
		{
			if (IsPGPError(PGPGetPrimaryUserIDNameBuffer(newKey,
					kPGPMaxUserIDSize, (char*)szUserID,  &nLength)))
			{
				PGPKeyIterNext(kiter, &newKey);
				continue;
			}
			szUserID[nLength] = 0;

			if (IsPGPError(PGPGetKeyNumber(newKey, kPGPKeyPropBits, 
							&nSignSize)))
			{
				PGPKeyIterNext(kiter, &newKey);
				continue;
			}

			if (lAlg == kPGPPublicKeyAlgorithm_DSA)
			{
				PGPKeyIterNextSubKey(kiter, &newSubKey);
				if (newSubKey) {
					if (IsPGPError(PGPGetSubKeyNumber(newSubKey, 
									kPGPKeyPropBits, &nEncryptSize)))
					{
						PGPKeyIterNext(kiter, &newKey);
						continue;
					}
					wsprintf(szSize, "%d/%d", nEncryptSize, nSignSize);
				}
				else
					wsprintf(szSize, "%d", nSignSize);
			}
			else
				wsprintf(szSize, "%d", nSignSize);

			if (IsPGPError(PGPGetKeyTime(newKey, kPGPKeyPropCreation, 
							&tCreation)))
			{
				PGPKeyIterNext(kiter, &newKey);
				continue;
			}
			PGPGetYMDFromPGPTime(tCreation, &year, &month, &day);
			wsprintf(szCreation, "%d/%d/%d", month, day, year);

			if (onlyThisKeySet != NULL)
			{
				if (!PGPKeySetIsMember(newKey, onlyThisKeySet))
				{
					PGPKeyIterNext(kiter, &newKey);
					continue;
				}
			}

			PGPGetKeyIDFromKey(newKey, &newKeyID);
			PGPGetKeyIDString(&newKeyID, kPGPKeyIDString_Full, szTempKeyID);
			
			lviKey.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
			lviKey.pszText = (char *) szUserID;
			lviKey.iItem = nItem;
			lviKey.iSubItem = 0;
			if (lAlg == kPGPPublicKeyAlgorithm_RSA)
				lviKey.iImage = IDX_RSAPUBKEY;
			if (lAlg == kPGPPublicKeyAlgorithm_DSA)
				lviKey.iImage = IDX_DSAPUBKEY;
			lviKey.lParam = (long) pgpAlloc(strlen(szTempKeyID)+1);
			strcpy((char *) lviKey.lParam, szTempKeyID);
			ListView_InsertItem(hList, &lviKey);

			lviKey.mask = LVIF_TEXT;
			lviKey.pszText = szCreation;
			lviKey.iItem = nItem;
			lviKey.iSubItem = 1;
			ListView_SetItem(hList, &lviKey);

			lviKey.mask = LVIF_TEXT;
			lviKey.pszText = szSize;
			lviKey.iItem = nItem;
			lviKey.iSubItem = 2;
			ListView_SetItem(hList, &lviKey);

			nItem++;
		}
		PGPKeyIterNext(kiter, &newKey);
	}
	PGPFreeKeyIter( kiter );
	PGPFreeKeyList( klist );

    /* Everything was OK */
    err = kPGPError_NoErr;
	PGPFreeKeySet(userKeySet);
	return;
}


BOOL SelectKeys(HWND hWnd,
				int nListCtrl,
				pgpConfigInfo *pConfig)
{
	int		nIndex;
	int		nNumItems;
	HANDLE	hList;
	LV_ITEM	lviKey;
	char *	szKeyID;
	BOOL	bSelected = FALSE;
	PGPPublicKeyAlgorithm	keyAlg;

	hList = GetDlgItem(hWnd, nListCtrl);
	nNumItems = ListView_GetItemCount(hList);

	for (nIndex=0; nIndex<nNumItems; nIndex++)
	{
		lviKey.mask = LVIF_PARAM | LVIF_IMAGE;
		lviKey.iItem = nIndex;
		lviKey.iSubItem = 0;
		ListView_GetItem(hList, &lviKey);

		szKeyID = (char *) lviKey.lParam;
		switch (lviKey.iImage)
		{
		case IDX_RSAPUBKEY:
			keyAlg = kPGPPublicKeyAlgorithm_RSA;
			break;

		case IDX_DSAPUBKEY:
			keyAlg = kPGPPublicKeyAlgorithm_DSA;
			break;
		}

		if (pConfig->szCorpKeyID != NULL)
		{
			if ((!strcmp(szKeyID, pConfig->szCorpKeyID)) &&
				(keyAlg == pConfig->corpKeyType))
			{
				ListView_SetItemState(hList, nIndex, LVIS_SELECTED, 0);
				bSelected = TRUE;
			}
		}
		else if (pConfig->szIncomingADKID != NULL)
		{
			if ((!strcmp(szKeyID, pConfig->szIncomingADKID)) &&
				(keyAlg == kPGPPublicKeyAlgorithm_DSA))
			{
				ListView_SetItemState(hList, nIndex, LVIS_SELECTED, 0);
				bSelected = TRUE;
			}
		}
		else if (pConfig->szOutgoingADKID != NULL)
		{
			if ((!strcmp(szKeyID, pConfig->szOutgoingADKID)) &&
				(keyAlg == pConfig->outgoingADKKeyType))
			{
				ListView_SetItemState(hList, nIndex, LVIS_SELECTED, 0);
				bSelected = TRUE;
			}
		}
	}

	return bSelected;
}


void AddSelectedKeys(HWND hWnd, 
					 int nListCtrl, 
					 PGPContextRef pgpContext,
					 PGPKeySetRef *pKeySet)
{
	int				nIndex;
	LV_ITEM			lviKey;
	HWND			hList;
	PGPKeyRef		newKey;
	PGPKeySetRef	userKeySet;
	PGPKeySetRef	newKeySet;
	PGPKeyID		newKeyID;
	PGPError		err;

	PGPPublicKeyAlgorithm	newKeyAlg;

    if(IsPGPError(err = PGPOpenDefaultKeyRings(pgpContext, 
							(PGPKeyRingOpenFlags) 0, &userKeySet)))
	{
		MessageBox(NULL, "PGPOpenDefaultKeyRings error", "debug", MB_OK);
		return;
	}

	hList = GetDlgItem(hWnd, nListCtrl);

	if (*pKeySet != NULL)
		PGPFreeKeySet(*pKeySet);
	
	*pKeySet = NULL;
	
	nIndex = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
	if (nIndex != -1)
		PGPNewKeySet(pgpContext, pKeySet);
	
	while (nIndex > -1)
	{
		lviKey.mask = LVIF_PARAM | LVIF_IMAGE;
		lviKey.iItem = nIndex;
		lviKey.iSubItem = 0;
		ListView_GetItem(hList, &lviKey);
		
		PGPGetKeyIDFromString((char *) lviKey.lParam, &newKeyID);

		switch (lviKey.iImage)
		{
		case IDX_RSAPUBKEY:
			newKeyAlg = kPGPPublicKeyAlgorithm_RSA;
			break;

		case IDX_DSAPUBKEY:
			newKeyAlg = kPGPPublicKeyAlgorithm_DSA;
			break;

		default:
			pgpDebugMsg("Image index isn't RSA or DSA!");
		}

		err = PGPGetKeyByKeyID(userKeySet, &newKeyID, newKeyAlg, &newKey);

		if ((newKey != NULL) && (IsntPGPError(err)))
		{
			PGPNewSingletonKeySet(newKey, &newKeySet);
			PGPAddKeys( newKeySet, *pKeySet);
			PGPFreeKeySet(newKeySet);
		}
		
		nIndex = ListView_GetNextItem(hList, 
			nIndex, LVNI_SELECTED);
	}

	PGPFreeKeySet(userKeySet);
	return;
}


void FreeKeyring(HWND hWnd, 
				 int nListCtrl)
{
	LV_ITEM	lviKey;
	int		nIndex;
	int		nCount;
	HWND	hList;

	hList	= GetDlgItem(hWnd, nListCtrl);
	nCount	= ListView_GetItemCount(hWnd);

	for (nIndex=0; nIndex<nCount; nIndex++)
	{
		lviKey.mask = LVIF_PARAM;
		lviKey.iItem = nIndex;
		lviKey.iSubItem = 0;
		ListView_GetItem(hList, &lviKey);
		pgpFree((void *) lviKey.lParam);
	}

	return;
}


void PaintWizBitmap(HWND hWnd, HBITMAP hBitmap)
{
	HDC			hDC, 
				hMemDC;
	PAINTSTRUCT ps;
	HBITMAP		hBitmapOld;
	BITMAP		bm;
	UINT		uXsize, 
				uYsize;
	UINT		uXpos, 
				uYpos;

	GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&bm);

	uXsize = bm.bmWidth;
	uYsize = bm.bmHeight;
	uXpos = 0;
	uYpos = 13;

	hDC = BeginPaint (hWnd, &ps);
	hMemDC = CreateCompatibleDC (hDC);
	hBitmapOld = SelectObject (hMemDC, hBitmap);
	BitBlt (hDC, uXpos, uYpos, uXsize, uYsize, hMemDC, 0, 0, SRCCOPY);
	SelectObject (hMemDC, hBitmapOld);
	DeleteDC (hMemDC);
	EndPaint (hWnd, &ps);
}


BOOL GetCommentString(PGPMemoryMgrRef memoryMgr, 
					  char *szComment, 
					  int nLength)
{
	PGPBoolean	bGotString	= FALSE;
	PGPPrefRef	prefRef		= NULL;

#if PGP_BUSINESS_SECURITY

#if PGP_BUSINESS_SECURITY && PGP_ADMIN_BUILD
	PGPclOpenAdminPrefs(memoryMgr, &prefRef, TRUE);
#else
	PGPclOpenAdminPrefs(memoryMgr, &prefRef, FALSE);
#endif

	PGPGetPrefStringBuffer(prefRef, kPGPPrefComments, nLength, szComment);
	if (strlen(szComment) > 0)
		bGotString = TRUE;
	PGPclCloseAdminPrefs(prefRef, FALSE);

	if (bGotString)
		return (BOOL) bGotString;

#endif	/* PGP_BUSINESS_SECURITY */

	PGPclOpenClientPrefs(memoryMgr, &prefRef);
	PGPGetPrefStringBuffer(prefRef, kPGPPrefComment, nLength, szComment);
	if (strlen(szComment) > 0)
		bGotString = TRUE;
	PGPclCloseClientPrefs(prefRef, FALSE);

	return (BOOL) bGotString;
}

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
