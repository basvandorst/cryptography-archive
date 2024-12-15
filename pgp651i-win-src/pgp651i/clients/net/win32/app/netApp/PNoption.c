/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PGPoption.c - PGPnet options property sheet handler
	

	$Id: PNoption.c,v 1.70.4.1.2.1 1999/06/29 18:46:09 pbj Exp $
____________________________________________________________________________*/

#include <windows.h>
#include <commctrl.h>
#include "resource.h"
#include "PGPnetApp.h"
#include "pgpNetIPC.h"
#include "pgpNetPaths.h"
#include "PGPcl.h"
#include "pgpClientLib.h"
#include "DurationControl.h"
#include "PGPnetHelp.h"

#include "pgpBuildFlags.h"

#include "pgpKeys.h"
#include "pgpMem.h"
#include "pgpSDKPrefs.h"

#define MINIKEMEGABYTES			4
#define MAXIKEMEGABYTES			4095
#define MAXIKEMEGABYTESDIGITS	4

#define MINIPSECMEGABYTES		4
#define MAXIPSECMEGABYTES		4095
#define MAXIPSECMEGABYTESDIGITS	4

#define MINEXPIRESECONDS		60

extern HINSTANCE			g_hinst;
extern PGPContextRef		g_context;
extern APPOPTIONSSTRUCT		g_AOS;
extern CHAR					g_szHelpFile[];				
extern BOOL					g_bReadOnly;	
extern BOOL					g_bAuthAccess;	
extern HWND					g_hwndOptions;

static DWORD aGeneralIds[] = {			// Help IDs
	IDC_EXPERTMODE,				IDH_PNOPTGEN_EXPERTMODE,
	IDC_ALLOWUNCONFIGURED,		IDH_PNOPTGEN_ALLOWUNCONFIGHOSTS,
	IDC_REQUIRESECURE,			IDH_PNOPTGEN_REQSECUREHOSTS,
	IDC_REQUIREVALIDKEY,		IDH_PNOPTGEN_REQUIREVALIDKEY,
	IDC_ENABLECACHE,			IDH_PNOPTGEN_CACHEPASSPHRASES,
	IDC_ENABLEIKETIMEEXPIRE,	IDH_PNOPTGEN_IKETIMEEXPIREENABLE,
	IDC_IKETIMEEXPIRE,			IDH_PNOPTGEN_IKETIMEEXPIRETIME,
	IDC_ENABLEIKEBYTEEXPIRE,	IDH_PNOPTGEN_IKEDATAEXPIREENABLE,
	IDC_IKEBYTES,				IDH_PNOPTGEN_IKEDATAEXPIREBYTES,
	IDC_ENABLEIPSECTIMEEXPIRE,	IDH_PNOPTGEN_IPSECTIMEEXPIREENABLE,
	IDC_IPSECTIMEEXPIRE,		IDH_PNOPTGEN_IPSECTIMEEXPIRETIME,
	IDC_ENABLEIPSECBYTEEXPIRE,	IDH_PNOPTGEN_IPSECDATAEXPIREENABLE,
	IDC_IPSECBYTES,				IDH_PNOPTGEN_IPSECDATAEXPIREBYTES,
	0,0
};

static DWORD aAuthIds[] = {			// Help IDs
	IDC_PUBRING,				IDH_PNOPTAUTH_PUBKEYRING,
	IDC_PUBRINGBROWSE,			IDH_PNOPTAUTH_BROWSEPUBKEYRING,
	IDC_SECRING,				IDH_PNOPTAUTH_SECKEYRING,
	IDC_SECRINGBROWSE,			IDH_PNOPTAUTH_BROWSESECKEYRING,
	IDC_SETTOCURRENT,			IDH_PNOPTAUTH_USEMYKEYRINGS,
    IDC_PGPKEYLIST,				IDH_PNOPTAUTH_PGPAUTHKEY, 
	IDC_CLEARPGPKEY,			IDH_PNOPTAUTH_CLEARPGPAUTHKEY,
	IDC_SELECTPGPKEY,			IDH_PNOPTAUTH_SELECTPGPAUTHKEY,
	IDC_X509KEYEDIT,			IDH_PNOPTAUTH_X509AUTHKEY,
	IDC_CLEARX509KEY,			IDH_PNOPTAUTH_CLEARX509AUTHKEY,
	IDC_SELECTX509KEY,			IDH_PNOPTAUTH_SELECTX509AUTHKEY,
	0,0
};


//	____________________________________
//
//	general options dialog proc

