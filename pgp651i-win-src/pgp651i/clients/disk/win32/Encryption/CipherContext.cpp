//////////////////////////////////////////////////////////////////////////////
// CipherContext.cpp
//
// Implementation of Class CipherContext.
//////////////////////////////////////////////////////////////////////////////

// $Id: CipherContext.cpp,v 1.7 1999/03/31 23:51:06 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#if defined(PGPDISK_MFC)

#include "StdAfx.h"

#elif defined(PGPDISK_95DRIVER)

#include <vtoolscp.h>

#elif defined(PGPDISK_NTDRIVER)

#define	__w64
#include <vdw.h>

#else
#error Define PGPDISK_MFC, PGPDISK_95DRIVER, or PGPDISK_NTDRIVER.
#endif	// PGPDISK_MFC

#include "Required.h"
#include "UtilityFunctions.h"
#include "CipherContext.h"


////////////
// Constants
////////////

#define	PGP_USE_ASSEMBLY 1

const PGPUInt16	kCipherBlockSize	= 512;
const PGPUInt32	kBlockSizeIn32		= kCipherBlockSize / sizeof(PGPUInt32);


//////////////////////////////////////////////
// Class CipherContext public member functions
//////////////////////////////////////////////

// Class CipherContext default constructor.

CipherContext::CipherContext()
{
	mAlgorithm		= kInvalidEncryptionAlgorithm;
	mInitialized	= FALSE;
	mIsFlipped		= FALSE;
}

// operator= copies one CipherContext to another.

CipherContext & 
CipherContext::operator=(const CipherContext& from)
{
	PGPUInt32 i = 0;

	if (this == &from)
		return (* this);

	mAlgorithm		= from.mAlgorithm;
	mSalt			= from.mSalt;
	mInitialized	= from.mInitialized;
	mCastCFB		= from.mCastCFB;

	return (* this);
}

// IsFlipped returns TRUE if the key is in a flipped state, FALSE otherwise.

PGPBoolean 
CipherContext::IsFlipped()
{
	pgpAssert(mInitialized);
	return mIsFlipped;
}

// FlipContext flips the bits of the key. Encryption/decryption cannot be
// performed while the key is flipped.

void 
CipherContext::FlipContext()
{
	pgpAssert(mInitialized);

	if (mAlgorithm == kCASTEncryptionAlgorithm)
		FlipBytes((PGPUInt8 *) &mCastCFB, sizeof(mCastCFB));

	mIsFlipped = !mIsFlipped;
}

// InitContext prepares the CiperContext for encryption and decryption. The
// 'salt' and 'key' parameters are only used for BEAs that require them.

void 
CipherContext::InitContext(
	PGPdiskEncryptionAlgorithm	algorithm, 
	const PGPUInt8				*key, 
	const PGPUInt8				*salt)
{
	PGPUInt32 i;

	pgpAssert(algorithm != kInvalidEncryptionAlgorithm);

	DestroyContext();
	mAlgorithm = algorithm;

	switch (mAlgorithm)
	{

	case kBrokenCASTEncryptionAlgorithm:
		pgpAssertAddrValid(salt, PGPUInt8);
		pgpAssertAddrValid(key, PGPUInt8);

		pgpCopyMemory(salt, mSalt.saltBytes, sizeof(mSalt.saltBytes));
		pgpCopyMemory(key, mCastCFB.expandedKey.keyBytes, 16);

#if PGP_USE_ASSEMBLY

		// The key schedule for assembly must be slightly different. The
		// rotate subkeys (every second word) must be XORed with 16. This
		// swaps the halves of some words internally in a way that the code
		// expects. Don't forget this!

		for (i=0; i<32; i+=2)
		{
			mCastCFB.expandedKey.keyDWords[i+1] = 
				mCastCFB.expandedKey.keyDWords[i+1] ^ 16;
		}

#else	// ] !PGP_USE_ASSEMBLY

		i;	// get rid of unreferenced local variable error

#endif	// ] PGP_USE_ASSEMBLY

		mInitialized = TRUE;
		break;

	case kCASTEncryptionAlgorithm:
		pgpAssertAddrValid(salt, PGPUInt8);
		pgpAssertAddrValid(key, PGPUInt8);

		pgpCopyMemory(salt, mSalt.saltBytes, sizeof(mSalt.saltBytes));

		// Expand the key.
		CAST5schedule(mCastCFB.expandedKey.keyDWords, key);

#if PGP_USE_ASSEMBLY

		// The key schedule for assembly must be slightly different. The
		// rotate subkeys (every second word) must be XORed with 16. This
		// swaps the halves of some words internally in a way that the code
		// expects. Don't forget this!

		for (i=0; i<32; i+=2)
		{
			mCastCFB.expandedKey.keyDWords[i+1] = 
				mCastCFB.expandedKey.keyDWords[i+1] ^ 16;
		}

#else	// ] !PGP_USE_ASSEMBLY

		i;	// get rid of unreferenced local variable error

#endif	// ] PGP_USE_ASSEMBLY

		mInitialized = TRUE;
		break;

	case kCopyDataEncryptionAlgorithm:
		mInitialized = TRUE;
		break;

	default:
		pgpAssert(FALSE);
		break;
	}
}

