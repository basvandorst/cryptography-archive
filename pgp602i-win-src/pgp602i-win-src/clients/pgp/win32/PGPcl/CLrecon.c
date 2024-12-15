/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	CLrecon.c - reconstitute key shares routines
	

	$Id: CLrecon.c,v 1.39 1998/08/11 14:43:14 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"	/* or pgpConfig.h in the CDK */

// project header files
#include "pgpclx.h"
#include "pgpkmx.h"

// pgp header files
#include "PGPskep.h"
#include "pgpShareFile.h"

// system header files
#include <commdlg.h>

// external globals
extern HINSTANCE		g_hInst;
extern CHAR g_szHelpFile[MAX_PATH];

// constant definitions
#define INITIAL_SHAREKEY_COLUMNWIDTH	250
#define UNKNOWN_SHARES_NEEDED			999999

// typedefs
typedef struct {
	PGPContextRef		context;
	PGPtlsContextRef	tlsContext;
	HWND				hwndDlg;
	HWND				hwndList;
	HIMAGELIST			hIml;
	PGPKeySetRef		keyset;
	PGPKeyRef			keyToReconstitute;
	PGPKeyID			keyidToReconstitute;
	PGPKeyRef			keyAuthenticating;
	PGPKeySetRef		keysetDecryption;
	PGPUInt32			iKeyIDCount;
	PGPKeyID*			keyidsDecryption;
	PGPKeySetRef		keysetToAdd;
	CHAR				szAuthUserID[kPGPMaxUserIDSize+1];
	LPSTR				pszPhraseAuth;
	PGPByte*			pPasskeyAuth;
	PGPSize				sizePasskeyAuth;
	UINT				uNeededShares;
	UINT				uCollectedShares;
	PGPShareRef			sharesCombined;
	PGPskepRef			skep;
	INT					iIconIndex;
	BOOL				bServerMode;
	BOOL				bStop;
	BOOL				bUserCancel;
	BOOL				bUserOK;
	BOOL				bBadPassphrase;
	CRITICAL_SECTION	critsecAddShare;
} RECONKEYSTRUCT, *PRECONKEYSTRUCT;

// help IDs
static DWORD aIds[] = {	
	IDC_NETWORK,		IDH_PGPCLRECON_STARTNETWORK, // must be first ID
	IDC_KEYTORECON,		IDH_PGPCLRECON_KEYTORECON,
	IDC_SHAREHOLDERS,	IDH_PGPCLRECON_SHAREHOLDERLIST,
	IDC_SHARESCOLLECTED,IDH_PGPCLRECON_SHARESCOLLECTED,
	IDC_SHARESNEEDED,	IDH_PGPCLRECON_SHARESNEEDED,
	IDC_STATUS,			IDH_PGPCLRECON_NETWORKSTATUS,
	IDC_AUTHENTICATION,	IDH_PGPCLRECON_NETWORKAUTHENTICATION,
	IDC_SHAREFILE,		IDH_PGPCLRECON_SELECTSHAREFILE,
    0,0 
}; 


//	______________________________________________
//
//  Initialize ListView

static VOID 
sInitKeyList (PRECONKEYSTRUCT prks) 
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
		prks->hIml = ImageList_Create (16, 16, ILC_COLOR|ILC_MASK, 
										NUM_BITMAPS, 0); 
		hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_IMAGES4BIT));
		ImageList_AddMasked (prks->hIml, hBmp, RGB(255, 0, 255));
		DeleteObject (hBmp);
	}
	else {
		prks->hIml = ImageList_Create (16, 16, ILC_COLOR24|ILC_MASK, 
										NUM_BITMAPS, 0); 
		hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_IMAGES24BIT));
		ImageList_AddMasked (prks->hIml, hBmp, RGB(255, 0, 255));
		DeleteObject (hBmp);
	}

	ListView_SetImageList (prks->hwndList, prks->hIml, LVSIL_SMALL);

	lvC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lvC.fmt = LVCFMT_LEFT; 
	lvC.pszText = sz;

	LoadString (g_hInst, IDS_USERID, sz, sizeof(sz));
	lvC.cx = INITIAL_SHAREKEY_COLUMNWIDTH;   
	lvC.iSubItem = 0;
	if (ListView_InsertColumn (prks->hwndList, 0, &lvC) == -1) return;

	LoadString (g_hInst, IDS_SHARES, sz, sizeof(sz));
	lvC.cx = 50;   
	lvC.iSubItem = 1;
	if (ListView_InsertColumn (prks->hwndList, 1, &lvC) == -1) return;
}


