#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "crypt.h"
#include "shs.h"

/* The default key setup iteration count.  This gives roughly 0.5s delay on
   a 10K dhrystone machine */

#define MDCSHS_DEFAULT_ITERATIONS	200

/* The size of an MDCSHS key */

#define MDCSHS_KEYSIZE				128		/* 1024 bits */

/****************************************************************************
*																			*
*							Init/Shutdown Routines							*
*																			*
****************************************************************************/

/* Perform auxiliary init and shutdown actions on an encryption context */

int mdcshsInitEx( CRYPT_INFO *cryptInfo, void *cryptInfoEx )
	{
	/* Allocate memory for the key and the algorithm-specific data within
	   the crypt context and set up any pointers we need */
	if( cryptInfo->key != NULL || cryptInfo->privateData != NULL )
		return( CRYPT_INITED );
	if( ( cryptInfo->key = malloc( MDCSHS_KEYSIZE ) ) == NULL )
		return( CRYPT_NOMEM );
	if( ( cryptInfo->privateData = malloc( sizeof( CRYPT_INFO_MDCSHS ) ) ) == NULL )
		{
		free( cryptInfo->key );
		cryptInfo->key = NULL;
		return( CRYPT_NOMEM );
		}
	memcpy( cryptInfo->privateData, cryptInfoEx, sizeof( CRYPT_INFO_MDCSHS ) );
	cryptInfo->keyLength = MDCSHS_KEYSIZE;

	return( CRYPT_OK );
	}

int mdcshsInit( CRYPT_INFO *cryptInfo )
	{
	CRYPT_INFO_MDCSHS cryptInfoEx;

	/* Use the default number of setup iterations */
	memset( &cryptInfoEx, 0, sizeof( CRYPT_INFO_MDCSHS ) );
	cryptInfoEx.keySetupIterations = MDCSHS_DEFAULT_ITERATIONS;

	/* Pass through to the extended setup routine */
	return( mdcshsInitEx( cryptInfo, &cryptInfoEx ) );
	}

int mdcshsEnd( CRYPT_INFO *cryptInfo )
	{
	/* Free any allocated memory */
	secureFree( &cryptInfo->key, cryptInfo->keyLength );
	secureFree( &cryptInfo->privateData, sizeof( CRYPT_INFO_MDCSHS ) );

	return( CRYPT_OK );
	}

/****************************************************************************
*																			*
*						MDC/SHS En/Decryption Routines						*
*																			*
****************************************************************************/

/* The SHS transformation */

#define shsTransform(cI)	SHSTransform( ( LONG * ) cI->currentIV, \
										  ( LONG * ) cI->key )

void SHSTransform( LONG *digest, LONG *data );

/* Encrypt data in CFB mode */

int mdcshsEncrypt( CRYPT_INFO *cryptInfo, BYTE *buffer, int noBytes )
	{
	int i, ivCount = cryptInfo->ivCount;

	/* If there's any encrypted material left in the IV, use it now */
	if( ivCount )
		{
		int bytesToUse;

		/* Find out how much material left in the encrypted IV we can use */
		bytesToUse = SHS_DIGESTSIZE - ivCount;
		if( noBytes < bytesToUse )
			bytesToUse = noBytes;

		/* Encrypt the data */
		for( i = 0; i < bytesToUse; i++ )
			buffer[ i ] ^= cryptInfo->currentIV[ i + ivCount ];
		memcpy( cryptInfo->currentIV + ivCount, buffer, bytesToUse );

		/* Adjust the byte count and buffer position */
		noBytes -= bytesToUse;
		buffer += bytesToUse;
		ivCount += bytesToUse;
		}

	while( noBytes )
		{
		ivCount = ( noBytes > SHS_DIGESTSIZE ) ? SHS_DIGESTSIZE : noBytes;

		/* Encrypt the IV */
		shsTransform( cryptInfo );

		/* XOR the buffer contents with the encrypted IV */
		for( i = 0; i < ivCount; i++ )
			buffer[ i ] ^= cryptInfo->currentIV[ i ];

		/* Shift ciphertext into IV */
		memcpy( cryptInfo->currentIV, buffer, ivCount );

		/* Move on to next block of data */
		noBytes -= ivCount;
		buffer += ivCount;
		}

	/* Remember how much of the IV is still available for use */
	cryptInfo->ivCount = ( ivCount % SHS_DIGESTSIZE );

	return( CRYPT_OK );
	}

/* Decrypt data in CFB mode.  Note that the transformation can be made
   faster (but less clear) with temp = buffer, buffer ^= iv, iv = temp
   all in one loop */

