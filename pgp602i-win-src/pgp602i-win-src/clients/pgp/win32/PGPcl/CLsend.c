/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	CLsend.c - send key shares routines
	

	$Id: CLsend.c,v 1.27 1998/08/13 19:19:59 pbj Exp $
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

// typedefs
typedef struct {
	PGPContextRef		context;
	PGPtlsContextRef	tlsContext;
	PGPKeySetRef		keyset;
	HWND				hwnd;
	HWND				hwndParent;
	LPSTR				pszPhrase;
	PGPByte*			pPasskey;
	PGPSize				sizePasskey;
	CHAR				szName[kPGPMaxUserIDSize];
	UINT				uNumShares;
	LPSTR				pszRemoteHost;
	PGPKeySetRef		keysetDecryption;
	PGPUInt32			iKeyIDCount;
	PGPKeyID*			keyidsDecryption;
	PGPKeySetRef		keysetToAdd;
	PGPKeyRef			keyAuthenticating;
	PGPShareRef			shares;
	PGPskepRef			skep;
	BOOL				bConventional;
	BOOL				bThreadRunning;
	BOOL				bFinished;
} SENDSHARESSTRUCT, *PSENDSHARESSTRUCT;

// external global variables
extern HINSTANCE		g_hInst;
extern PGPContextRef	g_ContextRef;
extern CHAR g_szHelpFile[MAX_PATH];

// local globals
static DWORD aIds[] = {			// Help IDs
	IDC_SENDSHARES,		IDH_PGPCLSEND_SENDSHARES,	// must be first in list
	IDC_SHAREFILE,		IDH_PGPCLSEND_SHAREFILE,
	IDC_NUMSHARES,		IDH_PGPCLSEND_NUMSHARESINFILE,
	IDC_REMOTECOMPUTER,	IDH_PGPCLSEND_REMOTECOMPUTER,
	IDC_NETSTATUS,		IDH_PGPCLSEND_NETWORKSTATUS,
	IDC_AUTHENTICATION,	IDH_PGPCLSEND_NETWORKAUTHENTICATION,
    0,0 
}; 


//	___________________________________________________
//
//  SKEP event handler

static PGPError
sHandlerSKEP (
		PGPskepRef		skep,
		PGPskepEvent*	event,
		PGPUserValue	userValue)
{
	BOOL				bCancel		= FALSE;
	PSENDSHARESSTRUCT	psss;
	UINT				uID;
	CHAR				sz[64];
	UINT				u;

	psss = (PSENDSHARESSTRUCT)userValue;

	uID = 0;

	switch (event->type) {
	case kPGPskepEvent_ConnectEvent :
		uID = IDS_STATUSCONNECTED;
		break;

	case kPGPskepEvent_AuthenticateEvent :
		if (IsntPGPError (PGPclConfirmRemoteAuthentication (
								psss->context,
								psss->hwnd, 
								psss->pszRemoteHost, 
								event->data.ad.remoteKey,
								psss->keyset,
								PGPCL_AUTHRECONSTITUTING))) 
		{
			uID = IDS_STATUSAUTHENTICATED;
			if (IsntPGPError (PGPGetPrimaryUserIDNameBuffer (
						event->data.ad.remoteKey, sizeof(sz), sz, &u))) {
				SetDlgItemText (psss->hwnd, IDC_AUTHENTICATION, sz);
			}
		}
		else
			bCancel = TRUE;
		break;

	case kPGPskepEvent_ProgressEvent :
		uID = IDS_STATUSSENDING;
		break;

	case kPGPskepEvent_CloseEvent :
		uID = IDS_STATUSCLOSING;
		break;

	}

	if (uID != 0) {
		LoadString (g_hInst, uID, sz, sizeof(sz));
		SetDlgItemText (psss->hwnd, IDC_NETSTATUS, sz);
	}

	if (bCancel)
		return kPGPError_UserAbort;
	else
		return kPGPError_NoErr;
}


//	___________________________________________________
//
// thread for actually sending the shares

static DWORD WINAPI 
sSendSharesThreadRoutine (LPVOID lpvoid)
{
	PSENDSHARESSTRUCT	psss		= (PSENDSHARESSTRUCT)lpvoid;
	PGPError			err;

	err = PGPskepSetEventHandler (psss->skep,
					(PGPskepEventHandler)sHandlerSKEP, (PGPUserValue)psss);

	if (IsntPGPError (err)) {
		err = PGPskepSendShares (psss->skep, 
									psss->keyAuthenticating,
									psss->pszPhrase, 
									psss->shares, 
									psss->pszRemoteHost);
	}

	SendMessage (psss->hwnd, WM_APP, 0, (LPARAM)err);

	psss->bThreadRunning = FALSE;
	return 0;
}


