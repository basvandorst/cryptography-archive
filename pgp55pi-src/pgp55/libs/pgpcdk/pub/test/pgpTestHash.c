/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: pgpTestHash.c,v 1.8 1997/10/14 01:48:47 heller Exp $
____________________________________________________________________________*/

#include "pgpConfig.h"
#include <stdlib.h>
#include <string.h>

#include "pgpHash.h"
#include "pgpErrors.h"
#include "pgpTest.h"









	static void
TestHashAlg(
	PGPContextRef 		context,
	PGPHashAlgorithm	algorithm,
	PGPUInt32			testSize )
{
	PGPError				err	= kPGPError_NoErr;
	PGPHashContextRef		ref;
	
	err	= PGPNewHashContext( context, algorithm, &ref );
	if ( IsntPGPError( err ) )
	{
		PGPHashContextRef		tempRef;
		const PGPByte *			data	= NULL;
	
		err	= PGPCopyHashContext( ref, &tempRef );
		if ( IsntPGPError( err ) )
		{
			err	= PGPFreeHashContext( tempRef );
		}
		
		err	= PGPResetHash( ref );
		pgpTestAssert( IsntPGPError( err ) );
		
		data	= (const PGPByte *)malloc( testSize );
		pgpTestAssert( IsntNull( data ) );
		if ( IsntNull( data ) )
		{
			PGPSize		hashSize;
			PGPByte		hashResult1[ 20 ];
			PGPByte		hashResult2[ 20 ];
			
			err	= PGPGetHashSize( ref, &hashSize );
			pgpTestAssert( IsntPGPError( err ) );
			pgpTestAssert( hashSize <= sizeof( hashResult1 ) );
			
			err	= PGPContinueHash( ref, data, testSize );
			pgpTestAssert( IsntPGPError( err ) );
			err	= PGPFinalizeHash( ref, hashResult1 );
			pgpTestAssert( IsntPGPError( err ) );
			
			/* reset and do it again to verify we get the same thing */
			err	= PGPResetHash( ref );
			pgpTestAssert( IsntPGPError( err ) );
			err	= PGPContinueHash( ref, data, testSize );
			pgpTestAssert( IsntPGPError( err ) );
			err	= PGPFinalizeHash( ref, hashResult2 );
			pgpTestAssert( IsntPGPError( err ) );
			
			pgpTestAssert( memcmp( hashResult1, hashResult2, hashSize ) == 0 );
			
			free( (void *)data );
			data	= NULL;
		}
		
		(void)PGPFreeHashContext( ref );
	}
}




	void
TestHash( PGPContextRef context )
{
	PGPError						err	= kPGPError_NoErr;
	PGPUInt32						testIndex;

	(void)err;

	/* create a lot of them */
	#define kNumTests		1
	for( testIndex = 0; testIndex < kNumTests; ++testIndex )
	{
		#define kHashTestSize	( 256 * 1024UL + 13 )
		TestHashAlg( context, kPGPHashAlgorithm_MD5, kHashTestSize );
		TestHashAlg( context, kPGPHashAlgorithm_SHA, kHashTestSize);
		TestHashAlg( context, kPGPHashAlgorithm_RIPEMD160, kHashTestSize);
	}
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