static BOOL CALLBACK
sGenOptionsDlgProc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam)
{
	POPTIONSSTRUCT	pos;

	switch (uMsg)
	{
	case WM_INITDIALOG :
	{
		RECT		rc;
		UINT		u;
		CHAR		sz[8];

		SetWindowLong (hwnd, GWL_USERDATA, ((PROPSHEETPAGE*)lParam)->lParam);
		pos = (POPTIONSSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);

		g_hwndOptions = hwnd;

		// initialize the check boxes and radio buttons
		if (g_AOS.pnconfig.bRequireValidKey)
			CheckDlgButton (hwnd, IDC_REQUIREVALIDKEY, BST_CHECKED);
		else
			CheckDlgButton (hwnd, IDC_REQUIREVALIDKEY, BST_UNCHECKED);

		if (g_AOS.pnconfig.bAllowUnconfigHost) 
		{
			CheckDlgButton (hwnd, IDC_ALLOWUNCONFIGURED, BST_CHECKED);
			EnableWindow (GetDlgItem (hwnd, IDC_REQUIRESECURE), TRUE);
		}
		else
		{
			CheckDlgButton (hwnd, IDC_ALLOWUNCONFIGURED, BST_UNCHECKED);
			EnableWindow (GetDlgItem (hwnd, IDC_REQUIRESECURE), FALSE);
		}

		if (g_AOS.pnconfig.bRequireSecure)
			CheckDlgButton (hwnd, IDC_REQUIRESECURE, BST_CHECKED);
		else
			CheckDlgButton (hwnd, IDC_REQUIRESECURE, BST_UNCHECKED);

		if (g_AOS.pnconfig.bCachePassphrases)
			CheckDlgButton (hwnd, IDC_ENABLECACHE, BST_CHECKED);
		else
			CheckDlgButton (hwnd, IDC_ENABLECACHE, BST_UNCHECKED);

		if (g_AOS.pnconfig.bExpertMode)
			CheckDlgButton (hwnd, IDC_EXPERTMODE, BST_CHECKED);
		else
			CheckDlgButton (hwnd, IDC_EXPERTMODE, BST_UNCHECKED);

		// create and initialize IKE expiration date/time picker control
		GetWindowRect (GetDlgItem (hwnd, IDC_IKETIMEEXPIRE), &rc);
		MapWindowPoints (NULL, hwnd, (LPPOINT)&rc, 2);
		pos->hwndIkeTimeExpire = CreateWindowEx (
				WS_EX_CLIENTEDGE, 
				WC_PGPDURATION, "",
				WS_CHILD|WS_TABSTOP|WS_VISIBLE,
				rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, 
				hwnd, (HMENU)IDC_IKETIMEEXPIRE, g_hinst, NULL);

		SetWindowPos (pos->hwndIkeTimeExpire, 
							GetDlgItem (hwnd, IDC_ENABLEIKETIMEEXPIRE),
							0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);

		LoadString (g_hinst, IDS_EXPIREDAYSTEXT, sz, sizeof(sz));
		SendMessage (pos->hwndIkeTimeExpire, PGP_DCM_SETTEXT, 
						(WPARAM)DAYS, (LPARAM)sz);

		LoadString (g_hinst, IDS_EXPIREHOURSTEXT, sz, sizeof(sz));
		SendMessage (pos->hwndIkeTimeExpire, PGP_DCM_SETTEXT, 
						(WPARAM)HOURS, (LPARAM)sz);

		LoadString (g_hinst, IDS_EXPIREMINSTEXT, sz, sizeof(sz));
		SendMessage (pos->hwndIkeTimeExpire, PGP_DCM_SETTEXT, 
						(WPARAM)MINS, (LPARAM)sz);

		SendMessage (pos->hwndIkeTimeExpire, PGP_DCM_SETMINDURATION, 0, 
						(LPARAM)MINEXPIRESECONDS);

		SendMessage (pos->hwndIkeTimeExpire, PGP_DCM_SETDURATION, 0, 
						(LPARAM)g_AOS.pnconfig.uIkeTimeExpiration);

		// initialize IKE time expiration on/off
		if (g_AOS.pnconfig.bIkeTimeExpiration) 
		{
			CheckDlgButton (hwnd, IDC_ENABLEIKETIMEEXPIRE, BST_CHECKED);
			EnableWindow (pos->hwndIkeTimeExpire, TRUE);
		}
		else 
		{
			CheckDlgButton (hwnd, IDC_ENABLEIKETIMEEXPIRE, BST_UNCHECKED);
			EnableWindow (pos->hwndIkeTimeExpire, FALSE);
		}

		// initialize IKE byte expiration controls
		SendDlgItemMessage (hwnd, IDC_IKEBYTESPIN, UDM_SETRANGE,
				0, (LPARAM)MAKELONG (MAXIKEMEGABYTES, MINIKEMEGABYTES));
		SendDlgItemMessage (hwnd, IDC_IKEBYTES, EM_SETLIMITTEXT, 
				MAXIKEMEGABYTESDIGITS, 0);

		u = g_AOS.pnconfig.uIkeKByteExpiration / 1024;
		SendDlgItemMessage (hwnd, IDC_IKEBYTESPIN, UDM_SETPOS,
				0, (LPARAM)MAKELONG (u, 0));

		if (g_AOS.pnconfig.bIkeKByteExpiration) 
		{
			CheckDlgButton (hwnd, IDC_ENABLEIKEBYTEEXPIRE, BST_CHECKED);
			EnableWindow (GetDlgItem (hwnd, IDC_IKEBYTES), TRUE);
			EnableWindow (GetDlgItem (hwnd, IDC_IKEBYTESPIN), TRUE);
		}
		else 
		{
			CheckDlgButton (hwnd, IDC_ENABLEIKEBYTEEXPIRE, BST_UNCHECKED);
			EnableWindow (GetDlgItem (hwnd, IDC_IKEBYTES), FALSE);
			EnableWindow (GetDlgItem (hwnd, IDC_IKEBYTESPIN), FALSE);
		}

		// create and initialize IPSEC expiration date/time picker control
		GetWindowRect (GetDlgItem (hwnd, IDC_IPSECTIMEEXPIRE), &rc);
		MapWindowPoints (NULL, hwnd, (LPPOINT)&rc, 2);
		pos->hwndIpsecTimeExpire = CreateWindowEx (
				WS_EX_CLIENTEDGE, 
				WC_PGPDURATION, "",
				WS_CHILD|WS_TABSTOP|WS_VISIBLE,
				rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, 
				hwnd, (HMENU)IDC_IPSECTIMEEXPIRE, g_hinst, NULL);
						
		SetWindowPos (pos->hwndIpsecTimeExpire, 
							GetDlgItem (hwnd, IDC_ENABLEIPSECTIMEEXPIRE),
							0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);

		LoadString (g_hinst, IDS_EXPIREDAYSTEXT, sz, sizeof(sz));
		SendMessage (pos->hwndIpsecTimeExpire, PGP_DCM_SETTEXT, 
						(WPARAM)DAYS, (LPARAM)sz);

		LoadString (g_hinst, IDS_EXPIREHOURSTEXT, sz, sizeof(sz));
		SendMessage (pos->hwndIpsecTimeExpire, PGP_DCM_SETTEXT, 
						(WPARAM)HOURS, (LPARAM)sz);

		LoadString (g_hinst, IDS_EXPIREMINSTEXT, sz, sizeof(sz));
		SendMessage (pos->hwndIpsecTimeExpire, PGP_DCM_SETTEXT, 
						(WPARAM)MINS, (LPARAM)sz);

		SendMessage (pos->hwndIpsecTimeExpire, PGP_DCM_SETMINDURATION, 0, 
						(LPARAM)MINEXPIRESECONDS);

		SendMessage (pos->hwndIpsecTimeExpire, PGP_DCM_SETDURATION, 0, 
						(LPARAM)g_AOS.pnconfig.uIpsecTimeExpiration);

		// initialize IPSEC time expiration on/off
		if (g_AOS.pnconfig.bIpsecTimeExpiration) 
		{
			CheckDlgButton (hwnd, IDC_ENABLEIPSECTIMEEXPIRE, BST_CHECKED);
			EnableWindow (pos->hwndIpsecTimeExpire, TRUE);
		}
		else 
		{
			CheckDlgButton (hwnd, IDC_ENABLEIPSECTIMEEXPIRE, BST_UNCHECKED);
			EnableWindow (pos->hwndIpsecTimeExpire, FALSE);
		}

		// initialize IPSEC byte expiration controls
		SendDlgItemMessage (hwnd, IDC_IPSECBYTESPIN, UDM_SETRANGE,
				0, (LPARAM)MAKELONG (MAXIPSECMEGABYTES, MINIPSECMEGABYTES));
		SendDlgItemMessage (hwnd, IDC_IPSECBYTES, EM_SETLIMITTEXT, 
				MAXIPSECMEGABYTESDIGITS, 0);

		u = g_AOS.pnconfig.uIpsecKByteExpiration / 1024;
		SendDlgItemMessage (hwnd, IDC_IPSECBYTESPIN, UDM_SETPOS,
				0, (LPARAM)MAKELONG (u, 0));

		if (g_AOS.pnconfig.bIpsecKByteExpiration) 
		{
			CheckDlgButton (hwnd, IDC_ENABLEIPSECBYTEEXPIRE, BST_CHECKED);
			EnableWindow (GetDlgItem (hwnd, IDC_IPSECBYTES), TRUE);
			EnableWindow (GetDlgItem (hwnd, IDC_IPSECBYTESPIN), TRUE);
		}
		else 
		{
			CheckDlgButton (hwnd, IDC_ENABLEIPSECBYTEEXPIRE, BST_UNCHECKED);
			EnableWindow (GetDlgItem (hwnd, IDC_IPSECBYTES), FALSE);
			EnableWindow (GetDlgItem (hwnd, IDC_IPSECBYTESPIN), FALSE);
		}

		// if we don't have privileges, remove the OK button
		if (g_bReadOnly)
			EnableWindow (GetDlgItem (GetParent(hwnd), IDOK), FALSE);

		return FALSE;
	}

	case WM_HELP: 
		WinHelp (((LPHELPINFO) lParam)->hItemHandle, g_szHelpFile, 
			HELP_WM_HELP, (DWORD) (LPVOID) aGeneralIds); 
		break; 
 
	case WM_CONTEXTMENU: 
		WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU, 
			(DWORD) (LPVOID) aGeneralIds); 
		break; 

	case WM_COMMAND :
		pos = (POPTIONSSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);

		switch (LOWORD (wParam)) {
		case IDC_ALLOWUNCONFIGURED :
			if (IsDlgButtonChecked (
					hwnd, IDC_ALLOWUNCONFIGURED) == BST_CHECKED)
			{
				EnableWindow (GetDlgItem (hwnd, IDC_REQUIRESECURE), TRUE);
			}
			else
			{
				EnableWindow (GetDlgItem (hwnd, IDC_REQUIRESECURE), FALSE);
			}
			break;

		case IDC_ENABLEIKETIMEEXPIRE :
			if (IsDlgButtonChecked (
					hwnd, IDC_ENABLEIKETIMEEXPIRE) == BST_CHECKED)
				EnableWindow (pos->hwndIkeTimeExpire, TRUE);
			else
				EnableWindow (pos->hwndIkeTimeExpire, FALSE);
			break;

		case IDC_ENABLEIKEBYTEEXPIRE :
			if (IsDlgButtonChecked (
					hwnd, IDC_ENABLEIKEBYTEEXPIRE) == BST_CHECKED)
			{
				EnableWindow (GetDlgItem (hwnd, IDC_IKEBYTES), TRUE);
				EnableWindow (GetDlgItem (hwnd, IDC_IKEBYTESPIN), TRUE);
			}
			else
			{
				EnableWindow (GetDlgItem (hwnd, IDC_IKEBYTES), FALSE);
				EnableWindow (GetDlgItem (hwnd, IDC_IKEBYTESPIN), FALSE);
			}
			break;

		case IDC_ENABLEIPSECTIMEEXPIRE :
			if (IsDlgButtonChecked (
					hwnd, IDC_ENABLEIPSECTIMEEXPIRE) == BST_CHECKED)
				EnableWindow (pos->hwndIpsecTimeExpire, TRUE);
			else
				EnableWindow (pos->hwndIpsecTimeExpire, FALSE);
			break;

		case IDC_ENABLEIPSECBYTEEXPIRE :
			if (IsDlgButtonChecked (
					hwnd, IDC_ENABLEIPSECBYTEEXPIRE) == BST_CHECKED)
			{
				EnableWindow (GetDlgItem (hwnd, IDC_IPSECBYTES), TRUE);
				EnableWindow (GetDlgItem (hwnd, IDC_IPSECBYTESPIN), TRUE);
			}
			else
			{
				EnableWindow (GetDlgItem (hwnd, IDC_IPSECBYTES), FALSE);
				EnableWindow (GetDlgItem (hwnd, IDC_IPSECBYTESPIN), FALSE);
			}
			break;

		case IDC_IKEBYTES :
			if (HIWORD (wParam) == EN_KILLFOCUS)
			{
				INT	iMegaBytes = 
						GetDlgItemInt (hwnd, IDC_IKEBYTES, NULL, FALSE);

				if (iMegaBytes < MINIKEMEGABYTES)
					SetDlgItemInt (
							hwnd, IDC_IKEBYTES, MINIKEMEGABYTES, FALSE);
				else if (iMegaBytes > MAXIKEMEGABYTES)
					SetDlgItemInt (
							hwnd, IDC_IKEBYTES, MAXIKEMEGABYTES, FALSE);
			}
			break;

		case IDC_IPSECBYTES :
			if (HIWORD (wParam) == EN_KILLFOCUS)
			{
				INT	iMegaBytes = 
						GetDlgItemInt (hwnd, IDC_IPSECBYTES, NULL, FALSE);

				if (iMegaBytes < MINIPSECMEGABYTES)
					SetDlgItemInt (
							hwnd, IDC_IPSECBYTES, MINIPSECMEGABYTES, FALSE);
				else if (iMegaBytes > MAXIPSECMEGABYTES)
					SetDlgItemInt (
							hwnd, IDC_IPSECBYTES, MAXIPSECMEGABYTES, FALSE);
			}
			break;
		}
		break;
	
	case WM_NOTIFY :
	{
		LPNMHDR pnmhdr = (LPNMHDR) lParam;

		pos = (POPTIONSSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);

		switch (pnmhdr->code) {

		case PSN_HELP :
			WinHelp (hwnd, g_szHelpFile, HELP_CONTEXT, IDH_PNOPTGEN_HELP); 
			break;

		case PSN_APPLY :
		{
			pos = (POPTIONSSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);

			g_AOS.pnconfig.bAllowUnconfigHost =
				(IsDlgButtonChecked (
					hwnd, IDC_ALLOWUNCONFIGURED) == BST_CHECKED);

			g_AOS.pnconfig.bRequireValidKey =
				(IsDlgButtonChecked (
					hwnd, IDC_REQUIREVALIDKEY) == BST_CHECKED);

			g_AOS.pnconfig.bRequireSecure =
				(IsDlgButtonChecked (
					hwnd, IDC_REQUIRESECURE) == BST_CHECKED);

			g_AOS.pnconfig.bCachePassphrases =
				(IsDlgButtonChecked (
					hwnd, IDC_ENABLECACHE) == BST_CHECKED);

			g_AOS.pnconfig.bExpertMode =
				(IsDlgButtonChecked (
					hwnd, IDC_EXPERTMODE) == BST_CHECKED);

			SendMessage (pos->hwndIkeTimeExpire, PGP_DCM_GETDURATION, 0, 
							(LPARAM)&g_AOS.pnconfig.uIkeTimeExpiration);

			g_AOS.pnconfig.bIkeTimeExpiration =
				(IsDlgButtonChecked (
					hwnd, IDC_ENABLEIKETIMEEXPIRE) == BST_CHECKED);

			g_AOS.pnconfig.bIkeKByteExpiration =
				(IsDlgButtonChecked (
					hwnd, IDC_ENABLEIKEBYTEEXPIRE) == BST_CHECKED);

			g_AOS.pnconfig.uIkeKByteExpiration =
				GetDlgItemInt (hwnd, IDC_IKEBYTES, NULL, FALSE) * 1024;

			SendMessage (pos->hwndIpsecTimeExpire, PGP_DCM_GETDURATION, 0, 
							(LPARAM)&g_AOS.pnconfig.uIpsecTimeExpiration);

			g_AOS.pnconfig.bIpsecTimeExpiration =
				(IsDlgButtonChecked (
					hwnd, IDC_ENABLEIPSECTIMEEXPIRE) == BST_CHECKED);

			g_AOS.pnconfig.bIpsecKByteExpiration =
				(IsDlgButtonChecked (
					hwnd, IDC_ENABLEIPSECBYTEEXPIRE) == BST_CHECKED);

			g_AOS.pnconfig.uIpsecKByteExpiration =
				GetDlgItemInt (hwnd, IDC_IPSECBYTES, NULL, FALSE) * 1024;

			SetWindowLong (hwnd, DWL_MSGRESULT, PSNRET_NOERROR);
			return TRUE;
		}

		case PSN_KILLACTIVE :
			g_AOS.pnconfig.bExpertMode =
				(IsDlgButtonChecked (
					hwnd, IDC_EXPERTMODE) == BST_CHECKED);

			SetWindowLong (hwnd, DWL_MSGRESULT, PSNRET_NOERROR);
			return TRUE;

		case PSN_RESET :
			pos = (POPTIONSSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
			pos->bUserCancel = TRUE;
			break;
		}
		break;
	}
	}

	return FALSE;
}


