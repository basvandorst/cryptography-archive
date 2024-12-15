/*____________________________________________________________________________
	Copyright (C) 1999 Network Associates, Inc.
	All rights reserved.
	
	PThotkey.c - PGPtray hotkey functions
	

	$Id: PThotkey.c,v 1.3 1999/03/01 22:04:34 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

#include "precomp.h"
#include "pgpTrayIPC.h"
#include "PGPcl.h"

#include "PThotkey.h"
#include "pgpUtilities.h"
#include "pgpClientPrefs.h"

UINT	uReloadPrefsMsg			= 0;

static VOID
sRemoveHotKeys (HWND hwnd)
{
	UnregisterHotKey (hwnd, kPurgePassphraseCache);
	UnregisterHotKey (hwnd, kEncryptCurrentWindow);
	UnregisterHotKey (hwnd, kSignCurrentWindow);
	UnregisterHotKey (hwnd, kEncryptSignCurrentWindow);
	UnregisterHotKey (hwnd, kDecryptCurrentWindow);
///	UnregisterHotKey (hwnd, kUnmountPGPDisks);
}


static BOOL
sSetSingleHotKey (
		HWND		hwnd,
		UINT		uID,
		PGPUInt32	uKey)
{
	if (uKey & kPGPHotKeyEnabled)
	{
		UINT	uMod	= 0;

		if (uKey & kPGPHotKeyAltModifier)
			uMod |= MOD_ALT;
		if (uKey & kPGPHotKeyCtrlModifier)
			uMod |= MOD_CONTROL;
		if (uKey & kPGPHotKeyShiftModifier)
			uMod |= MOD_SHIFT;

		return RegisterHotKey (hwnd, uID, uMod, (uKey & 0xFF));
	}
	else
		return TRUE;
}


static VOID
sSetHotKeys (HWND hwnd)
{
	PGPInt32		ids			= 0;
	PGPHotKeys		hotkeys;
	PGPHotKeys*		phks;
	PGPPrefRef		prefref;
	PGPSize			size;
	PGPError		err;
	PGPContextRef	context;

	err = PGPNewContext (kPGPsdkAPIVersion, &context);
	if (IsPGPError (err))
		return;

	err = PGPclOpenClientPrefs (PGPGetContextMemoryMgr (context), &prefref);
	if (IsPGPError (err))
		return;

	memcpy (&hotkeys, &sDefaultHotKeys, sizeof(hotkeys));
	err = PGPGetPrefData (prefref, kPGPPrefHotKeyData, &size, &phks);

	if (IsntPGPError (err))
	{
		if (size == sizeof (hotkeys))
			memcpy (&hotkeys, phks, size);

		PGPDisposePrefData (prefref, phks);
	}

	if (!sSetSingleHotKey (hwnd, kPurgePassphraseCache, 
			hotkeys.hotkeyPurgePassphraseCache))
	{
		hotkeys.hotkeyPurgePassphraseCache &= ~kPGPHotKeyEnabled;
		ids = IDS_CANNOTSETPURGEHOTKEY;
		goto done;
	}

	if (!sSetSingleHotKey (hwnd, kEncryptCurrentWindow, 
			hotkeys.hotkeyEncrypt))
	{
		hotkeys.hotkeyEncrypt &= ~kPGPHotKeyEnabled;
		ids = IDS_CANNOTSETENCRYPTHOTKEY;
		goto done;
	}

	if (!sSetSingleHotKey (hwnd, kSignCurrentWindow, 
			hotkeys.hotkeySign))
	{
		hotkeys.hotkeySign &= ~kPGPHotKeyEnabled;
		ids = IDS_CANNOTSETSIGNHOTKEY;
		goto done;
	}

	if (!sSetSingleHotKey (hwnd, kEncryptSignCurrentWindow, 
			hotkeys.hotkeyEncryptSign))
	{
		hotkeys.hotkeyEncryptSign &= ~kPGPHotKeyEnabled;
		ids = IDS_CANNOTSETENCRYPTSIGNHOTKEY;
		goto done;
	}

	if (!sSetSingleHotKey (hwnd, kDecryptCurrentWindow, 
			hotkeys.hotkeyDecrypt))
	{
		hotkeys.hotkeyDecrypt &= ~kPGPHotKeyEnabled;
		ids = IDS_CANNOTSETDECRYPTHOTKEY;
		goto done;
	}

///	if (!sSetSingleHotKey (hwnd, kUnmountPGPDisks, 
///			hotkeys.hotkeyDiskUnmount))
///	{
///		ids = IDS_CANNOTSETUNMOUNTHOTKEY;
///		goto done;
///	}

done:
	if (ids)
	{
		PGPSetPrefData (
				prefref, kPGPPrefHotKeyData, sizeof(hotkeys), &hotkeys);
		PGPclCloseClientPrefs (prefref, TRUE);

		PTMessageBox (hwnd, IDS_CAPTION, ids, MB_ICONEXCLAMATION|MB_OK);

		PGPclPreferences (context, hwnd, PGPCL_HOTKEYPREFS, NULL);
	}
	else
		PGPclCloseClientPrefs (prefref, FALSE);

	PGPFreeContext (context);
}


VOID 
PTHotKeyInit (void)
{
	uReloadPrefsMsg = RegisterWindowMessage (RELOADPREFSMSG);
}


VOID
PTCheckForHotKeyPrefsMsg (
		HWND	hwnd,
		UINT	msg)
{
	if (msg == uReloadPrefsMsg)
	{
		sRemoveHotKeys (hwnd);
		sSetHotKeys (hwnd);
	}
	else if (msg == PGPTRAY_M_DISABLEHOTKEYS)
	{
		sRemoveHotKeys (hwnd);
	}
}


VOID
PTLoadAndSetHotKeys (
		HWND hwnd)
{
	sSetHotKeys (hwnd);
}


VOID
PTProcessHotKey (
		HWND	hwnd, 
		WPARAM	wParam)
{
	switch (wParam) {
		case kPurgePassphraseCache :
			PGPclNotifyPurgePassphraseCache (
				PGPCL_DECRYPTIONCACHE|PGPCL_SIGNINGCACHE, 0);
			break;

		case kEncryptCurrentWindow :
			SendMessage (hwnd, WM_COMMAND, ID_ENCRYPTWINDOW, 0);
			break;

		case kSignCurrentWindow :
			SendMessage (hwnd, WM_COMMAND, ID_SIGNWINDOW, 0);
			break;

		case kEncryptSignCurrentWindow :
			SendMessage (hwnd, WM_COMMAND, ID_ENCRYPTSIGNWINDOW, 0);
			break;

		case kDecryptCurrentWindow :
			SendMessage (hwnd, WM_COMMAND, ID_DECRYPTVERIFYWINDOW, 0);
			break;

///		case kUnmountPGPDisks :
///			break;
	}
}


VOID
PTRemoveHotKeys (
		HWND hwnd)
{
	sRemoveHotKeys (hwnd);
}



