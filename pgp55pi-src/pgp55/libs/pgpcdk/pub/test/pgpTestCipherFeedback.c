/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: pgpTestCipherFeedback.c,v 1.8 1997/10/14 01:48:46 heller Exp $
____________________________________________________________________________*/

#include "pgpConfig.h"
#include <stdlib.h>
#include <string.h>

#include "pgpCFB.h"
#include "pgpErrors.h"
#include "pgpTest.h"

	
	static PGPUInt32
GetKeySizeForAlgorithm( PGPCipherAlgorithm	algorithm )
{
	PGPUInt32	keySize	= 0;
	
	switch( algorithm )
	{
		default:
			keySize	= 0;
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



/*____________________________________________________________________________
	Test a cipher feedback algorithm.
____________________________________________________________________________*/
	static void
TestCipherFeedbackAlg(
	PGPContextRef		context,
	PGPCipherAlgorithm	algorithm,
	PGPUInt16			interleave,
	PGPUInt32			testSize )
{
	PGPError						err	= kPGPError_NoErr;
	PGPSymmetricCipherContextRef	symmetricRef	= NULL;
	
	err	= PGPNewSymmetricCipherContext( context, algorithm,
			GetKeySizeForAlgorithm( algorithm), &symmetricRef );
	if ( IsntPGPError( err ) )
	{
		PGPCFBContextRef	ref;
		PGPByte				key[ 32 ];
		PGPByte *			iv	= NULL;
		PGPSize				blockSize;
		
		PGPGetSymmetricCipherSizes( symmetricRef, NULL, &blockSize );
		
		/*
		 * this is *NOT* a good choice for an iv, but for test
		 * purposes it suffices.
		 */
		iv	= (PGPByte *)malloc( blockSize * interleave );
		if ( IsNull( iv ) )
		{
			err	= kPGPError_OutOfMemory;
		}
		
		if ( IsntPGPError( err ) )
		{
			err	= PGPNewCFBContext( symmetricRef, interleave, &ref );
			symmetricRef	= NULL;	/* no longer belongs to us */
		}
		
		if ( IsntPGPError( err ) )
		{
			PGPCFBContextRef		tempRef	= NULL;

			/* test a simple copy */
			err	= PGPCopyCFBContext( ref, &tempRef );
			if ( IsntPGPError( err ) )
			{
				PGPFreeCFBContext( tempRef );
			}
			
			err	= PGPInitCFB( ref, key, iv );
			pgpTestAssert( IsntPGPError( err ) );
			if ( IsntPGPError( err ) )
			{
				PGPByte *	testBuffer		= NULL;
				
				testBuffer	= (PGPByte *)malloc( testSize * 3 );
				pgpTestAssert( IsntNull( testBuffer ) );
				if ( IsntNull( testBuffer ) )
				{
					PGPByte *	clearText		= NULL;
					PGPByte *	cipherText		= NULL;
					PGPByte *	decryptedText	= NULL;
					
					clearText		= testBuffer;
					cipherText		= clearText + testSize;
					decryptedText	= cipherText + testSize;
				
					err	= PGPCFBEncrypt( ref, clearText,
						testSize, cipherText );
					if ( IsntPGPError( err ) )
					{
						/* reinitialize for decryption */
						err	= PGPInitCFB( ref, key, iv );
						pgpTestAssert( IsntPGPError( err ) );
					
						err	= PGPCFBDecrypt( ref, cipherText,
							testSize, decryptedText );
					}
					pgpTestAssert( IsntPGPError( err ) );
					
					pgpTestAssert( memcmp( clearText,
						decryptedText, testSize ) == 0 );
					
					free( testBuffer );
				}
			}
			
			pgpTestAssert( IsntPGPError( err ) );
			err	= PGPFreeCFBContext( ref );
		}
		
		if ( IsntNull( iv ) )
		{
			free( iv );
		}
	}
	
	pgpTestAssert( IsntPGPError( err ) );
}


/*____________________________________________________________________________
	Test all cipher feedback algorithms
____________________________________________________________________________*/
	void
TestCipherFeedback( PGPContextRef context )
{
	PGPError						err	= kPGPError_NoErr;
	PGPUInt32						testIndex;

	(void)err;

	#define kNumTests		1
	for( testIndex = 0; testIndex < kNumTests; ++testIndex )
	{
		PGPUInt16	interleave;
		
		#define kCFBTestSize	( 32 * 1024UL + 1 )
		#define kNumInterleave	9
		
		for ( interleave = 1; interleave <= kNumInterleave; ++interleave )
		{
			TestCipherFeedbackAlg( context,
				kPGPCipherAlgorithm_IDEA, interleave, kCFBTestSize);
				
			TestCipherFeedbackAlg( context,
				kPGPCipherAlgorithm_3DES, interleave, kCFBTestSize);
				
			TestCipherFeedbackAlg( context,
			kPGPCipherAlgorithm_CAST5, interleave, kCFBTestSize);
		}
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