//	____________________________________
//
//	put name of key into listview

static VOID
sSetKeyLists (
		HWND			hwnd,
		POPTIONSSTRUCT	pos)
{
	LV_ITEM			lvi;
	CHAR			sz[kPGPMaxUserIDSize +1];
	CHAR			szID[kPGPMaxKeyIDStringSize +1];
	UINT			u;
	PGPKeyID		keyid;
	PGPKeyRef		keyPGP;
	PGPKeyRef		keyX509;
	PGPSigRef		sigX509;
	PGPError		err;
	BOOL			bEnableClear;
#if !PGP_FREEWARE
	PGPSize			size;
	HICON			hIcon;
#endif // !PGP_FREEWARE

	ListView_DeleteAllItems (pos->hwndPGPKeyList);
	SetDlgItemText (hwnd, IDC_X509KEYEDIT, "");

	PGPnetGetConfiguredAuthKeys (g_context, &pos->pnconfig, 
				g_AOS.keysetMain, &keyPGP, &keyX509, &sigX509);

	lvi.mask = LVIF_TEXT|LVIF_IMAGE;
	lvi.iItem = 0;
	lvi.iSubItem = 0;
	lvi.pszText = sz;
	sz[0] = '\0';

	// PGP Authentication key
	bEnableClear = FALSE;
	pos->bPGPKey = FALSE;

	if (pos->pnconfig.uPGPAuthKeyAlg != kPGPPublicKeyAlgorithm_Invalid)
	{
		if (PGPKeyRefIsValid (keyPGP))
		{
			lvi.iImage = PNDetermineKeyIcon (keyPGP, NULL);
			sz[0] = '\0';
			PGPGetPrimaryUserIDNameBuffer (keyPGP, sizeof(sz), sz, &u);
			pos->bPGPKey = TRUE;
		}
		else
		{
			lvi.iImage = IDX_NONE;
			err = PGPImportKeyID (pos->pnconfig.expkeyidPGPAuthKey, &keyid);
			if (IsntPGPError (err))
			{
				LoadString (g_hinst, IDS_UNKNOWNKEY, sz, sizeof(sz));
				PGPGetKeyIDString (&keyid, 
						kPGPKeyIDString_Abbreviated, szID);
				lstrcat (sz, szID);
			}
		}
		ListView_InsertItem (pos->hwndPGPKeyList, &lvi);
		InvalidateRect (pos->hwndPGPKeyList, NULL, FALSE);
		bEnableClear = TRUE;
	}
	EnableWindow (GetDlgItem (hwnd, IDC_CLEARPGPKEY), bEnableClear);

	// then get X509 auth key
	bEnableClear = FALSE;
	pos->bX509Key = FALSE;

#if !PGP_FREEWARE
	if (pos->pnconfig.uX509AuthKeyAlg != kPGPPublicKeyAlgorithm_Invalid)
	{
		if (PGPKeyRefIsValid (keyX509) &&
			PGPSigRefIsValid (sigX509))
		{
			sz[0] = '\0';
			PGPGetSigPropertyBuffer (sigX509,
						kPGPSigPropX509LongName, sizeof(sz), sz, &size);
			SetDlgItemText (hwnd, IDC_X509KEYEDIT, sz);

			u = PNDetermineCertIcon (sigX509, NULL);
			hIcon = ImageList_GetIcon (pos->hil, u, ILD_TRANSPARENT);

			SendDlgItemMessage (
						hwnd, IDC_X509ICON, STM_SETICON, (WPARAM)hIcon, 0);		

			bEnableClear = TRUE;
			pos->bX509Key = TRUE;
		}
		else
		{
			err = PGPImportKeyID (pos->pnconfig.expkeyidX509AuthKey, &keyid);
			if (IsntPGPError (err))
			{
				LoadString (g_hinst, IDS_UNKNOWNCERT, sz, sizeof(sz));
				PGPGetKeyIDString (&keyid, 
						kPGPKeyIDString_Abbreviated, szID);
				lstrcat (sz, szID);
				SetDlgItemText (hwnd, IDC_X509KEYEDIT, sz);
				bEnableClear = TRUE;
			}
		}
	}

	if (!pos->bX509Key)
		SendDlgItemMessage (hwnd, IDC_X509ICON, STM_SETICON, 0, 0);

	EnableWindow (GetDlgItem (hwnd, IDC_CLEARX509KEY), bEnableClear);
#endif // !PGP_FREEWARE
}


