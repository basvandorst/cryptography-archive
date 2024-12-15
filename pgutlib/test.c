#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "crypt.h"

/* The size of the test buffers */

#define TESTBUFFER_SIZE		256

/* Work routines: Set a pair of encrypt/decrypt buffers to a known state,
   and make sure they're still in that known state */

static void initTestBuffers( BYTE *buffer1, BYTE *buffer2 )
	{
	/* Set the buffers to a known state */
	memset( buffer1, '*', TESTBUFFER_SIZE );
	memcpy( buffer1, "1234", 4 );		/* For endianness check */
	memcpy( buffer2, buffer1, TESTBUFFER_SIZE );
	}

static void checkTestBuffers( BYTE *buffer1, BYTE *buffer2 )
	{
	/* Make sure everything went OK */
	if( memcmp( buffer1, buffer2, TESTBUFFER_SIZE ) )
		{
		puts( "Warning: Decrypted data != original plaintext." );

		/* Try and guess at endianness problems - we want "1234" */
		if( !memcmp( buffer1, "4321", 4 ) )
			puts( "\t It looks like the 32-bit word endianness is reversed." );
		else
			if( !memcmp( buffer1, "2143", 4 ) )
				puts( "\t It looks like the 16-bit word endianness is "
					  "reversed." );
		else
			if( buffer1[ 0 ] >= '1' && buffer1[ 0 ] <= '9' )
				puts( "\t It looks like there's some sort of endianness "
					  "problem which is more complex than just a reversal." );
			else
				puts( "\t It's probably more than just and endianness "
					  "problem." );
		}
	}

/* Report information on the encryption algorithm */

static void reportAlgorithmInformation( char *algorithmName,
										CRYPT_QUERY_INFO *cryptQueryInfo )
	{
	char speedFactor[ 50 ];

	/* Determine the speed factor relative to a block copy */
	if( cryptQueryInfo->speed == CRYPT_ERROR )
		strcpy( speedFactor, "unknown speed factor" );
	else
		sprintf( speedFactor, "0.%03d times the speed of a block copy",
				 cryptQueryInfo->speed );

	printf( "%s algorithm is available with\n"
				"  name `%s', block size %d bits, %s,\n"
				"  min key size %d bits, recommended key size %d bits, "
					"max key size %d bits,\n"
				"  min IV size %d bits, recommended IV size %d bits, "
					"max IV size %d bits.\n",
				algorithmName, cryptQueryInfo->algoName,
				bytesToBits( cryptQueryInfo->blockSize ), speedFactor,
				bytesToBits( cryptQueryInfo->minKeySize ),
				bytesToBits( cryptQueryInfo->keySize ),
				bytesToBits( cryptQueryInfo->maxKeySize ),
				bytesToBits( cryptQueryInfo->minIVsize ),
				bytesToBits( cryptQueryInfo->ivSize ),
				bytesToBits( cryptQueryInfo->maxIVsize ) );
	}

/* Perform a test en/decryption */

static void testCrypt( CRYPT_INFO *cryptInfo, CRYPT_INFO *decryptInfo,
					   BYTE *buffer )
	{
	CRYPT_QUERY_INFO cryptQueryInfo;
	BYTE iv[ 100 ];
	int status;

	/* Encrypt the buffer in two odd-sized chunks */
	status = encryptBuffer( cryptInfo, buffer, 79 );
	if( isStatusError( status ) )
		printf( "Couldn't encrypt data: Code %d.\n", status );
	status = encryptBuffer( cryptInfo, buffer + 79, TESTBUFFER_SIZE - 79 );
	if( isStatusError( status ) )
		printf( "Couldn't encrypt data: Code %d.\n", status );
	status = encryptBuffer( cryptInfo, buffer + TESTBUFFER_SIZE, 0 );

	/* Copy the IV from the encryption to the decryption context */
	queryContextInformation( cryptInfo, &cryptQueryInfo );
	status = retrieveIV( cryptInfo, iv );
	if( isStatusError( status ) )
		printf( "Couldn't retrieve IV after encryption: Code %d.\n", status );
	status = loadIV( decryptInfo, iv, cryptQueryInfo.ivSize );
	if( isStatusError( status ) )
		printf( "Couldn't load IV for decryption: Code %d.\n", status );

	/* Decrypt the buffer in different odd-size chunks */
	status = decryptBuffer( decryptInfo, buffer, 125 );
	if( isStatusError( status ) )
		printf( "Couldn't decrypt data: Code %d.\n", status );
	status = decryptBuffer( decryptInfo, buffer + 125, TESTBUFFER_SIZE - 125 );
	if( isStatusError( status ) )
		printf( "Couldn't decrypt data: Code %d.\n", status );
	}

