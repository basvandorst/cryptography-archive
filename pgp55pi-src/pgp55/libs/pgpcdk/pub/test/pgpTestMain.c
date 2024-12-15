/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: pgpTestMain.c,v 1.45 1997/10/13 23:58:07 hal Exp $
____________________________________________________________________________*/

#include "pgpConfig.h"
#include <stdio.h>
#include <string.h>

#include "pgpEncode.h"
#include "pgpErrors.h"
#include "pgpTest.h"
#include "pgpUtilities.h"


static void	TestDecode( PGPContextRef context );

	static void
TestCommand(
	PGPContextRef	context,
	char const *	commandString,
	char const *	baseName,
	char const *	inNameSuffix,
	char const *	outNameSuffix )
{
	char	cmd[ 512 ];
	
	sprintf( cmd, "%s %s%s %s%s",
		commandString,
		baseName, inNameSuffix, baseName, outNameSuffix );
	TestEncodeCmdLine( context, cmd );
}

/*____________________________________________________________________________
	This code requires that a test file called "testin" exists in your
	current directory.
____________________________________________________________________________*/
	static void
TestEncodeCanned(
	PGPContextRef	context,
	char const *	testName )
{
	static const char	kConvEncryptedSuffix[]			= "_C";
	static const char	kConvEncryptedArmoredSuffix[]	= "_CA";
	static const char	kConvCleartextSuffix[]			= "_Cclr";
	static const char	kEncryptedSuffix[]				= "_crpt";
	static const char	kEncryptedArmoredSuffix[]		= "_Acrpt";
	static const char	kDecryptedSuffix[]				= "_clr";
	static const char	kSignedSuffix[]					= "_sig";
	static const char	kVerifiedSuffix[]				= "_ver";
	static const char	kSignedDetachedSuffix[]			= ".sig";
	static const char	kSignedArmoredSuffix[]			= "_Asig";
	
	printf( "\n\nTestEncodeCanned, file '%s'...\n", testName);
	
	/* conventionally encrypt/decrypt the file */
	TestCommand( context, "-i force -c test",
		testName, "", kConvEncryptedSuffix );
	TestCommand( context, "-i auto -z test -d",
		testName, kConvEncryptedSuffix, kConvCleartextSuffix );
		
	/* conventionally encrypt/decrypt the file with ascii armor*/
	TestCommand( context, "-a -i force -c test",
		testName, "", kConvEncryptedArmoredSuffix );
	TestCommand( context, "-i auto -z test -d",
		testName, kConvEncryptedArmoredSuffix, kConvCleartextSuffix);
	
	/* encrypt known input file to output */
	/* decrypt output file to cleartext temp */
	TestCommand( context, "-i force -e <test@pgp.com>",
		testName, "", kEncryptedSuffix );
	TestCommand( context, "-i auto -z test -d",
		testName, kEncryptedSuffix, kDecryptedSuffix );
		
	/* same as above, but with ascii armor */
	TestCommand( context, "-a -i force -e <test@pgp.com>",
		testName, "", kEncryptedArmoredSuffix );
	TestCommand( context, "-i auto -z test -d",
		testName, kEncryptedArmoredSuffix, kDecryptedSuffix );
		
	/* sign file */
	TestCommand( context, "-i force -s <test@pgp.com>  -z test",
		testName, "", kSignedSuffix );
	TestCommand( context, "-i auto -d",
		testName, kSignedSuffix, kVerifiedSuffix);

	/* same as above, but with ascii armor */
	TestCommand( context, "-a -i force -s <test@pgp.com> -z test",
		testName, "", kSignedArmoredSuffix );
	TestCommand( context, "-i auto -d",
		testName, kSignedArmoredSuffix, kVerifiedSuffix);

	/* sign file with detached sig */
	TestCommand( context, "-i force -s <test@pgp.com>  -z test -b",
		testName, "", kSignedDetachedSuffix );
	TestCommand( context, "-i force -v",
		testName, kSignedDetachedSuffix, "" );
}


#if defined( PGP_MACINTOSH )

	static void
TestSpecificFiles( PGPContextRef	context )
{
	TestEncodeCanned( context, "dualforks" );
	TestEncodeCanned( context, "plaintext" );
}

