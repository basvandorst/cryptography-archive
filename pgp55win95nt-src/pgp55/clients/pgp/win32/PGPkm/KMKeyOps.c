//:KMKeyOps.c - implements various operations performed on keys. 
//
//	$Id: KMKeyOps.c,v 1.65.4.1 1997/11/18 18:26:59 pbj Exp $
//

#include "pgpkmx.h"
#include <shlobj.h>
#include "pgpSDKPrefs.h"
#include "pgpclientprefs.h"
#include "..\include\PGPpk.h"
#include "..\include\pgpphras.h"
#include "resource.h"

#define BITMAP_WIDTH	16
#define BITMAP_HEIGHT	16

#define INITIAL_SIGN_COLUMNWIDTH	210

#define SIG_NONEXPORTABLE	0
#define SIG_EXPORTABLE		1
#define SIG_TRUST			2
#define SIG_META			3

// external globals  
extern HINSTANCE g_hInst;

typedef struct {
	FARPROC lpfnCallback;
	PKEYMAN pKM;
	BOOL bItemModified;
	PGPKeyRef SigningKey;
	LPSTR szPhrase;
	PGPBoolean bExportable;
	PGPUInt32 iTrustLevel;
} CERTIFYSTRUCT;

typedef struct {
	FARPROC lpfnCallback;
	PGPContextRef Context;
	PKEYMAN pKM;
	LPSTR pszPrompt;
	BOOL bItemModified;
	BOOL bItemNotDeleted;
	BOOL bDeleteAll;
	BOOL bDeletedPrimaryUserID;
	PGPKeyRef DefaultKey;
	HTLITEM hPostDeleteFocusItem;
} DELETESTRUCT;

typedef struct {
	FARPROC lpfnCallback;
	PGPContextRef Context;
	PGPKeySetRef AddKeySet;
} ADDTOMAINSTRUCT;

typedef struct {
	FARPROC lpfnCallback;
	PKEYMAN pKM;
	PGPBoolean bSyncWithServer;
	INT	iSigType;
	HWND hWndList;
	HIMAGELIST hIml;
	INT iItem;
} CERTIFYCONFIRMSTRUCT;

typedef struct {
	PKEYMAN pKM;
	PGPKeyRef Key;
	PGPBoolean bSyncWithServer;
} REVOKECERTSTRUCT;

static DWORD aDeleteAllIds[] = {			// Help IDs
    IDOK,			IDH_PGPKM_DELETEKEY, 
    IDC_YESTOALL,	IDH_PGPKM_DELETEALLKEYS, 
    IDNO,			IDH_PGPKM_DONTDELETEKEY, 
    IDCANCEL,		IDH_PGPKM_CANCELDELETE, 
    0,0 
}; 

static DWORD aSignKeyIds[] = {			// Help IDs
	IDC_KEYLIST,		IDH_PGPKM_SIGNUSERIDLIST,
	IDC_MORECHOICES,	IDH_PGPKM_MORESIGCHOICES,
	IDC_FEWERCHOICES,	IDH_PGPKM_FEWERSIGCHOICES,
	IDC_EXPORTABLECHECK,IDH_PGPKM_ALLOWSIGEXPORT,
	IDC_NONEXPORTABLE,	IDH_PGPKM_SIGNONEXPORTABLE,
	IDC_EXPORTABLE,		IDH_PGPKM_SIGEXPORTABLE,
	IDC_TRUSTED,		IDH_PGPKM_SIGTRUSTED,
	IDC_META,			IDH_PGPKM_SIGMETA,
    0,0 
}; 


//----------------------------------------------------|
//  Certify a single object
//	routine called either from KMCertifyKeyOrUserID or as a
//	callback function from the TreeList control to 
//	certify a single item.
//
//	lptli	= pointer to TreeList item to certify

BOOL CALLBACK 
CertifySingleObject (TL_TREEITEM* lptli, 
					 LPARAM lParam) 
{
	CERTIFYSTRUCT*	pcs = (CERTIFYSTRUCT*)lParam;
	UINT			uError = 0;

	PGPKeyRef		Key;	
	PGPUserIDRef	UserID;
	PGPContextRef	Context;
	CHAR			sz512[512];
	CHAR			sz256[256];
	CHAR			sz64[64];
	PGPBoolean		bNeedsPhrase;

	switch (lptli->iImage) {
	case IDX_RSAPUBKEY :
	case IDX_RSAPUBDISKEY :
	case IDX_RSASECKEY :
	case IDX_RSASECDISKEY :
	case IDX_DSAPUBKEY :
	case IDX_DSAPUBDISKEY :
	case IDX_DSASECKEY :
	case IDX_DSASECDISKEY :
		Key = (PGPKeyRef)(lptli->lParam);
		PGPGetPrimaryUserID (Key, &UserID);
		break;

	case IDX_RSAUSERID :
	case IDX_DSAUSERID :
		UserID = (PGPUserIDRef)(lptli->lParam);
		Key = KMGetKeyFromUserID (pcs->pKM, UserID);
		break;

	case IDX_RSAPUBREVKEY :
	case IDX_RSAPUBEXPKEY :
	case IDX_RSASECREVKEY :
	case IDX_RSASECEXPKEY :
	case IDX_DSAPUBREVKEY :
	case IDX_DSAPUBEXPKEY :
	case IDX_DSASECREVKEY :
	case IDX_DSASECEXPKEY :
		return TRUE;

	default :
		return FALSE;
	}

	Context = pcs->pKM->Context;

	// make sure we have enough entropy
	PGPcomdlgRandom (Context, pcs->pKM->hWndParent, 0);

	uError = PGPSignUserID (UserID, pcs->SigningKey, 
				PGPOPassphrase (Context, pcs->szPhrase),
				PGPOExpiration (Context, 0),
				PGPOExportable (Context, pcs->bExportable),
				PGPOSigTrust (Context, pcs->iTrustLevel,
									kPGPKeyTrust_Complete),
				PGPOLastOption (Context));

	if (uError == kPGPError_BadPassphrase) {
		PGPGetKeyBoolean (pcs->SigningKey, 
			kPGPKeyPropNeedsPassphrase, &bNeedsPhrase);
		if (!bNeedsPhrase) return FALSE;

		while (uError == kPGPError_BadPassphrase) {
			PGPcomdlgErrorMessage (uError);
			if (pcs->szPhrase) {
				PGPcomdlgFreePhrase (pcs->szPhrase);
				pcs->szPhrase = NULL;
			}
			LoadString (g_hInst, IDS_SIGNKEYPASSPHRASE, sz256, 256);
			uError = PGPcomdlgGetPhrase (pcs->pKM->Context, 
									pcs->pKM->hWndParent, sz256, 
									&pcs->szPhrase, pcs->pKM->KeySetMain, 
									NULL, 0, &pcs->SigningKey, NULL, 0);
			PGPcomdlgErrorMessage (uError);
			if (uError != kPGPError_NoErr) {
				if (pcs->szPhrase) {
					PGPcomdlgFreePhrase (pcs->szPhrase);
					pcs->szPhrase = NULL;
				}
				return FALSE;
			}

			// make sure we have enough entropy
			PGPcomdlgRandom (Context, pcs->pKM->hWndParent, 0);

			uError = PGPSignUserID (UserID, pcs->SigningKey, 
						PGPOPassphrase (Context, pcs->szPhrase),
						PGPOExpiration (Context, 0),
						PGPOExportable (Context, pcs->bExportable),
						PGPOSigTrust (Context, pcs->iTrustLevel,
									kPGPKeyTrust_Complete),
						PGPOLastOption (Context));
		}		
	}

	if (IsntPGPError (uError)) {
		pcs->bItemModified = TRUE;
		return TRUE;
	}

	LoadString (g_hInst, IDS_CERTIFYERROR, sz64, 64); 
	PGPcomdlgErrorToString (uError, sz256, 256);
	wsprintf (sz512, sz64, lptli->pszText, sz256);
	LoadString (g_hInst, IDS_CAPTION, sz64, 64);
	if (KMMultipleSelected (pcs->pKM)) {
		if (MessageBox (pcs->pKM->hWndParent, sz512, sz64, 
						MB_OKCANCEL|MB_ICONEXCLAMATION) == IDOK)
			return TRUE;
	}
	else {
		if (MessageBox (pcs->pKM->hWndParent, sz512, sz64, 
						MB_OK|MB_ICONEXCLAMATION) == IDOK)
			return TRUE;
	}

	return FALSE;
}


