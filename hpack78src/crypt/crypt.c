/****************************************************************************
*																			*
*							HPACK Multi-System Archiver						*
*							===========================						*
*																			*
*						   Encryption Interface Routines					*
*							 CRYPT.C  Updated 11/08/92						*
*																			*
* This program is protected by copyright and as such any use or copying of	*
*  this code for your own purposes directly or indirectly is highly uncool	*
*  					and if you do so there will be....trubble.				*
* 				And remember: We know where your kids go to school.			*
*																			*
*		Copyright 1990 - 1992  Peter C.Gutmann.  All rights reserved		*
*																			*
****************************************************************************/

/* "Modern cryptography is nothing more than a mathematical framework for
	debating the implications of various paranoid delusions".
												- Don Alvarez */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef __MAC__
  #include "defs.h"
  #include "choice.h"
  #include "error.h"
  #include "filesys.h"
  #include "flags.h"
  #include "frontend.h"
  #include "hpacklib.h"
  #include "hpaktext.h"
  #include "system.h"
  #include "crc16.h"
  #include "crypt.h"
  #include "md5.h"
  #include "mdc.h"
  #include "packet.h"
  #include "rsa.h"
  #include "fastio.h"
  #include "hpackio.h"
#else
  #include "defs.h"
  #include "choice.h"
  #include "error.h"
  #include "filesys.h"
  #include "flags.h"
  #include "frontend.h"
  #include "hpacklib.h"
  #include "system.h"
  #include "crc/crc16.h"
  #include "crypt/crypt.h"
  #include "crypt/md5.h"
  #include "crypt/mdc.h"
  #include "crypt/packet.h"
  #include "crypt/rsa.h"
  #include "io/fastio.h"
  #include "io/hpackio.h"
  #include "language/hpaktext.h"
#endif /* __MAC__ */

/* The environment variable where various key files hide */

#if defined( __MSDOS__ ) && ( ARCHIVE_TYPE != 1 )
  #define PGPPATH				NULL
  #define PEMPATH				( NULL + 1 )
#else
  #define PGPPATH				"PGPPATH"	/* Env.var for PGP path */
  #define PEMPATH				"PEMPATH"	/* Env.var for PEM path */
#endif /* __MSDOS__ && ARCHIVE_TYPE != 1 */

/* These files use the environmental variable PGPPATH as a default path */

#ifdef __ARC__
char PGP_PUBKEY_FILENAME[] = "pubring";
char PGP_SECKEY_FILENAME[] = "secring";
#else
char PGP_PUBKEY_FILENAME[] = "pubring.pgp";
char PGP_SECKEY_FILENAME[] = "secring.pgp";
#endif /* __ARC__ */
#define PGP_PUBKEY_FILELENGTH	( sizeof( PGP_PUBKEY_FILENAME ) - 1 )
#define PGP_SECKEY_FILELENGTH	( sizeof( PGP_SECKEY_FILENAME ) - 1 )

/* These files use the environmental variable PEMPATH as a default path */

#ifdef __ARC__
char PEM_PUBKEY_FILENAME[] = "???";
char PEM_SECKEY_FILENAME[] = "???";
#else
char PEM_PUBKEY_FILENAME[] = "???.???";
char PEM_SECKEY_FILENAME[] = "???.???";
#endif /* __ARC__ */
#define PEM_PUBKEY_FILELENGTH	( sizeof( PEM_PUBKEY_FILENAME ) - 1 )
#define PEM_SECKEY_FILELENGTH	( sizeof( PEM_SECKEY_FILENAME ) - 1 )

/* These files can use either the PGP or PEM path as the default path */

#ifdef __ARC__
char RANDSEED_FILENAME[] = "hpakseed";
#else
char RANDSEED_FILENAME[] = "hpakseed.bin";
#endif /* __ARC__ */
#define RANDSEED_FILELENGTH		( sizeof( RANDSEED_FILENAME ) - 1 )

/* Symbolic defines to indicate whether we should set globalPrecision when
   calling readMPI() or not */

#define SET_PRECISION		TRUE
#define NO_SET_PRECISION	FALSE

/* The encryption password lengths.  A minimum length of 8 bytes provides a
   reasonable level of security */

#define MIN_KEYLENGTH	8
#define MAX_KEYLENGTH	80		/* Any reasonable value will do */

/* The encryption passwords */

char key[ MAX_KEYLENGTH + 1 ], secKey[ MAX_KEYLENGTH + 1 ];
int keyLength = 0, secKeyLength = 0;

/* The number of password attempts the user is allowed to decrypt their
   secret key */

#define PASSWORD_RETRIES	3

/* The information structure for public and private keys.  Both PEM/PKCS and
   PGP key packets contain more information than this but HPACK doesn't use
   any of it */

typedef struct {
			   /* Key status */
			   BOOLEAN isPemKey;			/* Whether it's a PGP or PEM key */
			   BOOLEAN isEncrypted;			/* Whether secret fields are encrypted */

			   /* General key ID information */
			   char userID[ 256 ];			/* userID associated with the key */
			   BYTE keyID[ KEYFRAG_SIZE ]; 	/* keyID for the key */
			   char ascKeyID[ ( KEYDISPLAY_SIZE * 2 ) + 1 ];
											/* ASCII hex keyID */
			   LONG timeStamp;				/* Timestamp for this key */

			   /* The key itself */
			   MP_REG n[ MAX_UNIT_PRECISION ], e[ MAX_UNIT_PRECISION ];
											/* Public key components */
			   MP_REG d[ MAX_UNIT_PRECISION ], p[ MAX_UNIT_PRECISION ], \
					  q[ MAX_UNIT_PRECISION ], u[ MAX_UNIT_PRECISION ];
											/* Secret key components */
			   } KEYINFO;

/* The following is defined in FRONTEND.C */

extern char choice;

/* The following are defined in MDC.C */

extern BYTE auxKey[];

/* Prototypes for functions in VIEWFILE.C */

void extractDate( const LONG time, int *date1, int *date2, int *date3, \
								   int *hours, int *minutes, int *seconds );

/* Prototypes for functions in ARCHIVE.C */

void blankLine( int length );
void blankChars( int length );

/* Prototypes for functions in GUI.C */

char *getPassword( char *passWord, const char *prompt );
void showSig( const BOOLEAN isGoodSig, const char *userID, const LONG timeStamp );

#ifdef __MAC__
/* Prototypes for functions in MAC.C */

FD openResourceFork( const char *filePath, const int mode );
void closeResourceFork( const FD theFD );
#endif /* __MAC__ */

/* The random key and IV, seedFile FD, MD5 context, and position in seed
   buffer used for public-key encryption */

BYTE mdcKey[ BLOCKSIZE ];
BYTE seedIV[ BLOCKSIZE ];	/* Needed for non-interference with main IV */
FD seedFileFD = ERROR;		/* No seedfile to begin with */
MD5_CTX mdSeedContext;
int seedPos = BLOCKSIZE;	/* Always reseed generator */

/* The general-purpose crypto buffer */

BYTE *cryptBuffer = NULL;

#if CRYPT_BUFSIZE < ( MAX_BYTE_PRECISION * 33 )
  #error Need to increase CRYPT_BUFSIZE
#endif /* Safety check for size of crypto buffer */

/* The keyInfo structures for the encryption and digital signature keys (made
   global to save on stack space, especially with very long keys) */

KEYINFO cryptKeyInfo, signKeyInfo;

/****************************************************************************
*																			*
* 						Initialise the Encryption System 					*
*																			*
****************************************************************************/

#ifndef GUI