#elif defined( PGP_WIN32 ) || defined( PGP_UNIX )

	static void
TestSpecificFiles( PGPContextRef	context )
{
	TestEncodeCanned( context, "./testfiles/testin" );
}

#endif

	void
TestMain()
	{
	PGPContextRef	context			= NULL;
	PGPError		err	= kPGPError_NoErr;

	printf( "beginning tests...\n" );
	

	err = PGPsdkInit( );
	if ( IsPGPError( err ) )
	{
		printf ("Initialization error: %d\n", err );
	}
	err	= PGPNewContext( kPGPsdkAPIVersion, &context );
	if ( IsntPGPError( err ) )
	{
		printf( "\n\nTestSymmetricCipher...\n" );
		TestSymmetricCipher( context );
		
		printf( "\n\nTestCipherFeedback...\n" );
		TestCipherFeedback( context );
		
		printf( "\n\nTestHash...\n" );
		TestHash( context );
		
		printf( "\n\nTestKeys...\n" );
		TestKeys( context );

		/* dump all the keys */
		TestEncodeCmdLine( context, "-l" );
		
		/* test a variety of operations on different files */
		TestSpecificFiles( context );
		
		TestDecode( context );
		
		(void)PGPFreeContext( context );
		(void)PGPsdkCleanup( );
	}
	
	printf( "done with tests.\n" );
}



static const char	sKeyBlock[]	=
"-----BEGIN PGP PUBLIC KEY BLOCK-----\n"
"Version: PGPsdk version 0.0.1 (C) 1997 Pretty Good Privacy, Inc\n"
"\n"
"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\n"
"=Vw/M\n"
"-----END PGP PUBLIC KEY BLOCK-----\n"
"\n\n"
"hello there\n";


	static void
TestDecode( PGPContextRef	context )
{
	PGPError	err	= kPGPError_NoErr;
	void *		buffer	= NULL;
	PGPSize		bufferSize;
	#define kMaxBufferSize		1024 * 1024
	PGPContextRef	c	= context;
	
	printf( "\n\nTesting various decode operations...\n" );

	/* decode key block, but ignore keys */
	err	= PGPDecode( c,
			PGPOInputBuffer( c, sKeyBlock, strlen( sKeyBlock )),
			PGPOAllocatedOutputBuffer( c,
				&buffer, kMaxBufferSize, &bufferSize ),
			PGPOLastOption( c)
		);
	pgpTestAssertNoErr( err );
	if ( IsntPGPError( err ) )
	{
		/* shouldn't be any output with above call */
		pgpTestAssert( IsNull( buffer ) );
	}
	
	
	/* decode key block, ignore keys, but ask for data */
	err	= PGPDecode( c,
			PGPOInputBuffer( c, sKeyBlock, strlen( sKeyBlock )),
			PGPOAllocatedOutputBuffer( c,
				&buffer, kMaxBufferSize, &bufferSize ),
			PGPOPassThroughIfUnrecognized( c, TRUE ),
			PGPOLastOption( c)
		);
	pgpTestAssertNoErr( err );
	if ( IsntPGPError( err ) )
	{
		/* shouldn't be any output with above call */
		pgpTestAssert( IsntNull( buffer ) );
		if ( IsntNull( buffer ) )
		{
			PGPFreeData( buffer );
			buffer	= NULL;
		}
	}
}





	static void
pgpTestShowError( PGPError	err )
{
	if ( IsPGPError( err ) )
	{
		char		errString[ 256 ];
		
		PGPGetErrorString( err, sizeof( errString), errString );
		fprintf( stderr, "ERROR: %s\n", errString );
	}
}


	void
pgpTestAssertNoErr( PGPError err )
{
	pgpTestShowError( err );
}


	void
_pgpTestAssert(
	PGPBoolean		condition,
	const char *	expr )
{
	if ( ! condition )
	{
		fprintf( stderr, "ASSERTION FAILED: %s\n", expr );
	}
}


	void
pgpTestDebugMsg( const char *msg )
{
	fprintf( stderr, "%s\n", msg );
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
