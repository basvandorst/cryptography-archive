/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PNselect.c - key selection dialog
	

	$Id: CLsel509.c,v 1.6 1999/02/11 17:19:16 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

// project header files
#include "pgpkmx.h"

// PGP SDK header files
#include "pgpClientPrefs.h"

#define TOP_LIST_OFFSET			32
#define HOR_LIST_OFFSET			8
#define BOTTOM_LIST_OFFSET		48
#define BOTTOM_BUTTON_OFFSET	10
#define BUTTON_HEIGHT			24
#define BUTTON_WIDTH			70
#define BUTTON_SPACING			10

// External globals
extern HINSTANCE	g_hInst;


typedef struct {
	PGPKeyRef		key;
	PGPSigRef		sig;
} X509CERT, *PX509CERT;

typedef struct {
	PGPContextRef	context;
	HIMAGELIST		hil;
	LPSTR			pszPrompt;
	UINT			uFlags;
	PGPKeySetRef	keyset;
	PGPKeySetRef	keysetMain;
	PGPKeyRef		keySelected;
	PGPSigRef		sigSelected;
	CHAR			szHelpFile[MAX_PATH];
} X509SELECTSTRUCT, *PX509SELECTSTRUCT;

// local globals
static DWORD aIds[] = {			// Help IDs
	IDOK,				IDH_PGPCLX509_SELECT,
	IDC_X509CERTLIST,	IDH_PGPCLX509_SELECTLIST,
	0,0 
}; 


//	____________________________________
//
//	initialize the list view control

static VOID
sInitCertList (
		HWND				hwnd,
		PX509SELECTSTRUCT	pxss)
{
	HBITMAP			hBmp; 
	HDC				hDC;
	INT				iNumBits;
	LV_COLUMN		lvc;
	RECT			rc;

	// Initialize the tree view window.
	// First create imagelist and load the appropriate bitmaps based on 
	// current display capabilities.
	hDC = GetDC (NULL);		// DC for desktop
	iNumBits = GetDeviceCaps (hDC, BITSPIXEL) * GetDeviceCaps (hDC, PLANES);
	ReleaseDC (NULL, hDC);

	if (iNumBits <= 8) {
		pxss->hil =	ImageList_Create (16, 16, ILC_COLOR|ILC_MASK, 
										NUM_BITMAPS, 0); 
		hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_IMAGES4BIT));
		ImageList_AddMasked (pxss->hil, hBmp, RGB(255, 0, 255));
		DeleteObject (hBmp);
	}
	else {
		pxss->hil =	ImageList_Create (16, 16, ILC_COLOR24|ILC_MASK, 
										NUM_BITMAPS, 0); 
		hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_IMAGES24BIT));
		ImageList_AddMasked (pxss->hil, hBmp, RGB(255, 0, 255));
		DeleteObject (hBmp);
	}

	// Associate the image list with the tree view control.
	ListView_SetImageList (GetDlgItem (hwnd, IDC_X509CERTLIST),
			pxss->hil, LVSIL_SMALL);

	// add column
	GetClientRect (GetDlgItem (hwnd, IDC_X509CERTLIST), &rc);
	lvc.mask = LVCF_WIDTH;
	lvc.cx = rc.right - rc.left;
	ListView_InsertColumn (GetDlgItem (hwnd, IDC_X509CERTLIST), 0, &lvc);
}


//	____________________________________
//
//	load the list view control with certs