//	____________________________________
//
//	initialize the list view control

static VOID
sInitKeyLists (
		POPTIONSSTRUCT	pos)
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
		pos->hil =	ImageList_Create (16, 16, ILC_COLOR|ILC_MASK, 
										NUM_KEY_BITMAPS, 0); 
		hBmp = LoadBitmap (g_hinst, MAKEINTRESOURCE (IDB_KEYIMG4));
		ImageList_AddMasked (pos->hil, hBmp, RGB(255, 0, 255));
		DeleteObject (hBmp);
	}
	else {
		pos->hil =	ImageList_Create (16, 16, ILC_COLOR24|ILC_MASK, 
										NUM_KEY_BITMAPS, 0); 
		hBmp = LoadBitmap (g_hinst, MAKEINTRESOURCE (IDB_KEYIMG24));
		ImageList_AddMasked (pos->hil, hBmp, RGB(255, 0, 255));
		DeleteObject (hBmp);
	}

	// Associate the image list with the tree view control.
	ListView_SetImageList (pos->hwndPGPKeyList, pos->hil, LVSIL_SMALL);

	// add columns
	GetClientRect (pos->hwndPGPKeyList, &rc);
	lvc.mask = LVCF_WIDTH;
	lvc.cx = rc.right - rc.left;
	ListView_InsertColumn (pos->hwndPGPKeyList, 0, &lvc);
}