int mdcshsDecrypt( CRYPT_INFO *cryptInfo, BYTE *buffer, int noBytes )
	{
	BYTE temp[ SHS_DIGESTSIZE ];
	int i, ivCount = cryptInfo->ivCount;

	/* If there's any encrypted material left in the IV, use it now */
	if( ivCount )
		{
		int bytesToUse;

		/* Find out how much material left in the encrypted IV we can use */
		bytesToUse = SHS_DIGESTSIZE - ivCount;
		if( noBytes < bytesToUse )
			bytesToUse = noBytes;

		/* Decrypt the data */
		memcpy( temp, buffer, bytesToUse );
		for( i = 0; i < bytesToUse; i++ )
			buffer[ i ] ^= cryptInfo->currentIV[ i + ivCount ];
		memcpy( cryptInfo->currentIV + ivCount, temp, bytesToUse );

		/* Adjust the byte count and buffer position */
		noBytes -= bytesToUse;
		buffer += bytesToUse;
		ivCount += bytesToUse;
		}

	while( noBytes )
		{
		ivCount = ( noBytes > SHS_DIGESTSIZE ) ? SHS_DIGESTSIZE : noBytes;

		/* Encrypt the IV */
		shsTransform( cryptInfo );

		/* Save ciphertext */
		memcpy( temp, buffer, ivCount );

		/* XOR the buffer contents with the encrypted IV */
		for( i = 0; i < ivCount; i++ )
			buffer[ i ] ^= cryptInfo->currentIV[ i ];

		/* Shift ciphertext into IV */
		memcpy( cryptInfo->currentIV, temp, ivCount );

		/* Move on to next block of data */
		noBytes -= ivCount;
		buffer += ivCount;
		}

	/* Remember how much of the IV is still available for use */
	cryptInfo->ivCount = ( ivCount % SHS_DIGESTSIZE );

	return( CRYPT_OK );
	}

/****************************************************************************
*																			*
*						MDC/SHS Key Management Routines						*
*																			*
****************************************************************************/

/* The size of the key buffer.  Note that increasing this value will result
   in a significant increase in the setup time, so it will be necessary to
   make a corresponding decrease in the iteration count */

#define KEYBUFFER_SIZE		256

/* Initialise an MDC/SHS key.  This is done by repeatedly encrypting the
   user key as follows:

	IV <- 0
	key <- 0
	repeat
		encrypt userkey with key
		key <- userkey

   The IV is updated transparently as part of the encryption process */

int mdcshsInitKey( CRYPT_INFO *cryptInfo )
	{
	BYTE keyData[ KEYBUFFER_SIZE ];
	int keySetupIterations = ( ( CRYPT_INFO_MDCSHS * ) cryptInfo->privateData )->keySetupIterations;
	int count;

	/* Copy the key information into the key data buffer.  We silently
	   truncate the key length to the size of the buffer, but this usually
	   is some hundreds of bytes so it shouldn't be a problem.
	   We also correct the endianness of the keyData at this point.  From
	   here on all data is in the local endianness format so there is no need
	   for further corrections */
	memset( keyData, 0, KEYBUFFER_SIZE );
	keyData[ 0 ] = ( BYTE ) ( cryptInfo->userKeyLength >> 8 );
	keyData[ 1 ] = ( BYTE ) cryptInfo->userKeyLength;
	cryptInfo->userKeyLength %= KEYBUFFER_SIZE - sizeof( WORD );
	memcpy( keyData + sizeof( WORD ), cryptInfo->userKey, cryptInfo->userKeyLength );

	/* Set the initial key and IV to null */
	memset( cryptInfo->currentIV, 0, MAX_IVSIZE );
	memset( cryptInfo->key, 0, MDCSHS_KEYSIZE );

	/* Convert the endianness of the input data from the canonical form to
	   the local form */
	longReverse( ( LONG * ) keyData, KEYBUFFER_SIZE );

	/* "Encrypt" the keyData with the given IV and then set the key to
	   the encrypted keyData.  The act of encryption also sets the IV.
	   This is particularly important in the case of SHS, since although only
	   the first SHS_DATASIZE bytes are copied into the key, the IV is
	   still affected by the entire buffer */
	for( count = 0; count < keySetupIterations; count++ )
		{
		mdcshsEncrypt( cryptInfo, keyData, KEYBUFFER_SIZE );
		memcpy( cryptInfo->key, keyData, MDCSHS_KEYSIZE );
		}

	/* Perform one last copy in case they've specified zero iterations and
	   the loop was never executed.  For MDCSHS, the transformed key is the
	   same as the raw encryption key, so we just copy it over */
	memcpy( cryptInfo->key, keyData, MDCSHS_KEYSIZE );

#if 0
	/* Set the key check byte to the last WORD of the encrypted keyData.
	   This is never used for anything (it's 191 bytes past the end of the
	   last value used to set the key) so we're not revealing much to an
	   attacker */
	longReverse( ( LONG * ) ( keyData + KEYBUFFER_SIZE - sizeof( LONG ) ), sizeof( LONG ) );
	cryptInfo->keyCheck = ( ( WORD ) keyData[ KEYBUFFER_SIZE - 2 ] << 8 ) | \
								   keyData[ KEYBUFFER_SIZE - 1 ];
#endif /* 0 */

	/* Wipe the keyData */
	memset( keyData, 0, KEYBUFFER_SIZE );

	/* Finally, create a copy of the transformed key in the canonical form
	   in case the user wants to access it */
	memcpy( cryptInfo->transUserKey, cryptInfo->key, MDCSHS_KEYSIZE );
	longReverse( ( LONG * ) cryptInfo->transUserKey, MDCSHS_KEYSIZE );
	cryptInfo->keySet = TRUE;

	return( CRYPT_OK );
	}

/* Initialise the IV */

int mdcshsInitIV( CRYPT_INFO *cryptInfo )
	{
	/* Copy the IV to internal storage and convert it from the canonical to
	   the internal form */
	memcpy( cryptInfo->currentIV, cryptInfo->iv, MAX_IVSIZE );
	longReverse( ( LONG * ) cryptInfo->currentIV, MAX_IVSIZE );

	return( CRYPT_OK );
	}
