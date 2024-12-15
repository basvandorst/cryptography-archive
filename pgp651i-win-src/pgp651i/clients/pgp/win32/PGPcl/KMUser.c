/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	KMUser.c - miscellaneous user interface routines
	

	$Id: KMUser.c,v 1.30 1999/05/15 16:26:17 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

// project header files
#include "pgpkmx.h"
#include "pgpUserInterface.h"

// external global variables
extern HINSTANCE g_hInst;

// local global variables
static DWORD aSignConfirmIds[] = {			// Help IDs
    0,0 
}; 

//	_______________________________________________
//
// Message box routine
//
//	iCaption and iMessage are string table resource IDs.

LRESULT 
KMMessageBox (
		HWND	hWnd, 
		INT		iCaption, 
		INT		iMessage,
		ULONG	ulFlags) 
{
	CHAR szCaption [128];
	CHAR szMessage [512];

	LoadString (g_hInst, iCaption, szCaption, sizeof(szCaption));
	LoadString (g_hInst, iMessage, szMessage, sizeof(szMessage));

	ulFlags |= MB_SETFOREGROUND;
	return (MessageBox (hWnd, szMessage, szCaption, ulFlags));
}


//	_______________________________________________
//
// Signature confirmation dialog message procedure

static BOOL CALLBACK 
sSignConfirmDlgProc (
		HWND	hWndDlg, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	PKEYMAN pKM;

	switch (uMsg) {
	case WM_INITDIALOG :
		SetWindowLong (hWndDlg, GWL_USERDATA, lParam);
		CheckDlgButton (hWndDlg, IDC_SENDSIG, BST_UNCHECKED);
		break;

	case WM_HELP: 
		pKM = (PKEYMAN)GetWindowLong (hWndDlg, GWL_USERDATA);
	    WinHelp (((LPHELPINFO) lParam)->hItemHandle, pKM->szHelpFile, 
	        HELP_WM_HELP, (DWORD) (LPSTR) aSignConfirmIds); 
	    break; 
 
	case WM_CONTEXTMENU: 
		pKM = (PKEYMAN)GetWindowLong (hWndDlg, GWL_USERDATA);
	    WinHelp ((HWND) wParam, pKM->szHelpFile, HELP_CONTEXTMENU, 
	        (DWORD) (LPVOID) aSignConfirmIds); 
	    break; 

	case WM_COMMAND :
		switch (LOWORD(wParam)) {
		case IDCANCEL :
			EndDialog (hWndDlg, 0);
			break;

		case IDOK :
			if (IsDlgButtonChecked (hWndDlg, IDC_SENDSIG) == BST_CHECKED) 
				EndDialog (hWndDlg, 2);
			else 
				EndDialog (hWndDlg, 1);
			break;
		}
		return TRUE;
	}
	return FALSE;
}

//	_______________________________________________
//
//	construct userid from name and address edit box contents

BOOL KMConstructUserID (
		HWND	hDlg, 
		UINT	uNameIDC, 
		UINT	uAddrIDC, 
		LPSTR*	pszUserID) 
{
	UINT uNameLen, uAddrLen, uUserIDLen;
	CHAR* p;

	uNameLen = SendDlgItemMessage (hDlg, uNameIDC, WM_GETTEXTLENGTH, 0, 0);
	if (uNameLen == 0) {
		KMMessageBox (hDlg, IDS_CAPTION, IDS_NOUSERNAME, 
			MB_OK|MB_ICONEXCLAMATION);
		SetFocus (GetDlgItem (hDlg, uNameIDC));
		return FALSE;
	}

	uAddrLen = SendDlgItemMessage (hDlg, uAddrIDC, WM_GETTEXTLENGTH, 0, 0);
	if (uAddrLen == 0) {
		if (KMMessageBox (hDlg, IDS_CAPTION, IDS_NOUSERADDR, 
			MB_YESNO|MB_DEFBUTTON2|MB_ICONEXCLAMATION) == IDNO) {
			SetFocus (GetDlgItem (hDlg, uAddrIDC));
			return FALSE;
		}
	}

	uUserIDLen = uNameLen + uAddrLen + 4;
	*pszUserID = KMAlloc (uUserIDLen * sizeof(char));
	if (!*pszUserID) return FALSE;

	p = *pszUserID;
	GetDlgItemText (hDlg, uNameIDC, p, uUserIDLen);

	if (uAddrLen > 0) {
		p += uNameLen;
		lstrcpy (p, " <");

		p += 2;
		GetDlgItemText (hDlg, uAddrIDC, p, uUserIDLen);

		p += uAddrLen;
		lstrcpy (p, ">");
	}

	return TRUE;
}