//	______________________________________________
//
//  decode event handler

static PGPError
sHandlerDecode (
		PGPContextRef	context,
		PGPEvent*		event,
		PGPUserValue	userValue)
{
	PGPError		err		= kPGPError_NoErr;
	PRECONKEYSTRUCT	prks;

	switch (event->type) {
	case kPGPEvent_PassphraseEvent:
		{
			LPSTR		psz			= NULL;
			PGPByte*	pbyte		= NULL;
			PGPSize		size;
			UINT		uLen;
			CHAR		szPrompt[64];

			prks = (PRECONKEYSTRUCT)userValue;

			if (!prks->bBadPassphrase)
				LoadString (g_hInst, IDS_DECRYPTSHARESPROMPT, 
											szPrompt, sizeof(szPrompt));
			else
				LoadString (g_hInst, IDS_BADSHAREFILEPASSPHRASE, 
											szPrompt, sizeof(szPrompt));

			if (event->data.passphraseData.fConventional) {
				err = KMGetKeyPhrase (context, prks->tlsContext, 
								prks->hwndDlg, szPrompt, prks->keyset, 
								NULL, &psz, &pbyte, &size);
				prks->iIconIndex = IDX_HEAD;
			}
			else {
				err = KMGetDecryptionPhrase (context, prks->tlsContext, 
								prks->hwndDlg, szPrompt, prks->keyset, 
								NULL, prks->keysetDecryption,
								prks->iKeyIDCount, prks->keyidsDecryption,
								&prks->keysetToAdd, &psz, &pbyte, &size);
				prks->iIconIndex = IDX_DSAUSERID;
			}

			if (IsntPGPError (err)) {
				if (psz) {
					uLen = lstrlen (psz);
					PGPAddJobOptions (event->job, 
						PGPOPassphraseBuffer (context, psz, uLen),
						PGPOLastOption (context));
				}
				else {
					PGPAddJobOptions (event->job, 
						PGPOPasskeyBuffer (context, pbyte, size),
						PGPOLastOption (context));
				}
			}

			if (psz) 
				KMFreePhrase (psz);

			if (pbyte)
				KMFreePasskey (pbyte, size);

			// If passphrase event comes up again, the passphrase
			// must have been bad

			prks->bBadPassphrase = TRUE;
		}
		break;

	case kPGPEvent_RecipientsEvent:
		{
			PGPUInt32	i;

			PGPEventRecipientsData *pData = &event->data.recipientsData;
			prks = (PRECONKEYSTRUCT)userValue;
		
			// Save recipient key set for passphrase dialog
			prks->keysetDecryption = pData->recipientSet;
			PGPIncKeySetRefCount (prks->keysetDecryption);

			// Save unknown keyids
			if (pData->keyCount > 0) {
				prks->iKeyIDCount = pData->keyCount;
				prks->keyidsDecryption = 
					(PGPKeyID *)clAlloc (pData->keyCount * sizeof(PGPKeyID));

				for (i=0; i<pData->keyCount; i++) {
					prks->keyidsDecryption[i] = pData->keyIDArray[i];
				}
			}
		}
		break;
	}

	return err;
}


//	______________________________________________
//
//  add shareholder to listview control

