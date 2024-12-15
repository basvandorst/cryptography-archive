/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	KMKeyOps.c - implements various operations performed on keys. 
	

	$Id: KMKeyOps.c,v 1.55 1999/04/14 23:23:45 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

// project header files
#include "pgpkmx.h"
#include "pgpclx.h"

// constant definitions
#define BITMAP_WIDTH	16
#define BITMAP_HEIGHT	16

#define INITIAL_SIGN_COLUMNWIDTH	210

#define SIG_NONEXPORTABLE	0
#define SIG_EXPORTABLE		1
#define SIG_TRUST			2
#define SIG_META			3

// external globals  
extern HINSTANCE g_hInst;

// typedefs
typedef struct {
	FARPROC			lpfnCallback;
	PKEYMAN			pKM;
	BOOL			bItemModified;
	PGPKeyRef		keySigning;
	PGPByte*		pPasskey;
	PGPSize			sizePasskey;
	PGPBoolean		bExportable;
	PGPUInt32		uTrustLevel;
	PGPUInt32		uExpireDays;
	CHAR			szRegExpression[256];
} CERTIFYSTRUCT, *PCERTIFYSTRUCT;

typedef struct {
	FARPROC			lpfnCallback;
	PGPContextRef	context;
	PKEYMAN			pKM;
	LPSTR			pszPrompt;
	BOOL			bItemModified;
	BOOL			bItemNotDeleted;
	BOOL			bDeleteAll;
	BOOL			bDeletedPrimaryUserID;
	PGPKeyRef		keyDefault;
	HTLITEM			hPostDeleteFocusItem;
} DELETESTRUCT, *PDELETESTRUCT;

typedef struct {
	FARPROC			lpfnCallback;
	PKEYMAN			pKM;
	PGPBoolean		bSyncWithServer;
	INT				iSigType;
	HWND			hwndList;
	HWND			hwndDTPicker;
	HIMAGELIST		hIml;
	INT				iItem;
	BOOL			bExpires;
	PGPInt32		iExpireDays;
	BOOL			bExpandedChoices;
	CHAR			szDomain[120];
} CERTIFYCONFIRMSTRUCT, *PCERTIFYCONFIRMSTRUCT;

typedef struct {
	HWND			hwndParent;
	HWND			hwndProgress;
	INT				iNumSigsTotal;
	BOOL			bCancel;
} REVERIFYSTRUCT, *PREVERIFYSTRUCT;

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
	IDC_EXPORTABLECHECK,IDH_PGPKM_ALLOWSIGEXPORT,
	IDC_FEWERCHOICES,	IDH_PGPKM_FEWERSIGCHOICES,
	IDC_NONEXPORTABLE,	IDH_PGPKM_SIGNONEXPORTABLE,
	IDC_EXPORTABLE,		IDH_PGPKM_SIGEXPORTABLE,
	IDC_TRUSTED,		IDH_PGPKM_SIGTRUSTED,
	IDC_META,			IDH_PGPKM_SIGMETA,
	IDC_DOMAIN,			IDH_PGPKM_DOMAINRESTRICTION,
	IDC_NEVEREXPIRES,	IDH_PGPKM_SIGNEVEREXPIRES,
	IDC_EXPIRES,		IDH_PGPKM_SIGEXPIRES,
	IDC_EXPIRATIONDATE,	IDH_PGPKM_SIGEXPIRATIONDATE,
    0,0 
}; 


//	_______________________________________________
//
//  Certify a single object
//	routine called either from KMCertifyKeyOrUserID or as a
//	callback function from the TreeList control to 
//	certify a single item.
//
//	lptli	= pointer to TreeList item to certify