//----------------------------------------------------|
// Wipe edit box clean

VOID 
KMWipeEditBox (HWND hDlg, UINT uID) 
{
	CHAR* p;
	INT i;

	i = SendDlgItemMessage (hDlg, uID, WM_GETTEXTLENGTH, 0, 0);
	if (i > 0) {
		p = KMAlloc (i+1);
		if (p) {
			FillMemory (p, i, ' ');
			SendDlgItemMessage (hDlg, uID, WM_SETTEXT, 0, (LPARAM)p);
			KMFree (p);
		}
	}
}

//----------------------------------------------------|
// get passphrase for key from user

PGPError  
KMGetKeyPhrase (
		PGPContextRef		context,
		PGPtlsContextRef	tlsContext,
		HWND				hwnd, 
		LPSTR				szPrompt,
		PGPKeySetRef		keyset,
		PGPKeyRef			key,
		LPSTR*				ppszPhrase,
		PGPByte**			ppPasskeyBuffer,
		PGPUInt32*			piPasskeyLength) 
{
	PGPError	err				= kPGPError_BadParams;
	PGPBoolean	bSplit			= FALSE;
	LPSTR		psz;
	UINT		uFlags;

	if (!ppPasskeyBuffer) return err;
	if (!piPasskeyLength) return err;

	psz = NULL;
	*ppPasskeyBuffer = NULL;
	*piPasskeyLength = 0;

	if (key) 
		PGPGetKeyBoolean (key, kPGPKeyPropIsSecretShared, &bSplit);

	do {
		if (bSplit) {
			err=PGPclReconstituteKey(
				context,			// in context
				tlsContext,			// in TLS context
				hwnd,				// in hwnd of parent
				keyset,				// in keyset
				key,				// in key
				ppPasskeyBuffer,	// out passkey buffer
				piPasskeyLength);	// out passkey length
		}
		else {
			if (key) uFlags = PGPCL_KEYPASSPHRASE;
			else uFlags = PGPCL_DECRYPTION;
			err = PGPclGetPhrase (
				context,			// in context
				keyset,				// in main keyset
				hwnd,				// in hwnd of parent
				szPrompt,			// in prompt
				&psz,				// out phrase
				NULL,				// in keyset
				NULL,				// in keyids
				0,					// in keyid count
				&key,				// in/out key
				NULL,				// out options
				uFlags,				// in flags
				ppPasskeyBuffer,	// out passkey buffer
				piPasskeyLength,	// out passkey length
				0,0,				// in min length/quality
				tlsContext,			// in tlsContext,
				NULL,				// out AddedKeys
				NULL);				

		}

		if (IsPGPError (err)) {
			if (psz) {
				PGPFreeData (psz);
				psz = NULL;
			}
			if (*ppPasskeyBuffer) {
				PGPFreeData (*ppPasskeyBuffer);
				*ppPasskeyBuffer = NULL;
			}
			PGPclErrorBox (hwnd, err);
		}
		else {
			if (ppszPhrase) *ppszPhrase = psz;
			else KMFreePhrase (psz);
		}

	} while (err == kPGPError_BadPassphrase);

	return err;
}

//----------------------------------------------------|
// get passphrase for decryption from user