static BOOL 
sAddShareHolderToList (
		PRECONKEYSTRUCT prks, 
		LPSTR			pszName,
		UINT			uShares)
{
	LV_ITEM		lvI;
	INT			iItem;	
	CHAR		sz[16];

	// figure item index to use
	iItem = ListView_GetItemCount (prks->hwndList);

	// insert listview item
	lvI.mask = LVIF_TEXT|LVIF_IMAGE|LVIF_STATE;
	lvI.state = 0;      
	lvI.stateMask = 0;
	lvI.iImage = prks->iIconIndex;

	lvI.iItem = iItem;
	lvI.iSubItem = 0;
	lvI.pszText	= pszName; 
	lvI.cchTextMax = 0;

	iItem = ListView_InsertItem (prks->hwndList, &lvI);
	if (iItem == -1) {
		return FALSE;
	}
	else {
		// add strings for Shares column
		wsprintf (sz, "%i", uShares);
		ListView_SetItemText (prks->hwndList, iItem, 1, sz);
		return TRUE;
	}
}


//	______________________________________________
//
//  add share file to list

static VOID 
sAddShareFile (
		HWND			hwnd, 
		PRECONKEYSTRUCT	prks)
{
	PFLFileSpecRef		fileref				= NULL;
	PGPShareFileRef		sharefileref		= NULL;
	PGPOptionListRef	optionsDecode		= NULL;
	PGPShareRef			shares				= NULL;
	PGPShareRef			sharesTemp			= NULL;
	PGPError			err					= kPGPError_NoErr;

	OPENFILENAME	ofn;
	LPSTR			p;
	CHAR			szFile[MAX_PATH];
	CHAR			szName[kPGPMaxUserIDSize+1];
	CHAR			sz[256];
	CHAR			szTitle[64];
	PGPUInt32		size;
	PGPUInt32		uNumShares;
	UINT			uThreshold;
	PGPKeyID		keyid;

	// initialize
	prks->keysetToAdd		= kInvalidPGPKeySetRef;
	prks->keysetDecryption	= kInvalidPGPKeySetRef;
	prks->iKeyIDCount		= 0;
	prks->keyidsDecryption	= NULL;

	// prompt user for name of share file to send
	szFile[0] = '\0';
	LoadString (g_hInst, IDS_SHAREFILEFILTER, sz, sizeof(sz));
	while (p = strrchr (sz, '@')) *p = '\0';
	LoadString (g_hInst, IDS_SHAREFILECAPTION, szTitle, sizeof(szTitle));

	ofn.lStructSize       = sizeof (OPENFILENAME);
	ofn.hwndOwner         = hwnd;
	ofn.hInstance         = (HANDLE)g_hInst;
	ofn.lpstrFilter       = sz;
	ofn.lpstrCustomFilter = (LPTSTR)NULL;
	ofn.nMaxCustFilter    = 0L;
	ofn.nFilterIndex      = 1L;
	ofn.lpstrFile         = szFile;
	ofn.nMaxFile          = sizeof (szFile);
	ofn.lpstrFileTitle    = NULL;
	ofn.nMaxFileTitle     = 0;
	ofn.lpstrInitialDir   = NULL;
	ofn.lpstrTitle        = szTitle;
	ofn.Flags			  = OFN_HIDEREADONLY;
	ofn.nFileOffset       = 0;
	ofn.nFileExtension    = 0;
	ofn.lpstrDefExt       = "";
	ofn.lCustData         = 0;

	EnterCriticalSection (&prks->critsecAddShare);

	if (GetOpenFileName (&ofn)) {
		err = PFLNewFileSpecFromFullPath (
				PGPGetContextMemoryMgr (prks->context), szFile, &fileref);
		if (IsPGPError (err)) goto AddCleanup;
 
		err = PGPOpenShareFile (fileref, &sharefileref);
		if (IsPGPError (err)) goto AddCleanup;

		err = PGPGetShareFileSharedKeyID (sharefileref, &keyid);
		if (IsPGPError (err)) goto AddCleanup;

		if (PGPCompareKeyIDs (&keyid, &(prks->keyidToReconstitute))) {
			PGPclMessageBox (prks->hwndDlg, IDS_CAPTION, 
						IDS_SHAREKEYMISMATCH, MB_OK|MB_ICONEXCLAMATION);
			goto AddCleanup;
		}

		// check that threshold corresponds to other share files
		uThreshold = PGPGetShareThresholdInFile (sharefileref);
		if (prks->uNeededShares != UNKNOWN_SHARES_NEEDED) {
			if (uThreshold != prks->uNeededShares) {
				PGPclMessageBox (prks->hwndDlg, IDS_CAPTION, 
						IDS_SHARENUMMISMATCH, MB_OK|MB_ICONEXCLAMATION);
				goto AddCleanup;
			}
		}

		err = PGPGetShareFileUserID (sharefileref,
									sizeof(szName), szName, &size);
		if (IsPGPError (err)) goto AddCleanup;

		uNumShares = PGPGetNumSharesInFile (sharefileref);

		// decrypt specified share file
		prks->bBadPassphrase = FALSE;
		PGPBuildOptionList (prks->context, &optionsDecode,
					PGPOKeySetRef (prks->context, prks->keyset),
					PGPOEventHandler (prks->context, sHandlerDecode, prks),
					PGPOLastOption (prks->context));
		err = PGPCopySharesFromFile (prks->context, sharefileref, 
						optionsDecode, &shares);
		if (IsPGPError (err)) goto AddCleanup;

		// add shares to collection
		if (prks->sharesCombined) {
			err = PGPCombineShares (shares, 
									prks->sharesCombined, &sharesTemp);
			if (IsPGPError (err)) 
				goto AddCleanup;
			PGPFreeShares (prks->sharesCombined);
			prks->sharesCombined = sharesTemp;
		}
		else {
			prks->sharesCombined = shares;
			shares = NULL;
		}

		// share is OK, add it to list
		prks->uNeededShares = uThreshold;
		SetDlgItemInt (prks->hwndDlg, IDC_SHARESNEEDED, 
					prks->uNeededShares, FALSE);

		prks->uCollectedShares += uNumShares;
		SetDlgItemInt (hwnd, IDC_SHARESCOLLECTED, 
								prks->uCollectedShares, FALSE);
		sAddShareHolderToList (prks, szName, uNumShares);
	}

AddCleanup :
	LeaveCriticalSection (&prks->critsecAddShare);

	if (shares) 
		PGPFreeShares (shares);

	if (sharefileref)
		PGPFreeShareFile (sharefileref);

	if (fileref)
		PFLFreeFileSpec (fileref);

	if (optionsDecode)
		PGPFreeOptionList(optionsDecode);

	if (PGPKeySetRefIsValid (prks->keysetDecryption))
		PGPFreeKeySet (prks->keysetDecryption);

	if (prks->keyidsDecryption)
		clFree (prks->keyidsDecryption);

	switch (err) {
		case kPGPClientError_IdenticalShares :
			PGPclMessageBox (hwnd, IDS_CAPTION, IDS_DUPLICATESHARES,
					MB_OK|MB_ICONEXCLAMATION);
			break;

		case kPGPClientError_DifferentSharePool :
			PGPclMessageBox (hwnd, IDS_CAPTION, IDS_SHARENUMMISMATCH, 
					MB_OK|MB_ICONEXCLAMATION);
			break;

		case kPGPClientError_DifferentSplitKeys :
			PGPclMessageBox (hwnd, IDS_CAPTION, IDS_SHAREKEYMISMATCH, 
					MB_OK|MB_ICONEXCLAMATION);
			break;

		default:
			PGPclErrorBox (hwnd, err);
	}

	if (PGPKeySetRefIsValid (prks->keysetToAdd)) {
		if (IsntPGPError (err)) {
			PGPclQueryAddKeys (prks->context, prks->tlsContext, hwnd,
					prks->keysetToAdd, prks->keyset);
		}
		PGPFreeKeySet (prks->keysetToAdd);
	}
}