/* Sample code to test the MDC/SHS implementation.  This code uses MDC/SHS,
   but can be adapted for any algorithm or mode by changing the setting of
   CRYPT_ALGO and CRYPT_MODE */

static int testMDCSHS( void )
	{
	BYTE buffer[ TESTBUFFER_SIZE ], testBuffer[ TESTBUFFER_SIZE ];
	CRYPT_INFO cryptInfo, decryptInfo;
	CRYPT_INFO_MDCSHS cryptInfoEx;
	CRYPT_QUERY_INFO cryptQueryInfo;
	CRYPT_ALGO cryptAlgo = CRYPT_ALGO_MDCSHS;
	CRYPT_MODE cryptMode = CRYPT_MODE_CFB;
	int status;

	/* Initialise the test buffers */
	initTestBuffers( buffer, testBuffer );

	/* Check the capabilities of the library */
	status = queryAlgoAvailability( cryptAlgo );
	printf( "queryAlgoAvailability() reports " );
	if( isStatusOK( status ) )
		puts( "MDC/SHS is available" );
	else
		{
		printf( "MDC/SHS is not available: Code %d.\n", status );
		return( FALSE );
		}
	status = queryModeAvailability( cryptAlgo, cryptMode );
	printf( "queryModeAvailability() reports " );
	if( isStatusOK( status ) )
		puts( "MDC/SHS-CFB is available" );
	else
		{
		printf( "MDC/SHS-CFB is not available: Code %d.\n", status );
		return( FALSE );
		}
	status = queryAlgoModeInformation( cryptAlgo, cryptMode, &cryptQueryInfo );
	printf( "queryModeAvailability() reports " );
	if( isStatusOK( status ) )
		reportAlgorithmInformation( "MDC/SHS-CFB", &cryptQueryInfo );
	else
		{
		printf( "no information available on MDC/SHS algorithm: Code %d.\n",
				status );
		return( FALSE );
		}

	/* Set up an encryption context, load a user key into it, and perform a
	   key setup.   We use an extended setup with reduced iteration count for
	   people who have to run this thing 2000 times while debugging */
	cryptInfoEx.keySetupIterations = 10;
	status = initCryptContextEx( &cryptInfo, cryptAlgo, cryptMode,
								 &cryptInfoEx );
	if( isStatusError( status ) )
		{
		printf( "initCryptContext(): Failed with error code %d.\n", status );
		return( FALSE );
		}
	status = loadCryptContext( &cryptInfo, "Test key", 8 );
	if( isStatusError( status ) )
		{
		printf( "loadCryptContext(): Failed with error code %d.\n", status );
		return( FALSE );
		}

	/* Create another crypt context for decryption.  The error checking here
	   is a bit less picky to save space */
	cryptInfoEx.keySetupIterations = 10;
	initCryptContextEx( &decryptInfo, cryptAlgo, cryptMode, &cryptInfoEx );
	status = loadCryptContext( &decryptInfo, "Test key", 8 );
	if( isStatusError( status ) )
		{
		printf( "loadCryptContext(): Failed with error code %d.\n", status );
		return( FALSE );
		}

	/* Perform a test en/decryption */
	testCrypt( &cryptInfo, &decryptInfo, buffer );

	/* Make sure everything went OK */
	checkTestBuffers( buffer, testBuffer );

	/* Destroy the encryption contexts */
	status = destroyCryptContext( &cryptInfo );
	if( isStatusError( status ) )
		{
		printf( "destroyCryptContext(): Failed with error code %d.\n", status );
		return( FALSE );
		}
	status = destroyCryptContext( &decryptInfo );
	if( isStatusError( status ) )
		{
		printf( "destroyCryptContext(): Failed with error code %d.\n", status );
		return( FALSE );
		}

	return( TRUE );
	}

/* Exercise various aspects of the encryption library */

void main( void )
	{
	int status;

	/* Initialise the library */
	status = initLibrary();
	if( isStatusError( status ) )
		{
		printf( "Couldn't init library, code %d.\n", status );
		exit( 0 );
		}

	/* Test the encryption routines contained in the library */
	if( !testMDCSHS() )
		{
		puts( "Tests aborted due to encryption library error." );
		exit( 0 );
		}

	/* Shut down the library */
	status = endLibrary();
	if( isStatusError( status ) )
		{
		printf( "endLibrary(): Failed with error code %d.\n", status );
		exit( 0 );
		}
	}