char *getPassword( char *passWord, const char *prompt )
	{
	int i;
	char ch;

	while( TRUE )
		{
		hprintf( prompt );

		/* Get a password string */
		i = 0;
		while( ( ch = hgetch() ) != '\r' && ch != '\n' )
			{
			/* Handle backspace key.  The way this is handled is somewhat
			   messy, since hitting BS at the last char position deletes
			   the previous char */
			if( ch == '\b' )
				{
				if( i )
					i--;
				else
					hputchar( '\007' );
				continue;
				}

			if( i == MAX_KEYLENGTH )
				hputchar( '\007' );
			else
				passWord[ i++ ] = ch;
			}

		/* Exit if format is valid */
		if( i >= MIN_KEYLENGTH )
			break;

		hputs( MESG_KEY_INCORRECT_LENGTH );
		}

	passWord[ i ] = '\0';
	blankLine( screenWidth - 1 );

	return( passWord );
	}
#endif /* !GUI */

/* Set up the encryption password */

void initPassword( const BOOLEAN isMainKey )
	{
	char key1[ MAX_KEYLENGTH + 1 ], key2[ MAX_KEYLENGTH + 1 ];
	BOOLEAN exitNow = FALSE;

	/* Set up the password */
	getPassword( key1, ( isMainKey ) ? MESG_ENTER_PASSWORD : MESG_ENTER_SEC_PASSWORD );
	if( choice == ADD || choice == FRESHEN || choice == REPLACE || choice == UPDATE )
		exitNow = strcmp( key1, getPassword( key2, MESG_REENTER_TO_CONFIRM ) );

	/* Set up the appropriate encryption key */
	if( isMainKey )
		{
		strcpy( key, key1 );
		keyLength = strlen( key );
		}
	else
		{
		strcpy( secKey, key1 );
		secKeyLength = strlen( key );
		}

	/* Scrub temporary passwords */
	memset( key1, 0, MAX_KEYLENGTH + 1 );
	memset( key2, 0, MAX_KEYLENGTH + 1 );

	if( exitNow )
		error( PASSWORDS_NOT_SAME );
	}

/* Clear out the ID fields of a public key */

static void clearIDfields( KEYINFO *keyInfo )
	{
	memset( keyInfo->userID, 0, sizeof( keyInfo->userID ) );
	memset( keyInfo->keyID, 0, sizeof( keyInfo->keyID ) );
	memset( keyInfo->ascKeyID, 0, sizeof( keyInfo->ascKeyID ) );
	}

/* Initialise the encryption system */

void initCrypt( void )
	{
	/* Allocate general-purpose crypt buffer */
	if( ( cryptBuffer = hmalloc( CRYPT_BUFSIZE ) ) == NULL )
		error( OUT_OF_MEMORY );

	/* Clear out ID fields in PKC keys */
	clearIDfields( &cryptKeyInfo );
	clearIDfields( &signKeyInfo );
	}

/* Shut down the encryption system */

void endCrypt( void )
	{
	/* Clear the MDC Mysterious Constants if necessary by resetting them to
	   the normal MD5 values */
	MD5SetConst( NULL );

	/* Write the random MDC seed to the seedFile if necessary */
	if( seedFileFD != ERROR )
		{
		/* Step the MD5 RNG to hide the previous key, then write it to the
		   seed file */
		MD5Update( &mdSeedContext, mdcKey, BLOCKSIZE );
		MD5Final( &mdSeedContext );
		hlseek( seedFileFD, 0L, SEEK_SET );
		hwrite( seedFileFD, mdSeedContext.digest, BLOCKSIZE );
		hclose( seedFileFD );
		}

	/* Scrub the encryption keys so other users can't find them by examining
	   core after HPACK has run */
	memset( key, 0, MAX_KEYLENGTH + 1 );
	memset( secKey, 0, MAX_KEYLENGTH + 1 );
	memset( mdcKey, 0, BLOCKSIZE );
	memset( auxKey, 0, MD5_BLOCKSIZE );
	memset( &cryptKeyInfo, 0, sizeof( KEYINFO ) );
	memset( &signKeyInfo, 0, sizeof( KEYINFO ) );

	/* Scrub the cryptBuffer if we've allocated it */
	if( cryptBuffer != NULL )
		{
		memset( cryptBuffer, 0, CRYPT_BUFSIZE );
		hfree( cryptBuffer );
		}
	}

/* Delete a file (if we're using encryption, "annihilate" would be a better
   term) */

void wipeFile( const char *filePath )
	{
	FILEINFO fileInfo;
	long fileSize, count;
	char renameFilePath[ MAX_PATH ];
	FD wipeFileFD;
#ifdef __MAC__
	FD resourceForkFD;
#endif /* __MAC__ */

	/* Perform a simple delete if we're not encrypting the data */
	if( !( flags & CRYPT ) )
		{
		hunlink( filePath );
		return;
		}

	/* If we're encrypting the data, we need to comprehensively destroy the
	   file.  We do this by getting its size, overwriting it with a pattern
	   of ones and zeroes, breaking it up into sector-size chunks (presumably
	   scattered all over the disk), resetting its attributes and timestamp
	   to zero, renaming it to 'X', and finally deleting it.  The most anyone
	   can ever recover from this is that there was once some file on the disk
	   which HPACK has now deleted */
	findFirst( filePath, ALLFILES_DIRS, &fileInfo );
	findEnd( &fileInfo );
	fileSize = fileInfo.fSize;
	hchmod( filePath, CREAT_ATTR );		/* Make sure we can get at the file */
	if( ( wipeFileFD = hopen( filePath, O_WRONLY ) ) != IO_ERROR )
		{
		/* Overwrite the data with a pattern of ones and zeroes */
		memset( _outBuffer, 0xAA, _BUFSIZE );
		for( count = 0; count < fileSize; count++ )
			{
			hwrite( wipeFileFD, _outBuffer, ( count + _BUFSIZE < fileSize ) ? \
					_BUFSIZE : ( int ) ( fileSize - count ) );
			count += _BUFSIZE;
			}

		/* Truncate the file every 512 bytes to break up the linked chain of
		   sectors */
		for( count = fileSize & ~0x1FFL; count >= 0; count -= 512 )
			{
			hlseek( wipeFileFD, count, SEEK_SET );
			htruncate( wipeFileFD );
			}
#ifdef __MAC__
		/* Wipe the resource fork as well */
		if( ( resourceForkFD = openResourceFork( filePath, O_WRONLY ) ) != IO_ERROR )
			{
			for( count = 0; count < fileSize; count++ )
				{
				hwrite( resourceForkFD, _outBuffer, ( count + _BUFSIZE < fileSize ) ? \
						_BUFSIZE : fileSize - count );
				count += _BUFSIZE;
				}
			for( count = fileSize; count >= 0; count -= 512 )
				{
				hlseek( resourceForkFD, count, SEEK_SET );
				htruncate( resourceForkFD );
				}
			closeResourceFork( resourceForkFD );
			}
#endif /* __MAC__ */
		hclose( wipeFileFD );
		}

	/* Rename the file, reset it's timestamp to zero, and delete it.  If a
	   file called 'X' already exists we just do it to the original file.
	   Note that the setting of the date can have interesting effects if the
	   epoch predates the epoch of the host filesystem */
	strcpy( renameFilePath, filePath );
	strcpy( findFilenameStart( renameFilePath ), "X" );
	if( hrename( filePath, renameFilePath ) != IO_ERROR )
		{
		setFileTime( renameFilePath, 0L );
		hunlink( renameFilePath );
		}
	else
		{
		setFileTime( filePath, 0L );
		hunlink( filePath );
		}
	}

