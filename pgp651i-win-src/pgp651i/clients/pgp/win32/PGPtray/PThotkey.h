/*____________________________________________________________________________
	Copyright (C) 1999 Network Associates, Inc.
	All rights reserved.
	
	PThotkey.h - PGPtray hotkey functions
	

	$Id: PThotkey.h,v 1.1 1999/02/24 01:06:38 pbj Exp $
____________________________________________________________________________*/
#ifndef Included_PThotkey_h	/* [ */
#define Included_PThotkey_h

#include "pgpPubTypes.h"
#include "pgpHotKeys.h"

// hotkey IDs
#define kPurgePassphraseCache		0x0000
#define kEncryptCurrentWindow		0x0001
#define kSignCurrentWindow			0x0002
#define kEncryptSignCurrentWindow	0x0003
#define kDecryptCurrentWindow		0x0004
#define kUnmountPGPDisks			0x0005	


//#include "pflPrefs.h"

PGP_BEGIN_C_DECLARATIONS


VOID 
PTHotKeyInit (void);

VOID
PTCheckForHotKeyPrefsMsg (HWND hwnd, UINT msg);

VOID
PTLoadAndSetHotKeys (HWND hwnd);

VOID
PTProcessHotKey (HWND hwnd, UINT msg);

VOID
PTRemoveHotKeys (HWND hwnd);


PGP_END_C_DECLARATIONS

#endif /* ] Included_PThotkey_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
