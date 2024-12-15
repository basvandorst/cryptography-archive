/****************************************************************************
*																			*
*							MDC.C - MDC en/decryption code					*
*																			*
* 		Written by Peter Gutmann, pgut1@cs.aukuni.ac.nz, September 1992		*
*					You can use this code in any way you want,				*
*		although it'd be nice if you kept my name + contact address on it 	*
*																			*
****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mdc.h"

/* Function to perform byte-reversal in an array of LONG's for little-endian
   CPU's */

void longReverse( LONG *buffer, int byteCount );

/* Information needed in CFB mode:  The IV and the count of bytes already
   processed in the current block */

BYTE iv[ BLOCKSIZE ];			/* CFB initialization vector */
int ivCount;					/* Count of bytes used in block */

/* A delay count for the key setup.  We repeat the key setup operation this
   many times to slow it down.  The key setup is inherently non-
   parallelizable, making it necessary to sequentially step through each
   setup operation.  The value below has been chosen to give a delay of about
   0.5s on a 10K-Dhrystone machine */

#define DELAYCOUNT		100

/****************************************************************************
*																			*
*				MDC Key Generation and Encryption Routines					*
*																			*
****************************************************************************/

/* The Mysterious Constants and IV are generated by running MD5 over a
   buffer containing the user key, with the MD5 initial value being set to
   the IV.  These are then substituted for the standard MD5 constants to
   create the MDC function.  The buffer looks as follows:

   [ Key length: 16 bits ] [ Key: 2032 bits ]

   These 2048 bits are used for the MDC constants - the IV is the MD5
   initial value.

   The transformation is in fact somewhat more complex than this, since MD5
   uses both the 'input' and 'output' data in its transformation, and reduces
   64 bytes of input to 16 bytes of output.  It works as follows:

   in[ 64 ] + out[ 16 ] -> out'[ 16 ]

   This problem is resolved by initially setting out[ 16 ] to the IV, and
   in[ 64 ] to zeroes.  Then, once the MDC constants have been set, we set
   in[ 64 ] to auxiliary key values derived from the key.  This means that
   even chosen plaintext attacks can only resolve 16 of 80 bytes of input.
   Thus the best attack possible is a 20%-chosen plaintext attack (128 bits
   known, 512 bits unknown).

   We are transforming a 640-bit input value of which 128 bits are known, to
   a 128-bit output value via the MDC tranformation, a one-way hash function
   which is controlled by a user-supplied key */

BYTE auxKey[ MD5_BLOCKSIZE ];	/* The auxiliary key in[ 64 ] */

void initKey( BYTE *key, int keyLength, const BYTE *userIV )
	{
	BYTE keyData[ MD5_ROUNDS * sizeof( LONG ) ];
	int delayCount;

	/* Copy the key information into the key data buffer */
	memset( keyData, 0, sizeof( keyData ) );
	keyData[ 0 ] = ( BYTE ) ( keyLength >> 8 );
	keyData[ 1 ] = ( BYTE ) keyLength;
	memcpy( keyData + sizeof( WORD ), key, keyLength );
	ivCount = 0;

	/* Set up the IV intermediate.  The MD5 transformation uses both the
	   source and destination value to produce the output value (ie
	   MDC( a, b ) transforms a and b to give b'), so we simply set b, the
	   temporary intermediate, to the IV, and a, the auxKey, to all zeroes */
	memcpy( iv, userIV, IV_SIZE );
	memcpy( iv + IV_SIZE, userIV, IV_SIZE );
	memset( auxKey, 0, MD5_BLOCKSIZE );

	/* Set the Mysterious Constants to the MD5 values and "encrypt" the
	   keyData with them, then set the Mysterious Constants to these values.
	   Note that the act of encryption also sets up the IV */
	MD5SetConst( NULL );
	for( delayCount = 0; delayCount < DELAYCOUNT; delayCount++ )
		{
		encryptCFB( keyData, MD5_ROUNDS * sizeof( LONG ) );
		MD5SetConst( keyData );
		}
	ivCount = 0;

	/* Now set the auxiliary key data to the Mysterious Constants encrypted
	   with themselves */
	encryptCFB( keyData, MD5_BLOCKSIZE );
	memcpy( auxKey, keyData, MD5_BLOCKSIZE );
#ifdef LITTLE_ENDIAN
	longReverse( ( LONG * ) auxKey, MD5_BLOCKSIZE );
#endif /* LITTLE_ENDIAN */
	ivCount = 0;

	/* Finally, wipe the keyData */
	memset( keyData, 0, sizeof( keyData ) );
	}

