/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: pgpTestKeys.c,v 1.7 1997/10/08 02:30:22 lloyd Exp $
____________________________________________________________________________*/
#include "pgpConfig.h"

#include <string.h>
#include <stdio.h>

#include "pgpKeys.h"
#include "pgpUtilities.h"
#include "pgpErrors.h"

#include "pgpTest.h"

static const char kSingleBlockWithTwoKeys[] =
"-----BEGIN PGP PUBLIC KEY BLOCK-----\r\n"
"Version: PGP for Personal Privacy 5.0\r\n"
"\r\n"
"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\r\n"
"=5Hu3\r\n"
"-----END PGP PUBLIC KEY BLOCK-----\r\n";


static const char kMultiBlockWithTwoKeys[] =
"-----BEGIN PGP PUBLIC KEY BLOCK-----\r\n"
"Version: PGP for Personal Privacy 5.0\r\n"
"\r\n"
"mQENAzLxHkoAAAEH/2dyJjKwOGNNWawJ6LOYjU1vuY/N0ZZ3DLg7wfF4wgGhSi7W\r\n"
"GKpiF4xwInKTfL5kI1KsrCngSZ0PDUU1O7esZ1o/ojvwsUgYjrMR5xHX9+tU33s+\r\n"
"42Ac80j2SXuUKjjQUrcdH1h+m+1fMCEGlGWCd0dDdWgb7soLolRR/cctRvZY7gRb\r\n"
"ccslk1dXNv87s3SJ556iSZd6e0Zzuqfgu3Ymb/aeFPVWj869cnYdIa++JKv0Snfz\r\n"
"N24kXJsNZnAEtjLy751M6x2VpuOMy2oY+yHN4OGh5L2qfI5Ru75L7ga5lQEHQvR0\r\n"
"tCqsArUxmOPYledGaXXe0biDHDhCCXa52l7M1I0ABRG0NVByZXR0eSBHb29kIFBy\r\n"
"aXZhY3ksIEluYy4gQ29ycG9yYXRlIEtleSA8cGdwQHBncC5jb20+iQCVAwUQMvEV\r\n"
"hWV5hLjHqWbdAQHgtAQAh85mL4aesW7SteK5RLnl+Y/AKCBbOSF8Ec1g0UqG6kny\r\n"
"9QvaFRX9vfIhQsCGocpT7bMd13LWRlmDN9Dbyt8PCGQ+tru7f3RcHAKxOWU7sLiw\r\n"
"xIQ6Vk8VL2KY5Td0jMUq7uiZuh63TNP8TLaMy+tfnLV1VUAPMBI0iXH7HTmRjqOJ\r\n"
"ARUDBRAzPGHme2PSDkhvadkBAZrvB/0SXHiIVBKIUKboAL+MszMUquITx+VfBrs2\r\n"
"qZiU5ZEqYY4NHhFg27BBagWRo1W1YfdQHrNspisJYlJPuqnMEVP7Hvngk4ZWU9+0\r\n"
"781VQixCTvJmiZ0o1CE7c0j6F7O3Lf+wDrni6E6wE/2KMx3aU4kQk7rn85qGNRmZ\r\n"
"s5yhj59OJvx6e93lKjqBMDVuTV+17uJA4x4K94u6CCE7XGQu9+8IdQfe5xYu1k+a\r\n"
"wEAyVRU4Vx0Op4Y90lGNa/GvEFsc4Y5UCnbqfxBJbA8GfrmHeRdb7Q4FfYh6EPl5\r\n"
"OS/ulDZkghsVDVL1UJMRXEd4zmFN7ehZje4WlHCqMQB+FK6ka0fc\r\n"
"=i/U6\r\n"
"-----END PGP PUBLIC KEY BLOCK-----\r\n"
"\r\n"
"-----BEGIN PGP PUBLIC KEY BLOCK-----\r\n"
"Version: PGP for Personal Privacy 5.0\r\n"
"\r\n"
"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\r\n"
"=maAF\r\n"
"-----END PGP PUBLIC KEY BLOCK-----\r\n";



	static void
TestKeyIDUsingString(
	PGPContextRef	context,
	const char *	string)
{
	char			keyString[ kPGPMaxKeyIDStringSize ];
	PGPByte			exportedData[ kPGPMaxExportedKeyIDSize ];
	PGPSize			exportedSize;
	PGPError		err	= kPGPError_NoErr;
	PGPKeyID		keyID;
	
	(void)context;
	/* create a new one from the string */
	err	= PGPGetKeyIDFromString( string, &keyID );
	pgpTestAssertNoErr( err );
	
	err	= PGPGetKeyIDString( &keyID, kPGPKeyIDString_Abbreviated, keyString );
	pgpTestAssertNoErr( err );
	
	err	= PGPGetKeyIDString( &keyID, kPGPKeyIDString_Full, keyString );
	pgpTestAssertNoErr( err );
	pgpTestAssert( strcmp( keyString, string ) == 0 );
	
	err	= PGPGetKeyIDFromString( keyString, &keyID );
	pgpTestAssertNoErr( err );
	
	err	= PGPExportKeyID( &keyID, exportedData, &exportedSize );
	pgpTestAssertNoErr( err );
	
	err	= PGPImportKeyID( exportedData, &keyID );
	pgpTestAssertNoErr( err );
}


	static void
TestKeyID( PGPContextRef context )
{
	printf( "TestKeyID...\n" );
	TestKeyIDUsingString( context, "0x0189ABEF" );
	TestKeyIDUsingString( context, "0xFEBA9810" );
	TestKeyIDUsingString( context, "0x0123456789ABCDEF" );
	TestKeyIDUsingString( context, "0xFEDCBA9876543210" );
}



	static void
TestImportKeys( PGPContextRef context )
{
	PGPKeySetRef	keySetRef	= NULL;
	PGPError		err			= kPGPError_NoErr;
	PGPUInt32		numKeys;
	PGPContextRef	c	= context;
	
	printf( "TestImportKeys...\n" );
	
	err	= PGPImportKeySet( context,
			&keySetRef,
			PGPOInputBuffer( c,
				kSingleBlockWithTwoKeys, strlen( kSingleBlockWithTwoKeys ) ),
			PGPOLastOption( c ) );
	pgpTestAssertNoErr( err );
	if ( IsntPGPError( err ) )
	{
		err	= PGPCountKeys( keySetRef, &numKeys);
		pgpTestAssertNoErr( err );
		pgpTestAssert( numKeys == 2 );
		PGPFreeKeySet( keySetRef );
		keySetRef	= NULL;
	}
	
	err	= PGPImportKeySet( context,
			&keySetRef,
			PGPOInputBuffer( c,
				kMultiBlockWithTwoKeys, strlen( kMultiBlockWithTwoKeys ) ),
			PGPOLastOption( c ) );
	pgpTestAssertNoErr( err );
	if ( IsntPGPError( err ) )
	{
		err	= PGPCountKeys( keySetRef, &numKeys);
		pgpTestAssertNoErr( err );
		pgpTestAssert( numKeys == 2 );
		PGPFreeKeySet( keySetRef );
		keySetRef	= NULL;
	}
}


	void
TestKeys( PGPContextRef context )
{
	TestKeyID( context );
	TestImportKeys( context );
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
