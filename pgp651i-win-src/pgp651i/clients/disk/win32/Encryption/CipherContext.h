//////////////////////////////////////////////////////////////////////////////
// CipherContext.h
//
// Declaration of class CipherContext.
//////////////////////////////////////////////////////////////////////////////

// $Id: CipherContext.h,v 1.4 1998/12/14 18:55:26 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CipherContext_h	// [
#define Included_CipherContext_h

#include "PGPdiskFileFormat.h"
#include "SecureMemory.h"

#include "Cast5.h"

// Align to 1.
#pragma pack(push, 1)

////////
// Types
////////

// A CipherOp specifies which cipher operation should be performed.

enum CipherOp
{
	kCipherOp_NoOp		= 0,
	kCipherOp_Encrypt	= 1,
	kCipherOp_Decrypt	= 2
};


//////////////////////
// Class CipherContext
//////////////////////

class CipherContext
{
public:
					CipherContext();
					~CipherContext() {DestroyContext();};

	CipherContext &	operator=(const CipherContext& from);

	PGPBoolean		IsFlipped();
	void			FlipContext();

	void			InitContext(PGPdiskEncryptionAlgorithm algorithm, 
						const PGPUInt8 *key, const PGPUInt8 *salt);
	void			DestroyContext();
	DualErr			ValidateContext();

	void			CipherBlocks(PGPUInt64 startBlockIndex, 
						PGPUInt32 numBlocks, const void *src, void *dest, 
						CipherOp op);

private:
	PGPdiskEncryptionAlgorithm	mAlgorithm;		// encryption algorithm used
	PassphraseSalt				mSalt;			// random salt data
	PGPBoolean					mInitialized;	// initialized context?
	PGPBoolean					mIsFlipped;		// is the key flipped?

	CASTCFBContext				mCastCFB;		// context for CAST cipher

	void	EncryptBlock512(PGPUInt64 blockNumber, const PGPUInt32 *src, 
				PGPUInt32 *dest);
	void	DecryptBlock512(PGPUInt64 blockNumber, const PGPUInt32 *src, 
				PGPUInt32 *dest);
};

// Restore alignment.
#pragma pack(pop)

#endif	// ] Included_CipherContext_h