static BOOL CALLBACK 
sCertifySingleObject (TL_TREEITEM* lptli, 
					 LPARAM lParam) 
{
	PCERTIFYSTRUCT	pcs			= (PCERTIFYSTRUCT)lParam;
	PGPError		err			= kPGPError_NoErr;

	PGPKeyRef		key;	
	PGPUserIDRef	userid;
	PGPContextRef	context;
	CHAR			sz512[512];
	CHAR			sz256[256];
	CHAR			sz64[64];

	switch (lptli->iImage) {
	case IDX_RSAPUBKEY :
	case IDX_RSAPUBDISKEY :
	case IDX_RSASECKEY :
	case IDX_RSASECDISKEY :
	case IDX_RSASECSHRKEY :
	case IDX_DSAPUBKEY :
	case IDX_DSAPUBDISKEY :
	case IDX_DSASECKEY :
	case IDX_DSASECDISKEY :
	case IDX_DSASECSHRKEY :
		key = (PGPKeyRef)(lptli->lParam);
		PGPGetPrimaryUserID (key, &userid);
		break;

	case IDX_RSAUSERID :
	case IDX_DSAUSERID :
	case IDX_PHOTOUSERID :
		userid = (PGPUserIDRef)(lptli->lParam);
		key = KMGetKeyFromUserID (pcs->pKM, userid);
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

	context = pcs->pKM->Context;

	// make sure we have enough entropy
	PGPclRandom (context, pcs->pKM->hWndParent, 0);

	err = PGPSignUserID (userid, pcs->keySigning, 
			PGPOExpiration (context, pcs->uExpireDays),
			PGPOExportable (context, pcs->bExportable),
			PGPOSigTrust (context, pcs->uTrustLevel, 
								kPGPKeyTrust_Complete),
			(pcs->pPasskey) ?
				PGPOPasskeyBuffer (context, pcs->pPasskey, pcs->sizePasskey) :
				PGPONullOption (context),
			(pcs->szRegExpression[0]) ?
				PGPOSigRegularExpression (context, pcs->szRegExpression) :
				PGPONullOption (context),
			PGPOLastOption (context));

	if (IsntPGPError (err)) {
		pcs->bItemModified = TRUE;
		return TRUE;
	}

	LoadString (g_hInst, IDS_CERTIFYERROR, sz64, 64); 
	PGPclErrorToString (err, sz256, 256);
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


//	_______________________________________________
//
//  Populate ListView with userids to sign

static BOOL CALLBACK 
sInsertOneID (TL_TREEITEM* lptli, LPARAM lParam) 
{
	PCERTIFYCONFIRMSTRUCT pccs = (PCERTIFYCONFIRMSTRUCT)lParam;

	PGPKeyRef		key;	
	PGPUserIDRef	userid;
	UINT			u, uAlgorithm;
	LV_ITEM			lvI;
	PGPByte			fingerprintBytes[256];
	CHAR			sz[kPGPMaxUserIDSize];

	switch (lptli->iImage) {
	case IDX_RSAPUBKEY :
	case IDX_RSAPUBDISKEY :
	case IDX_RSASECKEY :
	case IDX_RSASECDISKEY :
	case IDX_RSASECSHRKEY :
	case IDX_DSAPUBKEY :
	case IDX_DSAPUBDISKEY :
	case IDX_DSASECKEY :
	case IDX_DSASECDISKEY :
	case IDX_DSASECSHRKEY :
		key = (PGPKeyRef)(lptli->lParam);
		KMGetKeyName (key, sz, sizeof(sz));
		break;

	case IDX_RSAUSERID :
	case IDX_DSAUSERID :
	case IDX_PHOTOUSERID :
		userid = (PGPUserIDRef)(lptli->lParam);
		key = KMGetKeyFromUserID (pccs->pKM, userid);
		KMGetUserIDName (userid, sz, sizeof(sz));
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

	PGPGetKeyNumber (key, kPGPKeyPropAlgID, &uAlgorithm);
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

	if (ListView_InsertItem (pccs->hwndList, &lvI) == -1) return FALSE;

	PGPGetKeyPropertyBuffer (key, kPGPKeyPropFingerprint,
					sizeof(fingerprintBytes), fingerprintBytes, &u);
	KMConvertStringFingerprint (uAlgorithm, fingerprintBytes);
	ListView_SetItemText (pccs->hwndList, pccs->iItem, 1, fingerprintBytes);

	(pccs->iItem)++;
	
	return TRUE;
}


//	_______________________________________________
//
//  Populate ListView with userids to sign

static VOID 
sFillKeyList (PCERTIFYCONFIRMSTRUCT pccs) 
{

	LV_COLUMN	lvC; 
	CHAR		sz[256];
	HBITMAP		hBmp;
	HDC			hDC;
	INT			iNumBits;

	// create image list
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

	ListView_SetImageList (pccs->hwndList, pccs->hIml, LVSIL_SMALL);

	lvC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lvC.fmt = LVCFMT_LEFT; 
	lvC.pszText = sz;

	LoadString (g_hInst, IDS_USERID, sz, sizeof(sz));
	lvC.cx = INITIAL_SIGN_COLUMNWIDTH;   
	lvC.iSubItem = 0;
	if (ListView_InsertColumn (pccs->hwndList, 0, &lvC) == -1) return;

	LoadString (g_hInst, IDS_FINGERPRINT, sz, sizeof(sz));
	lvC.cx = 360;   
	lvC.iSubItem = 1;
	if (ListView_InsertColumn (pccs->hwndList, 1, &lvC) == -1) return;

	// populate control by iterating through selected items
	pccs->lpfnCallback = sInsertOneID;
	pccs->iItem = 0;
	TreeList_IterateSelected (pccs->pKM->hWndTree, pccs);
}


//	_______________________________________________
//
//	Convert domain string to regular expression

static VOID
sDomainToRegExpression (
	LPSTR		szDomain,
	LPSTR		szRegExp)
{
	LPSTR 		pszSrc		= szDomain;
	LPSTR		pszDst		= szRegExp;
	
	lstrcpy (pszDst, "<[^>]+[@.]");
	pszDst += lstrlen (pszDst);

	for ( ; *pszSrc; pszSrc++)
	{
		switch (*pszSrc)
		{
			case '*':
			case '+':
			case '?':
			case '.':
			case '^':
			case '$':
			case '\\':
			case '[':
			case ']':
			case '-':
				*pszDst++ = '\\';
				*pszDst++ = *pszSrc;
				break;
			default:
				*pszDst++ = *pszSrc;
				break;
		}
	}
	*pszDst++ = '>';
	*pszDst++ = '$';
	*pszDst++ = '\0';
}

//	_______________________________________________
//
//	Sign key dialog message procedure

static BOOL CALLBACK 
sSignKeyDlgProc (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	PCERTIFYCONFIRMSTRUCT	pccs;
	INT						iNewY, iNewWindowHeight, iOffset;
	HWND					hwndControl;
	RECT					rectControl;
	RECT					rc;

	switch (uMsg) {

	case WM_INITDIALOG :
	{
		SYSTEMTIME	st;

		SetWindowLong (hDlg, GWL_USERDATA, lParam);
		pccs = (PCERTIFYCONFIRMSTRUCT)lParam;
		pccs->hwndList = GetDlgItem (hDlg, IDC_KEYLIST);

		EnableWindow (GetDlgItem (hDlg, IDC_MORECHOICES), TRUE);
		ShowWindow (GetDlgItem (hDlg, IDC_MORECHOICES), SW_SHOW);
		pccs->bExpandedChoices = FALSE;

		sFillKeyList (pccs);

		// create and initialize date/time picker control
		GetWindowRect (GetDlgItem (hDlg, IDC_EXPIRATIONDATE), &rc);
		MapWindowPoints (NULL, hDlg, (LPPOINT)&rc, 2);
		pccs->hwndDTPicker = CreateWindowEx (0, DATETIMEPICK_CLASS,
                             "DateTime",
                             WS_BORDER|WS_CHILD|WS_TABSTOP,
                             rc.left, rc.top, 
							 rc.right-rc.left, rc.bottom-rc.top, 
							 hDlg, (HMENU)IDC_EXPIRATIONDATE, 
							 g_hInst, NULL);
		SetWindowPos (pccs->hwndDTPicker, 
						GetDlgItem (hDlg, IDC_EXPIRATIONDATE),
						0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
		SendMessage (pccs->hwndDTPicker, DTM_SETMCCOLOR, 
						MCSC_MONTHBK, (LPARAM)GetSysColor (COLOR_3DFACE));

		// initialize to one year from today
		GetLocalTime (&st);
		st.wYear++;
		SendMessage (pccs->hwndDTPicker, DTM_SETSYSTEMTIME,
							GDT_VALID, (LPARAM)&st);

		EnableWindow (pccs->hwndDTPicker, FALSE);
		CheckDlgButton (hDlg, IDC_NEVEREXPIRES, BST_CHECKED);
		
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
	}

    case WM_HELP: 
		pccs = (PCERTIFYCONFIRMSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
        WinHelp (((LPHELPINFO) lParam)->hItemHandle, pccs->pKM->szHelpFile, 
            HELP_WM_HELP, (DWORD) (LPSTR) aSignKeyIds); 
        break; 
 
    case WM_CONTEXTMENU:
		pccs = (PCERTIFYCONFIRMSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
        WinHelp ((HWND) wParam, pccs->pKM->szHelpFile, HELP_CONTEXTMENU, 
            (DWORD) (LPVOID) aSignKeyIds); 
        break; 

	case WM_NOTIFY:
		{
			LPNMHDR pnmh = (LPNMHDR) lParam;

			if (pnmh->code == DTN_DATETIMECHANGE) {
				SYSTEMTIME st;
				pccs = 
					(PCERTIFYCONFIRMSTRUCT)GetWindowLong(hDlg, GWL_USERDATA);
				SendMessage (pccs->hwndDTPicker, DTM_GETSYSTEMTIME,
										0, (LPARAM)&st);
				PGPclSystemTimeToDays (&st, &(pccs->iExpireDays));
				if (pccs->iExpireDays > 0) 
					EnableWindow (GetDlgItem (hDlg, IDOK), TRUE);
				else 
					EnableWindow (GetDlgItem (hDlg, IDOK), FALSE);
			}
		}
		break;

	case WM_PAINT :
		pccs = (PCERTIFYCONFIRMSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
		if (pccs->bExpandedChoices) 
		{
			// paint icons
			HDC	hdc = GetDC (hDlg);

			GetWindowRect (GetDlgItem (hDlg, IDC_NONEXPORTABLE), &rc);
			MapWindowPoints (NULL, hDlg, (LPPOINT)&rc, 2);
			ImageList_Draw (pccs->hIml, IDX_CERT, hdc, 
							rc.left-22, rc.top, ILD_TRANSPARENT);

			GetWindowRect (GetDlgItem (hDlg, IDC_EXPORTABLE), &rc);
			MapWindowPoints (NULL, hDlg, (LPPOINT)&rc, 2);
			ImageList_Draw (pccs->hIml, IDX_EXPORTCERT, hdc, 
							rc.left-22, rc.top, ILD_TRANSPARENT);

			GetWindowRect (GetDlgItem (hDlg, IDC_TRUSTED), &rc);
			MapWindowPoints (NULL, hDlg, (LPPOINT)&rc, 2);
			ImageList_Draw (pccs->hIml, IDX_TRUSTEDCERT, hdc, 
							rc.left-22, rc.top, ILD_TRANSPARENT);

			GetWindowRect (GetDlgItem (hDlg, IDC_META), &rc);
			MapWindowPoints (NULL, hDlg, (LPPOINT)&rc, 2);
			ImageList_Draw (pccs->hIml, IDX_METACERT, hdc, 
							rc.left-22, rc.top, ILD_TRANSPARENT);

			ReleaseDC (hDlg, hdc);
		}
		break;

	case WM_COMMAND:

		switch (LOWORD(wParam)) {
		case IDCANCEL:
			pccs = (PCERTIFYCONFIRMSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
			pccs->bSyncWithServer = FALSE;
			ImageList_Destroy (pccs->hIml);
			EndDialog (hDlg, 1);
			break;

		case IDOK:
			pccs = (PCERTIFYCONFIRMSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
			if (!(pccs->bExpandedChoices)) {
				pccs->bExpires = FALSE;
				if (IsDlgButtonChecked (hDlg, IDC_EXPORTABLECHECK) 
														== BST_CHECKED) 
				{
					pccs->iSigType = SIG_EXPORTABLE;
				}
				else {
					pccs->iSigType = SIG_NONEXPORTABLE;
				}
			}
			else {
				if (IsDlgButtonChecked (hDlg, IDC_NEVEREXPIRES) 
														== BST_CHECKED)
					pccs->bExpires = FALSE;
				else {
					SYSTEMTIME st;
					pccs->bExpires = TRUE;
					SendMessage (pccs->hwndDTPicker, DTM_GETSYSTEMTIME, 0, 
									(LPARAM)&st);
					PGPclSystemTimeToDays (&st, &(pccs->iExpireDays));
				}
			}
			GetDlgItemText (hDlg, IDC_DOMAIN, 
							pccs->szDomain, sizeof(pccs->szDomain));
			ImageList_Destroy (pccs->hIml);
			EndDialog (hDlg, 0);
			break;

		case IDC_EXPORTABLECHECK :
			pccs = (PCERTIFYCONFIRMSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
			if (IsDlgButtonChecked (hDlg, IDC_EXPORTABLECHECK) 
													== BST_CHECKED) {
				pccs->iSigType = SIG_EXPORTABLE;
			}
			else {
				pccs->iSigType = SIG_NONEXPORTABLE;
			}
			break;

		case IDC_NONEXPORTABLE :
			pccs = (PCERTIFYCONFIRMSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
			pccs->iSigType = SIG_NONEXPORTABLE;
			EnableWindow (GetDlgItem (hDlg, IDC_DOMAIN), FALSE);
			break;

		case IDC_EXPORTABLE :
			pccs = (PCERTIFYCONFIRMSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
			pccs->iSigType = SIG_EXPORTABLE;
			EnableWindow (GetDlgItem (hDlg, IDC_DOMAIN), FALSE);
			break;

		case IDC_TRUSTED :
			pccs = (PCERTIFYCONFIRMSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
			pccs->iSigType = SIG_TRUST;
			EnableWindow (GetDlgItem (hDlg, IDC_DOMAIN), TRUE);
			break;

		case IDC_META :
			pccs = (PCERTIFYCONFIRMSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
			pccs->iSigType = SIG_META;
			EnableWindow (GetDlgItem (hDlg, IDC_DOMAIN), FALSE);
			break;

		case IDC_NEVEREXPIRES :
		case IDC_EXPIRES :
			pccs = (PCERTIFYCONFIRMSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
			if (IsDlgButtonChecked (hDlg, IDC_EXPIRES) == BST_CHECKED) {
				SYSTEMTIME st;
				EnableWindow (pccs->hwndDTPicker, TRUE);

				SendMessage (pccs->hwndDTPicker, DTM_GETSYSTEMTIME,
										0, (LPARAM)&st);
				PGPclSystemTimeToDays (&st, &(pccs->iExpireDays));
				if (pccs->iExpireDays > 0) 
					EnableWindow (GetDlgItem (hDlg, IDOK), TRUE);
				else 
					EnableWindow (GetDlgItem (hDlg, IDOK), FALSE);
			}
			else {
				EnableWindow (pccs->hwndDTPicker, FALSE);
				EnableWindow (GetDlgItem (hDlg, IDOK), TRUE);
			}
			break;

		case IDC_MORECHOICES :
			pccs = (PCERTIFYCONFIRMSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
			pccs->bExpandedChoices = TRUE;
			ShowWindow (GetDlgItem (hDlg, IDC_MORECHOICES), SW_HIDE);
			ShowWindow (GetDlgItem (hDlg, IDC_EXPORTABLECHECK), SW_HIDE);
			ShowWindow (GetDlgItem (hDlg, IDC_FEWERCHOICES), SW_SHOW);
			ShowWindow (GetDlgItem (hDlg, IDC_SIGTYPE), SW_SHOW);
			ShowWindow (GetDlgItem (hDlg, IDC_NONEXPORTABLE), SW_SHOW);
			ShowWindow (GetDlgItem (hDlg, IDC_EXPORTABLE), SW_SHOW);
			ShowWindow (GetDlgItem (hDlg, IDC_TRUSTED), SW_SHOW);
			ShowWindow (GetDlgItem (hDlg, IDC_META), SW_SHOW);
			ShowWindow (GetDlgItem (hDlg, IDC_EXPIRATION), SW_SHOW);
			ShowWindow (GetDlgItem (hDlg, IDC_NEVEREXPIRES), SW_SHOW);
			ShowWindow (GetDlgItem (hDlg, IDC_EXPIRES), SW_SHOW);
			ShowWindow (GetDlgItem (hDlg, IDC_DOMAINTEXT), SW_SHOW);
			ShowWindow (GetDlgItem (hDlg, IDC_DOMAIN), SW_SHOW);
			ShowWindow (pccs->hwndDTPicker, SW_SHOW);

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
			EnableWindow (GetDlgItem (hDlg, IDC_DOMAIN), 
								(pccs->iSigType == SIG_META));

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
			pccs = (PCERTIFYCONFIRMSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
			pccs->bExpandedChoices = FALSE;
			ShowWindow (GetDlgItem (hDlg, IDC_MORECHOICES), SW_SHOW);
			ShowWindow (GetDlgItem (hDlg, IDC_EXPORTABLECHECK), SW_SHOW);
			ShowWindow (GetDlgItem (hDlg, IDC_FEWERCHOICES), SW_HIDE);
			ShowWindow (GetDlgItem (hDlg, IDC_SIGTYPE), SW_HIDE);
			ShowWindow (GetDlgItem (hDlg, IDC_NONEXPORTABLE), SW_HIDE);
			ShowWindow (GetDlgItem (hDlg, IDC_EXPORTABLE), SW_HIDE);
			ShowWindow (GetDlgItem (hDlg, IDC_TRUSTED), SW_HIDE);
			ShowWindow (GetDlgItem (hDlg, IDC_META), SW_HIDE);
			ShowWindow (GetDlgItem (hDlg, IDC_EXPIRATION), SW_HIDE);
			ShowWindow (GetDlgItem (hDlg, IDC_NEVEREXPIRES), SW_HIDE);
			ShowWindow (GetDlgItem (hDlg, IDC_EXPIRES), SW_HIDE);
			ShowWindow (GetDlgItem (hDlg, IDC_DOMAINTEXT), SW_HIDE);
			ShowWindow (GetDlgItem (hDlg, IDC_DOMAIN), SW_HIDE);
			ShowWindow (pccs->hwndDTPicker, SW_HIDE);

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
			else
				CheckDlgButton (hDlg, IDC_EXPORTABLECHECK, BST_UNCHECKED);

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
			pccs = (PCERTIFYCONFIRMSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
			WinHelp (hDlg, pccs->pKM->szHelpFile, HELP_CONTEXT, 
						IDH_PGPKM_SIGNDIALOG); 
			break;

		}
		return TRUE;
	}
	return FALSE;
}


//	_______________________________________________
//
//  Certify selected key or userid

BOOL 
KMCertifyKeyOrUserID (PKEYMAN pKM) 
{
	CHAR					sz256[256];
	PGPError				err;
	CERTIFYSTRUCT			cs;
	CERTIFYCONFIRMSTRUCT	ccs;
	PGPPrefRef				prefref;

	// initialize structures
	cs.lpfnCallback = sCertifySingleObject;
	cs.pKM = pKM;
	cs.bItemModified = FALSE;
	cs.keySigning = NULL;
	cs.pPasskey = NULL;
	
	ccs.pKM = pKM;
	ccs.iSigType = SIG_NONEXPORTABLE;
	ccs.bExpires = FALSE;
	ccs.iExpireDays = 0;

	PGPclOpenClientPrefs (PGPGetContextMemoryMgr (pKM->Context), &prefref);
	PGPGetPrefBoolean (prefref, kPGPPrefKeyServerSyncOnKeySign, 
						&(ccs.bSyncWithServer));
	PGPclCloseClientPrefs (prefref, FALSE);

	if (DialogBoxParam (g_hInst, MAKEINTRESOURCE(IDD_SIGNCONFIRM),
		pKM->hWndParent, sSignKeyDlgProc, (LPARAM)&ccs)) {
		return FALSE;
	}

	// convert user-entered signature type to flags to pass
	// and setup domain regular expression
	switch (ccs.iSigType) {
	case SIG_NONEXPORTABLE :
		cs.bExportable = FALSE;
		cs.uTrustLevel = 0;
		cs.szRegExpression[0] = '\0';
		break;

	case SIG_EXPORTABLE :
		cs.bExportable = TRUE;
		cs.uTrustLevel = 0;
		cs.szRegExpression[0] = '\0';
		break;

	case SIG_TRUST :
		cs.bExportable = TRUE;
		cs.uTrustLevel = 1;
		if (ccs.szDomain[0])
			sDomainToRegExpression (ccs.szDomain, cs.szRegExpression);
		else
			cs.szRegExpression[0] = '\0';
		break;

	case SIG_META :
		cs.bExportable = FALSE;
		cs.uTrustLevel = 2;
		cs.szRegExpression[0] = '\0';
		break;
	}

	// convert expiration info to expire days
	if ((ccs.bExpires) && (ccs.iExpireDays > 0))
		cs.uExpireDays = (UINT)ccs.iExpireDays;
	else
		cs.uExpireDays = 0;

	// get valid passphrase
	LoadString (g_hInst, IDS_SIGNKEYPASSPHRASE, sz256, 256);
	err = KMGetSigningKeyPhrase (pKM->Context, pKM->tlsContext, 
						pKM->hWndParent, sz256, 
						pKM->KeySetMain, FALSE, &cs.keySigning,
						NULL, &cs.pPasskey, &cs.sizePasskey);

	if (IsntPGPError (err)) {

		// update from server
		if (ccs.bSyncWithServer) {
			if (!KMGetFromServerInternal (pKM, FALSE, FALSE, FALSE)) {
				if (KMMessageBox (pKM->hWndParent, IDS_CAPTION, 
									IDS_QUERYCONTINUESIGNING, 
									MB_YESNO|MB_ICONEXCLAMATION) == IDNO) {
					if (cs.pPasskey) {
						KMFreePasskey (cs.pPasskey, cs.sizePasskey);
						cs.pPasskey = NULL;
					}
					return FALSE;
				}
			}
		}

		// call callback for all selected items
		TreeList_IterateSelected (pKM->hWndTree, &cs);

		// changes have been made; save them and update all validities
		if (cs.bItemModified) {
			KMCommitKeyRingChanges (pKM);
			KMLoadKeyRingIntoTree (pKM, FALSE, FALSE, FALSE);
			InvalidateRect (pKM->hWndTree, NULL, TRUE);

			// send key to server, if selected
			if (ccs.bSyncWithServer) {
				KMSendToServer (pKM, PGPCL_DEFAULTSERVER);
			}
		}
	}

	if (cs.pPasskey) {
		KMFreePasskey (cs.pPasskey, cs.sizePasskey);
		cs.pPasskey = NULL;
	}
	return (cs.bItemModified);
}


//	_______________________________________________
//
//  Enable selected key

BOOL 
KMEnableKey (PKEYMAN pKM, PGPKeyRef key) 
{
	if (IsntPGPError (PGPclErrorBox (NULL, PGPEnableKey (key)))) {
		KMCommitKeyRingChanges (pKM);
		KMUpdateKeyInTree (pKM, key, FALSE);
		return TRUE;
	}
	return FALSE;
}


//	_______________________________________________
//
//  Disable selected key

BOOL 
KMDisableKey (PKEYMAN pKM, PGPKeyRef key) 
{
	if (IsntPGPError (PGPclErrorBox (NULL, PGPDisableKey (key)))) {
		KMCommitKeyRingChanges (pKM);
		KMUpdateKeyInTree (pKM, key, FALSE);
		return TRUE;
	}
	return FALSE;
}


//	_______________________________________________
//
//	Delete All dialog message procedure

static BOOL CALLBACK 
sDeleteAllDlgProc (
		HWND	hWndDlg, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	PDELETESTRUCT pds;

	switch (uMsg) {

	case WM_INITDIALOG :
		SetWindowLong (hWndDlg, GWL_USERDATA, lParam);
		pds = (PDELETESTRUCT)lParam;
		SetDlgItemText (hWndDlg, IDC_STRING, pds->pszPrompt);
		SetFocus (GetDlgItem (hWndDlg, IDNO));
		return FALSE;

    case WM_HELP: 
		pds = (PDELETESTRUCT)GetWindowLong (hWndDlg, GWL_USERDATA);
        WinHelp (((LPHELPINFO) lParam)->hItemHandle, pds->pKM->szHelpFile, 
            HELP_WM_HELP, (DWORD) (LPSTR) aDeleteAllIds); 
        break; 
 
    case WM_CONTEXTMENU:
		pds = (PDELETESTRUCT)GetWindowLong (hWndDlg, GWL_USERDATA);
        WinHelp ((HWND) wParam, pds->pKM->szHelpFile, HELP_CONTEXTMENU, 
            (DWORD) (LPVOID) aDeleteAllIds); 
        break; 

	case WM_COMMAND:

		switch (LOWORD(wParam)) {
		case IDCANCEL:
			pds = (PDELETESTRUCT)GetWindowLong (hWndDlg, GWL_USERDATA);
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
			pds = (PDELETESTRUCT)GetWindowLong (hWndDlg, GWL_USERDATA);
			pds->bDeleteAll = TRUE;
			EndDialog (hWndDlg, IDYES);
			break;
		}
		return TRUE;
	}
	return FALSE;
}


//	_______________________________________________
//
//  Ask user for delete confirmation

static INT 
sDeleteConfirm (
		TL_TREEITEM*	lptli, 
		INT				iPromptID, 
		PDELETESTRUCT	pds) 
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
			pds->pKM->hWndParent, sDeleteAllDlgProc, (LPARAM)pds);
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


//	_______________________________________________
//
//  Get handle of nearby item
//
//	lptli	= pointer to TreeList item

static HTLITEM 
sGetAdjacentItem (
		HWND			hWndTree, 
		TL_TREEITEM*	lptli) 
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

	
//	_______________________________________________
//
//  Delete a single object
//	routine called either from KMDeleteObject or as a
//	callback function from the TreeList control to 
//	delete a single item.
//
//	lptli	= pointer to TreeList item to delete

static BOOL CALLBACK 
sDeleteSingleObject (
		TL_TREEITEM*	lptli, 
		LPARAM			lParam) 
{
	PDELETESTRUCT	pds			= (PDELETESTRUCT)lParam;
	PGPKeySetRef	keyset;
	PGPKeyRef		key;
	PGPUserIDRef	userid;
	BOOL			bPrimary;
	INT				iConfirm;
	LONG			lVal;
	PGPError		err;

	switch (lptli->iImage) {
	case IDX_RSASECKEY :
	case IDX_RSASECDISKEY :
	case IDX_RSASECSHRKEY :
	case IDX_DSASECKEY :
	case IDX_DSASECDISKEY :
	case IDX_DSASECSHRKEY :
		iConfirm = sDeleteConfirm (lptli, IDS_DELCONFPRIVKEY, pds);
		if (iConfirm == IDYES) {
			if ((PGPKeyRef)(lptli->lParam) == pds->keyDefault) {
				if (KMMessageBox (pds->pKM->hWndParent, IDS_CAPTION, 
					IDS_DELCONFDEFKEY,
					MB_YESNO|MB_TASKMODAL|MB_DEFBUTTON2|MB_ICONWARNING)
					==IDNO) 
						return TRUE;
			}
			PGPNewSingletonKeySet ((PGPKeyRef)(lptli->lParam),
									&keyset);
			KMGetKeyUserVal (pds->pKM, (PGPKeyRef)(lptli->lParam), &lVal);
			KMSetKeyUserVal (pds->pKM, (PGPKeyRef)(lptli->lParam), 0);
			if (IsntPGPError (PGPclErrorBox (NULL, PGPRemoveKeys (
					keyset, pds->pKM->KeySetDisp)))) {
				KMDeletePropertiesKey (pds->pKM, (PGPKeyRef)(lptli->lParam));
				pds->bItemModified = TRUE;
				if (!pds->bItemNotDeleted) 
					pds->hPostDeleteFocusItem = 
							sGetAdjacentItem (pds->pKM->hWndTree, lptli); 
				TreeList_DeleteItem (pds->pKM->hWndTree, lptli);
			}
			else 
				KMSetKeyUserVal (pds->pKM, (PGPKeyRef)(lptli->lParam), lVal);

			PGPFreeKeySet (keyset);
		}
		if (iConfirm == IDCANCEL) return FALSE; 
		else return TRUE;

	case IDX_RSASECREVKEY :
	case IDX_RSASECEXPKEY :
	case IDX_DSASECREVKEY :
	case IDX_DSASECEXPKEY :
		iConfirm = sDeleteConfirm (lptli, IDS_DELCONFPRIVKEY, pds);
		if (iConfirm == IDYES) {
			PGPNewSingletonKeySet ((PGPKeyRef)(lptli->lParam),
									&keyset);
			KMGetKeyUserVal (pds->pKM, (PGPKeyRef)(lptli->lParam), &lVal);
			KMSetKeyUserVal (pds->pKM, (PGPKeyRef)(lptli->lParam), 0);
			if (IsntPGPError (PGPclErrorBox (NULL, PGPRemoveKeys (
					keyset, pds->pKM->KeySetDisp)))) {
				KMDeletePropertiesKey (pds->pKM, (PGPKeyRef)(lptli->lParam));
				pds->bItemModified = TRUE;
				if (!pds->bItemNotDeleted) 
					pds->hPostDeleteFocusItem = 
							sGetAdjacentItem (pds->pKM->hWndTree, lptli); 
				TreeList_DeleteItem (pds->pKM->hWndTree, lptli);
			}
			else 
				KMSetKeyUserVal (pds->pKM, (PGPKeyRef)(lptli->lParam), lVal);

			PGPFreeKeySet (keyset);
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
		iConfirm = sDeleteConfirm (lptli, IDS_DELCONFKEY, pds);
		if (iConfirm == IDYES) {
			PGPNewSingletonKeySet ((PGPKeyRef)(lptli->lParam), 
									&keyset);
			KMGetKeyUserVal (pds->pKM, (PGPKeyRef)(lptli->lParam), &lVal);
			KMSetKeyUserVal (pds->pKM, (PGPKeyRef)(lptli->lParam), 0);
			if (IsntPGPError (PGPclErrorBox (NULL, PGPRemoveKeys (
								keyset, pds->pKM->KeySetDisp)))) {
				KMDeletePropertiesKey (pds->pKM, (PGPKeyRef)(lptli->lParam));
				pds->bItemModified = TRUE;
				if (!pds->bItemNotDeleted) 
					pds->hPostDeleteFocusItem = 
							sGetAdjacentItem (pds->pKM->hWndTree, lptli); 
				TreeList_DeleteItem (pds->pKM->hWndTree, lptli);
			}
			else
				KMSetKeyUserVal (pds->pKM, (PGPKeyRef)(lptli->lParam), lVal);
			PGPFreeKeySet (keyset);
		}
		if (iConfirm == IDCANCEL) return FALSE;
		else return TRUE;

	case IDX_RSAUSERID :
	case IDX_DSAUSERID :
		if (KMIsThisTheOnlyUserID (pds->pKM, (PGPUserIDRef)(lptli->lParam))) {
			KMMessageBox (pds->pKM->hWndParent, IDS_CAPTION, 
						IDS_DELONLYUSERID, MB_OK|MB_ICONEXCLAMATION);
			break;
		}
		// fall through

	case IDX_PHOTOUSERID :
	case IDX_INVALIDUSERID :
		iConfirm = sDeleteConfirm (lptli, IDS_DELCONFUSERID, pds);
		if (iConfirm == IDYES) {
			key = KMGetKeyFromUserID (pds->pKM, 
											(PGPUserIDRef)(lptli->lParam));

			PGPGetPrimaryUserID (key, &userid);
			if (userid == (PGPUserIDRef)(lptli->lParam)) 
				bPrimary = TRUE;
			else 
				bPrimary = FALSE;

			KMGetUserIDUserVal (pds->pKM,(PGPUserIDRef)(lptli->lParam),&lVal);
			KMSetUserIDUserVal (pds->pKM,(PGPUserIDRef)(lptli->lParam),0);
			err = PGPRemoveUserID ((PGPUserIDRef)(lptli->lParam));
			if (IsntPGPError (err)) {			
				pds->bItemModified = TRUE;
				if (bPrimary) {
					pds->bDeletedPrimaryUserID = TRUE;
				}
				else { 
					if (!pds->bItemNotDeleted) 
						pds->hPostDeleteFocusItem = 
							sGetAdjacentItem (pds->pKM->hWndTree, lptli); 
					TreeList_DeleteItem (pds->pKM->hWndTree, lptli);
				}
			}
			else {
				PGPclErrorBox (NULL, err);
				KMSetUserIDUserVal (pds->pKM, 
									(PGPUserIDRef)(lptli->lParam), lVal);
			}
		}
		if (iConfirm == IDCANCEL) return FALSE;
		else return TRUE;

	case IDX_CERT :
	case IDX_REVCERT :
	case IDX_BADCERT :
	case IDX_EXPCERT :
	case IDX_EXPORTCERT :
	case IDX_TRUSTEDCERT :
	case IDX_METACERT :
	case IDX_X509CERT :
	case IDX_X509EXPCERT :
	case IDX_X509REVCERT :
		iConfirm = sDeleteConfirm (lptli, IDS_DELCONFCERT, pds);
		if (iConfirm == IDYES) {
			KMGetCertUserVal (pds->pKM, (PGPSigRef)(lptli->lParam), &lVal);
			KMSetCertUserVal (pds->pKM, (PGPSigRef)(lptli->lParam), 0);
			if (IsntPGPError (PGPclErrorBox (NULL, PGPRemoveSig (
											(PGPSigRef)(lptli->lParam))))) {
				pds->bItemModified = TRUE;
				if (!pds->bItemNotDeleted) 
					pds->hPostDeleteFocusItem = 
							sGetAdjacentItem (pds->pKM->hWndTree, lptli); 
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


//	_______________________________________________
//
//  Delete selected key or keys

BOOL 
KMDeleteObject (PKEYMAN pKM) 
{
	TL_TREEITEM		tli;
	DELETESTRUCT	ds;

	ds.lpfnCallback = sDeleteSingleObject;
	ds.context = pKM->Context;
	ds.pKM = pKM;
	ds.bItemModified = FALSE;
	ds.bDeleteAll = FALSE;
	ds.bItemNotDeleted = FALSE;
	ds.bDeletedPrimaryUserID = FALSE;
	ds.hPostDeleteFocusItem = NULL;
	PGPGetDefaultPrivateKey (pKM->KeySetMain, &ds.keyDefault);

	// call callback function for all selected objects
	TreeList_IterateSelected (pKM->hWndTree, &ds);

	if (ds.bItemModified) {
		KMCommitKeyRingChanges (pKM);
		if (ds.bDeletedPrimaryUserID) {
			KMSetFocus (pKM, NULL, FALSE);
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
				KMSetFocus (pKM, NULL, FALSE);
			}
		}
		InvalidateRect (pKM->hWndTree, NULL, TRUE);
	}
	return (ds.bItemModified);
}


//	_______________________________________________
//
//	Set focused key to be default signing key

BOOL 
KMSetDefaultKey (PKEYMAN pKM) 
{
	PGPKeyRef	keyNewDef;
	PGPKeyRef	keyOldDef;
	PGPError	err;

	PGPGetDefaultPrivateKey (pKM->KeySetMain, &keyOldDef);
	keyNewDef = (PGPKeyRef) KMFocusedObject (pKM);

	err = PGPSetDefaultPrivateKey (keyNewDef);
	if (IsntPGPError (PGPclErrorBox (pKM->hWndParent, err))) {

		PGPclErrorBox (NULL, PGPsdkSavePrefs (pKM->Context));
		if (keyOldDef)
			KMUpdateKeyInTree (pKM, keyOldDef, FALSE);
		KMUpdateKeyInTree (pKM, keyNewDef, FALSE);

		return TRUE;
	}
	return FALSE;
}


//	_______________________________________________
//
//	Set focused UID to be primary UID

BOOL 
KMSetPrimaryUserID (PKEYMAN pKM) 
{
	PGPByte*		pPasskey			= NULL;
	LPSTR			pszPhrase			= NULL;

	PGPUserIDRef	userid;
	PGPKeyRef		key;
	PGPUInt32		iAlg;
	CHAR			sz[128];
	PGPError		err;
	PGPSize			sizePasskey;

	userid = (PGPUserIDRef) KMFocusedObject (pKM);
	key = KMGetKeyFromUserID (pKM, userid);

	PGPGetKeyNumber (key, kPGPKeyPropAlgID, &iAlg);
	if (iAlg == kPGPPublicKeyAlgorithm_RSA) 
	{
		err = PGPSetPrimaryUserID (userid);
	}

	else if (iAlg == kPGPPublicKeyAlgorithm_DSA) 
	{
		// get valid passphrase
		LoadString (g_hInst, IDS_SELKEYPASSPHRASE, sz, sizeof(sz)); 
		err = KMGetKeyPhrase (pKM->Context, pKM->tlsContext,
						pKM->hWndParent, sz,
						pKM->KeySetMain, key,
						&pszPhrase, &pPasskey, &sizePasskey);
		PGPclErrorBox (NULL, err);

		// now we have a valid passphrase
		if (IsntPGPError (err)) 
		{
			err = PGPCertifyPrimaryUserID (userid,
				pPasskey ?
					PGPOPasskeyBuffer (pKM->Context, pPasskey, sizePasskey) :
					PGPOPassphrase (pKM->Context, pszPhrase),
				PGPOLastOption (pKM->Context));
		}

		if (pszPhrase)
			KMFreePhrase (pszPhrase);
		if (pPasskey)
			KMFreePasskey (pPasskey, sizePasskey);
	}

	if (IsntPGPError (PGPclErrorBox (pKM->hWndParent, err)))
	{
		KMCommitKeyRingChanges (pKM);
		KMUpdateKeyInTree (pKM, key, TRUE);
		InvalidateRect (pKM->hWndTree, NULL, TRUE);
		return TRUE;
	}
	else
		return FALSE;
}


//	_______________________________________________
//
//	add selected keys to main keyset

BOOL 
KMAddSelectedToMain (PKEYMAN pKM) 
{
	PGPKeySetRef		keysetMain	= kInvalidPGPKeySetRef;
	PGPKeySetRef		keysetToAdd	= kInvalidPGPKeySetRef;
	PGPError			err			= kPGPError_NoErr;
	BOOL				bSecret		= FALSE;
	BOOL				bRet		= FALSE;

	err = KMGetSelectedKeys (pKM, &keysetToAdd, NULL);

	if (PGPKeySetRefIsValid (keysetToAdd)) 
	{
		if (pKM->bMainKeySet)
			keysetMain = pKM->KeySetMain;

		bSecret = KMCheckForSecretKeys (keysetToAdd);
		bRet = CLAddKeysToMain (pKM->Context, pKM->hWndParent, 
							keysetToAdd, keysetMain);

		if (bRet && bSecret)
		{
			KMMessageBox (pKM->hWndParent, IDS_CAPTION, 
					IDS_IMPORTEDSECRETKEYS, 
					MB_OK|MB_ICONEXCLAMATION);
		}

		PGPFreeKeySet (keysetToAdd);
	}

	return bRet;
}
	
//	_______________________________________________
//
//	routine called as a callback function from the 
//  PGPCheckKeyRingSigs function

static PGPError 
sKeyCheckEventHandler(
		PGPContextRef	context,
		PGPEvent		*event, 
		PGPUserValue	userValue)
{
	PREVERIFYSTRUCT prvs;
	CHAR			sz[64];
	INT				iTotal;

	prvs = (PREVERIFYSTRUCT)userValue;

	iTotal = LOWORD (event->data.nullData.bytesTotal);
	if (iTotal != prvs->iNumSigsTotal) {
		prvs->iNumSigsTotal = iTotal;

		EnableWindow (GetDlgItem (prvs->hwndProgress, IDCANCEL), TRUE);

		LoadString (g_hInst, IDS_VERIFYING, sz, sizeof(sz));
		SetDlgItemText (prvs->hwndProgress, IDC_TEXT, sz);

		SendDlgItemMessage (prvs->hwndProgress, IDC_PROGRESSBAR, 
					PBM_SETRANGE, 0, MAKELPARAM (0, prvs->iNumSigsTotal));
	}

	SendDlgItemMessage (prvs->hwndProgress, IDC_PROGRESSBAR, 
			PBM_SETPOS, LOWORD (event->data.nullData.bytesWritten), 0);

	if (prvs->bCancel)
		return kPGPError_UserAbort;
	else 
		return kPGPError_NoErr;
}
	
//	_______________________________________________
//
//	reverify signature progress dialog message procedure

static BOOL CALLBACK 
sReverifyProgressDlgProc (
		HWND	hDlg, 
		UINT	uMsg, 								
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	PREVERIFYSTRUCT prvs;
	CHAR			sz[64];

	switch (uMsg) {
	case WM_INITDIALOG :
		SetWindowLong (hDlg, GWL_USERDATA, lParam);
		prvs = (PREVERIFYSTRUCT)lParam;
		LoadString (g_hInst, IDS_PREPARINGTOVERIFY, sz, sizeof(sz));
		SetDlgItemText (hDlg, IDC_TEXT, sz);
		prvs->hwndProgress = hDlg;
		return TRUE;

	case WM_APP :
		prvs = (PREVERIFYSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
		SetForegroundWindow (prvs->hwndParent);
		EndDialog (hDlg, 0);
		break;

	case WM_COMMAND :
		switch (LOWORD (wParam)) {
		case IDCANCEL :
			prvs = (PREVERIFYSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
			prvs->bCancel = TRUE;
			break;
		}
		return TRUE;
	}
	return FALSE;
}

//	_______________________________________________
//
//	reverify signatures thread routine

static DWORD WINAPI 
sReverifyProgressThreadRoutine (LPVOID lpvoid)
{
	PREVERIFYSTRUCT		prvs	= (PREVERIFYSTRUCT)lpvoid;

	DialogBoxParam (g_hInst, 
					MAKEINTRESOURCE(IDD_REVERIFYING), NULL, 
					sReverifyProgressDlgProc, (LPARAM)prvs);
								
	return 0;
}

//	_______________________________________________
//
//	reverify signatures of selected keys

BOOL 
KMReverifySigs (PKEYMAN pKM) 
{	
	PGPKeySetRef		keysetReverify	= kInvalidPGPKeySetRef;
	PGPError			err				= kPGPError_NoErr;
	REVERIFYSTRUCT		rvs;
	DWORD				dw;

	rvs.hwndParent = pKM->hWndParent;
	rvs.hwndProgress = NULL;
	rvs.iNumSigsTotal = -1;
	rvs.bCancel = FALSE;
	CreateThread (NULL, 0, sReverifyProgressThreadRoutine, &rvs, 0, &dw);

	// wait for dialog box
	EnableWindow (pKM->hWndParent, FALSE);
	while (rvs.hwndProgress == NULL) Sleep (100);

	err = KMGetSelectedKeys (pKM, &keysetReverify, NULL);

	if (PGPKeySetRefIsValid (keysetReverify)) 
	{
		err = PGPCheckKeyRingSigs (keysetReverify,
					pKM->KeySetMain, TRUE, sKeyCheckEventHandler, &rvs);
	}

	// send message to close down dialog
	SendMessage (rvs.hwndProgress, WM_APP, 0, 0);
	EnableWindow (pKM->hWndParent, TRUE);
					
	if (IsntPGPError (PGPclErrorBox (NULL, err))) {
		KMCommitKeyRingChanges (pKM);
		KMLoadKeyRingIntoTree (pKM, FALSE, FALSE, FALSE);
		InvalidateRect (pKM->hWndTree, NULL, TRUE);
	}

	if (PGPKeySetRefIsValid (keysetReverify)) 
		PGPFreeKeySet (keysetReverify);

	return (IsntPGPError (err));
}