//	___________________________________________________
//
// Sign key dialog message procedure

static BOOL CALLBACK 
sSendKeySharesDlgProc (
				HWND hDlg, 
				UINT uMsg, 
				WPARAM wParam, 
				LPARAM lParam) 
{
	PSENDSHARESSTRUCT	psss;
	CHAR				sz[64];
	INT					i;
	DWORD				dw;
	PGPError			err;

	switch (uMsg) {

	case WM_INITDIALOG :
		SetWindowLong (hDlg, GWL_USERDATA, lParam);
		psss = (PSENDSHARESSTRUCT)lParam;

		psss->hwnd = hDlg;
		SetDlgItemText (hDlg, IDC_SHAREFILE, psss->szName);
		SetDlgItemInt (hDlg, IDC_NUMSHARES, psss->uNumShares, FALSE);

		LoadString (g_hInst, IDS_STATUSNOTCONNECTED, sz, sizeof(sz));
		SetDlgItemText (hDlg, IDC_NETSTATUS, sz);

		EnableWindow (GetDlgItem (hDlg, IDC_SENDSHARES), FALSE);
		return TRUE;

	case WM_HELP: 
	    WinHelp (((LPHELPINFO) lParam)->hItemHandle, g_szHelpFile, 
	        HELP_WM_HELP, (DWORD) (LPSTR) aIds); 
	    break; 

	case WM_CONTEXTMENU: 
		WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU, 
		    (DWORD) (LPVOID) aIds); 
		break; 

	case WM_APP :
		psss = (PSENDSHARESSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
		err = (PGPError)lParam;
		if (IsPGPError (err)) {
			SetDlgItemText (hDlg, IDC_AUTHENTICATION, "");
			LoadString (g_hInst, IDS_STATUSNOTCONNECTED, sz, sizeof(sz));
			SetDlgItemText (hDlg, IDC_NETSTATUS, sz);
			PGPclErrorBox (hDlg, err);
	
			EnableWindow (GetDlgItem (hDlg, IDC_SENDSHARES), TRUE);
		}
		else {
			LoadString (g_hInst, IDS_STATUSFINISHED, sz, sizeof(sz));
			SetDlgItemText (hDlg, IDC_NETSTATUS, sz);
			PGPclMessageBox (hDlg, IDS_PGP, IDS_SENDSUCCESS, MB_OK);

			LoadString (g_hInst, IDS_DONE, sz, sizeof(sz));
			SetDlgItemText (hDlg, IDC_SENDSHARES, sz);
			EnableWindow (GetDlgItem (hDlg, IDCANCEL), FALSE);
			EnableWindow (GetDlgItem (hDlg, IDC_SENDSHARES), TRUE);
			SetFocus (GetDlgItem (hDlg, IDC_SENDSHARES));
			aIds[1] = IDH_PGPCLSEND_DONE;	// change help ID for this button
			psss->bFinished = TRUE;
		}

		PGPFreeSKEP (psss->skep);
		break;

	case WM_COMMAND :
		switch (LOWORD(wParam)) {
		case IDCANCEL :
			psss = (PSENDSHARESSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
			if (psss->bThreadRunning) 
				PGPskepCancel (psss->skep);
			else
				EndDialog (hDlg, 0);
			break;

		case IDC_SENDSHARES :
			psss = (PSENDSHARESSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);

			if (psss->bFinished) {
				EndDialog (hDlg, 0);
				break;
			}

			EnableWindow (GetDlgItem (hDlg, IDC_SENDSHARES), FALSE);
			if (psss->pszRemoteHost) 
				clFree (psss->pszRemoteHost);

			i = GetWindowTextLength (GetDlgItem (hDlg, IDC_REMOTECOMPUTER))+1;
			psss->pszRemoteHost = clAlloc (i);
			GetDlgItemText (hDlg, IDC_REMOTECOMPUTER, psss->pszRemoteHost, i);

			LoadString (g_hInst, IDS_STATUSCONNECTING, sz, sizeof(sz));
			SetDlgItemText (hDlg, IDC_NETSTATUS, sz);

			psss->bThreadRunning = TRUE;
			PGPNewSKEP (psss->context, psss->tlsContext, &psss->skep);
			CreateThread (NULL, 0, sSendSharesThreadRoutine, 
									(void*)psss, 0, &dw);
			break;

		case IDC_REMOTECOMPUTER :
			if (HIWORD (wParam) == EN_CHANGE) {
				if (GetWindowTextLength ((HWND)lParam) > 0)
					EnableWindow (GetDlgItem (hDlg, IDC_SENDSHARES), TRUE);
				else
					EnableWindow (GetDlgItem (hDlg, IDC_SENDSHARES), FALSE);
			}
			break;
		}
		return TRUE;
	}
	return FALSE;
}


//	___________________________________________________
//
//  decode event handler

static PGPError
sHandlerDecode (
		PGPContextRef	context,
		PGPEvent*		event,
		PGPUserValue	userValue)
{
	PSENDSHARESSTRUCT	psss;
	PGPError			err;

	switch (event->type) {
	case kPGPEvent_PassphraseEvent:
		{
			PGPEventPassphraseData	*pData = &event->data.passphraseData;

			UINT		uLen;
			CHAR		szPrompt[64];

			psss = (PSENDSHARESSTRUCT)userValue;
			psss->bConventional = pData->fConventional;

			LoadString (g_hInst, IDS_DECRYPTSHARESPROMPT, 
										szPrompt, sizeof(szPrompt));

			// free phrase if this is a retry
			if (psss->pszPhrase) {
				KMFreePhrase (psss->pszPhrase);
				psss->pszPhrase = NULL;
			}
			if (psss->pPasskey) {
				KMFreePasskey (psss->pPasskey, psss->sizePasskey);
				psss->pPasskey = NULL;
			}

			if (event->data.passphraseData.fConventional) {
				err = KMGetKeyPhrase (context, psss->tlsContext, 
								psss->hwndParent, szPrompt, psss->keyset, 
								NULL, &(psss->pszPhrase), 
								&(psss->pPasskey), &(psss->sizePasskey));
			}
			else {
				psss->keyAuthenticating = NULL;
				err = KMGetDecryptionPhrase (context, psss->tlsContext, 
								psss->hwndParent, szPrompt, psss->keyset, 
								&psss->keyAuthenticating, 
								psss->keysetDecryption,
								psss->iKeyIDCount, psss->keyidsDecryption,
								&psss->keysetToAdd, 
								&(psss->pszPhrase), 
								&(psss->pPasskey), &(psss->sizePasskey));
			}

			if (IsPGPError (err))
				return err;

			if (psss->pszPhrase) {
				uLen = lstrlen (psss->pszPhrase);
				PGPAddJobOptions (event->job, 
					PGPOPassphraseBuffer (context, psss->pszPhrase, uLen),
					PGPOLastOption (context));
			}
			else {
				PGPAddJobOptions (event->job, 
					PGPOPasskeyBuffer (context, 
								psss->pPasskey, psss->sizePasskey),
					PGPOLastOption (context));
			}
		}
		break;

	case kPGPEvent_RecipientsEvent:
		{
			PGPEventRecipientsData	*pData = &event->data.recipientsData;
			PGPUInt32				i;
			
			psss = (PSENDSHARESSTRUCT)userValue;
			psss->keyAuthenticating = kPGPInvalidRef;
		
			// Save recipient key set for passphrase dialog
			psss->keysetDecryption = pData->recipientSet;
			PGPIncKeySetRefCount (psss->keysetDecryption);

			// Save unknown keyids
			if (pData->keyCount > 0) {
				psss->iKeyIDCount = pData->keyCount;
				psss->keyidsDecryption = 
					(PGPKeyID *)clAlloc (pData->keyCount * sizeof(PGPKeyID));

				for (i=0; i<pData->keyCount; i++) {
					psss->keyidsDecryption[i] = pData->keyIDArray[i];
				}
			}
		}
		break;
	}

	return kPGPError_NoErr;
}


//	___________________________________________________
//
//  send shares

PGPError PGPclExport
PGPclSendShares (
		PGPContextRef		context, 
		PGPtlsContextRef	tlsContext,
		HWND				hwndParent,
		PGPKeySetRef		keyset) 
{
	PFLFileSpecRef		fileref				= NULL;
	PGPShareFileRef		sharefileref		= NULL;
	PGPOptionListRef	optionsDecode		= NULL;
	PGPError			err					= kPGPError_NoErr;

	OPENFILENAME		ofn;
	SENDSHARESSTRUCT	sss;
	CHAR				szFile[MAX_PATH];
	CHAR				sz[256];
	CHAR				szTitle[64];
	CHAR*				p;
	PGPSize				size;

	// initialize struct
	sss.context				= context;
	sss.tlsContext			= tlsContext;
	sss.hwnd				= NULL;
	sss.hwndParent			= hwndParent;
	sss.keyset				= keyset;
	sss.keysetDecryption	= kInvalidPGPKeySetRef;
	sss.keyidsDecryption	= NULL;
	sss.keysetToAdd			= kInvalidPGPKeySetRef;
	sss.keyAuthenticating	= kInvalidPGPKeyRef;
	sss.pszPhrase			= NULL;
	sss.pPasskey			= NULL;
	sss.shares				= NULL;
	sss.pszRemoteHost		= NULL;
	sss.bConventional		= FALSE;
	sss.bThreadRunning		= FALSE;
	sss.bFinished			= FALSE;

	// prompt user for name of share file to send
	szFile[0] = '\0';
	LoadString (g_hInst, IDS_SHAREFILEFILTER, sz, sizeof(sz));
	while (p = strrchr (sz, '@')) *p = '\0';
	LoadString (g_hInst, IDS_SHAREFILECAPTION, szTitle, sizeof(szTitle));

	ofn.lStructSize       = sizeof (OPENFILENAME);
	ofn.hwndOwner         = hwndParent;
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
	ofn.Flags			  = OFN_HIDEREADONLY|OFN_NOCHANGEDIR;
	ofn.nFileOffset       = 0;
	ofn.nFileExtension    = 0;
	ofn.lpstrDefExt       = "";
	ofn.lCustData         = 0;

	if (GetOpenFileName (&ofn)) {
		err = PFLNewFileSpecFromFullPath (PGPGetContextMemoryMgr (context), 
											szFile, &fileref);
		if (IsPGPError (err)) goto SendCleanup;
 
		err = PGPOpenShareFile (fileref, &sharefileref);
		if (IsPGPError (err)) goto SendCleanup;

		err = PGPGetShareFileUserID (sharefileref,
						sizeof(sss.szName), sss.szName, &size);
		if (IsPGPError (err)) goto SendCleanup;

		sss.uNumShares = PGPGetNumSharesInFile (sharefileref);

		// decrypt specified share file
		PGPBuildOptionList(context, &optionsDecode,
						PGPOKeySetRef (context, keyset),
						PGPOEventHandler (context, sHandlerDecode, &sss),
						PGPOLastOption (context));
		err = PGPCopySharesFromFile (context, sharefileref, 
						optionsDecode, &sss.shares);

		if (IsPGPError (err)) goto SendCleanup;

		// get authentication key and passphrase
		if (sss.bConventional) {

			// the phrase and passkey we got from user is no good here
			if (sss.pszPhrase) {
				KMFreePhrase (sss.pszPhrase);
				sss.pszPhrase = NULL;
			}
			if (sss.pPasskey) {
				KMFreePasskey (sss.pPasskey, sss.sizePasskey);
				sss.pPasskey = NULL;
			}

			// get new phrase
			LoadString (g_hInst, IDS_SENDAUTHPROMPT, sz, sizeof(sz));
			err = KMGetSigningKeyPhrase (context, tlsContext, hwndParent, 
						sz, keyset, TRUE, &sss.keyAuthenticating,
						&sss.pszPhrase, &sss.pPasskey, &sss.sizePasskey); 
			if (IsPGPError (err)) goto SendCleanup;
		}

		// launch send key shares dialog
		DialogBoxParam (g_hInst, MAKEINTRESOURCE(IDD_SENDSHARES), hwndParent, 
						sSendKeySharesDlgProc, (LPARAM)&sss);
	}

SendCleanup :
	if (sss.pszPhrase)
		KMFreePhrase (sss.pszPhrase);

	if (sss.pPasskey)
		KMFreePasskey (sss.pPasskey, sss.sizePasskey);

	if (sss.pszRemoteHost)
		clFree (sss.pszRemoteHost);

	if (sss.shares) 
		PGPFreeShares (sss.shares);

	if (PGPKeySetRefIsValid (sss.keysetDecryption))
		PGPFreeKeySet (sss.keysetDecryption);

	if (sss.keyidsDecryption)
		clFree (sss.keyidsDecryption);

	if (sharefileref)
		PGPFreeShareFile (sharefileref);

	if (fileref)
		PFLFreeFileSpec (fileref);

	if (optionsDecode)
		PGPFreeOptionList(optionsDecode);

	if (PGPKeySetRefIsValid (sss.keysetToAdd)) {
		if (IsntPGPError (err)) {
			PGPclQueryAddKeys (context, tlsContext, hwndParent,
					sss.keysetToAdd, keyset);
		}
		PGPFreeKeySet (sss.keysetToAdd);
	}

	PGPclErrorBox (hwndParent, err);

	return err;
}