/* Save and restore the current states of the encryption */

static BYTE savedIV[ BLOCKSIZE ];
static int savedIvCount;

void saveCryptState( void )
	{
	memcpy( savedIV, iv, BLOCKSIZE );
	savedIvCount = ivCount;
	}

void restoreCryptState( void )
	{
	memcpy( iv, savedIV, BLOCKSIZE );
	ivCount = savedIvCount;
	}

/* Get the IV value.  It doesn't matter much what it is, as long as it's
   completely different for each call */

BYTE *getIV( void )
	{
	static BOOLEAN initialised = FALSE;
	static BYTE randomIV[ BLOCKSIZE ];

	if( !initialised )
		{
		/* Seed the data with a value which is guaranteed to be different
		   each time (unless the entire program is rerun more than twice a
		   second, which is doubtful) */
		MD5SetConst( NULL );
		memset( randomIV, 0, BLOCKSIZE );
		time( ( time_t * ) randomIV );
		initialised = TRUE;
		}

	/* Shuffle the bits.  It doesn't really matter whether we use MD5 or MDC
	   constants here since all we want to do is shuffle them */
	mdcTransform( randomIV );
	return( randomIV );
	}

/****************************************************************************
*																			*
*						Cipher Feedback Mode Routines						*
*																			*
****************************************************************************/

/* Encrypt data in CFB mode */

void encryptCFB( BYTE *buffer, int noBytes )
	{
	int bytesToUse, i;

	if( ivCount )
		{
		/* Use any material left in the encrypted IV */
		bytesToUse = BLOCKSIZE - ivCount;
		if( noBytes < bytesToUse )
			bytesToUse = noBytes;
		for( i = 0; i < bytesToUse; i++ )
			buffer[ i ] ^= iv[ i + ivCount ];
		memcpy( iv + ivCount, buffer, bytesToUse );
		noBytes -= bytesToUse;
		buffer += bytesToUse;
		ivCount += bytesToUse;
		}

	while( noBytes )
		{
		ivCount = ( noBytes > BLOCKSIZE ) ? BLOCKSIZE : noBytes;

		/* Encrypt the IV */
		mdcTransform( iv );

		/* XOR the buffer contents with the encrypted IV */
		for( i = 0; i < ivCount; i++ )
			buffer[ i ] ^= iv[ i ];

		/* Shift ciphertext into IV */
		memcpy( iv, buffer, ivCount );

		/* Move on to next block of data */
		noBytes -= ivCount;
		buffer += ivCount;
		}
	}

/* Decrypt data in CFB mode */

void decryptCFB( BYTE *buffer, int noBytes )
	{
	int bytesToUse, i;
	BYTE temp[ BLOCKSIZE ];

	if( ivCount )
		{
		/* Use any material left in the encrypted IV */
		bytesToUse = BLOCKSIZE - ivCount;
		if( noBytes < bytesToUse )
			bytesToUse = noBytes;
		memcpy( temp, buffer, bytesToUse );
		for( i = 0; i < bytesToUse; i++ )
			buffer[ i ] ^= iv[ i + ivCount ];
		memcpy( iv + ivCount, temp, bytesToUse );
		noBytes -= bytesToUse;
		buffer += bytesToUse;
		ivCount += bytesToUse;
		}

	while( noBytes )
		{
		ivCount = ( noBytes > BLOCKSIZE ) ? BLOCKSIZE : noBytes;

		/* Encrypt the IV */
		mdcTransform( iv );

		/* Save ciphertext */
		memcpy( temp, buffer, ivCount );

		/* XOR the buffer contents with the encrypted IV */
		for( i = 0; i < ivCount; i++ )
			buffer[ i ] ^= iv[ i ];

		/* Shift ciphertext into IV */
		memcpy( iv, temp, ivCount );

		/* Move on to next block of data */
		noBytes -= ivCount;
		buffer += ivCount;
		}
	}