static VOID
sLoadCertList (
		HWND				hwnd,
		PX509SELECTSTRUCT	pxss)
{
	PGPKeyListRef	keylist			= kInvalidPGPKeyListRef;
	PGPKeyIterRef	keyiter			= kInvalidPGPKeyIterRef;
	PGPFilterRef	filterPGP		= kInvalidPGPFilterRef;
	PGPKeyRef		key				= kInvalidPGPKeyRef;
	PGPError		err				= kPGPError_NoErr;

	PGPUserIDRef	userid;
	PGPSigRef		cert;
	PGPSize			size;
	PX509CERT		pcert;

	BOOL			bIsX509;
	LV_ITEM			lvi;
	CHAR			sz[kPGPMaxUserIDSize +1];

	ListView_DeleteAllItems (GetDlgItem (hwnd, IDC_X509CERTLIST));

	lvi.mask = LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
	lvi.iItem = 0;
	lvi.iSubItem = 0;
	lvi.pszText = sz;

	// then get X509 auth key
	err = PGPOrderKeySet (pxss->keyset, kPGPAnyOrdering, &keylist); CKERR;
	err = PGPNewKeyIter (keylist, &keyiter); CKERR;

	err = PGPKeyIterNext (keyiter, &key); CKERR;
	while (key) 
	{
		PGPKeyIterNextUserID (keyiter, &userid);
		while (userid) 
		{
			PGPKeyIterNextUIDSig (keyiter, &cert);
			while (cert) 
			{
				lvi.iImage = KMDetermineCertIcon (cert, NULL, &bIsX509);
				if (bIsX509)
				{
					err = kPGPError_NoErr;

					if (pxss->uFlags & PGPCL_CACERTSONLY)
					{
						PGPSigRef	sigSigner;
						
						err = PGPGetSigX509CertifierSig (cert,
									pxss->keyset, &sigSigner);
						if (sigSigner != cert)
							err = kPGPError_UserAbort;
					}

					if (IsntPGPError (err))
					{
						PGPGetSigPropertyBuffer (cert, 
							kPGPSigPropX509LongName, sizeof(sz), sz, &size);

						pcert = PGPNewData (
								PGPGetContextMemoryMgr (pxss->context),
								sizeof(X509CERT), kPGPMemoryMgrFlags_Clear);
						if (pcert)
						{
							pcert->key = key;
							pcert->sig = cert;

							lvi.lParam = (LPARAM)pcert;
							ListView_InsertItem (
								GetDlgItem (hwnd, IDC_X509CERTLIST), &lvi);
						}
					}
				}

				PGPKeyIterNextUIDSig (keyiter, &cert);
			}
			PGPKeyIterNextUserID (keyiter, &userid);
		}
		PGPKeyIterNext (keyiter, &key);
	}
	InvalidateRect (GetDlgItem (hwnd, IDC_X509CERTLIST), NULL, FALSE);

done :
	if (PGPKeyIterRefIsValid (keyiter))
		PGPFreeKeyIter (keyiter);
	if (PGPKeyListRefIsValid (keylist))
		PGPFreeKeyList (keylist);
}


//	____________________________________
//
//	free the data structures associated with the list

static VOID
sDestroyCertList (
		HWND				hwnd,
		PX509SELECTSTRUCT	pxss)
{
	HWND		hwndList;
	INT			i, iNumItems;
	LV_ITEM		lvi;

	hwndList = GetDlgItem (hwnd, IDC_X509CERTLIST);
	iNumItems = ListView_GetItemCount (hwndList);

	lvi.mask = LVIF_PARAM;
	for (i=0; i<iNumItems; i++)
	{
		lvi.iItem = i;
		ListView_GetItem (hwndList, &lvi);
		if (lvi.lParam)
		{
			PGPFreeData ((PVOID)lvi.lParam);
		}
	}
}


//	____________________________________
//
//	size the list control and move the buttons

