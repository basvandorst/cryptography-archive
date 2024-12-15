/****************************************************************************
*																			*
*							HPACK Multi-System Archiver						*
*							===========================						*
*																			*
*						  MDC Message Digest Cipher Header					*
*							  MDC.C  Updated 12/08/92						*
*																			*
* This program is protected by copyright and as such any use or copying of	*
*  this code for your own purposes directly or indirectly is highly uncool	*
*  					and if you do so there will be....trubble.				*
* 				And remember: We know where your kids go to school.			*
*																			*
*			Copyright 1992  Peter C.Gutmann.  All rights reserved			*
*																			*
****************************************************************************/

#ifdef __MAC__
  #include "defs.h"
  #include "md5.h"
#else
  #include "defs.h"
  #include "crypt/md5.h"
#endif /* __MAC__ */

/* The block size (in bytes) */

#define BLOCKSIZE	16

/* The default IV value used to seed the cipher in initKey().  Changing this
   for each file precludes the use of precomputed encrypted data for very
   fast checking against known plaintext */

#define DEFAULT_IV		( ( BYTE * ) "\0\0\0\0\0\0\0\0" )

#define IV_SIZE			8

/* Define for simple block encryption */

void MD5Transform( LONG *digest, LONG *data );
void longReverse( LONG *buffer, int byteCount );

#ifdef LITTLE_ENDIAN
  #define mdcTransform(iv)	longReverse( ( LONG * ) iv, BLOCKSIZE ); \
							MD5Transform( ( LONG * ) iv, ( LONG * ) auxKey ); \
							longReverse( ( LONG * ) iv, BLOCKSIZE )
#else
  #define mdcTransform(iv)	MD5Transform( ( LONG * ) iv, ( LONG * ) auxKey )
#endif /* LITTLE_ENDIAN */

/* Prototypes for functions in MDC.C */

void initKey( BYTE *key, int keyLength, const BYTE *iv );
void encryptCFB( BYTE *buffer, int length );
void decryptCFB( BYTE *buffer, int length );
BYTE *getIV( void );