//	______________________________________________
//
//  SKEP event handler

static PGPError
sHandlerSKEP (
		PGPskepRef		skep,
		PGPskepEvent*	event,
		PGPUserValue	userValue)
{
	BOOL				bCancel		= FALSE;
	PRECONKEYSTRUCT		prks;
	UINT				uID;
	UINT				u;
	CHAR				sz[64];
	LPSTR				psz;

	prks = (PRECONKEYSTRUCT)userValue;

	uID = 0;

	switch (event->type) {
	case kPGPskepEvent_ConnectEvent :
		uID = IDS_STATUSCONNECTED;
		break;

	case kPGPskepEvent_AuthenticateEvent :
		if (event->data.ad.remoteHostname)
			psz = (LPSTR)event->data.ad.remoteHostname;
		else if (event->data.ad.remoteIPAddress)
			psz = (LPSTR)event->data.ad.remoteIPAddress;
		else {
			LoadString (g_hInst, IDS_UNIDENTIFIEDHOST, sz, sizeof(sz));
			psz = sz;
		}

		if (IsntPGPError (PGPclConfirmRemoteAuthentication (
								prks->context,
								prks->hwndDlg, 
								psz, 
								event->data.ad.remoteKey,
								prks->keyset,
								PGPCL_AUTHRECONSTITUTING))) 
		{
			uID = IDS_STATUSAUTHENTICATED;
			if (IsntPGPError (PGPGetPrimaryUserIDNameBuffer (
						event->data.ad.remoteKey, kPGPMaxUserIDSize, 
						prks->szAuthUserID, &u))) {
				SetDlgItemText (prks->hwndDlg, 
							IDC_AUTHENTICATION, prks->szAuthUserID);
			}
		}
		else
		{
			bCancel = TRUE;
			uID = IDS_STATUSLISTENING;
		}
		break;

	case kPGPskepEvent_ShareEvent:
		{
			PGPShareRef			shares;
			PGPShareRef			sharesTemp;
			UINT				uNumShares;
			UINT				uThreshold;
			PGPError			err = kPGPError_NoErr;
			PGPKeyID			keyid;
		
			shares = event->data.sd.shares;
			EnterCriticalSection (&prks->critsecAddShare);
			
			uNumShares = PGPGetNumberOfShares (shares);
			
			// check that shares belong to this key
			err = PGPGetKeyIDFromShares (shares, &keyid);
			if (IsPGPError (err)) goto NetContinue;
			
			if (PGPCompareKeyIDs (&keyid, &(prks->keyidToReconstitute))) {
				PGPclMessageBox (prks->hwndDlg, IDS_CAPTION, 
					IDS_SHAREKEYMISMATCH, MB_OK|MB_ICONEXCLAMATION);
				goto NetContinue;
			}
			
			// check that threshold corresponds to other share files
			uThreshold = PGPGetShareThreshold (shares);
			if (prks->uNeededShares != UNKNOWN_SHARES_NEEDED) {
				if (uThreshold != prks->uNeededShares) {
					PGPclMessageBox (prks->hwndDlg, IDS_CAPTION, 
						IDS_SHARENUMMISMATCH, MB_OK|MB_ICONEXCLAMATION);
					goto NetContinue;
				}
			}
			
			// add shares to collection
			if (prks->sharesCombined) {
				err = PGPCombineShares (shares, 
					prks->sharesCombined, &sharesTemp);
				if (IsPGPError (err)) goto NetContinue;
				PGPFreeShares (prks->sharesCombined);
				prks->sharesCombined = sharesTemp;
			}
			else {
				prks->sharesCombined = shares;
				shares = NULL;
			}
			
			// share is OK, add it to list
			prks->uNeededShares = uThreshold;
			SetDlgItemInt (prks->hwndDlg, IDC_SHARESNEEDED, 
				prks->uNeededShares, FALSE);
			
			prks->uCollectedShares += uNumShares;
			SetDlgItemInt (prks->hwndDlg, IDC_SHARESCOLLECTED, 
				prks->uCollectedShares, FALSE);
			sAddShareHolderToList (prks, prks->szAuthUserID, uNumShares);
			
			if (prks->uCollectedShares >= prks->uNeededShares)
				EnableWindow (GetDlgItem (prks->hwndDlg, IDOK), TRUE);

NetContinue:
			LeaveCriticalSection (&prks->critsecAddShare);
		
			if (shares)
				PGPFreeShares (shares);
			PGPclErrorBox (prks->hwndDlg, err);
			uID = IDS_STATUSLISTENING;
		}
		break;

	case kPGPskepEvent_ProgressEvent :
		uID = IDS_STATUSRECEIVING;
		break;

	case kPGPskepEvent_CloseEvent :
		uID = IDS_STATUSCLOSING;
		SetDlgItemText (prks->hwndDlg, IDC_AUTHENTICATION, "");
		break;

	}

	if (uID != 0) {
		LoadString (g_hInst, uID, sz, sizeof(sz));
		SetDlgItemText (prks->hwndDlg, IDC_STATUS, sz);
	}

	if (prks->bStop || bCancel)
		return kPGPError_UserAbort;
	else
		return kPGPError_NoErr;
}