PGPError  
KMGetDecryptionPhrase (
		PGPContextRef		context,
		PGPtlsContextRef	tlsContext,
		HWND				hwnd, 
		LPSTR				szPrompt,
		PGPKeySetRef		keysetMain,
		PGPKeyRef*			pkey,
		PGPKeySetRef		keysetDecryption,
		PGPUInt32			iKeyIDCount,
		PGPKeyID*			keyidsDecryption,
		PGPKeySetRef*		pkeysetToAdd,
		LPSTR*				ppszPhrase,
		PGPByte**			ppPasskeyBuffer,
		PGPUInt32*			piPasskeyLength) 
{
	PGPError	err				= kPGPError_BadParams;
	PGPBoolean	bSplit			= FALSE;
	LPSTR		psz;

	if (!ppPasskeyBuffer) return err;
	if (!piPasskeyLength) return err;

	psz = NULL;
	*ppPasskeyBuffer = NULL;
	*piPasskeyLength = 0;

	if (pkey) 
		if (*pkey)
			PGPGetKeyBoolean (*pkey, kPGPKeyPropIsSecretShared, &bSplit);

	do {
		if (bSplit) {
			err=PGPclReconstituteKey(
				context,			// in context
				tlsContext,			// in TLS context
				hwnd,				// in hwnd of parent
				keysetMain,			// in keyset
				*pkey,				// in key
				ppPasskeyBuffer,	// out passkey buffer
				piPasskeyLength);	// out passkey length
		}
		else {
			err = PGPclGetPhrase (
				context,			// in context
				keysetMain,			// in main keyset
				hwnd,				// in hwnd of parent
				szPrompt,			// in prompt
				&psz,				// out phrase
				keysetDecryption,	// in keyset
				keyidsDecryption,	// in keyids
				iKeyIDCount,		// in keyid count
				pkey,				// in/out key
				NULL,				// out options
				PGPCL_DECRYPTION,	// in flags
				ppPasskeyBuffer,	// out passkey buffer
				piPasskeyLength,	// out passkey length
				0,0,				// in min length/quality
				tlsContext,			// in tlsContext,
				pkeysetToAdd,		// out AddedKeys
				NULL);

		}

		if (IsPGPError (err)) {
			if (psz) {
				PGPFreeData (psz);
				psz = NULL;
			}
			if (*ppPasskeyBuffer) {
				PGPFreeData (*ppPasskeyBuffer);
				*ppPasskeyBuffer = NULL;
			}
			PGPclErrorBox (hwnd, err);
		}
		else {
			if (ppszPhrase) *ppszPhrase = psz;
			else KMFreePhrase (psz);
		}

	} while (err == kPGPError_BadPassphrase);

	return err;
}

//----------------------------------------------------|
// get passphrase for signing key from user

PGPError  
KMGetSigningKeyPhrase (
		PGPContextRef		context,
		PGPtlsContextRef	tlsContext,
		HWND				hwnd, 
		LPSTR				szPrompt,
		PGPKeySetRef		keyset,
		BOOL				bRejectSplitKeys,
		PGPKeyRef*			pkey,
		LPSTR*				ppszPhrase,
		PGPByte**			ppPasskeyBuffer,
		PGPUInt32*			piPasskeyLength) 
{
	PGPError	err			= kPGPError_BadParams;
	LPSTR		psz;
	UINT		uFlags;

	if (!ppPasskeyBuffer) return err;
	if (!piPasskeyLength) return err;

	psz = NULL;
	*ppPasskeyBuffer = NULL;
	*piPasskeyLength = 0;

	if (bRejectSplitKeys)
		uFlags = PGPCL_REJECTSPLITKEYS;
	else
		uFlags = 0;

	err = PGPclGetPhrase (
		context,			// in context
		keyset,				// in main keyset
		hwnd,				// in hwnd of parent
		szPrompt,			// in prompt
		&psz,				// out phrase
		NULL,				// in keyset
		NULL,				// in keyids
		0,					// in keyid count
		pkey,				// in/out key
		NULL,				// out options
		uFlags,				// in flags
		ppPasskeyBuffer,	// out passkey buffer
		piPasskeyLength,	// out passkey length
		0,0,				// in min length/quality
		tlsContext,			// in tlsContext
		NULL,NULL);			// out AddedKeys

	if (IsPGPError (err)) {
		if (psz) {
			PGPFreeData (psz);
			psz = NULL;
		}
		if (*ppPasskeyBuffer) {
			PGPFreeData (*ppPasskeyBuffer);
			*ppPasskeyBuffer = NULL;
		}
	}
	else {
		if (ppszPhrase) *ppszPhrase = psz;
		else KMFreePhrase (psz);
	}

	if (bRejectSplitKeys) {
		if (err == kPGPError_KeyUnusableForSignature) {
			KMMessageBox (hwnd, IDS_CAPTION, IDS_SPLITKEYSNOTALLOWED,
					MB_OK|MB_ICONEXCLAMATION);
			err = kPGPError_UserAbort;
		}
	}

	return err;
}