//	____________________________________
//
//	clear the PGP authentication key

static BOOL
sClearPGPAuthKey (
		HWND			hwnd,
		POPTIONSSTRUCT	pos)
{
	if (pos->pnconfig.uPGPAuthKeyAlg == kPGPPublicKeyAlgorithm_Invalid)
		return FALSE;

	if (PNMessageBox (hwnd, IDS_CAPTION, IDS_CLEARPGPKEYPROMPT, 
			MB_YESNO|MB_ICONEXCLAMATION) == IDNO)
		return FALSE;

	pos->pnconfig.uPGPAuthKeyAlg = kPGPPublicKeyAlgorithm_Invalid;

	return TRUE;
}

//	____________________________________
//
//	clear the X.509 authentication key

static BOOL
sClearX509AuthKey (
		HWND			hwnd,
		POPTIONSSTRUCT	pos)
{
	if (pos->pnconfig.uX509AuthKeyAlg == kPGPPublicKeyAlgorithm_Invalid)
		return FALSE;

	if (PNMessageBox (hwnd, IDS_CAPTION, IDS_CLEARX509KEYPROMPT, 
			MB_YESNO|MB_ICONEXCLAMATION) == IDNO)
		return FALSE;

	pos->pnconfig.uX509AuthKeyAlg = kPGPPublicKeyAlgorithm_Invalid;

	return TRUE;
}


//	____________________________________
//
//	prompt user for keyrings

static BOOL
sSelectKeyring (
		HWND			hwnd,
		BOOL			bSecretRing,
		POPTIONSSTRUCT	pos)
{
	CHAR	szFile[MAX_PATH];
	CHAR	szFilter[128];
	CHAR	szTitle[128];
	CHAR	szDefExt[8];
	LPSTR	p;

	OPENFILENAME	ofn;

	if (bSecretRing)
	{
		GetDlgItemText (hwnd, IDC_SECRING, szFile, sizeof(szFile));
		LoadString (g_hinst, IDS_SECRINGFILTER, szFilter, sizeof(szFilter));
		LoadString (g_hinst, IDS_SECRINGCAPTION, szTitle, sizeof(szTitle));
		LoadString (g_hinst, IDS_DEFSECRINGEXT, szDefExt, sizeof(szDefExt));
	}
	else
	{
		GetDlgItemText (hwnd, IDC_PUBRING, szFile, sizeof(szFile));
		LoadString (g_hinst, IDS_PUBRINGFILTER, szFilter, sizeof(szFilter));
		LoadString (g_hinst, IDS_PUBRINGCAPTION, szTitle, sizeof(szTitle));
		LoadString (g_hinst, IDS_DEFPUBRINGEXT, szDefExt, sizeof(szDefExt));
	}

	while (p = strrchr (szFilter, '@')) *p = '\0';

	ofn.lStructSize			= sizeof(OPENFILENAME);
	ofn.hwndOwner			= hwnd;
	ofn.hInstance			= g_hinst;
	ofn.lpstrFilter			= szFilter;
	ofn.lpstrCustomFilter	= NULL;
	ofn.nMaxCustFilter		= 0L;
	ofn.nFilterIndex		= 1L;
	ofn.lpstrFile			= szFile;
	ofn.nMaxFile			= sizeof(szFile);
	ofn.lpstrFileTitle		= NULL;
	ofn.nMaxFileTitle		= 0;
	ofn.lpstrInitialDir		= NULL;
	ofn.lpstrTitle			= szTitle;
	ofn.Flags				= OFN_HIDEREADONLY|OFN_FILEMUSTEXIST;
	ofn.nFileOffset			= 0;
	ofn.nFileExtension		= 0;
	ofn.lpstrDefExt			= szDefExt;
	ofn.lCustData			= 0;

	if (GetOpenFileName (&ofn)) 
	{
		if (bSecretRing)
			SetDlgItemText (hwnd, IDC_SECRING, szFile);
		else
			SetDlgItemText (hwnd, IDC_PUBRING, szFile);

		return TRUE;
	}
	else
		return FALSE;
}


//	____________________________________
//
//	compare set keyrings to current user's and enable button accordingly

static VOID
sCompareToCurrent (
		HWND			hwnd)
{
	BOOL			bSameAsCurrentUser	= TRUE;
	PGPContextRef	context;
	PGPFileSpecRef	fileRef;
	CHAR			sz[MAX_PATH];
	LPSTR			pszCurrent;

	PGPNewContext (kPGPsdkAPIVersion, &context);
	if (PGPContextRefIsValid (context))
	{
		PGPsdkLoadDefaultPrefs (context);

		GetDlgItemText (hwnd, IDC_PUBRING, sz, sizeof(sz));

		pszCurrent = NULL;
		PGPsdkPrefGetFileSpec (
				context, kPGPsdkPref_PublicKeyring, &fileRef);
		PGPGetFullPathFromFileSpec (fileRef, &pszCurrent);

		if (PGPFileSpecRefIsValid (fileRef))
		{
			if (pszCurrent)
			{
				if (strcmpi (sz, pszCurrent))
					bSameAsCurrentUser = FALSE;
				PGPFreeData (pszCurrent);
			}
			PGPFreeFileSpec (fileRef);
		}

		GetDlgItemText (hwnd, IDC_SECRING, sz, sizeof(sz));

		pszCurrent = NULL;
		PGPsdkPrefGetFileSpec (
				context, kPGPsdkPref_PrivateKeyring, &fileRef);
		PGPGetFullPathFromFileSpec (fileRef, &pszCurrent);

		if (PGPFileSpecRefIsValid (fileRef))
		{
			if (pszCurrent)
			{
				if (strcmpi (sz, pszCurrent))
					bSameAsCurrentUser = FALSE;
				PGPFreeData (pszCurrent);
			}
			PGPFreeFileSpec (fileRef);
		}
		PGPFreeContext (context);
	}

	if (bSameAsCurrentUser)
		EnableWindow (GetDlgItem (hwnd, IDC_SETTOCURRENT), FALSE);
	else
		EnableWindow (GetDlgItem (hwnd, IDC_SETTOCURRENT), TRUE);
}