/****************************************************************************
*																			*
*						General Encryption I/O Routines						*
*																			*
****************************************************************************/

/* Endianness-reverse an MPI */

static void byteReverse( BYTE *regPtr, int count )
	{
	int sourceCount = 0;
	BYTE temp;

	/* Swap endianness of MPI */
	for( count--; count > sourceCount; count--, sourceCount++ )
		{
		temp = regPtr[ sourceCount ];
		regPtr[ sourceCount ] = regPtr[ count ];
		regPtr[ count ] = temp;
		}
	}

#ifdef BIG_ENDIAN

/* Endianness-reverse the words in an MPI */

static void wordReverse( BYTE *regPtr, int count )
	{
	BYTE temp;
	int i;

	for( i = 0; i < count; i += sizeof( WORD ) )
		{
		temp = regPtr[ i ];
		regPtr[ i ] = regPtr[ i + 1 ];
		regPtr[ i + 1 ] = temp;
		}
	}
#endif /* BIG_ENDIAN */

/* Get the length of a packet.  If it's an HPACK packet we have to make sure
   we calculate the checksum on the header bytes, if it's a PGP packet we
   need to look out for longer packet types */

static int getPacketLength( const BYTE ctb )
	{
	/* If reading a PGP packet we don't calculate a checksum, and need to
	   take longer packet types into account */
	if( ctbLength( ctb ) == CTB_LEN_BYTE )
		return( ( int ) fgetByte() );
	else
		if( ctbLength( ctb ) == CTB_LEN_WORD )
			return( ( int ) fgetWord() );
		else
			return( ( int ) fgetLong() );
	}

static int getCheckPacketLength( const BYTE ctb )
	{
	BYTE buffer[ sizeof( BYTE ) + sizeof( BYTE ) ];

	/* If reading an HPACK packet we need to calculate a checksum on the
	   packet header as we read it */
	crc16 = 0;
	buffer[ 0 ] = ctb;
	buffer[ 1 ] = fgetByte();
	crc16buffer( buffer, sizeof( BYTE ) + sizeof( BYTE ) );
	return( ( int ) buffer[ 1 ] );
	}

/* Read a MPI in the form of a byte array preceded by a 16-bit bitcount from
   a file or memory into a multiprecision register */

static int readMPI( MP_REG *mpReg, BOOLEAN doSetPrecision )
	{
	int count, byteCount;
	BYTE *regPtr = ( BYTE * ) mpReg;
	WORD bitCount;

	/* Read in the MPI itself from the file.  First, read in the bit count
	   and set the global precision accordingly */
	mp_init( mpReg, 0 );
	bitCount = fgetWord();
	if( ( count = byteCount = bits2bytes( bitCount ) ) > MAX_BYTE_PRECISION )
		/* Impossibly long MPI value */
		return( ERROR );
	if( doSetPrecision )
		/* Set the precision to that specified by the number read */
		setPrecision( bits2units( bitCount + SLOP_BITS ) );

#ifdef _BIG_ENDIAN
	/* Read in the value */
	while( count-- )
		*regPtr++ = fgetByte();
#else
	/* Read in the value with endianness reversal */
	regPtr += count - 1;
	while( count-- )
		*regPtr-- = fgetByte();
#endif /* BIG_ENDIAN */

	/* Return number of bytes processed */
	return( byteCount + sizeof( WORD ) );
	}

static int getMPI( BYTE *buffer, MP_REG *mpReg )
	{
	int count, byteCount;
	BYTE *regPtr = ( BYTE * ) mpReg;

	/* Get the MPI from an in-memory buffer.  We don't need to bother with
	   setting the precision since this has already been done */
	mp_init( mpReg, 0 );
	if( ( count = byteCount = bits2bytes( mgetWord( buffer ) ) ) > MAX_BYTE_PRECISION )
		/* Impossibly long MPI value */
		return( ERROR );
	buffer += sizeof( WORD );

	/* Read in the value with endianness reversal */
	regPtr += count - 1;
	while( count-- )
		*regPtr-- = *buffer++;

	/* Return number of bytes processed */
	return( byteCount + sizeof( WORD ) );
	}

/* Decrypt the encrypted fields of a previously-read key packet */

BYTE keyBuffer[ ( 4 * MAX_BYTE_PRECISION ) + sizeof( WORD ) ];
BYTE keyIV[ IV_SIZE ];
int keyBufSize;

static BOOLEAN decryptKeyPacket( KEYINFO *keyInfo )
	{
	BYTE decryptBuffer[ ( 4 * MAX_BYTE_PRECISION ) + sizeof( WORD ) ];
	char keyPassword[ MAX_KEYLENGTH + 1 ];
	WORD savedCRC16 = crc16;
	int i, j, k, l;

	/* Try and decrypt the key information */
	for( i = 0; i < PASSWORD_RETRIES; i++ )
		{
		getPassword( keyPassword, MESG_ENTER_SECKEY_PASSWORD );
		initKey( ( BYTE * ) keyPassword, strlen( keyPassword ), keyIV );

		/* Read in key and try to decrypt it */
		memcpy( decryptBuffer, keyBuffer, keyBufSize );
		decryptCFB( decryptBuffer, keyBufSize );
		crc16 = 0;
		crc16buffer( decryptBuffer, keyBufSize - sizeof( WORD ) );
		if( crc16 != mgetWord( decryptBuffer + keyBufSize - sizeof( WORD ) ) )
#ifdef GUI
			alert( ALERT_PASSWORD_INCORRECT, NULL );
#else
			hputs( MESG_PASSWORD_INCORRECT );
#endif /* GUI */
		else
			/* Key was successfully decrypted, exit */
			break;
		}

	/* Restore callers checksum information */
	crc16 = savedCRC16;

	/* If we couldn't get the password, invaldiate key and give up */
	if( i == PASSWORD_RETRIES )
		return( FALSE );

	/* Get information from buffer */
	i = getMPI( decryptBuffer, keyInfo->d );
	j = getMPI( decryptBuffer + i, keyInfo->p );
	k = getMPI( decryptBuffer + i + j, keyInfo->q );
	l = getMPI( decryptBuffer + i + j + k, keyInfo->u );

	/* Check for possible problems in reading MPI's */
	if( ( i | j | k | l ) == ERROR )
		error( BAD_KEYFILE );

#ifdef BIG_ENDIAN
	/* Perform an endianness-reversal if necessary */
	wordReverse( ( BYTE * ) keyInfo->d, globalPrecision * sizeof( MP_REG ) );
	wordReverse( ( BYTE * ) keyInfo->p, globalPrecision * sizeof( MP_REG ) );
	wordReverse( ( BYTE * ) keyInfo->q, globalPrecision * sizeof( MP_REG ) );
	wordReverse( ( BYTE * ) keyInfo->u, globalPrecision * sizeof( MP_REG ) );
#endif /* BIG_ENDIAN */

	keyInfo->isEncrypted = FALSE;	/* Key is now decrypted */
	return( TRUE );
	}

/* Read in a key packet from a file.  It will return the ctb, timestamp,
   userid, keyID, public key components n and e, and private key components
   d, p, q and u if necessary.  The strategy we use is to keep parsing data
   until we find a userID, then return.   Since userID's always follow keys,
   and since there can be multiple keys attached to each userID, each call
   yields a new userID (but not necessarily a new key) */

#define IS_PUBLICKEY	FALSE	/* Whether we should try to read secret fields */
#define IS_SECRETKEY	TRUE

