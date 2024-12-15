//////////////////////////////////////////////////////////////////////////////
// CPublicKeyDialog.h
//
// Declaration of class CPublicKeyDialog.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPublicKeyDialog.h,v 1.4 1999/02/26 04:09:57 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CPublicKeyDialog_h	// [
#define Included_CPublicKeyDialog_h

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

	DualErr	AskForKeys(PGPKeySetRef allKeys, PGPRecipientSpec *oldRecipKeys, 
		PGPUInt32 numRecipKeys);

};

#endif	// Included_CPublicKeyDialog_h