//	____________________________________
//
//	fill-in the keyring controls

static VOID
sGetKeyringPrefs (
		HWND			hwnd,
		POPTIONSSTRUCT	pos)
{
	PGPFileSpecRef	fileRef;
	PGPError		err;
	LPSTR			psz;

	err = PGPsdkPrefGetFileSpec (
				g_context, kPGPsdkPref_PublicKeyring, &fileRef);
	if (IsntPGPError (err) && fileRef) 
	{
		PGPGetFullPathFromFileSpec (fileRef, &psz);
		SetDlgItemText (hwnd, IDC_PUBRING, psz);
		PGPFreeFileSpec (fileRef);
		PGPFreeData (psz);
	}

	err = PGPsdkPrefGetFileSpec (
				g_context, kPGPsdkPref_PrivateKeyring, &fileRef);
	if (IsntPGPError (err) && fileRef) 
	{
		PGPGetFullPathFromFileSpec (fileRef, &psz);
		SetDlgItemText (hwnd, IDC_SECRING, psz);
		PGPFreeFileSpec (fileRef);
		PGPFreeData (psz);
	}

	sCompareToCurrent (hwnd);
}


//	____________________________________
//
//	get keyrings from controls and set sdk prefs

static VOID
sSetKeyringPrefs (
		HWND			hwnd,
		BOOL			bSecretRing,
		POPTIONSSTRUCT	pos)
{
	PGPFileSpecRef	fileRef;
	PGPFileSpecRef	fileRefOrig;
	PGPError		err;
	CHAR			szFile[MAX_PATH];

	if (bSecretRing) 
	{
		PGPsdkPrefGetFileSpec (
				g_context, kPGPsdkPref_PrivateKeyring, &fileRefOrig);
		GetDlgItemText (hwnd, IDC_SECRING, szFile, sizeof(szFile));
		err = PGPNewFileSpecFromFullPath (g_context, szFile, &fileRef);

		if (!PGPclErrorBox (hwnd, err)) 
		{
			err = PGPsdkPrefSetFileSpec (g_context, 
							kPGPsdkPref_PrivateKeyring, fileRef);
			PGPclErrorBox (hwnd, err);
			PGPFreeFileSpec (fileRef);
		}
	}

	else
	{
		PGPsdkPrefGetFileSpec (
				g_context, kPGPsdkPref_PublicKeyring, &fileRefOrig);
		GetDlgItemText (hwnd, IDC_PUBRING, szFile, sizeof(szFile));
		err = PGPNewFileSpecFromFullPath (g_context, szFile, &fileRef);

		if (!PGPclErrorBox (hwnd, err)) 
		{
			err = PGPsdkPrefSetFileSpec (g_context, 
							kPGPsdkPref_PublicKeyring, fileRef);
			PGPclErrorBox (hwnd, err);
			PGPFreeFileSpec (fileRef);
		}
	}

	if (!PNLoadKeyRings (hwnd, &g_AOS.keysetMain)) 
	{
		if (bSecretRing)
		{
			PGPsdkPrefSetFileSpec (g_context, 
							kPGPsdkPref_PrivateKeyring, fileRefOrig);
		}
		else
		{
			PGPsdkPrefSetFileSpec (g_context, 
							kPGPsdkPref_PublicKeyring, fileRefOrig);
		}
	}

	if (PGPFileSpecRefIsValid (fileRefOrig))
		PGPFreeFileSpec (fileRefOrig);
}


//	____________________________________
//
//	get the current user's keyrings and set controls to them

static BOOL
sSetToCurrentUsersKeyrings (
		HWND			hwnd,
		POPTIONSSTRUCT	pos)
{
	PGPContextRef	context;
	PGPFileSpecRef	fileRef;
	PGPError		err;

	err = PGPNewContext (kPGPsdkAPIVersion, &context);
	if (IsntPGPError (err))
	{
		PGPsdkLoadDefaultPrefs (context);

		PGPsdkPrefGetFileSpec (
				context, kPGPsdkPref_PublicKeyring, &fileRef);
		PGPsdkPrefSetFileSpec (
				g_context, kPGPsdkPref_PublicKeyring, fileRef);
		PGPFreeFileSpec (fileRef);

		PGPsdkPrefGetFileSpec (
				context, kPGPsdkPref_PrivateKeyring, &fileRef);
		PGPsdkPrefSetFileSpec (
				g_context, kPGPsdkPref_PrivateKeyring, fileRef);
		PGPFreeFileSpec (fileRef);

		PNLoadKeyRings (hwnd, &g_AOS.keysetMain);

		pos->bKeyringsChanged = TRUE;
		PGPFreeContext (context);
	}

	if (IsntPGPError (err))
		return TRUE;
	else
		return FALSE;
}


//	____________________________________
//
//	authentication options dialog proc