static VOID
sSizeControls (
		HWND	hwnd,
		WORD	wXsize,
		WORD	wYsize)
{
	HDWP	hdwp; 
	HWND	hwndList;
	RECT	rc;

	// size the list control accordingly
	hdwp = BeginDeferWindowPos (3); 
	hwndList = GetDlgItem (hwnd, IDC_X509CERTLIST);
	DeferWindowPos (hdwp, 
		hwndList, NULL, 
		HOR_LIST_OFFSET, TOP_LIST_OFFSET, 
		wXsize - (2*HOR_LIST_OFFSET), 
		wYsize - BOTTOM_LIST_OFFSET - TOP_LIST_OFFSET,
		SWP_NOZORDER);  
	
	// move the "cancel" button
	DeferWindowPos (hdwp, 
		GetDlgItem (hwnd, IDCANCEL), NULL, 
		wXsize - HOR_LIST_OFFSET - BUTTON_WIDTH, 
		wYsize - BOTTOM_BUTTON_OFFSET - BUTTON_HEIGHT, 
		BUTTON_WIDTH, BUTTON_HEIGHT, 
		SWP_NOZORDER); 

	// move the "OK" button
	DeferWindowPos (hdwp, 
		GetDlgItem (hwnd, IDOK), NULL, 
		wXsize - HOR_LIST_OFFSET - BUTTON_SPACING - 2*BUTTON_WIDTH, 
		wYsize - BOTTOM_BUTTON_OFFSET - BUTTON_HEIGHT, 
		BUTTON_WIDTH, BUTTON_HEIGHT, 
		SWP_NOZORDER); 

	EndDeferWindowPos (hdwp);

	// reset column width
	GetClientRect (hwndList, &rc);
	ListView_SetColumnWidth (hwndList, 0, rc.right);
}


//	____________________________________
//
//	dialog proc of X509 certificate selection dialog