//----------------------------------------------------|
//  Populate ListView with userids to sign

BOOL CALLBACK InsertOneID (TL_TREEITEM* lptli, LPARAM lParam) {
	PGPKeyRef Key;	
	PGPUserIDRef UserID;
	UINT u, uAlgorithm;
	LV_ITEM lvI;
	PGPByte	fingerprintBytes[256];
	CHAR sz[kPGPMaxUserIDSize];
	CERTIFYCONFIRMSTRUCT* pccs = (CERTIFYCONFIRMSTRUCT*)lParam;

	switch (lptli->iImage) {
	case IDX_RSAPUBKEY :
	case IDX_RSAPUBDISKEY :
	case IDX_RSASECKEY :
	case IDX_RSASECDISKEY :
	case IDX_DSAPUBKEY :
	case IDX_DSAPUBDISKEY :
	case IDX_DSASECKEY :
	case IDX_DSASECDISKEY :
		Key = (PGPKeyRef)(lptli->lParam);
		KMGetKeyName (Key, sz, sizeof(sz));
		break;

	case IDX_RSAUSERID :
	case IDX_DSAUSERID :
		UserID = (PGPUserIDRef)(lptli->lParam);
		Key = KMGetKeyFromUserID (pccs->pKM, UserID);
		KMGetUserIDName (UserID, sz, sizeof(sz));
		break;

	case IDX_RSAPUBREVKEY :
	case IDX_RSAPUBEXPKEY :
	case IDX_RSASECREVKEY :
	case IDX_RSASECEXPKEY :
	case IDX_DSAPUBREVKEY :
	case IDX_DSAPUBEXPKEY :
	case IDX_DSASECREVKEY :
	case IDX_DSASECEXPKEY :
		return TRUE;

	default :
		return FALSE;
	}

	PGPGetKeyNumber (Key, kPGPKeyPropAlgID, &uAlgorithm);
	switch (uAlgorithm) {
		case kPGPPublicKeyAlgorithm_RSA :	lvI.iImage = IDX_RSAUSERID;	break;
		case kPGPPublicKeyAlgorithm_DSA :	lvI.iImage = IDX_DSAUSERID;	break;
		default :							lvI.iImage = IDX_RSAUSERID; break;
	}

	lvI.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
	lvI.state = 0;      
	lvI.stateMask = 0;  

	lvI.iItem = pccs->iItem;
	lvI.iSubItem = 0;
	lvI.pszText	= sz; 
	lvI.cchTextMax = 0;

	if (ListView_InsertItem (pccs->hWndList, &lvI) == -1) return FALSE;

	PGPGetKeyPropertyBuffer(Key, kPGPKeyPropFingerprint,
		sizeof( fingerprintBytes ), fingerprintBytes, &u);
	KMConvertStringFingerprint (uAlgorithm, fingerprintBytes);
	ListView_SetItemText (pccs->hWndList, pccs->iItem, 1, fingerprintBytes);

	(pccs->iItem)++;
	
	return TRUE;
}


//----------------------------------------------------|
//  Populate ListView with userids to sign

VOID 
FillKeyList (CERTIFYCONFIRMSTRUCT* pccs) 
{

	LV_COLUMN	lvC; 
	CHAR		sz[256];
	HBITMAP		hBmp;
	HDC			hDC;
	INT			iNumBits;

	// create image list
	pccs->hIml = ImageList_Create (BITMAP_WIDTH, BITMAP_HEIGHT, 
							ILC_COLOR24 | ILC_MASK, 2, 0);

	hDC = GetDC (NULL);		// DC for desktop
	iNumBits = GetDeviceCaps (hDC, BITSPIXEL) * GetDeviceCaps (hDC, PLANES);
	ReleaseDC (NULL, hDC);

	hDC = GetDC (NULL);		// DC for desktop
	iNumBits = GetDeviceCaps (hDC, BITSPIXEL) * GetDeviceCaps (hDC, PLANES);
	ReleaseDC (NULL, hDC);

	if (iNumBits <= 8) {
		pccs->hIml = ImageList_Create (16, 16, ILC_COLOR|ILC_MASK, 
										NUM_BITMAPS, 0); 
		hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_IMAGES4BIT));
		ImageList_AddMasked (pccs->hIml, hBmp, RGB(255, 0, 255));
		DeleteObject (hBmp);
	}
	else {
		pccs->hIml = ImageList_Create (16, 16, ILC_COLOR24|ILC_MASK, 
										NUM_BITMAPS, 0); 
		hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_IMAGES24BIT));
		ImageList_AddMasked (pccs->hIml, hBmp, RGB(255, 0, 255));
		DeleteObject (hBmp);
	}

	ListView_SetImageList (pccs->hWndList, pccs->hIml, LVSIL_SMALL);

	lvC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lvC.fmt = LVCFMT_LEFT; 
	lvC.cx = INITIAL_SIGN_COLUMNWIDTH;   
	LoadString (g_hInst, IDS_USERID, sz, sizeof(sz));
	lvC.pszText = sz;
	lvC.iSubItem = 0;
	if (ListView_InsertColumn (pccs->hWndList, 0, &lvC) == -1) return;

	LoadString (g_hInst, IDS_FINGERPRINT, sz, sizeof(sz));
	lvC.cx = 360;   
	lvC.iSubItem = 1;
	if (ListView_InsertColumn (pccs->hWndList, 1, &lvC) == -1) return;

	// populate control by iterating through selected items
	pccs->lpfnCallback = InsertOneID;
	pccs->iItem = 0;
	TreeList_IterateSelected (pccs->pKM->hWndTree, pccs);
}


//----------------------------------------------------|
// Sign key dialog message procedure