static BOOLEAN readKeyPacket( BOOLEAN isSecretKey, KEYINFO *keyInfo )
	{
	int ctb;
	WORD certLength;
	int i, j, k, l;
	BOOLEAN needMoreInfo = TRUE, gotKeyPacket = TRUE;
	BYTE ch;

	/* Keep reading packets until we have a key packet and userID packet we
	   can use */
	while( needMoreInfo )
		{
		/*  Get key certificate CTB and length info */
		if( ( ctb = fgetByte() ) == FEOF )
			break;
		certLength = getPacketLength( ctb );

		/* If it's a userID packet, read the ID */
		switch( ctb )
			{
			case CTB_USERID:
				/* Get userID as Pascal string (I am NAN!) */
				for( i = 0; i < certLength; i++ )
					keyInfo->userID[ i ] = fgetByte();
				keyInfo->userID[ i ] = '\0';
				certLength = 0;

				/* Work out whether we can exit now or not */
				if( gotKeyPacket )
					needMoreInfo = FALSE;
				break;

			case CTB_CERT_PUBKEY:
			case CTB_CERT_SECKEY:
				/* Get version byte, return if it was created by a version
				   we don't know how to handle */
				gotKeyPacket = FALSE;		/* We haven't decoded the packet yet */
				certLength -= sizeof( BYTE );
				if( fgetByte() != PGP_VERSION )
					break;

				/* Get timestamp, validity period, and PKE algorithm ID */
				certLength -= sizeof( LONG ) + sizeof( WORD ) + sizeof( BYTE );
				keyInfo->timeStamp = fgetLong();
				fgetWord();
				if( fgetByte() != PKE_ALGORITHM_RSA )
					break;

				/* We're past certificate headers, get initial key material */
				i = readMPI( keyInfo->n, SET_PRECISION );
				j = readMPI( keyInfo->e, NO_SET_PRECISION );
				certLength -= i + j;

				/* Copy the keyID from the low bits of n, convert it to
				   ASCII hex, and perform an endianness reversal if necessary */
				memcpy( keyInfo->keyID, keyInfo->n, KEYFRAG_SIZE );
				for( k = KEYDISPLAY_SIZE - 1, l = 0; k >= 0; k-- )
					{
					ch = ( keyInfo->keyID[ k ] >> 4 ) + '0';
					keyInfo->ascKeyID[ l++ ] = ( ch <= '9' ) ? ch : ch + 'A' - '9' - 1;
					ch = ( keyInfo->keyID[ k ] & 0x0F ) + '0';
					keyInfo->ascKeyID[ l++ ] = ( ch <= '9' ) ? ch : ch + 'A' - '9' - 1;
					}
				keyInfo->ascKeyID[ l ] = '\0';
#ifdef BIG_ENDIAN
				wordReverse( ( BYTE * ) keyInfo->n, globalPrecision * sizeof( MP_REG ) );
				wordReverse( ( BYTE * ) keyInfo->e, globalPrecision * sizeof( MP_REG ) );
#endif /* BIG_ENDIAN */

				/* Check for possible problems in reading MPI's */
				if( ( i | j ) == ERROR )
					error( BAD_KEYFILE );

				/* Read the secret key fields if necessary */
				keyInfo->isEncrypted = FALSE;
				if( isSecretKey )
					{
					/* Check if we can process the following MPI fields */
					certLength -= sizeof( BYTE );
					if( ( ctb = fgetByte() ) != CKE_ALGORITHM_NONE )
						{
						/* If we can't decrypt the remainder of the
						   information, skip it */
						if( ctb != CKE_ALGORITHM_MDC )
							break;

						/* Get decryption information for the key, and the
						   encrypted key itself */
						for( i = 0; i < IV_SIZE; i++ )
							keyIV[ i ] = fgetByte();
						certLength -= IV_SIZE;
						for( i = 0; i < certLength; i++ )
							keyBuffer[ i ] = fgetByte();
						keyBufSize = certLength;

						certLength = 0;
						keyInfo->isEncrypted = TRUE;
						}

					/* Read key fields if they're non-encrypted */
					if( !keyInfo->isEncrypted )
						{
						/* Get information from file */
						i = readMPI( keyInfo->d, NO_SET_PRECISION );
						j = readMPI( keyInfo->p, NO_SET_PRECISION );
						k = readMPI( keyInfo->q, NO_SET_PRECISION );
						l = readMPI( keyInfo->u, NO_SET_PRECISION );

						/* Check for possible problems in reading MPI's */
						if( ( i | j | k | l ) == ERROR )
							error( BAD_KEYFILE );

						fgetWord();		/* Skip checksum */
						certLength -= i + j + k + l + sizeof( WORD );
#ifdef BIG_ENDIAN
						/* Perform an endianness-reversal if necessary */
						wordReverse( ( BYTE * ) keyInfo->d, globalPrecision * sizeof( MP_REG ) );
						wordReverse( ( BYTE * ) keyInfo->p, globalPrecision * sizeof( MP_REG ) );
						wordReverse( ( BYTE * ) keyInfo->q, globalPrecision * sizeof( MP_REG ) );
						wordReverse( ( BYTE * ) keyInfo->u, globalPrecision * sizeof( MP_REG ) );
#endif /* BIG_ENDIAN */
						}
					}

				/* Now we've got a valid key packet */
				gotKeyPacket = TRUE;
			}

		/* Skip rest of this key certificate */
		if( certLength )
			skipSeek( ( LONG ) certLength );
		}

	/* Return TRUE if we've got a packet, FALSE otherwise */
	return( ctb != FEOF );
	}

/* Get the public or private key corresponding to a given keyID/userID from
   the public/private key file */

#define IS_KEYID	TRUE
#define IS_USERID	FALSE

#define IS_PEM_KEY	TRUE
#define IS_PGP_KEY	FALSE

static BOOLEAN getKey( const BOOLEAN isPemKey, const BOOLEAN isSecretKey, \
					   const BOOLEAN isKeyID, char *keyFileName, \
					   BYTE *wantedID, KEYINFO *keyInfo )
	{
	FD keyFileFD, savedInFD = getInputFD();
	BOOLEAN matched, firstTime = TRUE;
	char *matchID, firstChar;
	int userIDlength, i;

	/* Open key file */
	if( ( keyFileFD = hopen( keyFileName, O_RDONLY | S_DENYWR ) ) == IO_ERROR )
		return( FALSE );
	setInputFD( keyFileFD );
	resetFastIn();

	/* Set up key type information */
	keyInfo->isPemKey = isPemKey;

	/* Set up userID info if necessary */
	if( !isKeyID )
		{
		/* Check for a hexadecimal userID, signifying the user wants to
		   match by hex ASCII keyID and not by userID */
		if( !strnicmp( ( char * ) wantedID, "0x", 2 ) )
			{
			wantedID += 2;
			matchID = keyInfo->ascKeyID;
			}
		else
			matchID = keyInfo->userID;

		userIDlength = strlen( ( char * ) wantedID );
		firstChar = toupper( *wantedID );
		}

	/* Grovel through the file looking for the key */
	while( TRUE )
		{
		/* Try and read a key packet, unless its the first time through
		   the loop in which case the information may already be in memory */
		if( !firstTime && !readKeyPacket( isSecretKey, keyInfo ) )
			break;
		firstTime = FALSE;

		matched = FALSE;
		if( isKeyID )
			{
			/* wantedID contains key fragment, check it against n from keyfile */
			if( !memcmp( wantedID, keyInfo->keyID, KEYFRAG_SIZE ) )
				matched = TRUE;
			}
		else
			/* wantedID contains partial userID, check it against userID from keyfile */
			for( i = 0; matchID[ i ]; i++ )
				if( ( toupper( matchID[ i ] ) == firstChar ) && \
					!strnicmp( ( char * ) wantedID, matchID + i, userIDlength ) )
					matched = TRUE;

		/* Found key matching ID */
		if( matched )
			{
			/* If it's a secret key, try and decrypt it */
			if( keyInfo->isEncrypted && !decryptKeyPacket( keyInfo ) )
				{
				/* Couldn't decrypt, invalidate key ID fields so we don't
				   match the key the next time around, and continue */
				clearIDfields( keyInfo );
				continue;
				}

			/* Got valid key, clean up and exit */
			hclose( keyFileFD );
			setInputFD( savedInFD );
			return( TRUE );
			}
		}

    /* Couldn't find key */
	hclose( keyFileFD );
	setInputFD( savedInFD );
	return( FALSE );
	}