// DestroyContext clears any variables used by the current cipher algorithm,
// then marks the context as unitialized.

void
CipherContext::DestroyContext()
{
	if (!mInitialized)
		return;

	pgpAssert(mAlgorithm != kInvalidEncryptionAlgorithm);

	switch (mAlgorithm)
	{

	case kBrokenCASTEncryptionAlgorithm: 
	case kCASTEncryptionAlgorithm:
		pgpClearMemory(&mSalt, sizeof(mSalt));
		pgpClearMemory(&mCastCFB, sizeof(mCastCFB));

		mInitialized = FALSE;
		break;

	case kCopyDataEncryptionAlgorithm:
		mInitialized = FALSE;
		break;

	default:
		pgpAssert(FALSE);
		break;
	}
}

// ValidateContext makes sure the context isn't damaged.

DualErr 
CipherContext::ValidateContext()
{
	DualErr derr;

	if (!mInitialized)
		derr = DualErr(kPGDMinorError_ContextNotInitialized);

	if (derr.IsntError())
	{
		switch (mAlgorithm)
		{

		case kBrokenCASTEncryptionAlgorithm: 
		case kCASTEncryptionAlgorithm:
			if (!VerifyCastSBoxesValid())
				derr = DualErr(kPGDMinorError_InvalidSBoxes);
			break;

		case kCopyDataEncryptionAlgorithm:
			break;

		default:
			derr = DualErr(kPGDMinorError_InvalidAlgorithm);	// no assert
			break;
		}
	}

	return derr;
}

// CipherBlocks performs a cipher operation on the given blocks using the
// object's context.

void 
CipherContext::CipherBlocks(
	PGPUInt64	startBlockIndex,
	PGPUInt32	numBlocks,
	const void	*src,
	void		*dest,
	CipherOp	op)
{
	PGPUInt32		blockIndex;
	const PGPUInt8	*curSrc		= (const PGPUInt8 *) src;
	PGPUInt8		*curDest	= (PGPUInt8 *) dest;

	pgpAssertAddrValid(src, VoidAlign);
	pgpAssertAddrValid(dest, VoidAlign);
	pgpAssert(mInitialized);
	pgpAssert(!mIsFlipped);
	pgpAssert(numBlocks != 0);

	for (blockIndex = 0; blockIndex < numBlocks; blockIndex++)
	{
		PGPUInt64 seedValue = startBlockIndex + blockIndex;

		switch (op)
		{			
		case kCipherOp_Decrypt:
			DecryptBlock512(seedValue, (const PGPUInt32 *) curSrc, 
				(PGPUInt32 *) curDest);
			break;

		case kCipherOp_Encrypt:
			EncryptBlock512(seedValue, (const PGPUInt32 *) curSrc, 
				(PGPUInt32 *) curDest);
			break;

		case kCipherOp_NoOp:
			break;

		default:
			pgpAssert(FALSE);
			break;
		}

		curSrc	+= kCipherBlockSize;
		curDest	+= kCipherBlockSize;
	}
}


///////////////////////////////////////////////
// Class CipherContext private member functions
///////////////////////////////////////////////

// EncryptBlock512 encrypts the given blocks using the object's context.

void
CipherContext::EncryptBlock512(
	PGPUInt64		blockNumber,
	const PGPUInt32	*src,
	PGPUInt32		*dest)
{
	pgpAssertAddrValid(src, PGPUInt32);
	pgpAssertAddrValid(dest, PGPUInt32);
	pgpAssert(mInitialized);
	pgpAssert(!mIsFlipped);
	pgpAssert(mAlgorithm != kInvalidEncryptionAlgorithm);
	
	switch (mAlgorithm)
	{

	case kBrokenCASTEncryptionAlgorithm: 
	case kCASTEncryptionAlgorithm:
		{
		PGPUInt32 iv0, iv1, sum0, sum1;

		pgpAssert(sizeof(mSalt.saltDWords[0]) == sizeof(PGPUInt32));

		sum0 = mSalt.saltDWords[0] + GetHighDWord(blockNumber) + 
			GetLowDWord(blockNumber);

		sum1 = mSalt.saltDWords[1] + GetHighDWord(blockNumber) + 
			GetLowDWord(blockNumber);

		for (PGPUInt32 i = 0; i < kBlockSizeIn32 - 2; i++)
		{
			sum1	+= src[i];
			sum0	+= sum1;		// simple Fletcher checksum of block
		}
	
		iv0	= sum0 ^ src[kBlockSizeIn32 - 2];
		iv1	= sum1 ^ src[kBlockSizeIn32 - 1];
	
	#if PGP_USE_ASSEMBLY

		CAST5encryptCFBdblAsm(mCastCFB.expandedKey.keyDWords, iv0, iv1, 
			~iv0, ~iv1, src, dest, kCipherBlockSize / (8+8));

	#else	// !PGP_USE_ASSEMBLY

		CAST5encryptCFBdbl(mCastCFB.expandedKey.keyDWords, iv0, iv1, ~iv0, 
			~iv1, src, dest, kCipherBlockSize / (8+8));

	#endif	// PGP_USE_ASSEMBLY

		dest[kBlockSizeIn32 - 2] ^= sum0;
		dest[kBlockSizeIn32 - 1] ^= sum1;
		}
		break;

	case kCopyDataEncryptionAlgorithm:
		pgpCopyMemory(src, dest, kCipherBlockSize);
		break;

	default:
		pgpAssert(FALSE);
		break;
	}
}

