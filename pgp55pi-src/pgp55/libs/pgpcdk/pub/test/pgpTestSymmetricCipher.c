/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: pgpTestSymmetricCipher.c,v 1.10 1997/10/14 01:48:47 heller Exp $
____________________________________________________________________________*/

#include "pgpSymmetricCipher.h"
#include "pgpErrors.h"
#include "pgpTest.h"



	  
	static PGPUInt32
GetKeySizeForAlgorithm( PGPCipherAlgorithm	algorithm )
{
	PGPUInt32	keySize	= 0;
	
	switch( algorithm )
	{
		default:
			pgpTestDebugMsg( "unknown algorithm" );
			break;
		
		case kPGPCipherAlgorithm_IDEA:
		case kPGPCipherAlgorithm_CAST5:
			keySize	= 128 / 8 ;
			break;
		
		case kPGPCipherAlgorithm_3DES:
			keySize	= 192 / 8 ;
			break;
			
	}
	return( keySize );
}


	static void
TestSymmetricCipherAlg(
	PGPContextRef		context,
	PGPCipherAlgorithm	algorithm )
{
	PGPError						err	= kPGPError_NoErr;
	PGPSymmetricCipherContextRef	symmetricRef	= NULL;
	
	err	= PGPNewSymmetricCipherContext( context, algorithm,
			GetKeySizeForAlgorithm( algorithm), &symmetricRef );
	if ( IsntPGPError( err ) )
	{
		PGPSymmetricCipherContextRef	tempRef;
		PGPSize							keySize;
		PGPSize							blockSize;
		PGPByte							key[ 32 ];
		#define kBlockSize		8
		PGPByte							clearText[ kBlockSize ];
		PGPByte							cipherText[ kBlockSize ];
		PGPByte							decryptedText[ kBlockSize ];
		
		pgpTestAssert( GetKeySizeForAlgorithm( algorithm) <= sizeof( key ) );
		
		err	= PGPCopySymmetricCipherContext( symmetricRef, &tempRef );
		pgpTestAssert( IsntPGPError( err ) );
		if ( IsntPGPError( err ) )
		{
			err	= PGPFreeSymmetricCipherContext( tempRef );
			pgpTestAssert( IsntPGPError( err ) );
		}
		
		err	= PGPGetSymmetricCipherSizes( symmetricRef, &keySize, &blockSize );
		pgpTestAssert( IsntPGPError( err ) );
		pgpTestAssert( keySize == GetKeySizeForAlgorithm( algorithm) );
		pgpTestAssert( blockSize == kBlockSize );
		
		/* key shoudld be inited to random value */
		err	= PGPInitSymmetricCipher( symmetricRef, key );
		pgpTestAssert( IsntPGPError( err ) );
		
		err	= PGPSymmetricCipherEncrypt( symmetricRef, clearText, cipherText );
		pgpTestAssert( IsntPGPError( err ) );
		
		err	= PGPSymmetricCipherDecrypt( symmetricRef,
				cipherText, decryptedText );
		pgpTestAssert( IsntPGPError( err ) ||
			err == kPGPError_FeatureNotAvailable );
		err	= kPGPError_NoErr;
		
		err	= PGPFreeSymmetricCipherContext( symmetricRef );
		pgpTestAssert( IsntPGPError( err ) );
	}
	
	pgpTestAssert( IsntPGPError( err ) );
}



	void
TestSymmetricCipher( PGPContextRef	context )
{
	PGPError						err	= kPGPError_NoErr;
	PGPUInt32						testIndex;

	(void)err;
#if 0	/* keep; use to test bad params */
	{
	PGPSymmetricCipherContextRef	symmetricRef	= NULL;
	err	= PGPNewSymmetricCipher( NULL,
			kPGPCipherAlgorithm_IDEA, 16, &symmetricRef );
	pgpTestAssert(  err == kPGPError_BadParams );
	
	err	= PGPNewSymmetricCipher( context, kPGPCipherAlgorithm_None, 16, NULL );
	pgpTestAssert(  err == kPGPError_BadParams );
	
	err	= PGPNewSymmetricCipher( context,
			kPGPCipherAlgorithm_None, 16, &symmetricRef );
	pgpTestAssert(  err == kPGPError_BadParams );
	
	err	= PGPNewSymmetricCipher( context,
			kPGPCipherAlgorithm_None, 16, &symmetricRef );
	pgpTestAssert(  err == kPGPError_BadParams );
	}
#endif

	
	/* create a lot of them */
	#define kNumTests		10
	for( testIndex = 0; testIndex < kNumTests; ++testIndex )
	{
		TestSymmetricCipherAlg( context, kPGPCipherAlgorithm_IDEA );
		TestSymmetricCipherAlg( context, kPGPCipherAlgorithm_3DES );
		TestSymmetricCipherAlg( context, kPGPCipherAlgorithm_CAST5 );
	}
	#undef kNumTests
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