/* Build the path to a keyring file */

static BOOLEAN buildKeyringPath( char *keyFileName, const char *keyringName, \
								 const int keyringNameLength )
	{
	char *pgpPath;
	BOOLEAN hasPath = FALSE;
	int i;

	*keyFileName = '\0';
	if( ( pgpPath = getenv( PGPPATH ) ) != NULL )
		{
		if( strlen( pgpPath ) + keyringNameLength > MAX_PATH )
			error( PATH_ss_TOO_LONG, pgpPath, keyringName );
		strcpy( keyFileName, pgpPath );
		if( ( i = keyFileName[ strlen( keyFileName ) - 1 ] ) != '/' && i != '\\' )
			/* Add trailing slash if necessary */
			strcat( keyFileName, "/" );
		hasPath = TRUE;
		}
	strcat( keyFileName, keyringName );

	return( hasPath );
	}

/* Generate a cryptographically strong random MDC key */

static BYTE *getStrongRandomKey( void )
	{
	char keyFileName[ MAX_PATH ];
	BOOLEAN hasPath;

	/* Open the MDC seed file if we need to */
	if( seedFileFD == ERROR )
		{
		/* Build path to seed file and read in the key */
		hasPath = buildKeyringPath( keyFileName, RANDSEED_FILENAME, RANDSEED_FILELENGTH );
		if( ( ( seedFileFD = hopen( keyFileName, O_RDWR | S_DENYRDWR ) ) == ERROR && \
			  !( hasPath && ( seedFileFD = hopen( RANDSEED_FILENAME, O_RDWR | S_DENYRDWR ) ) != ERROR ) ) || \
			( hread( seedFileFD, mdcKey, BLOCKSIZE ) < BLOCKSIZE ) )
			{
			seedFileFD = ERROR;
			error( CANNOT_READ_RANDOM_SEEDFILE );
			}
		}

	/* Step the MD5 RNG and init the MDC system with the random key */
	MD5Init( &mdSeedContext );
	MD5Update( &mdSeedContext, mdcKey, BLOCKSIZE );
	MD5Final( &mdSeedContext );
	memcpy( mdcKey, mdSeedContext.digest, BLOCKSIZE );
	initKey( mdcKey, BLOCKSIZE, DEFAULT_IV );

	return( mdcKey );
	}

/* Return cryptographically strong random BYTE, WORD, LONG.  This code
   assumes initKey() has already been called, which is always the case
   in HPACK */

BYTE getStrongRandomByte( void )
	{
	BYTE *auxKey = cryptBuffer;		/* Used in mdcTransform */

	/* Step the MDC RNG if necessary */
	if( seedPos == BLOCKSIZE )
		{
		/* Transform the seedIV, with the cryptBuffer as the auxKey.  It
		   doesn't really matter what's in it */
		mdcTransform( seedIV );
		seedPos = 0;
		}

	return( mdcKey[ seedPos++ ] );
	}

WORD getStrongRandomWord( void )
	{
	/* Note that although both sides of the '|' have side-effects, we're not
	   particularly worried about the order of evaluation */
	return( ( ( WORD ) getStrongRandomByte() << 8 ) | getStrongRandomByte() );
	}

LONG getStrongRandomLong( void )
	{
	return( ( ( LONG ) getStrongRandomWord() << 16 ) | getStrongRandomWord() );
	}

/* Get the first/next userID in a list of userID's.  Since PGP doesn't
   currently implement mailing lists, this is a fairly simple-minded process
   of pulling subsequent userID's from a comma-seperated list.  In future
   this will be expanded to handle full mailing lists as soon as some
   mechanism is defined in PGP */

char *userIDlistPos, userIDvalue[ 255 ];

BOOLEAN getNextUserID( char **userID )
	{
	char *userIDlistPtr = userIDlistPos;
	int length;

	/* Extract the next userID from the list.  When there is a proper
	   mailing list mechanism defined this will simply involve returning
	   a pointer to the next name in a linked list */
	while( *userIDlistPtr && *userIDlistPtr != ',' )
		userIDlistPtr++;
	length = ( int ) ( userIDlistPtr - userIDlistPos );
	strncpy( userIDvalue, userIDlistPos, length );
	userIDvalue[ length ] = '\0';
	*userID = userIDvalue;
	if( *userIDlistPtr == ',' )
		userIDlistPtr++;	/* Skip comma separator */
	userIDlistPos = userIDlistPtr;

	return( ( *userIDlistPos ) ? TRUE : FALSE );
	}

BOOLEAN getFirstUserID( char **userID, const BOOLEAN isMainKey )
	{
	userIDlistPos = ( isMainKey ) ? mainUserID : secUserID;
	return( getNextUserID( userID ) );
	}

/****************************************************************************
*																			*
*						Data Authentication Routines						*
*																			*
****************************************************************************/

/* Calculate the MD5 message digest for a section of a file */

static void md5file( const long startPos, long noBytes, MD5_CTX *mdContext )
	{
	int bytesToProcess;

	/* Calculate the message digest for the data */
	vlseek( startPos, SEEK_SET );
	MD5Init( mdContext );
	while( noBytes )
		{
		bytesToProcess = ( noBytes < _BUFSIZE ) ? \
						 ( int ) noBytes : _BUFSIZE;
		vread( _inBuffer, bytesToProcess );
		noBytes -= bytesToProcess;

		/* Calculate the MD5 checksum for the buffer contents */
		MD5Update( mdContext, _inBuffer, bytesToProcess );
		}
	MD5Final( mdContext );
	}

/* Convert plaintext message into an integer less than the modulus n by
   making it 1 byte shorter than the normalized modulus, as per PKCS #1,
   Section 8.  Since padding is easier for little-endian MPI's, we initially
   reverse the MPI to make it little-endian and then perform the padding */

#define BLOCKTYPE_PUBLIC	TRUE
#define BLOCKTYPE_PRIVATE	FALSE

static void preblock( BYTE *regPtr, int byteCount, MP_REG *modulus, BOOLEAN padOpPublic )
	{
	int bytePrecision, leadingZeroes, blockSize, padSize;

	/* Reverse the initial MPI */
	byteReverse( regPtr, byteCount );

	/* Calculate no.leading zeroes and blocksize (incl.data plus pad bytes) */
	bytePrecision = units2bytes( globalPrecision );
	leadingZeroes = bytePrecision - countbytes( modulus ) + 1;
	blockSize = bytePrecision - leadingZeroes;

	/* Calculate number of padding bytes (-2 is for leading 0, trailing 1) */
	padSize = blockSize - byteCount - 2;

	/* Perform padding as per PKCS #1 and RFC 1115 to generate the octet
	   string EB = 00 BT PS 00 D */
	regPtr[ byteCount++ ] = 0;
	while( padSize-- )
		regPtr[ byteCount++ ] = ( padOpPublic ) ? getStrongRandomByte() : 0xFF;
	regPtr[ byteCount++ ] = padOpPublic ? BT_PUBLIC : BT_PRIVATE;

	/* Add leading zeroes */
	while( leadingZeroes-- )
		regPtr[ byteCount++ ] = 0;

#ifdef BIG_ENDIAN
	/* Swap endianness of padded MPI */
	wordReverse( regPtr, byteCount );
#endif /* BIG_ENDIAN */
	}