static BOOL CALLBACK
sAuthOptionsDlgProc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam)
{
	POPTIONSSTRUCT	pos;

	switch (uMsg)
	{
	case WM_INITDIALOG :
		SetWindowLong (hwnd, GWL_USERDATA, ((PROPSHEETPAGE*)lParam)->lParam);
		pos = (POPTIONSSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);

		g_hwndOptions = hwnd;
		pos->hwndPGPKeyList = GetDlgItem (hwnd, IDC_PGPKEYLIST);

		sInitKeyLists (pos);
		g_AOS.hwndAuthDlg = hwnd;

#if !PGP_FREEWARE
		ShowWindow (GetDlgItem (hwnd, IDC_X509GROUPBOX), SW_SHOW);
		ShowWindow (GetDlgItem (hwnd, IDC_X509BORDER), SW_SHOW);
		ShowWindow (GetDlgItem (hwnd, IDC_X509ICON), SW_SHOW);
		ShowWindow (GetDlgItem (hwnd, IDC_X509KEYEDIT), SW_SHOW);
		ShowWindow (GetDlgItem (hwnd, IDC_SELECTX509KEY), SW_SHOW);
		ShowWindow (GetDlgItem (hwnd, IDC_CLEARX509KEY), SW_SHOW);
#endif //PGP_FREEWARE

		// enable key selection only if there are enough privileges
		if (g_bAuthAccess && !g_bReadOnly)
		{
			EnableWindow (GetDlgItem (hwnd, IDC_SELECTPGPKEY), TRUE);
#if !PGP_FREEWARE
			EnableWindow (GetDlgItem (hwnd, IDC_SELECTX509KEY), TRUE);
#endif //PGP_FREEWARE
			sGetKeyringPrefs (hwnd, pos);
		}

		PostMessage (hwnd, WM_APP, 0, 0);	
		return FALSE;

	case WM_APP :
		pos = (POPTIONSSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		if (g_bAuthAccess)
		{
			sSetKeyLists (hwnd, pos); 
		}
		break;

	case WM_PAINT :
		pos = (POPTIONSSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);

		break;

	case PGPNET_M_RELOADKEYRINGS :
		PostMessage (hwnd, WM_APP, 0, 0);
		break;

	case WM_NOTIFY :
	{
		pos = (POPTIONSSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);

		switch (((NMHDR FAR *) lParam)->code) {
		case PSN_HELP :
			WinHelp (hwnd, g_szHelpFile, HELP_CONTEXT, IDH_PNOPTAUTH_HELP); 
			break;

		case PSN_RESET :
			pos = (POPTIONSSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
			pos->bUserCancel = TRUE;
			break;

		case PSN_APPLY :
			pos = (POPTIONSSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
			if ((pos->bPGPKey != g_AOS.bPGPKey) ||
				(pos->bX509Key != g_AOS.bX509Key))
				g_AOS.bUpdateHostList = TRUE;

			g_AOS.bPGPKey = pos->bPGPKey;
			g_AOS.bX509Key = pos->bX509Key;

			g_AOS.pnconfig.uPGPAuthKeyAlg = pos->pnconfig.uPGPAuthKeyAlg;
			pgpCopyMemory (	pos->pnconfig.expkeyidPGPAuthKey,
							&g_AOS.pnconfig.expkeyidPGPAuthKey, 
							kPGPMaxExportedKeyIDSize);

			g_AOS.pnconfig.uX509AuthKeyAlg = pos->pnconfig.uX509AuthKeyAlg;
			pgpCopyMemory (	pos->pnconfig.expkeyidX509AuthKey,
							&g_AOS.pnconfig.expkeyidX509AuthKey, 
							kPGPMaxExportedKeyIDSize);

			if (g_AOS.pnconfig.pX509AuthCertIASN)
				PGPFreeData (g_AOS.pnconfig.pX509AuthCertIASN);
			g_AOS.pnconfig.pX509AuthCertIASN = 
							pos->pnconfig.pX509AuthCertIASN;
			g_AOS.pnconfig.uX509AuthCertIASNLength = 
							pos->pnconfig.uX509AuthCertIASNLength;

			pos->pnconfig.pX509AuthCertIASN = NULL;

			break;
		}
		break;
	}

	case WM_DESTROY :
		pos = (POPTIONSSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		ImageList_Destroy (pos->hil);
		break;

	case WM_HELP: 
		WinHelp (((LPHELPINFO) lParam)->hItemHandle, g_szHelpFile, 
			HELP_WM_HELP, (DWORD) (LPVOID) aAuthIds); 
		break; 
 
	case WM_CONTEXTMENU: 
		WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU, 
			(DWORD) (LPVOID) aAuthIds); 
		break; 

	case WM_COMMAND :
	{
		PGPError	err;

		pos = (POPTIONSSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);

		switch (LOWORD (wParam)) {
		case IDC_PUBRINGBROWSE :
			if (sSelectKeyring (hwnd, FALSE, pos))
			{
				pos->bKeyringsChanged = TRUE;
				sSetKeyringPrefs (hwnd, FALSE, pos);
				sCompareToCurrent (hwnd);
				PostMessage (hwnd, WM_APP, 0, 0);
			}
			break;

		case IDC_SECRINGBROWSE :
			if (sSelectKeyring (hwnd, TRUE, pos))
			{
				pos->bKeyringsChanged = TRUE;
				sSetKeyringPrefs (hwnd, TRUE, pos);
				sCompareToCurrent (hwnd);
				PostMessage (hwnd, WM_APP, 0, 0);
			}
			break;

		case IDC_SETTOCURRENT :
			if (sSetToCurrentUsersKeyrings (hwnd, pos))
			{
				sGetKeyringPrefs (hwnd, pos);
				PostMessage (hwnd, WM_APP, 0, 0);
			}
			break;

		case IDC_SELECTPGPKEY :
			err = PNSelectPGPKey (g_context, g_AOS.keysetMain, hwnd,
						&pos->pnconfig.uPGPAuthKeyAlg, 
						pos->pnconfig.expkeyidPGPAuthKey);

			PostMessage (hwnd, WM_APP, 0, 0);
			break;

		case IDC_CLEARPGPKEY :
			if (sClearPGPAuthKey (hwnd, pos)) 
				PostMessage (hwnd, WM_APP, 0, 0);
			break;

		case IDC_SELECTX509KEY :
		{
			PGPKeyRef	key;
			PGPSigRef	sig;

			err = PGPclSelectX509Cert (g_context, hwnd, NULL,
						g_AOS.keysetMain, 
						PGPCL_CANSIGNKEYSONLY|PGPCL_NOSPLITKEYS,
						&key, &sig);
			if (IsntPGPError (err))
			{
				if (IsntNull (pos->pnconfig.pX509AuthCertIASN))
					PGPFreeData (pos->pnconfig.pX509AuthCertIASN);

				PGPX509CertToExport (g_context, key, sig,
						&(pos->pnconfig.uX509AuthKeyAlg), 
						pos->pnconfig.expkeyidX509AuthKey,
						&(pos->pnconfig.pX509AuthCertIASN),
						&(pos->pnconfig.uX509AuthCertIASNLength));
			}

			PostMessage (hwnd, WM_APP, 0, 0);
			break;
		}

		case IDC_CLEARX509KEY :
			if (sClearX509AuthKey (hwnd, pos)) 
				PostMessage (hwnd, WM_APP, 0, 0);
			break;
		}
		return 0;
	}
	}

	return FALSE;
}

//	____________________________________
//
//	dialog proc of hosts dialog

VOID
PNOptionsPropSheet (
		HWND	hwnd,
		UINT	uStartPage)
{
	PROPSHEETHEADER		psh;
	PROPSHEETPAGE		psp[PGPNET_NUMOPTIONS];
	CHAR				szCaption[64];
	OPTIONSSTRUCT		os;
	PGPUInt32			uNumProposals;
	PGPSize				size;

	pgpClearMemory (&os, sizeof(os));

	// initialize option structure 
	os.iSelectedCertItem	= -1;
	os.iSelectedCertParam	= -1;
	os.bUserCancel			= FALSE;
	os.bKeyringsChanged		= FALSE;

	// copy entire PNCONFIG structure
	pgpCopyMemory (&g_AOS.pnconfig, &os.pnconfig, sizeof(PNCONFIG));

	// copy authentication info for local use
	if (g_AOS.pnconfig.pX509AuthCertIASN)
	{
		os.pnconfig.pX509AuthCertIASN = 
				PGPNewData (PGPGetContextMemoryMgr (g_context),
							g_AOS.pnconfig.uX509AuthCertIASNLength, 
							kPGPMemoryMgrFlags_Clear);

		pgpCopyMemory (	g_AOS.pnconfig.pX509AuthCertIASN, 
						os.pnconfig.pX509AuthCertIASN,
						g_AOS.pnconfig.uX509AuthCertIASNLength);
	}

	// copy IKE proposal list for local use
	uNumProposals = 
		g_AOS.pnconfig.IkeIkeProposalPrefs.u.ikeProposals.numTransforms;
												;
	size = uNumProposals * sizeof(PGPikeTransform);

	os.pnconfig.IkeIkeProposalPrefs.u.ikeProposals.t =
			PGPNewData (PGPGetContextMemoryMgr (g_context),
					size, kPGPMemoryMgrFlags_Clear);
	pgpCopyMemory (	g_AOS.pnconfig.IkeIkeProposalPrefs.u.ikeProposals.t,
					os.pnconfig.IkeIkeProposalPrefs.u.ikeProposals.t,
					size);

	// copy IPSEC proposal list for local use
	uNumProposals = 
		g_AOS.pnconfig.IkeIpsecProposalPrefs.u.ipsecProposals.numTransforms;
												;
	size = uNumProposals * sizeof(PGPipsecTransform);

	os.pnconfig.IkeIpsecProposalPrefs.u.ipsecProposals.t = 
			PGPNewData (PGPGetContextMemoryMgr (g_context),
					size, kPGPMemoryMgrFlags_Clear);
	pgpCopyMemory (	g_AOS.pnconfig.IkeIpsecProposalPrefs.u.ipsecProposals.t, 
					os.pnconfig.IkeIpsecProposalPrefs.u.ipsecProposals.t,
					size);

	psp[PGPNET_GENOPTIONS].dwSize		= sizeof(PROPSHEETPAGE);
	psp[PGPNET_GENOPTIONS].dwFlags		= PSP_HASHELP;
	psp[PGPNET_GENOPTIONS].hInstance	= g_hinst;
	psp[PGPNET_GENOPTIONS].pszTemplate	= MAKEINTRESOURCE (IDD_GENOPTIONS);
	psp[PGPNET_GENOPTIONS].hIcon		= NULL;
	psp[PGPNET_GENOPTIONS].pszTitle		= NULL;
	psp[PGPNET_GENOPTIONS].pfnDlgProc	= sGenOptionsDlgProc;
	psp[PGPNET_GENOPTIONS].lParam		= (LPARAM)&os;
	psp[PGPNET_GENOPTIONS].pfnCallback	= NULL;
	psp[PGPNET_GENOPTIONS].pcRefParent	= NULL;

	psp[PGPNET_AUTHOPTIONS].dwSize		= sizeof(PROPSHEETPAGE);
	psp[PGPNET_AUTHOPTIONS].dwFlags		= PSP_HASHELP;
	psp[PGPNET_AUTHOPTIONS].hInstance	= g_hinst;
	psp[PGPNET_AUTHOPTIONS].pszTemplate	= MAKEINTRESOURCE (IDD_AUTHOPTIONS);
	psp[PGPNET_AUTHOPTIONS].hIcon		= NULL;
	psp[PGPNET_AUTHOPTIONS].pszTitle	= NULL;
	psp[PGPNET_AUTHOPTIONS].pfnDlgProc	= sAuthOptionsDlgProc;
	psp[PGPNET_AUTHOPTIONS].lParam		= (LPARAM)&os;
	psp[PGPNET_AUTHOPTIONS].pfnCallback	= NULL;
	psp[PGPNET_AUTHOPTIONS].pcRefParent	= NULL;

	psp[PGPNET_ADVOPTIONS].dwSize		= sizeof(PROPSHEETPAGE);
	psp[PGPNET_ADVOPTIONS].dwFlags		= PSP_HASHELP;
	psp[PGPNET_ADVOPTIONS].hInstance	= g_hinst;
	psp[PGPNET_ADVOPTIONS].pszTemplate	= MAKEINTRESOURCE (IDD_ADVANCEDOPTIONS);
	psp[PGPNET_ADVOPTIONS].hIcon		= NULL;
	psp[PGPNET_ADVOPTIONS].pszTitle		= NULL;
	psp[PGPNET_ADVOPTIONS].pfnDlgProc	= PNAdvancedOptionsDlgProc;
	psp[PGPNET_ADVOPTIONS].lParam		= (LPARAM)&os;
	psp[PGPNET_ADVOPTIONS].pfnCallback	= NULL;
	psp[PGPNET_ADVOPTIONS].pcRefParent	= NULL;

	LoadString (g_hinst, IDS_OPTIONSCAPTION, szCaption, sizeof(szCaption));
	psh.dwSize		= sizeof(PROPSHEETHEADER);
	psh.dwFlags		= PSH_NOAPPLYNOW|PSH_PROPSHEETPAGE|PSH_HASHELP;
	psh.hwndParent	= hwnd;
	psh.hInstance	= g_hinst;
	psh.hIcon		= NULL;
	psh.pszCaption	= szCaption;
	psh.nPages		= PGPNET_NUMOPTIONS;
	psh.nStartPage	= uStartPage;
	psh.ppsp		= &psp[0];
	psh.pfnCallback	= NULL;

	PropertySheet (&psh);

	g_AOS.hwndAuthDlg = NULL;

	if (os.bUserCancel)
	{
		PGPnetLoadSDKPrefs (g_context);
		if (os.bKeyringsChanged)
		{
			PNLoadKeyRings (hwnd, &g_AOS.keysetMain);
		}
	}
	else
	{
		PGPsdkSavePrefs (g_context);
		PNSaveConfiguration (hwnd, &g_AOS.pnconfig, os.bKeyringsChanged);
	}

	if (IsntNull (os.pnconfig.pX509AuthCertIASN))
		PGPFreeData (os.pnconfig.pX509AuthCertIASN);
	if (IsntNull (os.pnconfig.IkeIkeProposalPrefs.u.ikeProposals.t))
		PGPFreeData (os.pnconfig.IkeIkeProposalPrefs.u.ikeProposals.t);
	if (IsntNull (os.pnconfig.IkeIpsecProposalPrefs.u.ipsecProposals.t))
		PGPFreeData (os.pnconfig.IkeIpsecProposalPrefs.u.ipsecProposals.t);

	g_hwndOptions = NULL;
}


