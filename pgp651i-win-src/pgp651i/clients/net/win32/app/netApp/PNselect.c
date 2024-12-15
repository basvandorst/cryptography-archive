/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PNselect.c - key selection dialog
	

	$Id: PNselect.c,v 1.17 1999/03/30 20:47:07 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

#include <windows.h>
#include <commctrl.h>

#include "resource.h"
#include "PGPnetApp.h"

#include "PGPcl.h"

#include "pgpKeys.h"

#define TOP_LIST_OFFSET			32
#define HOR_LIST_OFFSET			8
#define BOTTOM_LIST_OFFSET		48
#define BOTTOM_BUTTON_OFFSET	10
#define BUTTON_HEIGHT			24
#define BUTTON_WIDTH			70
#define BUTTON_SPACING			10

// External globals
extern HINSTANCE	g_hinst;


//	___________________________________________________
//
//  Display and handle selection of PGP keypair dialog

PGPError 
PNSelectPGPKey (
		PGPContextRef		context,
		PGPKeySetRef		keysetMain,
		HWND				hwndParent,
		PGPUInt32*			puPGPAlg,
		PGPByte*			pexpkeyidPGP)
{

	PGPKeySetRef	keysetToDisplay	= kInvalidPGPKeySetRef;
	PGPKeySetRef	keysetPGP		= kInvalidPGPKeySetRef;
	PGPKeySetRef	keysetNew		= kInvalidPGPKeySetRef;
	PGPKeySetRef	keysetOld		= kInvalidPGPKeySetRef;
	PGPKeySetRef	keysetSelected	= kInvalidPGPKeySetRef;
	PGPKeyListRef	keylist			= kInvalidPGPKeyListRef;
	PGPKeyIterRef	keyiter			= kInvalidPGPKeyIterRef;
	PGPFilterRef	filterPGP		= kInvalidPGPFilterRef;
	PGPKeyRef		key				= kInvalidPGPKeyRef;
	PGPError		err				= kPGPError_NoErr;
	BOOL			bFoundSecretKey	= FALSE;

	PGPKeyID		keyid;
	PGPSize			size;
	PGPBoolean		bCanSign;
	PGPBoolean		bSplit;
	CHAR			sz[128];

	LoadString (g_hinst, IDS_SELECTPGPKEY, sz, sizeof(sz));

	// create keyset of all keys with PGP sigs 
	err = PGPNewSigBooleanFilter (
				context, kPGPSigPropIsX509, FALSE, &filterPGP); CKERR;
	err = PGPFilterKeySet (keysetMain, filterPGP, &keysetPGP); CKERR;

	// create keyset of all keypairs 
	err = PGPOrderKeySet (keysetPGP, kPGPAnyOrdering, &keylist); CKERR;
	err = PGPNewKeyIter (keylist, &keyiter); CKERR;
	err = PGPNewEmptyKeySet (keysetMain, &keysetToDisplay); CKERR;

	err = PGPKeyIterNext (keyiter, &key); CKERR;
	while (key) 
	{
		err = PGPGetKeyBoolean (key, kPGPKeyPropCanSign, &bCanSign); CKERR;
		err = PGPGetKeyBoolean (key, 
								kPGPKeyPropIsSecretShared, &bSplit); CKERR;
		if (bCanSign && !bSplit)
		{
			bFoundSecretKey = TRUE;

			err = PGPNewSingletonKeySet (key, &keysetNew); CKERR;
			keysetOld = keysetToDisplay;
			err = PGPUnionKeySets (keysetOld, 
								keysetNew, &keysetToDisplay); CKERR;
			PGPFreeKeySet (keysetOld); CKERR;
			PGPFreeKeySet (keysetNew); CKERR;
		}

		PGPKeyIterNext (keyiter, &key);
	}

	if (bFoundSecretKey)
	{
		err = PGPclSelectKeysEx (context, NULL, hwndParent, sz, 
						keysetToDisplay, keysetMain, 
						PGPCL_SINGLESELECTION, 
						&keysetSelected); CKERR;

		PGPFreeKeyIter (keyiter);
		keyiter = kInvalidPGPKeyIterRef;

		PGPFreeKeyList (keylist);
		keylist = kInvalidPGPKeyListRef;

		err = PGPOrderKeySet (keysetSelected, 
									kPGPAnyOrdering, &keylist); CKERR;
		err = PGPNewKeyIter (keylist, &keyiter); CKERR;
		err = PGPKeyIterNext (keyiter, &key); CKERR;

		if (PGPKeyRefIsValid (key))
		{
			err = PGPGetKeyNumber (key, 
							kPGPKeyPropAlgID, puPGPAlg); CKERR;
			err = PGPGetKeyIDFromKey (key, &keyid); CKERR;
			err = PGPExportKeyID (&keyid, pexpkeyidPGP, &size); CKERR;
		}
	}
	else
	{
		PNMessageBox (hwndParent, IDS_CAPTION, IDS_NOSECRETKEYS, 
						MB_OK|MB_ICONEXCLAMATION);
		err = kPGPError_UserAbort;
	}

done:
	if (PGPKeyIterRefIsValid (keyiter))
		PGPFreeKeyIter (keyiter);
	if (PGPKeyListRefIsValid (keylist))
		PGPFreeKeyList (keylist);
	if (PGPFilterRefIsValid (filterPGP))
		PGPFreeFilter (filterPGP);
	if (PGPKeySetRefIsValid (keysetPGP))
		PGPFreeKeySet (keysetPGP);
	if (PGPKeySetRefIsValid (keysetToDisplay))
		PGPFreeKeySet (keysetToDisplay);
	if (PGPKeySetRefIsValid (keysetSelected))
		PGPFreeKeySet (keysetSelected);

	return err;
}