/* Reverse the above blocking operation */

static void postunblock( BYTE *regPtr, int count )
	{
#ifdef BIG_ENDIAN
	/* First reverse the individual words */
	wordReverse( regPtr, count );
#endif /* BIG_ENDIAN */

	/* Bytereverse the data */
	byteReverse( regPtr, count );
	}

/* Check a block of data's signature */

BOOLEAN checkSignature( LONG startPos, LONG noBytes )
	{
	char keyFileName[ MAX_PATH ];
	BYTE decryptedSignature[ MAX_BYTE_PRECISION ];
	BYTE *mdInfo = decryptedSignature + sizeof( BYTE ) + sizeof( BYTE );
	LONG timeStamp;
	MP_REG signature[ MAX_UNIT_PRECISION ];	/* MP_REG's are now long-aligned */
	BYTE wantedID[ KEYFRAG_SIZE ];
	MD5_CTX mdContext;
	int i;
	BOOLEAN hasPath, badSig = FALSE;
#ifndef GUI
	int time1, time2, time3, hours, minutes, seconds;
#endif /* !GUI */

	/* Build path to keyring file */
	hasPath = buildKeyringPath( keyFileName, PGP_PUBKEY_FILENAME, PGP_PUBKEY_FILELENGTH );

	/* Set up checksumming for rest of packet */
	checksumSetInput( ( long ) getCheckPacketLength( fgetByte() ), NO_RESET_CHECKSUM );

	/* Make sure we know how to check this signature */
	if( fgetByte() != SIG_ALGORITHM_RSA )
		{
		/* Don't know how to handle this signature scheme, complain and
		   process file anyway */
#ifdef GUI
		alert( ALERT_CANT_FIND_PUBLIC_KEY, NULL );
#else
		hputs( WARN_CANT_FIND_PUBLIC_KEY );
#endif /* GUI */
		return( TRUE );
		}

	/* Copy rest of key fragment */
	for( i = 0; i < KEYFRAG_SIZE; i++ )
		wantedID[ i ] = fgetByte();

	/* Get signed message digest, first, setting precision to max.value to be
	   on the safe side */
	setPrecision( MAX_UNIT_PRECISION );
	i = readMPI( signature, NO_SET_PRECISION );
	if( i == ERROR || fgetWord() != crc16 )
		{
		/* Authentication information bad or corrupted, complain and process
		   file anyway */
#ifdef GUI
		alert( ALERT_SEC_INFO_CORRUPTED, NULL );
#else
		hputs( WARN_SEC_INFO_CORRUPTED );
#endif /* GUI */
		return( FALSE );
		}

	/* Perform a context switch from the general input buffer */
	saveInputState();

	/* Use wantedID prefix to get and validate public key from a key file.
	   Check for the keyFile both on the key paths (if they exist) and in the
	   current directory */
	if( !( getKey( IS_PGP_KEY, IS_PUBLICKEY, IS_KEYID, keyFileName, wantedID, &signKeyInfo ) || \
		 ( hasPath && getKey( IS_PGP_KEY, IS_PUBLICKEY, IS_KEYID, PGP_PUBKEY_FILENAME, wantedID, &signKeyInfo ) ) ) )
		{
		/* Can't get public key, complain and process file anyway */
#ifdef GUI
		alert( ALERT_CANT_FIND_PUBLIC_KEY, NULL );
#else
		hputs( WARN_CANT_FIND_PUBLIC_KEY );
#endif /* GUI */
		restoreInputState();	/* Switch back to the general input buffer */
		return( TRUE );
		}

	/* Recover message digest via public key */
#ifdef BIG_ENDIAN
	wordReverse( ( BYTE * ) signature, globalPrecision * sizeof( MP_REG ) );
#endif /* BIG_ENDIAN */
	mp_modexp( ( MP_REG * ) decryptedSignature, signature, signKeyInfo.e, signKeyInfo.n );
	postunblock( decryptedSignature, MD_PACKET_LENGTH );

	/* Look at nested stuff within RSA block and make sure it's the correct
	   message digest algorithm */
	if( ctbType( decryptedSignature[ 0 ] ) != CTBTYPE_MD || \
				 decryptedSignature[ 1 ] != MD_INFO_LENGTH || \
				 decryptedSignature[ 2 ] != MD_ALGORITHM_MD5 )
		{
		badSig = TRUE;		/* Bad RSA decrypt */
		timeStamp = 0L;		/* Can't establish timestamp for data */
		}
	else
		{
		/* Extract date and compute message digest for rest of file */
		timeStamp = mgetLong( mdInfo + MD_TIME_OFS );
		setInputFD( archiveFD );
		md5file( startPos, noBytes, &mdContext );
		}

	/* Switch back to the general input buffer */
	restoreInputState();

	/* Check that everything matches */
	if( badSig || memcmp( mdContext.digest, mdInfo + MD_DIGEST_OFS, 16 ) )
		{
		badSig = TRUE;
#ifdef GUI
		showSig( FALSE, signKeyInfo.userID, timeStamp );
#else
		hprintf( MESG_BAD_SIGNATURE );
#endif /* GUI */
		}
	else
#ifdef GUI
		showSig( TRUE, signKeyInfo.userID, timeStamp );
#else
		hprintf( MESG_GOOD_SIGNATURE );
	extractDate( timeStamp, &time1, &time2, &time3, &hours, &minutes, &seconds );
	if( timeStamp )
		hprintf( MESG_SIGNATURE_FROM_s_DATE_dddddd, \
				 signKeyInfo.userID, time1, time2, time3, hours, minutes, seconds );
	else
		hprintf( MESG_SIGNATURE_FROM_s_DATE_dddddd, signKeyInfo.userID, 0, 0, 0, 0, 0, 0 );
#endif /* GUI */

	return( !badSig );		/* Return status of signature */
	}

/* Create a signature for a block of data */