//	______________________________________________
//
// thread for actually receiving the shares

static DWORD WINAPI 
sNetworkThreadRoutine (LPVOID lpvoid)
{
	PRECONKEYSTRUCT		prks		= (PRECONKEYSTRUCT)lpvoid;
	PGPError			err;
	CHAR				sz[64];

	err = PGPskepSetEventHandler (prks->skep,
					(PGPskepEventHandler)sHandlerSKEP, (PGPUserValue)prks);

	do
	{
		LoadString (g_hInst, IDS_STATUSLISTENING, sz, sizeof(sz));
		SetDlgItemText (prks->hwndDlg, IDC_STATUS, sz);

		err = PGPskepReceiveShares (prks->skep,
									prks->keyAuthenticating,
									prks->pszPhraseAuth);

		if (err != kPGPError_UserAbort)
			PGPclErrorBox (prks->hwndDlg, err);

		SetDlgItemText (prks->hwndDlg, IDC_AUTHENTICATION, "");
	}
	while (err == kPGPClientError_RejectedSKEPAuthentication);

	LoadString (g_hInst, IDS_STATUSNOTCONNECTED, sz, sizeof(sz));
	SetDlgItemText (prks->hwndDlg, IDC_STATUS, sz);

	PostMessage (prks->hwndDlg, WM_APP, 0, 0);

	return 0;
}


