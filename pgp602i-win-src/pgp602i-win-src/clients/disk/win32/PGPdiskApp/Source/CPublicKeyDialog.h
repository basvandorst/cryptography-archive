//////////////////////////////////////////////////////////////////////////////
// CPublicKeyDialog.h
//
// Declaration of class CPublicKeyDialog.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPublicKeyDialog.h,v 1.1.2.5 1998/07/22 01:19:26 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CPublicKeyDialog_h	// [
#define Included_CPublicKeyDialog_h

#if PGPDISK_PUBLIC_KEY

#include "PGPKeys.h"
#include "PGPUserInterface.H"

#include "DualErr.h"


/////////////////////////
// Class CPublicKeyDialog
/////////////////////////

class CPublicKeyDialog
{

public:
	PGPBoolean			mGotNewKeys, mNewKeysReadOnly;
	PGPKeySetRef		mNewKeySet;
	PGPRecipientSpec	*mNewRecipKeys;
	PGPUInt32			mNumNewKeys;

			CPublicKeyDialog();
			~CPublicKeyDialog();

	void	CleanUp();

	DualErr	AskForKeys(PGPContextRef context, PGPKeySetRef allKeys, 
				PGPRecipientSpec *oldRecipKeys, PGPUInt32 numRecipKeys);

};

#endif	// PGPDISK_PUBLIC_KEY

#endif	// Included_CPublicKeyDialog_h