int createSignature( LONG startPos, LONG noBytes, char *wantedID )
	{
	char keyFileName[ MAX_PATH ];
	LONG timeStamp;
	MD5_CTX MD;
	MP_REG buffer2[ MAX_UNIT_PRECISION ];
	BYTE buffer1[ MAX_BYTE_PRECISION ], *bufPtr = buffer1 + sizeof( BYTE ) + sizeof( BYTE );
	int bitCount, byteCount, i;
	BOOLEAN hasPath;

	/* Build path to keyring file */
	hasPath = buildKeyringPath( keyFileName, PGP_SECKEY_FILENAME, PGP_SECKEY_FILELENGTH );

	/* Get the secret key, setting precision to max value to be on the safe
	   side.  Check for the keyFile both on the key paths (if they exist) and
	   in the current directory */
	setPrecision( MAX_UNIT_PRECISION );
	if( !( getKey( IS_PGP_KEY, IS_SECRETKEY, IS_USERID, keyFileName, ( BYTE * ) wantedID, &signKeyInfo ) || \
		 ( hasPath && getKey( IS_PGP_KEY, IS_SECRETKEY, IS_USERID, PGP_SECKEY_FILENAME, ( BYTE * ) wantedID, &signKeyInfo ) ) ) )
		error( CANNOT_FIND_SECRET_KEY_FOR_s, wantedID );

	/* Build the packet header */
	buffer1[ 0 ] = CTB_MD;
	buffer1[ 1 ] = MD_INFO_LENGTH;
	buffer1[ 2 ] = MD_ALGORITHM_MD5;

	/* Generate the message digest for the data and add it to the packet */
	setInputFD( archiveFD );
	md5file( startPos, noBytes, &MD );
	memcpy( bufPtr + MD_DIGEST_OFS, MD.digest, 16 );

	/* Add the timestamp */
	time( ( time_t * ) &timeStamp );
	mputLong( bufPtr + MD_TIME_OFS, timeStamp );

	/* Pre-block mdPacket */
	preblock( buffer1, MD_INFO_LENGTH + 2, signKeyInfo.n, BLOCKTYPE_PRIVATE );

	/* Perform the RSA signature calculation */
#ifndef GUI
	hprintf( MESG_WAIT );
	hflush( stdout );
#endif /* !GUI */
	rsaDecrypt( buffer2, ( MP_REG * ) buffer1, \
				signKeyInfo.d, signKeyInfo.p, signKeyInfo.q, signKeyInfo.u );
#ifndef GUI
	blankChars( strlen( MESG_WAIT ) );
#endif /* !GUI */
#ifdef BIG_ENDIAN
	wordReverse( ( BYTE * ) buffer2, globalPrecision * sizeof( MP_REG ) );
#endif /* BIG_ENDIAN */

	/* Calculate the size of the MPI needed to store the signature */
	bitCount = countbits( buffer2 );
	byteCount = bits2bytes( bitCount );

	/* Create the full signature packet */
	checksumBegin( RESET_CHECKSUM );
	fputByte( CTB_SKE );			/* Write header */
	fputByte( sizeof( BYTE ) + KEYFRAG_SIZE + ( 2 + ( BYTE ) byteCount ) );
	fputByte( SIG_ALGORITHM_RSA );
	for( i = 0; i < KEYFRAG_SIZE; i++ )	/* Write key ID */
		fputByte( signKeyInfo.keyID[ i ] );
	fputWord( bitCount );			/* Write signature as MPI */
	bufPtr = ( BYTE * ) buffer2 + byteCount - 1;
	for( i = 0; i < byteCount; i++ )
		fputByte( *bufPtr-- );
	checksumEnd();
	fputWord( crc16 );				/* Write packet checksum */

	return( sizeof( BYTE ) + sizeof( BYTE ) + sizeof( BYTE ) + \
			KEYFRAG_SIZE + ( 2 + byteCount ) + sizeof( WORD ) );
	}

/****************************************************************************
*																			*
*						Public-Key Encryption Routines						*
*																			*
****************************************************************************/

static int pkeEncrypt( BYTE *pkePacket, const char *wantedID, BYTE *keyInfo )
	{
	char keyFileName[ MAX_PATH ];
	MP_REG buffer2[ MAX_UNIT_PRECISION ];
	BOOLEAN hasPath;
	BYTE buffer1[ MAX_BYTE_PRECISION ], *buf2ptr;
	int count, byteCount;

	/* Build path to keyring file */
	hasPath = buildKeyringPath( keyFileName, PGP_PUBKEY_FILENAME, PGP_PUBKEY_FILELENGTH );

	/* Use userID to get and validate public key from a key file.  Check for
	   the keyFile both on the key paths (if they exist) and in the current
	   directory */
	setPrecision( MAX_UNIT_PRECISION );
	if( !( getKey( IS_PGP_KEY, IS_PUBLICKEY, IS_USERID, keyFileName, ( BYTE * ) wantedID, &cryptKeyInfo ) || \
		 ( hasPath && getKey( IS_PGP_KEY, IS_PUBLICKEY, IS_USERID, PGP_PUBKEY_FILENAME, ( BYTE * ) wantedID, &cryptKeyInfo ) ) ) )
		error( CANNOT_FIND_PUBLIC_KEY_FOR_s, wantedID );

	/* Copy the algorithm ID and keyID to the PKE packet */
	*pkePacket++ = PKE_ALGORITHM_RSA;
	memcpy( pkePacket, cryptKeyInfo.keyID, KEYFRAG_SIZE );
	pkePacket += KEYFRAG_SIZE;

	/* Assemble a CKE info packet and pre-block it */
	buffer1[ 0 ] = CTB_CKEINFO;
	buffer1[ 1 ] = sizeof( BYTE ) + BLOCKSIZE;
	buffer1[ 2 ] = CKE_ALGORITHM_MDC;
	memcpy( buffer1 + 3, keyInfo, BLOCKSIZE );
	preblock( buffer1, sizeof( BYTE ) + sizeof( BYTE ) + sizeof( BYTE ) + BLOCKSIZE, \
			  cryptKeyInfo.n, BLOCKTYPE_PUBLIC );

	/* Now encrypt the crap out of it */
	mp_modexp( buffer2, ( MP_REG * ) buffer1, cryptKeyInfo.e, cryptKeyInfo.n );
#ifdef BIG_ENDIAN
	wordReverse( ( BYTE * ) buffer2, globalPrecision * sizeof( MP_REG ) );
#endif /* BIG_ENDIAN */

	/* Calculate the size of the MPI needed to store the data */
	count = countbits( buffer2 );
	byteCount = bits2bytes( count );

	/* Write MPI to PKE packet with endianness reversal */
	mputWord( pkePacket, count );
	pkePacket += sizeof( WORD );
	buf2ptr = ( BYTE * ) buffer2 + ( byteCount - 1 );
	count = byteCount;
	while( count-- )
		*pkePacket++ = *buf2ptr--;

	return( sizeof( BYTE ) + KEYFRAG_SIZE + sizeof( WORD ) + byteCount );
	}

static BOOLEAN pkeDecrypt( MP_REG *pkePacket, BYTE *wantedID, BYTE *decryptedMPI )
	{
	char keyFileName[ MAX_PATH ];
	BOOLEAN hasPath, foundKey = TRUE;

	/* Build path to keyring file */
	hasPath = buildKeyringPath( keyFileName, PGP_SECKEY_FILENAME, PGP_SECKEY_FILELENGTH );

	/* Perform a context switch from the general input buffer */
	saveInputState();

	/* Get the secret key (the precision has already been set when the
	   pkePacket was read).  Check for the keyFile both on the key paths (if
	   they exist) and in the current directory */
	if( !( getKey( IS_PGP_KEY, IS_SECRETKEY, IS_KEYID, keyFileName, wantedID, &cryptKeyInfo ) || \
		 ( hasPath && getKey( IS_PGP_KEY, IS_SECRETKEY, IS_KEYID, PGP_SECKEY_FILENAME, wantedID, &cryptKeyInfo ) ) ) )
		foundKey = FALSE;

	/* Switch back to the general input buffer */
	restoreInputState();

	/* Leave now if we couldn't find a matching key (there may be one in a
	   following PKE packet) */
	if( !foundKey )
		return( FALSE );

	/* Recover CKE packet */
#ifndef GUI
	hprintf( MESG_WAIT );
	hflush( stdout );
#endif /* !GUI */
#ifdef BIG_ENDIAN
	wordReverse( ( BYTE * ) pkePacket, globalPrecision * sizeof( MP_REG ) );
#endif /* BIG_ENDIAN */
	rsaDecrypt( ( MP_REG * ) decryptedMPI, pkePacket, \
				cryptKeyInfo.d, cryptKeyInfo.p, cryptKeyInfo.q, cryptKeyInfo.u );
	postunblock( decryptedMPI, PKE_PACKET_LENGTH );
#ifndef GUI
	blankChars( strlen( MESG_WAIT ) );
#endif /* !GUI */
	return( TRUE );
	}