BOOL CALLBACK 
SignKeyDlgProc (HWND hDlg, 
				UINT uMsg, 
				WPARAM wParam, 
				LPARAM lParam) 
{
	CERTIFYCONFIRMSTRUCT*	pccs;
	INT						iNewY, iNewWindowHeight, iOffset;
	HWND					hwndControl;
	RECT					rectControl;
	RECT					rc;

	switch (uMsg) {

	case WM_INITDIALOG :
		SetWindowLong (hDlg, GWL_USERDATA, lParam);
		pccs = (CERTIFYCONFIRMSTRUCT*)lParam;
		pccs->hWndList = GetDlgItem (hDlg, IDC_KEYLIST);

#if PGP_BUSINESS_SECURITY
		EnableWindow (GetDlgItem (hDlg, IDC_MORECHOICES), TRUE);
		ShowWindow (GetDlgItem (hDlg, IDC_MORECHOICES), SW_SHOW);
#else
		EnableWindow (GetDlgItem (hDlg, IDC_MORECHOICES), FALSE);
		ShowWindow (GetDlgItem (hDlg, IDC_MORECHOICES), SW_HIDE);
#endif

		FillKeyList (pccs);

		// user "more" button as desired Y location
		hwndControl = GetDlgItem (hDlg, IDC_MORECHOICES);
		GetWindowRect (hwndControl, &rectControl);
		MapWindowPoints (NULL, hDlg, (LPPOINT)&rectControl, 2);

		GetWindowRect (hDlg, &rc);
		iOffset = rc.bottom-rc.top;
		GetClientRect (hDlg, &rc);
		iOffset -= rc.bottom;
		iOffset += (rectControl.bottom - rectControl.top) / 2;

		iNewY = rectControl.top;
		iNewWindowHeight = rectControl.bottom + iOffset;

		// move OK Button
		hwndControl = GetDlgItem (hDlg, IDOK);
		GetWindowRect (hwndControl, &rectControl);
		MapWindowPoints (NULL, hDlg, (LPPOINT)&rectControl, 2);
		MoveWindow(	hwndControl, 
					rectControl.left,
					iNewY,
					rectControl.right - rectControl.left,
					rectControl.bottom - rectControl.top,
					TRUE);

		// move Cancel Button
		hwndControl = GetDlgItem (hDlg, IDCANCEL);
		GetWindowRect (hwndControl, &rectControl);
		MapWindowPoints (NULL, hDlg, (LPPOINT)&rectControl, 2);
		MoveWindow(	hwndControl, 
					rectControl.left,
					iNewY,
					rectControl.right - rectControl.left,
					rectControl.bottom - rectControl.top,
					TRUE);

		// move Help Button
		hwndControl = GetDlgItem (hDlg, IDHELP);
		GetWindowRect (hwndControl, &rectControl);
		MapWindowPoints (NULL, hDlg, (LPPOINT)&rectControl, 2);
		MoveWindow(	hwndControl, 
					rectControl.left,
					iNewY,
					rectControl.right - rectControl.left,
					rectControl.bottom - rectControl.top,
					TRUE);

		// size Window 
		GetWindowRect (hDlg, &rectControl);
		MoveWindow(	hDlg, 
					rectControl.left,
					rectControl.top,
					rectControl.right - rectControl.left,
					iNewWindowHeight,
					TRUE);

		break;

    case WM_HELP: 
		pccs = (CERTIFYCONFIRMSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
        WinHelp (((LPHELPINFO) lParam)->hItemHandle, pccs->pKM->szHelpFile, 
            HELP_WM_HELP, (DWORD) (LPSTR) aSignKeyIds); 
        break; 
 
    case WM_CONTEXTMENU:
		pccs = (CERTIFYCONFIRMSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
        WinHelp ((HWND) wParam, pccs->pKM->szHelpFile, HELP_CONTEXTMENU, 
            (DWORD) (LPVOID) aSignKeyIds); 
        break; 

	case WM_COMMAND:

		switch (LOWORD(wParam)) {
		case IDCANCEL:
			pccs = (CERTIFYCONFIRMSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
			pccs->bSyncWithServer = FALSE;
			ImageList_Destroy (pccs->hIml);
			EndDialog (hDlg, 1);
			break;

		case IDOK:
			pccs = (CERTIFYCONFIRMSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
			ImageList_Destroy (pccs->hIml);
			EndDialog (hDlg, 0);
			break;

		case IDC_EXPORTABLECHECK :
			pccs = (CERTIFYCONFIRMSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
			if (IsDlgButtonChecked (hDlg, IDC_EXPORTABLECHECK) 
													== BST_CHECKED) {
				pccs->iSigType = SIG_EXPORTABLE;
			}
			else {
				pccs->iSigType = SIG_NONEXPORTABLE;
			}
			break;

		case IDC_NONEXPORTABLE :
			pccs = (CERTIFYCONFIRMSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
			pccs->iSigType = SIG_NONEXPORTABLE;
			break;

		case IDC_EXPORTABLE :
			pccs = (CERTIFYCONFIRMSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
			pccs->iSigType = SIG_EXPORTABLE;
			break;

		case IDC_TRUSTED :
			pccs = (CERTIFYCONFIRMSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
			pccs->iSigType = SIG_TRUST;
			break;

		case IDC_META :
			pccs = (CERTIFYCONFIRMSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
			pccs->iSigType = SIG_META;
			break;

		case IDC_MORECHOICES :
			pccs = (CERTIFYCONFIRMSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
			ShowWindow (GetDlgItem (hDlg, IDC_MORECHOICES), SW_HIDE);
			ShowWindow (GetDlgItem (hDlg, IDC_EXPORTABLECHECK), SW_HIDE);
			ShowWindow (GetDlgItem (hDlg, IDC_FEWERCHOICES), SW_SHOW);
			ShowWindow (GetDlgItem (hDlg, IDC_SIGTYPE), SW_SHOW);
			ShowWindow (GetDlgItem (hDlg, IDC_NONEXPORTABLE), SW_SHOW);
			ShowWindow (GetDlgItem (hDlg, IDC_EXPORTABLE), SW_SHOW);
			ShowWindow (GetDlgItem (hDlg, IDC_TRUSTED), SW_SHOW);
			ShowWindow (GetDlgItem (hDlg, IDC_META), SW_SHOW);
			ShowWindow (GetDlgItem (hDlg, IDC_ADVANCEDTEXT1), SW_SHOW);
			ShowWindow (GetDlgItem (hDlg, IDC_ADVANCEDTEXT2), SW_SHOW);
			ShowWindow (GetDlgItem (hDlg, IDC_ADVANCEDTEXT3), SW_SHOW);
			ShowWindow (GetDlgItem (hDlg, IDC_ADVANCEDTEXT4), SW_SHOW);

			// user "fewer" button as desired Y location
			hwndControl = GetDlgItem (hDlg, IDC_FEWERCHOICES);
			GetWindowRect (hwndControl, &rectControl);
			MapWindowPoints (NULL, hDlg, (LPPOINT)&rectControl, 2);

			GetWindowRect (hDlg, &rc);
			iOffset = rc.bottom-rc.top;
			GetClientRect (hDlg, &rc);
			iOffset -= rc.bottom;
			iOffset += (rectControl.bottom - rectControl.top) / 2;

			iNewY = rectControl.top;
			iNewWindowHeight = rectControl.bottom + iOffset;

			// move OK Button
			hwndControl = GetDlgItem (hDlg, IDOK);
			GetWindowRect (hwndControl, &rectControl);
			MapWindowPoints (NULL, hDlg, (LPPOINT)&rectControl, 2);
			MoveWindow(	hwndControl, 
						rectControl.left,
						iNewY,
						rectControl.right - rectControl.left,
						rectControl.bottom - rectControl.top,
						TRUE);

			// move Cancel Button
			hwndControl = GetDlgItem (hDlg, IDCANCEL);
			GetWindowRect (hwndControl, &rectControl);
			MapWindowPoints (NULL, hDlg, (LPPOINT)&rectControl, 2);
			MoveWindow(	hwndControl, 
						rectControl.left,
						iNewY,
						rectControl.right - rectControl.left,
						rectControl.bottom - rectControl.top,
						TRUE);

			// move Help Button
			hwndControl = GetDlgItem (hDlg, IDHELP);
			GetWindowRect (hwndControl, &rectControl);
			MapWindowPoints (NULL, hDlg, (LPPOINT)&rectControl, 2);
			MoveWindow(	hwndControl, 
						rectControl.left,
						iNewY,
						rectControl.right - rectControl.left,
						rectControl.bottom - rectControl.top,
						TRUE);

			// set radio buttons
			CheckRadioButton (hDlg, IDC_NONEXPORTABLE, IDC_META, 
								IDC_NONEXPORTABLE + pccs->iSigType);

			// size Window 
			GetWindowRect (hDlg, &rectControl);
			MoveWindow(	hDlg, 
						rectControl.left,
						rectControl.top,
						rectControl.right - rectControl.left,
						iNewWindowHeight,
						TRUE);
			break;

		case IDC_FEWERCHOICES :
			pccs = (CERTIFYCONFIRMSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
			ShowWindow (GetDlgItem (hDlg, IDC_FEWERCHOICES), SW_HIDE);
			ShowWindow (GetDlgItem (hDlg, IDC_SIGTYPE), SW_HIDE);
			ShowWindow (GetDlgItem (hDlg, IDC_NONEXPORTABLE), SW_HIDE);
			ShowWindow (GetDlgItem (hDlg, IDC_EXPORTABLE), SW_HIDE);
			ShowWindow (GetDlgItem (hDlg, IDC_TRUSTED), SW_HIDE);
			ShowWindow (GetDlgItem (hDlg, IDC_META), SW_HIDE);
			ShowWindow (GetDlgItem (hDlg, IDC_MORECHOICES), SW_SHOW);
			ShowWindow (GetDlgItem (hDlg, IDC_EXPORTABLECHECK), SW_SHOW);
			ShowWindow (GetDlgItem (hDlg, IDC_ADVANCEDTEXT1), SW_HIDE);
			ShowWindow (GetDlgItem (hDlg, IDC_ADVANCEDTEXT2), SW_HIDE);
			ShowWindow (GetDlgItem (hDlg, IDC_ADVANCEDTEXT3), SW_HIDE);
			ShowWindow (GetDlgItem (hDlg, IDC_ADVANCEDTEXT4), SW_HIDE);

			// user "more" button as desired Y location
			hwndControl = GetDlgItem (hDlg, IDC_MORECHOICES);
			GetWindowRect (hwndControl, &rectControl);
			MapWindowPoints (NULL, hDlg, (LPPOINT)&rectControl, 2);

			GetWindowRect (hDlg, &rc);
			iOffset = rc.bottom-rc.top;
			GetClientRect (hDlg, &rc);
			iOffset -= rc.bottom;
			iOffset += (rectControl.bottom - rectControl.top) / 2;

			iNewY = rectControl.top;
			iNewWindowHeight = rectControl.bottom + iOffset;

			// move OK Button
			hwndControl = GetDlgItem (hDlg, IDOK);
			GetWindowRect (hwndControl, &rectControl);
			MapWindowPoints (NULL, hDlg, (LPPOINT)&rectControl, 2);
			MoveWindow(	hwndControl, 
						rectControl.left,
						iNewY,
						rectControl.right - rectControl.left,
						rectControl.bottom - rectControl.top,
						TRUE);

			// move Cancel Button
			hwndControl = GetDlgItem (hDlg, IDCANCEL);
			GetWindowRect (hwndControl, &rectControl);
			MapWindowPoints (NULL, hDlg, (LPPOINT)&rectControl, 2);
			MoveWindow(	hwndControl, 
						rectControl.left,
						iNewY,
						rectControl.right - rectControl.left,
						rectControl.bottom - rectControl.top,
						TRUE);

			// move Help Button
			hwndControl = GetDlgItem (hDlg, IDHELP);
			GetWindowRect (hwndControl, &rectControl);
			MapWindowPoints (NULL, hDlg, (LPPOINT)&rectControl, 2);
			MoveWindow(	hwndControl, 
						rectControl.left,
						iNewY,
						rectControl.right - rectControl.left,
						rectControl.bottom - rectControl.top,
						TRUE);

			// set radio buttons
			if (pccs->iSigType == SIG_EXPORTABLE) 
				CheckDlgButton (hDlg, IDC_EXPORTABLECHECK, BST_CHECKED);
			else {
				pccs->iSigType = SIG_NONEXPORTABLE;
				CheckDlgButton (hDlg, IDC_EXPORTABLECHECK, BST_UNCHECKED);
			}

			// size Window 
			GetWindowRect (hDlg, &rectControl);
			MoveWindow(	hDlg, 
						rectControl.left,
						rectControl.top,
						rectControl.right - rectControl.left,
						iNewWindowHeight,
						TRUE);
			break;

		case IDHELP :
			pccs = (CERTIFYCONFIRMSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
			WinHelp (hDlg, pccs->pKM->szHelpFile, HELP_CONTEXT, 
						IDH_PGPKM_SIGNDIALOG); 
			break;

		}
		return TRUE;
	}
	return FALSE;
}


//----------------------------------------------------|
//  Certify selected key or userid

BOOL 
KMCertifyKeyOrUserID (PKEYMAN pKM) 
{
	TL_TREEITEM				tli;
	CHAR					sz256[256];
	UINT					uError;
	CERTIFYSTRUCT			cs;
	CERTIFYCONFIRMSTRUCT	ccs;
	PGPPrefRef				prefref;

	// initialize structures
	cs.lpfnCallback = CertifySingleObject;
	cs.pKM = pKM;
	cs.bItemModified = FALSE;
	cs.SigningKey = NULL;
	cs.szPhrase = NULL;
	
	ccs.pKM = pKM;
	ccs.iSigType = SIG_NONEXPORTABLE;

	PGPcomdlgOpenClientPrefs (&prefref);
	PGPGetPrefBoolean (prefref, kPGPPrefKeyServerSyncOnKeySign, 
						&(ccs.bSyncWithServer));
	PGPcomdlgCloseClientPrefs (prefref, FALSE);

	if (DialogBoxParam (g_hInst, MAKEINTRESOURCE(IDD_SIGNCONFIRM),
		pKM->hWndParent, SignKeyDlgProc, (LPARAM)&ccs)) {
		return FALSE;
	}

	// convert user-entered signature type to flags to pass
	switch (ccs.iSigType) {
	case SIG_NONEXPORTABLE :
		cs.bExportable = FALSE;
		cs.iTrustLevel = 0;
///		ccs.bSyncWithServer = FALSE;
		break;

	case SIG_EXPORTABLE :
		cs.bExportable = TRUE;
		cs.iTrustLevel = 0;
		break;

	case SIG_TRUST :
		cs.bExportable = TRUE;
		cs.iTrustLevel = 1;
		break;

	case SIG_META :
		cs.bExportable = FALSE;
		cs.iTrustLevel = 2;
///		ccs.bSendToServer = FALSE;
		break;
	}

	// get valid passphrase
	LoadString (g_hInst, IDS_SIGNKEYPASSPHRASE, sz256, 256);
	uError = PGPcomdlgGetPhrase (pKM->Context, pKM->hWndParent, sz256, 
								&cs.szPhrase, pKM->KeySetMain, 
								NULL, 0, &cs.SigningKey, NULL, 0);

	if (uError == kPGPError_NoErr) {

		// update from server
		if (ccs.bSyncWithServer) {
			if (!KMGetFromServerInternal (pKM, FALSE, FALSE)) {
				if (KMMessageBox (pKM->hWndParent, IDS_CAPTION, 
									IDS_QUERYCONTINUESIGNING, 
									MB_YESNO|MB_ICONEXCLAMATION) == IDNO) {
					if (cs.szPhrase) {
						PGPcomdlgFreePhrase (cs.szPhrase);
						cs.szPhrase = NULL;
					}
					return FALSE;
				}
			}
		}

		// more than one item selected
		if (KMMultipleSelected (pKM)) {
			TreeList_IterateSelected (pKM->hWndTree, &cs);
		}

		// only one item selected
		else {
			tli.hItem = KMFocusedItem (pKM);
			tli.pszText = sz256;
			tli.cchTextMax = sizeof(sz256);
			tli.mask = TLIF_IMAGE | TLIF_PARAM | TLIF_TEXT;
			if (TreeList_GetItem (pKM->hWndTree, &tli)) {	
				CertifySingleObject (&tli, (LPARAM)&cs);
			}
		}

		// changes have been made; save them and update all validities
		if (cs.bItemModified) {
			KMCommitKeyRingChanges (pKM);
			KMLoadKeyRingIntoTree (pKM, FALSE, FALSE, FALSE);
			InvalidateRect (pKM->hWndTree, NULL, TRUE);

			// send key to server, if selected
			if (ccs.bSyncWithServer) {
				pKM->szPutKeyserver[0] = '\0';
				KMSendToServer (pKM);
			}
		}
	}

	if (cs.szPhrase) {
		PGPcomdlgFreePhrase (cs.szPhrase);
		cs.szPhrase = NULL;
	}
	return (cs.bItemModified);
}


//----------------------------------------------------|
//  Enable selected key

BOOL KMEnableKey (PKEYMAN pKM, PGPKeyRef Key) 
{
	TL_TREEITEM tli;

	if (!PGPcomdlgErrorMessage (PGPEnableKey (Key))) {
		KMGetKeyUserVal (pKM, Key, (long*)&tli.hItem);
		tli.iImage = KMDetermineKeyIcon (pKM, Key, NULL);
		tli.state = 0;
		tli.stateMask = TLIS_ITALICS;
		tli.mask = TLIF_IMAGE | TLIF_STATE;
		TreeList_SetItem (pKM->hWndTree, &tli);

		KMRepaintKeyInTree (pKM, Key);

		KMSetFocus (pKM, tli.hItem, FALSE);
		KMCommitKeyRingChanges (pKM);
		return TRUE;
	}
	return FALSE;
}


//----------------------------------------------------|
//  Disable selected key

BOOL 
KMDisableKey (PKEYMAN pKM, PGPKeyRef Key) 
{
	TL_TREEITEM tli;

	if (!PGPcomdlgErrorMessage (PGPDisableKey (Key))) {
		KMGetKeyUserVal (pKM, Key, (long*)&tli.hItem);
		tli.iImage = KMDetermineKeyIcon (pKM, Key, NULL);
		tli.state = TLIS_ITALICS;
		tli.stateMask = TLIS_ITALICS;
		tli.mask = TLIF_IMAGE | TLIF_STATE;
		TreeList_SetItem (pKM->hWndTree, &tli);

		KMRepaintKeyInTree (pKM, Key);

		KMSetFocus (pKM, tli.hItem, FALSE);
		KMCommitKeyRingChanges (pKM);
		return TRUE;
	}
	return FALSE;
}


//----------------------------------------------------|
//  Revoke selected key

BOOL 
KMRevokeKey (PKEYMAN pKM) 
{
	LPSTR		szRevPhrase		= NULL;
	BOOL		bRetVal			= TRUE;
	PGPBoolean	bNeedsPhrase;
	UINT		uError;
	PGPKeyRef	Key;
	PGPKeyRef	DefKey;
	CHAR		sz128[128];
	PGPBoolean	bSyncWithServer;
	PGPPrefRef	prefref;

	Key = (PGPKeyRef) KMFocusedObject (pKM);
	PGPGetDefaultPrivateKey (pKM->KeySetMain, &DefKey);

	PGPcomdlgOpenClientPrefs (&prefref);
	PGPGetPrefBoolean (prefref, kPGPPrefKeyServerSyncOnRevocation, 
						&bSyncWithServer);
	PGPcomdlgCloseClientPrefs (prefref, FALSE);

	if (Key == DefKey) {
		if (KMMessageBox (pKM->hWndParent, IDS_CAPTION, IDS_REVCONFDEFKEY,
			MB_YESNO|MB_TASKMODAL|MB_DEFBUTTON2|MB_ICONWARNING)==IDNO) 
			return FALSE;
	}
	else if (KMMessageBox (pKM->hWndParent, IDS_CAPTION, IDS_REVOKECONFIRM, 
						MB_YESNO|MB_ICONEXCLAMATION) == IDNO) 
			return FALSE;

	uError = PGPGetKeyBoolean ((PGPKeyRef) KMFocusedObject (pKM), 
						kPGPKeyPropNeedsPassphrase, &bNeedsPhrase);
	if (PGPcomdlgErrorMessage (uError)) return FALSE;

	// get valid passphrase, if required
	if (bNeedsPhrase) uError = kPGPError_BadPassphrase;
	while ((uError == kPGPError_BadPassphrase) && bNeedsPhrase) {
		LoadString (g_hInst, IDS_SELKEYPASSPHRASE, sz128, 128); 
		uError = PGPcomdlgGetPhrase (pKM->Context, pKM->hWndParent, sz128, 
							&szRevPhrase, NULL, NULL, 0, NULL, NULL, 0);
		if (uError == kPGPError_NoErr) {
			if (!PGPPassphraseIsValid (Key, szRevPhrase)) {
				uError = kPGPError_BadPassphrase;
				if (szRevPhrase) {
					PGPcomdlgFreePhrase (szRevPhrase);
					szRevPhrase = NULL;
				}
			}
		}
		PGPcomdlgErrorMessage (uError);
	}

	// now we have a valid passphrase, if required
	if (uError == kPGPError_NoErr) {

		// update from server
		if (bSyncWithServer) {
			if (!KMGetFromServerInternal (pKM, FALSE, FALSE)) {
				if (KMMessageBox (pKM->hWndParent, IDS_CAPTION, 
									IDS_QUERYCONTINUEREVOKINGKEY, 
									MB_YESNO|MB_ICONEXCLAMATION) == IDNO) {
					bRetVal = FALSE;
				}
			}
		}
		
		if (bRetVal) {

			// make sure we have enough entropy
			PGPcomdlgRandom (pKM->Context, pKM->hWndParent, 0);

			if (bNeedsPhrase) {
				uError = PGPRevokeKey (Key, 
						PGPOPassphrase (pKM->Context, szRevPhrase),
						PGPOLastOption (pKM->Context));
			}
			else {
				uError = PGPRevokeKey (Key, 
						PGPOLastOption (pKM->Context));
			}
						
			if (!PGPcomdlgErrorMessage (uError)) {
				KMCommitKeyRingChanges (pKM);
				KMUpdateKeyInTree (pKM, Key);
				KMUpdateAllValidities (pKM);
				InvalidateRect (pKM->hWndTree, NULL, TRUE);

				// send to server
				if (bSyncWithServer) {
					pKM->szPutKeyserver[0] = '\0';
					KMSendToServer (pKM);
				}
			}
			else bRetVal = FALSE;
		}
	}
	else bRetVal = FALSE;

	if (szRevPhrase) {
		PGPcomdlgFreePhrase (szRevPhrase);
		szRevPhrase = NULL;
	}

	return bRetVal;
}


//----------------------------------------------------|
// Revoke signature dialog message procedure

BOOL CALLBACK 
RevokeCertDlgProc (HWND hDlg, 
				   UINT uMsg, 								
				   WPARAM wParam, 
				   LPARAM lParam) 
{
	REVOKECERTSTRUCT* prcs;

	switch (uMsg) {

	case WM_INITDIALOG :
		{
			CHAR sz[kPGPMaxUserIDSize];
			SetWindowLong (hDlg, GWL_USERDATA, lParam);
			prcs = (REVOKECERTSTRUCT*)lParam;
			KMGetKeyIDFromKey (prcs->Key, sz, sizeof (sz));
			SetDlgItemText (hDlg, IDC_KEYID, sz);
			KMGetKeyName (prcs->Key, sz, sizeof (sz));
			SetDlgItemText (hDlg, IDC_NAME, sz);
		}
		break;

	case WM_COMMAND:

		switch (LOWORD(wParam)) {
		case IDCANCEL:
			prcs = (REVOKECERTSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
			prcs->bSyncWithServer = FALSE;
			EndDialog (hDlg, 1);
			break;

		case IDOK:
			prcs = (REVOKECERTSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
			EndDialog (hDlg, 0);
			break;

		case IDHELP :
			prcs = (REVOKECERTSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
			WinHelp (hDlg, prcs->pKM->szHelpFile, HELP_CONTEXT, 
						IDH_PGPKM_REVOKECERTDIALOG); 
			break;
		}
		return TRUE;
	}
	return FALSE;
}


//----------------------------------------------------|
//  Revoke selected Certification

BOOL 
KMRevokeCert (PKEYMAN pKM) 
{
	BOOL				bRetVal		= TRUE;
	LPSTR				szRevPhrase = NULL;

	TL_TREEITEM			tli;
	PGPSigRef			cert;
	PGPKeyRef			keySigning, keyParent;
	PGPBoolean			bNeedsPhrase;
	UINT				uError;
	CHAR				sz128[128];
	REVOKECERTSTRUCT	rcs;
	PGPPrefRef			prefref;
	PGPError			err;

	cert = (PGPSigRef) KMFocusedObject (pKM);

	err = PGPGetSigCertifierKey (cert, pKM->KeySetMain, &keySigning);
	if (err == kPGPError_ItemNotFound) {
		keySigning = NULL;
		err = kPGPError_NoErr;
	}

	if (!PGPcomdlgErrorMessage (err)) {
		if (!keySigning) {
			KMMessageBox (pKM->hWndParent, IDS_CAPTION, IDS_CERTKEYNOTONRING, 
							MB_OK|MB_ICONEXCLAMATION);
			return FALSE;
		}

		rcs.pKM = pKM;
		rcs.Key = KMGetKeyFromCert (pKM, cert);

		PGPcomdlgOpenClientPrefs (&prefref);
		PGPGetPrefBoolean (prefref, kPGPPrefKeyServerSyncOnRevocation, 
						&(rcs.bSyncWithServer));
		PGPcomdlgCloseClientPrefs (prefref, FALSE);

		if (DialogBoxParam (g_hInst, MAKEINTRESOURCE(IDD_REVOKECERT), 
							pKM->hWndParent, RevokeCertDlgProc, 
							(LPARAM)&rcs)) {
			return FALSE;
		}
		
		uError = PGPGetKeyBoolean (keySigning, 
						kPGPKeyPropNeedsPassphrase, &bNeedsPhrase);
		if (PGPcomdlgErrorMessage (uError)) return FALSE;

		// get valid passphrase, if required
		if (bNeedsPhrase) uError = kPGPError_BadPassphrase;
		while ((uError == kPGPError_BadPassphrase) && bNeedsPhrase) {
			LoadString (g_hInst, IDS_SIGNKEYPASSPHRASE, sz128, 128); 
			uError = PGPcomdlgGetPhrase (pKM->Context, pKM->hWndParent, sz128, 
							&szRevPhrase, NULL, NULL, 0, NULL, NULL, 0);
			if (uError == kPGPError_NoErr) {
				if (!PGPPassphraseIsValid (keySigning, szRevPhrase)) {
					uError = kPGPError_BadPassphrase;
					if (szRevPhrase) {
						PGPcomdlgFreePhrase (szRevPhrase);
						szRevPhrase = NULL;
					}
				}
			}
			PGPcomdlgErrorMessage (uError);
		}

		// now we have a valid passphrase, if required
		if (uError == kPGPError_NoErr) {

			// update from server
			if (rcs.bSyncWithServer) {
				if (!KMGetFromServerInternal (pKM, FALSE, FALSE)) {
					if (KMMessageBox (pKM->hWndParent, IDS_CAPTION, 
									IDS_QUERYCONTINUEREVOKINGCERT, 
									MB_YESNO|MB_ICONEXCLAMATION) == IDNO) {
						bRetVal = FALSE;
					}
				}
			}
		
			if (bRetVal) {

				// make sure we have enough entropy
				PGPcomdlgRandom (pKM->Context, pKM->hWndParent, 0);

				if (bNeedsPhrase) {
					uError = PGPRevokeSig (
						(PGPSigRef) KMFocusedObject (pKM), 
						pKM->KeySetMain,
						PGPOPassphrase (pKM->Context, szRevPhrase),
						PGPOLastOption (pKM->Context));
				}
				else {
					uError = PGPRevokeSig (
						(PGPSigRef) KMFocusedObject (pKM), 
						pKM->KeySetMain,
						PGPOLastOption (pKM->Context));
				}
						
				if (!PGPcomdlgErrorMessage (uError)) {
					tli.hItem = KMFocusedItem (pKM);
					tli.iImage = IDX_REVCERT;
					tli.state = TLIS_ITALICS;
					tli.stateMask = TLIS_ITALICS;
					tli.mask = TLIF_IMAGE | TLIF_STATE;
					TreeList_SetItem (pKM->hWndTree, &tli);

					keyParent = KMGetKeyFromCert (pKM, cert);
					KMRepaintKeyInTree (pKM, keyParent);

					KMSetFocus (pKM, tli.hItem, FALSE);
					KMCommitKeyRingChanges (pKM);
					KMUpdateAllValidities (pKM);

					// send key to server, if selected
					if (rcs.bSyncWithServer) {
						pKM->szPutKeyserver[0] = '\0';
						KMSendToServer (pKM);
					}
				}
				else bRetVal = FALSE;
			}
		}
		else bRetVal = FALSE;
	}
	else bRetVal = FALSE;

	if (szRevPhrase) {
		PGPcomdlgFreePhrase (szRevPhrase);
		szRevPhrase = NULL;
	}

	return bRetVal;
}


//----------------------------------------------------|
// Delete All dialog message procedure

BOOL CALLBACK 
DeleteAllDlgProc (HWND hWndDlg, 
				  UINT uMsg, 
				  WPARAM wParam, 
				  LPARAM lParam) 
{
	DELETESTRUCT* pds;

	switch (uMsg) {

	case WM_INITDIALOG :
		SetWindowLong (hWndDlg, GWL_USERDATA, lParam);
		pds = (DELETESTRUCT*)lParam;
		SetDlgItemText (hWndDlg, IDC_STRING, pds->pszPrompt);
		break;

    case WM_HELP: 
		pds = (DELETESTRUCT*)GetWindowLong (hWndDlg, GWL_USERDATA);
        WinHelp (((LPHELPINFO) lParam)->hItemHandle, pds->pKM->szHelpFile, 
            HELP_WM_HELP, (DWORD) (LPSTR) aDeleteAllIds); 
        break; 
 
    case WM_CONTEXTMENU:
		pds = (DELETESTRUCT*)GetWindowLong (hWndDlg, GWL_USERDATA);
        WinHelp ((HWND) wParam, pds->pKM->szHelpFile, HELP_CONTEXTMENU, 
            (DWORD) (LPVOID) aDeleteAllIds); 
        break; 

	case WM_COMMAND:

		switch (LOWORD(wParam)) {
		case IDCANCEL:
			pds = (DELETESTRUCT*)GetWindowLong (hWndDlg, GWL_USERDATA);
			pds->bItemNotDeleted = TRUE;
			EndDialog (hWndDlg, IDCANCEL);
			break;

		case IDOK:
		case IDYES:
			EndDialog (hWndDlg, IDYES);
			break;

		case IDNO:
			EndDialog (hWndDlg, IDNO);
			break;

		case IDC_YESTOALL :
			pds = (DELETESTRUCT*)GetWindowLong (hWndDlg, GWL_USERDATA);
			pds->bDeleteAll = TRUE;
			EndDialog (hWndDlg, IDYES);
			break;
		}
		return TRUE;
	}
	return FALSE;
}


//----------------------------------------------------|
//  Ask user for delete confirmation

INT 
DeleteConfirm (TL_TREEITEM* lptli, 
			   INT iPromptID, 
			   DELETESTRUCT* pds) 
{
	CHAR sz256[256];
	CHAR sz512[512];
	INT iRetVal;

	if (pds->bDeleteAll) return IDYES;

	LoadString (g_hInst, iPromptID, sz256, 256); 
	wsprintf (sz512, sz256, lptli->pszText);

	if (KMMultipleSelected (pds->pKM)) {
		pds->pszPrompt = sz512;
		iRetVal = DialogBoxParam (g_hInst, MAKEINTRESOURCE (IDD_DELETEALL), 
			pds->pKM->hWndParent, DeleteAllDlgProc, (LPARAM)pds);
		if (!pds->bItemNotDeleted && (iRetVal == IDNO)) {
			pds->bItemNotDeleted = TRUE;
			pds->hPostDeleteFocusItem = lptli->hItem;
		}
	}
	else {
		LoadString (g_hInst, IDS_DELCONFCAPTION, sz256, 256);
		iRetVal = MessageBox (pds->pKM->hWndParent, sz512, sz256,
				MB_YESNO|MB_TASKMODAL|MB_DEFBUTTON2|MB_ICONWARNING);
	}

	return iRetVal;
}


//----------------------------------------------------|
//  Get handle of nearby item
//
//	lptli	= pointer to TreeList item

HTLITEM 
GetAdjacentItem (HWND hWndTree, 
				 TL_TREEITEM* lptli) 
{
	TL_TREEITEM tli;

	tli.hItem = lptli->hItem;
	tli.mask = TLIF_NEXTHANDLE;
	TreeList_GetItem (hWndTree, &tli);
	if (!tli.hItem) {
		tli.hItem = lptli->hItem;
		tli.mask = TLIF_PREVHANDLE;
		TreeList_GetItem (hWndTree, &tli);
		if (!tli.hItem) {
			tli.hItem = lptli->hItem;
			tli.mask = TLIF_PARENTHANDLE;
			TreeList_GetItem (hWndTree, &tli);
		}
	}

	return tli.hItem;
}

	
//----------------------------------------------------|
//  Delete a single object
//	routine called either from KMDeleteObject or as a
//	callback function from the TreeList control to 
//	delete a single item.
//
//	lptli	= pointer to TreeList item to delete

BOOL CALLBACK 
DeleteSingleObject (TL_TREEITEM* lptli, 
					LPARAM lParam) 
{
	DELETESTRUCT*	pds			= (DELETESTRUCT*)lParam;
	PGPKeySetRef	KeySet;
	PGPKeyRef		Key;
	PGPUserIDRef	UserID;
	BOOL			bPrimary;
	INT iError,		iConfirm;
	LONG			lVal;

	switch (lptli->iImage) {
	case IDX_RSASECKEY :
	case IDX_RSASECDISKEY :
	case IDX_DSASECKEY :
	case IDX_DSASECDISKEY :
		iConfirm = DeleteConfirm (lptli, IDS_DELCONFPRIVKEY, pds);
		if (iConfirm == IDYES) {
			if ((PGPKeyRef)(lptli->lParam) == pds->DefaultKey) {
				if (KMMessageBox (pds->pKM->hWndParent, IDS_CAPTION, 
					IDS_DELCONFDEFKEY,
					MB_YESNO|MB_TASKMODAL|MB_DEFBUTTON2|MB_ICONWARNING)
					==IDNO) 
						return TRUE;
			}
			PGPNewSingletonKeySet ((PGPKeyRef)(lptli->lParam),
									&KeySet);
			KMGetKeyUserVal (pds->pKM, (PGPKeyRef)(lptli->lParam), &lVal);
			KMSetKeyUserVal (pds->pKM, (PGPKeyRef)(lptli->lParam), 0);
			if (!PGPcomdlgErrorMessage (PGPRemoveKeys (
					KeySet, pds->pKM->KeySetDisp))) {
				KMDeletePropertiesKey (pds->pKM, (PGPKeyRef)(lptli->lParam));
				pds->bItemModified = TRUE;
				if (!pds->bItemNotDeleted) 
					pds->hPostDeleteFocusItem = 
							GetAdjacentItem (pds->pKM->hWndTree, lptli); 
				TreeList_DeleteItem (pds->pKM->hWndTree, lptli);
			}
			else 
				KMSetKeyUserVal (pds->pKM, (PGPKeyRef)(lptli->lParam), lVal);

			PGPFreeKeySet (KeySet);
		}
		if (iConfirm == IDCANCEL) return FALSE; 
		else return TRUE;

	case IDX_RSASECREVKEY :
	case IDX_RSASECEXPKEY :
	case IDX_DSASECREVKEY :
	case IDX_DSASECEXPKEY :
		iConfirm = DeleteConfirm (lptli, IDS_DELCONFPRIVKEY, pds);
		if (iConfirm == IDYES) {
			PGPNewSingletonKeySet ((PGPKeyRef)(lptli->lParam),
									&KeySet);
			KMGetKeyUserVal (pds->pKM, (PGPKeyRef)(lptli->lParam), &lVal);
			KMSetKeyUserVal (pds->pKM, (PGPKeyRef)(lptli->lParam), 0);
			if (!PGPcomdlgErrorMessage (PGPRemoveKeys (
					KeySet, pds->pKM->KeySetDisp))) {
				KMDeletePropertiesKey (pds->pKM, (PGPKeyRef)(lptli->lParam));
				pds->bItemModified = TRUE;
				if (!pds->bItemNotDeleted) 
					pds->hPostDeleteFocusItem = 
							GetAdjacentItem (pds->pKM->hWndTree, lptli); 
				TreeList_DeleteItem (pds->pKM->hWndTree, lptli);
			}
			else 
				KMSetKeyUserVal (pds->pKM, (PGPKeyRef)(lptli->lParam), lVal);

			PGPFreeKeySet (KeySet);
		}
		if (iConfirm == IDCANCEL) return FALSE; 
		else return TRUE;

	case IDX_RSAPUBKEY :
	case IDX_RSAPUBDISKEY :
	case IDX_RSAPUBREVKEY :
	case IDX_RSAPUBEXPKEY :
	case IDX_DSAPUBKEY :
	case IDX_DSAPUBDISKEY :
	case IDX_DSAPUBREVKEY :
	case IDX_DSAPUBEXPKEY :
		iConfirm = DeleteConfirm (lptli, IDS_DELCONFKEY, pds);
		if (iConfirm == IDYES) {
			PGPNewSingletonKeySet ((PGPKeyRef)(lptli->lParam), 
									&KeySet);
			KMGetKeyUserVal (pds->pKM, (PGPKeyRef)(lptli->lParam), &lVal);
			KMSetKeyUserVal (pds->pKM, (PGPKeyRef)(lptli->lParam), 0);
			if (!PGPcomdlgErrorMessage (PGPRemoveKeys (
								KeySet, pds->pKM->KeySetDisp))) {
				KMDeletePropertiesKey (pds->pKM, (PGPKeyRef)(lptli->lParam));
				pds->bItemModified = TRUE;
				if (!pds->bItemNotDeleted) 
					pds->hPostDeleteFocusItem = 
							GetAdjacentItem (pds->pKM->hWndTree, lptli); 
				TreeList_DeleteItem (pds->pKM->hWndTree, lptli);
			}
			else
				KMSetKeyUserVal (pds->pKM, (PGPKeyRef)(lptli->lParam), lVal);
			PGPFreeKeySet (KeySet);
		}
		if (iConfirm == IDCANCEL) return FALSE;
		else return TRUE;

	case IDX_RSAUSERID :
	case IDX_DSAUSERID :
		if (KMIsThisTheOnlyUserID (pds->pKM, 
						(PGPUserIDRef)(lptli->lParam))) {
			KMMessageBox (pds->pKM->hWndParent, IDS_CAPTION, 
						IDS_DELONLYUSERID, MB_OK|MB_ICONEXCLAMATION);
			break;
		}

		iConfirm = DeleteConfirm (lptli, IDS_DELCONFUSERID, pds);
		if (iConfirm == IDYES) {
			Key = KMGetKeyFromUserID (pds->pKM, 
											(PGPUserIDRef)(lptli->lParam));

			PGPGetPrimaryUserID (Key, &UserID);
			if (UserID == (PGPUserIDRef)(lptli->lParam)) 
				bPrimary = TRUE;
			else 
				bPrimary = FALSE;

			KMGetUserIDUserVal (pds->pKM,(PGPUserIDRef)(lptli->lParam),&lVal);
			KMSetUserIDUserVal (pds->pKM,(PGPUserIDRef)(lptli->lParam),0);
			iError = PGPRemoveUserID ((PGPUserIDRef)(lptli->lParam));
			switch (iError) {			
			case kPGPError_NoErr :
				pds->bItemModified = TRUE;
				if (bPrimary) {
					pds->bDeletedPrimaryUserID = TRUE;
				}
				else { 
					if (!pds->bItemNotDeleted) 
						pds->hPostDeleteFocusItem = 
							GetAdjacentItem (pds->pKM->hWndTree, lptli); 
					TreeList_DeleteItem (pds->pKM->hWndTree, lptli);
				}
				break;
				
			default :
				PGPcomdlgErrorMessage (iError);
				KMSetUserIDUserVal (pds->pKM, 
									(PGPUserIDRef)(lptli->lParam), lVal);
				break;
			}
		}
		if (iConfirm == IDCANCEL) return FALSE;
		else return TRUE;

	case IDX_CERT :
	case IDX_REVCERT :
	case IDX_BADCERT :
	case IDX_EXPORTCERT :
	case IDX_METACERT :
	case IDX_EXPORTMETACERT :
		iConfirm = DeleteConfirm (lptli, IDS_DELCONFCERT, pds);
		if (iConfirm == IDYES) {
			KMGetCertUserVal (pds->pKM, (PGPSigRef)(lptli->lParam), &lVal);
			KMSetCertUserVal (pds->pKM, (PGPSigRef)(lptli->lParam), 0);
			if (!PGPcomdlgErrorMessage (PGPRemoveSig (
											(PGPSigRef)(lptli->lParam)))) {
				pds->bItemModified = TRUE;
				if (!pds->bItemNotDeleted) 
					pds->hPostDeleteFocusItem = 
							GetAdjacentItem (pds->pKM->hWndTree, lptli); 
				TreeList_DeleteItem (pds->pKM->hWndTree, lptli);
			}
			else
				KMSetCertUserVal (pds->pKM, (PGPSigRef)(lptli->lParam), lVal);
		}
		if (iConfirm == IDCANCEL) return FALSE;
		else return TRUE;
	}

	return FALSE;
}


//----------------------------------------------------|
//  Delete selected key or keys

BOOL 
KMDeleteObject (PKEYMAN pKM) 
{
	TL_TREEITEM		tli;
	CHAR			sz256[256];
	DELETESTRUCT	ds;

	ds.lpfnCallback = DeleteSingleObject;
	ds.Context = pKM->Context;
	ds.pKM = pKM;
	ds.bItemModified = FALSE;
	ds.bDeleteAll = FALSE;
	ds.bItemNotDeleted = FALSE;
	ds.bDeletedPrimaryUserID = FALSE;
	ds.hPostDeleteFocusItem = NULL;
	PGPGetDefaultPrivateKey (pKM->KeySetMain, &ds.DefaultKey);

	if (KMMultipleSelected (pKM)) {
		TreeList_IterateSelected (pKM->hWndTree, &ds);
	}
	else {
		tli.hItem = KMFocusedItem (pKM);
		tli.pszText = sz256;
		tli.cchTextMax = 256;
		tli.mask = TLIF_IMAGE | TLIF_TEXT | TLIF_PARAM;
		if (!TreeList_GetItem (pKM->hWndTree, &tli)) return FALSE;	
		DeleteSingleObject (&tli, (LPARAM)&ds);
	}

	if (ds.bItemModified) {
		KMCommitKeyRingChanges (pKM);
		if (ds.bDeletedPrimaryUserID) {
			TreeList_DeleteTree (pKM->hWndTree, TRUE);
			KMAddColumns (pKM);
			KMLoadKeyRingIntoTree (pKM, FALSE, FALSE, TRUE);
		}
		else {
			KMLoadKeyRingIntoTree (pKM, FALSE, FALSE, FALSE);

			if (ds.bItemNotDeleted) {
				if (ds.hPostDeleteFocusItem) {
					tli.hItem = ds.hPostDeleteFocusItem;
					TreeList_Select (pKM->hWndTree, &tli, FALSE);
				}
				else KMSetFocus (pKM, NULL, FALSE);
			}
			else {
				if (ds.hPostDeleteFocusItem) {
					tli.hItem = ds.hPostDeleteFocusItem;
					TreeList_Select (pKM->hWndTree, &tli, TRUE);
					tli.stateMask = TLIS_SELECTED;
					tli.state = 0;
					tli.mask = TLIF_STATE;
					TreeList_SetItem (pKM->hWndTree, &tli);
				}
				else KMSetFocus (pKM, NULL, FALSE);
			}
		}
		InvalidateRect (pKM->hWndTree, NULL, TRUE);
	}
	return (ds.bItemModified);
}


//----------------------------------------------------|
// Set focused key to be default signing key

BOOL 
KMSetDefaultKey (PKEYMAN pKM) 
{
	PGPKeyRef	Key;
	PGPKeyRef	DefKey;
	TL_TREEITEM tli;

	PGPGetDefaultPrivateKey (pKM->KeySetMain, &DefKey);
	Key = (PGPKeyRef) KMFocusedObject (pKM);

	if (!PGPcomdlgErrorMessage (PGPSetDefaultPrivateKey (Key))) {
		// set old default to non-bold
		if (DefKey) {
			KMGetKeyUserVal (pKM, DefKey, (long*)&(tli.hItem));
			tli.state = 0;
			tli.stateMask = TLIS_BOLD;
			tli.mask = TLIF_STATE;
			TreeList_SetItem (pKM->hWndTree, &tli);
		}
		// set new default to bold
		tli.hItem = KMFocusedItem (pKM);
		tli.state = TLIS_BOLD;
		tli.stateMask = TLIS_BOLD;
		tli.mask = TLIF_STATE;
		TreeList_SetItem (pKM->hWndTree, &tli);
		PGPcomdlgErrorMessage (PGPsdkSavePrefs (pKM->Context));
		KMCommitKeyRingChanges (pKM);
		KMUpdateAllValidities (pKM);
		return TRUE;
	}
	return FALSE;
}


//----------------------------------------------------|
// Set focused UID to be primary UID

BOOL 
KMSetPrimaryUserID (PKEYMAN pKM) 
{
	PGPUserIDRef	UserID;
	PGPKeyRef		Key;

	UserID = (PGPUserIDRef) KMFocusedObject (pKM);

	if (!PGPcomdlgErrorMessage (PGPSetPrimaryUserID (UserID))) {
		KMCommitKeyRingChanges (pKM);
		Key = KMGetKeyFromUserID (pKM, UserID);
		KMUpdateKeyInTree (pKM, Key);
		InvalidateRect (pKM->hWndTree, NULL, TRUE);
		return TRUE;
	}
	return FALSE;
}


//----------------------------------------------------|
//	routine called as a callback function from the 
//  TreeList control to add single key to keyset for export
//
//	lptli	= pointer to TreeList item to add

BOOL CALLBACK 
AddKeyToAddKeySet (TL_TREEITEM* lptli, 
				   LPARAM lParam) 
{
	ADDTOMAINSTRUCT*	pas		= (ADDTOMAINSTRUCT*)lParam;
	PGPKeySetRef		KeySet;

	PGPNewSingletonKeySet ((PGPKeyRef)(lptli->lParam),
							&KeySet);
	PGPcomdlgErrorMessage (PGPAddKeys (KeySet, pas->AddKeySet));
	PGPCommitKeyRingChanges (pas->AddKeySet);
	PGPFreeKeySet (KeySet);

	return TRUE;
}

	
//	____________________________________
//
//	Get HWND of PGPkeys application

HWND 
GetPGPkeysWindow (VOID)
{
    HWND			hWndMe		= NULL;
    HANDLE			hSem;

    // Create or open a named semaphore. 
    hSem = CreateSemaphore (NULL, 0, 1, SEMAPHORENAME);

    // return HWND if existing semaphore was opened.
    if (hSem != NULL) {
		if (GetLastError() == ERROR_ALREADY_EXISTS) {
		    hWndMe = FindWindow (WINCLASSNAME, WINDOWTITLE);
		}
        CloseHandle(hSem);
	}

	return hWndMe;
}

//----------------------------------------------------|
// Add selected keys to main keyset

BOOL 
KMAddSelectedToMain (PKEYMAN pKM) 
{
	PGPError			err			= 0;
	PGPKeySetRef		keysetMain;
	ADDTOMAINSTRUCT		as;
	UINT				uReloadMessage;
	HWND				hwndPGPkeys;
	LPVOID				pBuffer;
	INT					slen;
	COPYDATASTRUCT		cds;
	HCURSOR				hCursorOld;


	if (KMMultipleSelected (pKM)) {
		PGPNewKeySet (pKM->Context, &(as.AddKeySet));
		if (as.AddKeySet) {
			as.lpfnCallback = AddKeyToAddKeySet;
			as.Context = pKM->Context;
			TreeList_IterateSelected (pKM->hWndTree, &as);
		}
	}
	else {
		PGPNewSingletonKeySet((PGPKeyRef)KMFocusedObject (pKM),
								&(as.AddKeySet));
	}

	if (pKM->bMainKeySet) {
		if (PGPcomdlgErrorMessage (
				PGPAddKeys (as.AddKeySet, pKM->KeySetMain))) {
			PGPFreeKeySet (as.AddKeySet);
			return FALSE;
		}
	}
	// no main keyset, we're supposed to try to add it to default keyring
	else {
		hwndPGPkeys = GetPGPkeysWindow ();
		// PGPkeys is running ... send it the key block
		if (hwndPGPkeys) {
			if (!PGPcomdlgErrorMessage (
				PGPExportKeySet (as.AddKeySet, 
							PGPOAllocatedOutputBuffer (pKM->Context,
									&pBuffer, 0x40000000, &slen),
							PGPOLastOption (pKM->Context)))) {
				cds.dwData = PGPPK_IMPORTKEYBUFFER;
				cds.cbData = slen+1;
				cds.lpData = pBuffer;
				err = SendMessage (hwndPGPkeys , WM_COPYDATA, 
													0, (LPARAM)&cds);
				if (err) err = kPGPError_NoErr;
				else err = kPGPError_UnknownError;
				PGPFreeData (pBuffer);
			}
		}

		// PGPkeys is not running ... try to add keys to default keyring
		else {
			PGPsdkLoadDefaultPrefs (pKM->Context);
			err = PGPOpenDefaultKeyRings (	pKM->Context, 
											kPGPKeyRingOpenFlags_Mutable, 
											&keysetMain);
			if (IsntPGPError (err) && keysetMain) {
				err = PGPAddKeys (as.AddKeySet, keysetMain);
				if (IsntPGPError (err)) {
					err = PGPCommitKeyRingChanges (keysetMain);
					if (IsntPGPError (err)) {
						uReloadMessage = 
							RegisterWindowMessage (RELOADKEYRINGMSG);
						PostMessage (HWND_BROADCAST, uReloadMessage, 
							MAKEWPARAM (LOWORD (pKM->hWndParent), FALSE), 
							GetCurrentProcessId ());
					}
				}
				PGPFreeKeySet (keysetMain);
			}
			PGPcomdlgErrorMessage (err);
		}
		PGPFreeKeySet (as.AddKeySet);
		return (IsntPGPError (err));
	}

	hCursorOld = SetCursor (LoadCursor (NULL, IDC_WAIT));
	err = PGPCommitKeyRingChanges (pKM->KeySetMain);
	SetCursor (hCursorOld);
	PGPFreeKeySet (as.AddKeySet);

	if (!PGPcomdlgErrorMessage (err)) {
		uReloadMessage = RegisterWindowMessage (RELOADKEYRINGMSG);
		PostMessage (HWND_BROADCAST, uReloadMessage, 
				MAKEWPARAM (LOWORD (pKM->hWndParent), TRUE), 
				GetCurrentProcessId ());
	}

	return (IsntPGPError (err));
}