// DecryptBlock512 decrypts the given blocks using the object's context.

void
CipherContext::DecryptBlock512(
	PGPUInt64		blockNumber,
	const PGPUInt32	*src,
	PGPUInt32		*dest)
{
	pgpAssertAddrValid(src, PGPUInt32);
	pgpAssertAddrValid(dest, PGPUInt32);
	pgpAssert(mInitialized);
	pgpAssert(!mIsFlipped);
	pgpAssert(mAlgorithm != kInvalidEncryptionAlgorithm);

	switch (mAlgorithm)
	{

	case kBrokenCASTEncryptionAlgorithm: 
	case kCASTEncryptionAlgorithm: 
		{
		PGPUInt32	iv0, iv1;
		PGPUInt32	kSplitPointIn32;
		size_t		kBytesBeforeSplit;
		size_t		kBytesAfterSplit;

		kSplitPointIn32		= kBlockSizeIn32/2;
		kBytesBeforeSplit	= kSplitPointIn32 * sizeof(PGPUInt32);
		kBytesAfterSplit	= (kBlockSizeIn32-kSplitPointIn32) * 
			sizeof(PGPUInt32);

	#if PGP_USE_ASSEMBLY

		// This must be done in two halves to recover the IV.
		CAST5decryptCFBdblAsm(mCastCFB.expandedKey.keyDWords,
			src[kSplitPointIn32 - 4], src[kSplitPointIn32 - 3],
			src[kSplitPointIn32 - 2], src[kSplitPointIn32 - 1],
			src + kSplitPointIn32, dest + kSplitPointIn32, 
			kBytesAfterSplit/(8+8));

		iv0	= dest[kBlockSizeIn32 - 2];
		iv1	= dest[kBlockSizeIn32 - 1];

		CAST5decryptCFBdblAsm(mCastCFB.expandedKey.keyDWords, iv0, iv1, 
			~iv0, ~iv1, src, dest, kBytesBeforeSplit/(8+8));

	#else	// PGP_USE_ASSEMBLY

		// This must be done in two halves to recover the IV.
		CAST5decryptCFBdbl(mCastCFB.expandedKey.keyDWords,
			src[kSplitPointIn32 - 4], src[kSplitPointIn32 - 3],
			src[kSplitPointIn32 - 2], src[kSplitPointIn32 - 1],
			src + kSplitPointIn32, dest + kSplitPointIn32, 
			kBytesAfterSplit/(8+8));

		iv0	= dest[kBlockSizeIn32 - 2];
		iv1	= dest[kBlockSizeIn32 - 1];

		CAST5decryptCFBdbl(mCastCFB.expandedKey.keyDWords, iv0, iv1, ~iv0, 
			~iv1, src, dest, kBytesBeforeSplit/(8+8));

	#endif	// PGP_USE_ASSEMBLY

		// Undo the checksum.
		iv0 = mSalt.saltDWords[0] + GetHighDWord(blockNumber) + 
			GetLowDWord(blockNumber);

		iv1 = mSalt.saltDWords[1] + GetHighDWord(blockNumber) + 
			GetLowDWord(blockNumber);

		for (PGPUInt32 i = 0; i < kBlockSizeIn32 - 2; i++)
		{
			iv1	+= dest[i];
			iv0	+= iv1;
		}

		dest[kBlockSizeIn32 - 2]	^= iv0;
		dest[kBlockSizeIn32 - 1]	^= iv1;
		}
		break;

	case kCopyDataEncryptionAlgorithm:
		pgpCopyMemory(src, dest, kCipherBlockSize);
		break;

	default:
		pgpAssert(FALSE);
		break;
	}
}