/****************************************************************************
*																			*
*				Encryption/Security Packet Management Routines				*
*																			*
****************************************************************************/

/* Parse an encryption information packet */

BOOLEAN getEncryptionInfo( int *length )
	{
	BYTE decryptedMPI[ MAX_BYTE_PRECISION ], keyID[ KEYFRAG_SIZE ], ctb;
	MP_REG pkePacket[ MAX_UNIT_PRECISION ];
	BOOLEAN packetOK = TRUE, morePackets = TRUE, foundKey = FALSE;
	int dataLength, totalLength = 0, i;
	BYTE iv[ IV_SIZE ];

	while( packetOK && morePackets )
		{
		/* Pull apart the security info to see what we've got */
		if( !isCTB( ctb = fgetByte() ) )
			{
			packetOK = FALSE;
			break;
			}
		if( !hasMore( ctb ) )
			morePackets = FALSE;
		dataLength = getCheckPacketLength( ctb );
		checksumSetInput( ( long ) dataLength, NO_RESET_CHECKSUM );
		totalLength += sizeof( BYTE ) + sizeof( BYTE );

		/* Process the encryption information packet unless we've already
		   found the key we were looking for */
		if( !foundKey )
			switch( ctbType( ctb ) )
				{
				case CTBTYPE_PKE:
					/* Public-key encryption packet.  First, skip the length
					   field and make sure it's a known PKE algorithm */
					totalLength += sizeof( BYTE );
					dataLength -= sizeof( BYTE );
					if( fgetByte() != PKE_ALGORITHM_RSA )
						{
						packetOK = FALSE;
						break;
						}

					/* If we've guessed at the archive being block encrypted with
					   a CKE, change this to a PKE */
					if( cryptFlags & CRYPT_CKE_ALL )
						cryptFlags ^= CRYPT_CKE_ALL | CRYPT_PKE_ALL;

					/* Read in key ID fragment and ckePacket, setting the
					   precision to the max.precision to be on the safe side */
					for( i = 0; i < KEYFRAG_SIZE; i++ )
						keyID[ i ] = fgetByte();
					setPrecision( MAX_UNIT_PRECISION );
					i += readMPI( pkePacket, SET_PRECISION );
					totalLength += i;	/* Adjust for KEYFRAG and MPI size */
					dataLength -= i;

					/* Try and decrypt the MPI containing the CKE info packet */
					if( !pkeDecrypt( pkePacket, keyID, decryptedMPI ) )
						break;
					foundKey = TRUE;	/* We've found a usable key */

					/* Now process the conventional-key information packet.
					   First make sure it's the right packet, skip its length
					   info, and make sure it's a known CKE algorithm */
					if( decryptedMPI[ 0 ] != CTB_CKEINFO || \
						decryptedMPI[ 2 ] != CKE_ALGORITHM_MDC )
						{
						packetOK = FALSE;
						break;
						}

					/* Finally, we have the CKE keying information and can
					   perform the necessary setup operations with it */
					initKey( decryptedMPI + 3, BLOCKSIZE, DEFAULT_IV );
					break;

				case CTBTYPE_CKE:
					/* Conventional-key encryption packet.  Make sure it's a
					   known algorithm, and rekey the encryption system if
					   necessary */
					foundKey = TRUE;	/* We've found a usable key */
					totalLength += sizeof( BYTE ) + IV_SIZE;
					dataLength -= sizeof( BYTE ) + IV_SIZE;
					if( ( ctb = fgetByte() ) == CKE_ALGORITHM_MDC_R )
						{
						/* If we don't have a secondary password yet, get it now */
						if( !secKeyLength )
							initPassword( SECONDARY_KEY );

						for( i = 0; i < IV_SIZE; i++ )
							iv[ i ] = fgetByte();
						initKey( ( BYTE * ) secKey, secKeyLength, iv );
						}
					else
						if( ctb == CKE_ALGORITHM_MDC )
							{
							/* If we don't have a password yet, get it now */
							if( !keyLength )
								initPassword( MAIN_KEY );

							for( i = 0; i < IV_SIZE; i++ )
								iv[ i ] = fgetByte();
							initKey( ( BYTE * ) key, keyLength, iv );
							}
						else
							packetOK = FALSE;
					break;

				default:
					/* Unknown encryption packet */
					packetOK = FALSE;
				}

		/* Read any unread bytes still in the packet, or skip the packet
		   entirely if we've already found the key we were looking for */
		if( dataLength )
			{
			totalLength += dataLength;
			while( dataLength-- )
				fgetByte();
			}

		/* Make sure the packet wasn't corrupted */
		totalLength += sizeof( WORD );
		if( fgetWord() != crc16 )
			{
			packetOK = FALSE;
			totalLength = 0;	/* Length may be corrupted - need to perform error recovery */
			}
		}

	/* If we haven't been able to find a matching key for a PKE packet, complain */
	if( !foundKey && packetOK )
		error( CANNOT_FIND_SECRET_KEY );

	*length = totalLength;
	return( packetOK );
	}

/* Write an encryption information packet to a file */

int putEncryptionInfo( const BOOLEAN isMainKey )
	{
	int cryptInfoLength, packetLength, i;
	BYTE pkePacket[ sizeof( BYTE ) + KEYFRAG_SIZE + sizeof( WORD ) + MAX_BYTE_PRECISION ];
	BYTE iv[ IV_SIZE ], *randomKey;
	BOOLEAN moreKeys, keyPresent;
	char *userID;

	if( cryptFlags & ( CRYPT_CKE | CRYPT_CKE_ALL ) )
		{
		checksumBegin( RESET_CHECKSUM );

		/* Write a conventional-key encryption information packet:
		   CTB, length, algorithm ID, IV */
		memcpy( iv, getIV(), IV_SIZE );
		fputByte( CTB_CKE );
		fputByte( sizeof( BYTE ) + IV_SIZE );
		fputByte( ( isMainKey ) ? CKE_ALGORITHM_MDC : CKE_ALGORITHM_MDC_R );
		for( i = 0; i < IV_SIZE; i++ )
			fputByte( iv[ i ] );
		checksumEnd();
		fputWord( crc16 );
		cryptInfoLength = sizeof( BYTE ) + sizeof( BYTE ) + \
						  sizeof( BYTE ) + IV_SIZE + sizeof( WORD );
		initKey( ( isMainKey ) ? ( BYTE * ) key : ( BYTE * ) secKey, keyLength, iv );
		}
	else
		{
		cryptInfoLength = 0;
		moreKeys = getFirstUserID( &userID, isMainKey );
		keyPresent = TRUE;
		randomKey = getStrongRandomKey();
		do
			{
			checksumBegin( RESET_CHECKSUM );

			/* Write a public-key encryption information packet: CTB, length,
			   algorithm ID, keyID, encrypted CKE info packet */
			if( moreKeys )
				fputByte( CTB_PKE | CTB_MORE );
			else
				{
				fputByte( CTB_PKE );
				keyPresent = FALSE;
				}
			packetLength = pkeEncrypt( pkePacket, userID, randomKey );
			fputByte( ( BYTE ) packetLength );
			for( i = 0; i < packetLength; i++ )
				fputByte( pkePacket[ i ] );
			checksumEnd();
			fputWord( crc16 );
			cryptInfoLength += sizeof( BYTE ) + sizeof( BYTE ) + \
							   packetLength + sizeof( WORD );

			/* Look for another keyID */
			moreKeys = getNextUserID( &userID );
			}
		while( keyPresent );
		}

	return( cryptInfoLength );
	}