//----------------------------------------------------|
// get passphrase for key from user

PGPError  
KMGetConfirmationPhrase (
		PGPContextRef	context,
		HWND			hwnd, 
		LPSTR			szPrompt,
		PGPKeySetRef	keyset,
		INT				iMinLen,
		INT				iMinQual,
		LPSTR*			ppszPhrase)
{
	PGPError	err			= kPGPError_BadParams;

	if (!ppszPhrase) return err;

	err = PGPclGetPhrase (
		context,			// in context
		keyset,				// in main keyset
		hwnd,				// in hwnd of parent
		szPrompt,			// in prompt
		ppszPhrase,			// out phrase
		NULL,				// in keyset
		NULL,				// in keyids
		0,					// in keyid count
		NULL,				// in/out key
		NULL,				// out options
		PGPCL_ENCRYPTION,	// in flags
		NULL,				// out passkey buffer
		NULL,				// out passkey length
		iMinLen,iMinQual,	// min length/quality
		NULL,NULL,NULL);	// tlsContext/Added Keys

	if (IsPGPError (err)) {
		if (*ppszPhrase) {
			PGPFreeData (*ppszPhrase);
			*ppszPhrase = NULL;
		}
	}

	return err;
}

//----------------------------------------------------|
// get conventional passphrase from user

PGPError  
KMGetConventionalPhrase (
		PGPContextRef	context,
		HWND			hwnd, 
		LPSTR			pszPrompt,
		LPSTR*			ppszPhrase)
{
	PGPError			err			= kPGPError_BadParams;
	PGPOptionListRef	optionList	= kInvalidPGPOptionListRef;

	if (!ppszPhrase) return err;

	err = PGPBuildOptionList (context, &optionList,
			PGPOUIOutputPassphrase (context, ppszPhrase),
			PGPOUIParentWindowHandle (context, hwnd),
			PGPOLastOption (context));

	if (IsntPGPError (err))
	{
		// If we have a prompt, use it
		if (IsntNull (pszPrompt))
		{
			err = PGPAppendOptionList (optionList,
				PGPOUIDialogPrompt (context, pszPrompt),
				PGPOLastOption (context));
		}

		if (IsntPGPError (err))
		{
			err = PGPConventionalDecryptionPassphraseDialog (context,
					optionList,
					PGPOLastOption (context));
		}
	}

	if (IsPGPError (err)) 
	{
		if (*ppszPhrase) 
		{
			PGPFreeData (*ppszPhrase);
			*ppszPhrase = NULL;
		}
	}

	if (PGPOptionListRefIsValid (optionList))
		PGPFreeOptionList (optionList);

	return err;
}

//----------------------------------------------------|
// free passphrase for key

VOID
KMFreePhrase (LPSTR pszPhrase)
{
	if (pszPhrase) {
		PGPFreeData (pszPhrase);
	}
}

//----------------------------------------------------|
// free passkey for key

VOID
KMFreePasskey (PGPByte* pbyte, PGPSize size)
{
	if (pbyte) {
		FillMemory (pbyte, size, 0x00);
		PGPFreeData (pbyte);
	}
}