//	______________________________________________
//
//	Reconstitute key dialog message procedure

static BOOL CALLBACK 
sReconKeyDlgProc (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	PRECONKEYSTRUCT		prks;
	CHAR				sz[kPGPMaxUserIDSize];
	DWORD				dw;
	PGPError			err;

	switch (uMsg) {

	case WM_INITDIALOG :
	{
		CHAR		szTitle[kPGPMaxUserIDSize + 32];
		PGPUInt32	size;

		// save address of struct
		SetWindowLong (hDlg, GWL_USERDATA, lParam);
		prks = (PRECONKEYSTRUCT)lParam;
		prks->hwndDlg = hDlg;

		// initialize shareholder list
		prks->hwndList = GetDlgItem (hDlg, IDC_SHAREHOLDERS);
		sInitKeyList (prks);

		// initialize name of key to reconstitute
		LoadString (g_hInst, IDS_RECONKEYTITLE, szTitle, sizeof(szTitle));
		PGPGetPrimaryUserIDNameBuffer (prks->keyToReconstitute, 
										sizeof(sz), sz, &size);
		SetDlgItemText (hDlg, IDC_KEYTORECON, sz);
		lstrcat (szTitle, sz);
		SetWindowText (hDlg, szTitle);

		// initialize numbers of shares
		SetDlgItemText (hDlg, IDC_SHARESNEEDED, "?");
		SetDlgItemInt (hDlg, IDC_SHARESCOLLECTED, 
											prks->uCollectedShares, FALSE);

		// initialize Network controls
		LoadString (g_hInst, IDS_STARTNETWORK, sz, sizeof(sz));
		SetDlgItemText (hDlg, IDC_NETWORK, sz);
		ShowWindow (GetDlgItem (hDlg, IDC_STATUS), SW_HIDE);
		ShowWindow (GetDlgItem (hDlg, IDC_STATUSTEXT), SW_HIDE);
		ShowWindow (GetDlgItem (hDlg, IDC_AUTHENTICATION), SW_HIDE);
		ShowWindow (GetDlgItem (hDlg, IDC_AUTHENTICATIONTEXT), SW_HIDE);
		ShowWindow (GetDlgItem (hDlg, IDC_NETWORKINSTRUCTIONS), SW_SHOW);

		// OK button is initially disabled
		EnableWindow (GetDlgItem (hDlg, IDOK), FALSE);

		return TRUE;
	}

	case WM_HELP: 
	    WinHelp (((LPHELPINFO) lParam)->hItemHandle, g_szHelpFile, 
	        HELP_WM_HELP, (DWORD) (LPSTR) aIds); 
	    break; 

	case WM_CONTEXTMENU: 
		WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU, 
		    (DWORD) (LPVOID) aIds); 
		break; 

	case WM_DESTROY :
		prks = (PRECONKEYSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
		ImageList_Destroy (prks->hIml);
		break;

	case WM_APP :
		prks = (PRECONKEYSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);

		// event signaling end of thread
		if (prks->skep != NULL)
		{
			PGPFreeSKEP (prks->skep);
			prks->skep = NULL;
		}

		// reset controls
		prks->bServerMode = FALSE;
		LoadString (g_hInst, IDS_STARTNETWORK, sz, sizeof(sz));
		SetDlgItemText (hDlg, IDC_NETWORK, sz);
		ShowWindow (GetDlgItem (hDlg, IDC_STATUS), SW_HIDE);
		ShowWindow (GetDlgItem (hDlg, IDC_STATUSTEXT), SW_HIDE);
		ShowWindow (GetDlgItem (hDlg, IDC_AUTHENTICATION), SW_HIDE);
		ShowWindow (GetDlgItem (hDlg, IDC_AUTHENTICATIONTEXT), SW_HIDE);
		ShowWindow (
					GetDlgItem (hDlg, IDC_NETWORKINSTRUCTIONS), SW_SHOW);

		if (prks->uCollectedShares >= prks->uNeededShares)
			EnableWindow (GetDlgItem (hDlg, IDOK), TRUE);

		if (prks->bUserCancel)
			EndDialog (hDlg, 0);
		if (prks->bUserOK)
			EndDialog (hDlg, 1);
		break;

	case WM_COMMAND:

		switch (LOWORD(wParam)) {
		case IDCANCEL :
			prks = (PRECONKEYSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
			if (prks->bServerMode) {
				prks->bStop = TRUE;
				prks->bUserCancel = TRUE;
				if (prks->skep != NULL)
					PGPskepCancel (prks->skep);
			}
			else {
				EndDialog (hDlg, 0);
			}
			break;

		case IDOK :
			prks = (PRECONKEYSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
			if (prks->bServerMode) {
				prks->bStop = TRUE;
				prks->bUserOK = TRUE;
				if (prks->skep != NULL)
					PGPskepCancel (prks->skep);
			}
			else {
				EndDialog (hDlg, 1);
			}
			break;

		case IDC_NETWORK :
			prks = (PRECONKEYSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
			prks->bServerMode = !prks->bServerMode;

			if (prks->bServerMode) {
				// if this is a retry, free phrase and passkey
				if (prks->pszPhraseAuth) {
					KMFreePhrase (prks->pszPhraseAuth);
					prks->pszPhraseAuth = NULL;
				}
				if (prks->pPasskeyAuth) {
					KMFreePasskey (prks->pPasskeyAuth,prks->sizePasskeyAuth);
					prks->pPasskeyAuth = NULL;
				}

				// get authentication key and phrase
				LoadString (g_hInst, IDS_RECONAUTHPROMPT, sz, sizeof(sz));
				err = KMGetSigningKeyPhrase (prks->context, 
						prks->tlsContext, hDlg, sz, 
						prks->keyset, TRUE, &(prks->keyAuthenticating),
						&(prks->pszPhraseAuth), &(prks->pPasskeyAuth),
						&(prks->sizePasskeyAuth));
				if (IsPGPError (err))
				{
					prks->bServerMode = FALSE;
					break;
				}

				// setup controls
				LoadString (g_hInst, IDS_STOPNETWORK, sz, sizeof(sz));
				SetDlgItemText (hDlg, IDC_NETWORK, sz);
				ShowWindow (GetDlgItem (hDlg, IDC_STATUS), SW_SHOW);
				ShowWindow (GetDlgItem (hDlg, IDC_STATUSTEXT), SW_SHOW);
				ShowWindow (GetDlgItem (hDlg, IDC_AUTHENTICATION), SW_SHOW);
				ShowWindow (
					GetDlgItem (hDlg, IDC_AUTHENTICATIONTEXT), SW_SHOW);
				ShowWindow (
					GetDlgItem (hDlg, IDC_NETWORKINSTRUCTIONS), SW_HIDE);

				SetDlgItemText (hDlg, IDC_STATUS, "");
				SetDlgItemText (hDlg, IDC_AUTHENTICATION, "");

				EnableWindow (GetDlgItem (hDlg, IDOK), FALSE);

				// start the thread
				prks->bStop = FALSE;
				PGPNewSKEP (prks->context, prks->tlsContext, &prks->skep);
				CreateThread (NULL, 0, sNetworkThreadRoutine, prks, 0, &dw);
			}
			else {
				// set flag which will stop thread
				prks->bStop = TRUE;
				if (prks->skep != NULL)
					PGPskepCancel (prks->skep);
			}

			// change the help ID based on if the button says start or stop
			if (prks->bServerMode) {
				aIds[1] = IDH_PGPCLRECON_STOPNETWORK;
			}
			else {
				aIds[1] = IDH_PGPCLRECON_STARTNETWORK;
			}

			break;

		case IDC_SHAREFILE :
			prks = (PRECONKEYSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
			sAddShareFile (hDlg, prks);
			if (prks->uCollectedShares >= prks->uNeededShares)
				EnableWindow (GetDlgItem (hDlg, IDOK), TRUE);
			break;

		}
		return TRUE;
	}
	return FALSE;
}


//	______________________________________________
//
//  Reconstitute specified key

PGPError PGPclExport
PGPclReconstituteKey (
		PGPContextRef		context,
		PGPtlsContextRef	tlsContext,
		HWND				hwnd,
		PGPKeySetRef		keyset,
		PGPKeyRef			key,
		PGPByte**			ppPasskey,
		PGPUInt32*			piPasskeyLength) 
{
	RECONKEYSTRUCT		rks;
	PGPError			err;

	// initialize struct
	rks.context				= context;
	rks.tlsContext			= tlsContext;
	rks.keyset				= keyset;
	rks.keyToReconstitute	= key;
	PGPGetKeyIDFromKey (key, &(rks.keyidToReconstitute));
	rks.uCollectedShares	= 0;
	rks.uNeededShares		= UNKNOWN_SHARES_NEEDED;
	rks.sharesCombined		= NULL;
	rks.bServerMode			= FALSE;
	rks.bUserCancel			= FALSE;
	rks.bUserOK				= FALSE;
	rks.bBadPassphrase		= FALSE;
	rks.pszPhraseAuth		= NULL;
	rks.pPasskeyAuth		= NULL;
	rks.skep				= NULL;
	rks.iIconIndex			= -1;

	InitializeCriticalSection (&rks.critsecAddShare);

	do {
		if (DialogBoxParam (g_hInst, MAKEINTRESOURCE (IDD_RECONSTITUTEKEY), 
			hwnd, sReconKeyDlgProc, (LPARAM)&rks)) {
			err = PGPGetPasskeyFromShares (rks.sharesCombined, 
							ppPasskey, piPasskeyLength);
			PGPclErrorBox (hwnd, err);

			if (!PGPPassphraseIsValid (key, 
					PGPOPasskeyBuffer (context, 
								*ppPasskey, *piPasskeyLength),
					PGPOLastOption (context))) {
				PGPclMessageBox (hwnd, IDS_CAPTION, IDS_BADPASSKEY, 
									MB_OK|MB_ICONSTOP);
				err = kPGPError_BadPassphrase;
			}
		}
		else {
			*ppPasskey = NULL;
			*piPasskeyLength = 0;
			err = kPGPError_UserAbort;
		}
	} while (err == kPGPError_BadPassphrase);

	DeleteCriticalSection (&rks.critsecAddShare);

	if (rks.sharesCombined)
		PGPFreeShares (rks.sharesCombined);

	if (rks.pszPhraseAuth)
		KMFreePhrase (rks.pszPhraseAuth);

	if (rks.pPasskeyAuth)
		KMFreePasskey (rks.pPasskeyAuth, rks.sizePasskeyAuth);

	return err;
}