static BOOL CALLBACK
sX509SelectDlgProc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam)
{
	PX509SELECTSTRUCT	pxss;

	switch (uMsg) {
	case WM_INITDIALOG :
	{
		RECT	rc;

		SetWindowLong (hwnd, GWL_USERDATA, lParam);
		pxss = (PX509SELECTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);

		GetClientRect (hwnd, &rc);
		sSizeControls (hwnd, (WORD)rc.right, (WORD)rc.bottom);
		sInitCertList (hwnd, pxss);
		sLoadCertList (hwnd, pxss);

		if (pxss->pszPrompt)
			SetDlgItemText (hwnd, IDC_SELECTPROMPT, pxss->pszPrompt);

		return FALSE;
	}

	case WM_DESTROY :
		pxss = (PX509SELECTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		sDestroyCertList (hwnd, pxss);
		ImageList_Destroy (pxss->hil);
		break;

	case WM_SIZE :
		sSizeControls (hwnd, LOWORD(lParam), HIWORD(lParam));
		return 0;

	case WM_NOTIFY :
		if (wParam == IDC_X509CERTLIST)
		{
			NM_LISTVIEW* pnmlv = (NM_LISTVIEW*)lParam;

			switch (pnmlv->hdr.code) {

			case LVN_ITEMCHANGED :
				pxss = (PX509SELECTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
				if (pnmlv->iItem >= 0)
				{
					if (pnmlv->uNewState & LVIS_SELECTED)
					{
						PX509CERT pcert;

						pcert = (PX509CERT)pnmlv->lParam;
						pxss->keySelected = pcert->key;
						pxss->sigSelected = pcert->sig;

						EnableWindow (GetDlgItem (hwnd, IDOK), TRUE);
					}
					else
					{
						pxss->keySelected = kInvalidPGPKeyRef;
						pxss->sigSelected = kInvalidPGPSigRef;
						EnableWindow (GetDlgItem (hwnd, IDOK), FALSE);
					}
				}	
				break;
			}
		}
		break;

    case WM_HELP: 
		pxss = (PX509SELECTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		WinHelp (((LPHELPINFO) lParam)->hItemHandle, pxss->szHelpFile, 
				HELP_WM_HELP, (DWORD) (LPSTR) aIds); 
        break; 
 
    case WM_CONTEXTMENU: 
		pxss = (PX509SELECTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		WinHelp ((HWND) wParam, pxss->szHelpFile, HELP_CONTEXTMENU, 
				(DWORD) (LPVOID) aIds); 
        break; 

	case WM_COMMAND :
		pxss = (PX509SELECTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);

		switch (LOWORD (wParam)) {

		case IDOK :
			EndDialog (hwnd, 1);
			break;

		case IDCANCEL :
			EndDialog (hwnd, 0);
			break;
		}
		break;
	}

	return FALSE;
}


//	___________________________________________________
//
//  Display and handle selection of X509 keypair dialog

PGPError PGPclExport 
PGPclSelectX509Cert (
		PGPContextRef		context,
		HWND				hwndParent, 
		LPSTR				pszPrompt,
		PGPKeySetRef		keysetMain,
		UINT				uFlags,
		PGPKeyRef*			pkeyCert,
		PGPSigRef*			psigCert)
{

	PGPKeySetRef	keysetX509		= kInvalidPGPKeySetRef;
	PGPKeyRef		keySelected		= kInvalidPGPKeyRef;
	PGPFilterRef	filter			= kInvalidPGPFilterRef;
	PGPFilterRef	filter2			= kInvalidPGPFilterRef;
	PGPError		err				= kPGPError_NoErr;

	PGPUInt32		uNumKeys;
	CHAR			sz[MAX_PATH];

	PGPValidatePtr (pkeyCert);
	PGPValidatePtr (psigCert);

	err = PGPNewSigBooleanFilter (
				context, kPGPSigPropIsX509, TRUE, &filter); CKERR;

	if (uFlags & PGPCL_CANSIGNKEYSONLY)
	{
		err = PGPNewKeyBooleanFilter (
				context, kPGPKeyPropCanSign, TRUE, &filter2); CKERR;
		err = PGPIntersectFilters (filter, filter2, &filter); CKERR;
		filter2 = kInvalidPGPFilterRef;
	}

	if (uFlags & PGPCL_NOSPLITKEYS)
	{
		err = PGPNewKeyBooleanFilter (
				context, kPGPKeyPropIsSecretShared, FALSE, &filter2); CKERR;
		err = PGPIntersectFilters (filter, filter2, &filter); CKERR;
		filter2 = kInvalidPGPFilterRef;
	}

	err = PGPFilterKeySet (keysetMain, filter, &keysetX509); CKERR;

	err = PGPCountKeys (keysetX509, &uNumKeys); CKERR;
	if (uNumKeys > 0)
	{
		X509SELECTSTRUCT	xss;

		xss.context			= context;
		xss.hil				= NULL;
		xss.pszPrompt		= pszPrompt;
		xss.uFlags			= uFlags;
		xss.keyset			= keysetX509;
		xss.keysetMain		= keysetMain;
		xss.keySelected		= kInvalidPGPKeyRef;
		xss.sigSelected		= kInvalidPGPSigRef;

		PGPclGetPGPPath (xss.szHelpFile, sizeof(xss.szHelpFile));
		LoadString (g_hInst, IDS_HELPFILENAME, sz, sizeof(sz));
		lstrcat (xss.szHelpFile, sz);

		if (DialogBoxParam (g_hInst, MAKEINTRESOURCE(IDD_SELECTX509CERT), 
					hwndParent, sX509SelectDlgProc, (LPARAM)&xss))
			err = kPGPError_NoErr;
		else
			err = kPGPError_UserAbort;

		*pkeyCert = xss.keySelected;
		*psigCert = xss.sigSelected;
	}
	else
	{
		KMMessageBox (hwndParent, IDS_CAPTION, IDS_NOX509CERTS, 
						MB_OK|MB_ICONEXCLAMATION);
		err = kPGPError_UserAbort;
	}

done:
	if (PGPFilterRefIsValid (filter))
		PGPFreeFilter (filter);
	if (PGPFilterRefIsValid (filter2))
		PGPFreeFilter (filter2);
	if (PGPKeySetRefIsValid (keysetX509))
		PGPFreeKeySet (keysetX509);

	return err;
}

