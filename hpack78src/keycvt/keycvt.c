/* PGP 2.0 secret key conversion utility.

   Written in one long hacking session on 7 November 1992 by
	 Peter Gutmann (pgut1@cs.aukuni.ac.nz).
   Placed in the public domain like the PGP code itself.

   Known bugs: Doesn't handle schizophrenic keys too well.  These are caused
					by a bug in PGP 2.0.
			   There are several bugs in the PGP format and format docs.
					The code contains some workarounds */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mdc.h"

/* Buffering with getchar() is extremely braindamaged.  Under DOS and OS/2
   we can use getch(), otherwise use the ostrich algorithm */

#ifdef __MSDOS__
  int getch( void );

  #undef getchar
  #define getchar	getch
#endif /* __MSDOS__ */

/* Magic numbers for PGP 2.0 keyrings */

#define PGP_CTB_SECKEY		0x95	/* Secret key packet CTB */
#define PGP_CTB_TRUST		0xB0	/* Trust packet */
#define PGP_CTB_USERID		0xB4	/* Userid packet CTB */

#define PGP_VERSION_BYTE	2		/* Version number byte for PGP 2.0 */

#define PKE_ALGORITHM_RSA	1		/* RSA public-key encryption algorithm */

#define CKE_ALGORITHM_NONE	0		/* No CKE algorithm */
#define CKE_ALGORITHM_IDEA	1		/* IDEA cipher */
#define CKE_ALGORITHM_MDC	64		/* MDC cipher */

/* The size of the IDEA IV */

#define IDEA_IV_SIZE		8

/* Conversion from bitcount to bytecount */

#define bits2bytes(x)	( ( ( x ) + 7 ) >> 3 )

/* Storage for MPI's */

#define MAX_MPI_SIZE		256

BYTE n[ MAX_MPI_SIZE ], e[ MAX_MPI_SIZE ], d[ MAX_MPI_SIZE ];
BYTE p[ MAX_MPI_SIZE ], q[ MAX_MPI_SIZE ], u[ MAX_MPI_SIZE ];
int nLen, eLen, dLen, pLen, qLen, uLen;

/* Information needed by the IDEA cipher */

char password[ 80 ];			/* The user's password */
BYTE ideaKey[ 16 ];				/* IDEA cipher key */
BYTE ideaIV[ IDEA_IV_SIZE ];	/* IDEA cipher IV */

/* Prototypes for functions in IDEA.C */

void initcfb_idea( BYTE iv[ IDEA_IV_SIZE ], BYTE ideakey[ 16 ], BOOLEAN decrypt );
void ideacfb( BYTE *buffer, int count );
void close_idea( void );

/* The buffers needed for the MDC encryption */

BYTE cryptBuffer[ 2048 ], outBuffer[ 1024 ];

/* Whether we've converted any keys or not */

BOOLEAN keysConverted;

/* The crc16 table and crc16 variable itself */

WORD crc16tbl[ 256 ];
WORD crc16;

/* The block crc16 calculation routine.  Ideally this should be done in
   assembly language for speed */

void crc16buffer( BYTE *bufPtr, int length )
	{
	while( length-- )
		crc16 = crc16tbl[ ( BYTE ) crc16 ^ *bufPtr++ ] ^ ( crc16 >> 8 );
	}

/* The initialisation routine for the crc16 table */

void initCRC16( void )
	{
	int bitCount, tblIndex;
	WORD crcVal;

	for( tblIndex = 0; tblIndex < 256; tblIndex++ )
		{
		crcVal = tblIndex;
		for( bitCount = 0; bitCount < 8; bitCount++ )
			if( crcVal & 0x01 )
				crcVal = ( crcVal >> 1 ) ^ 0xA001;
			else
				crcVal >>=  1;
		crc16tbl[ tblIndex ] = crcVal;
		}
	}
/* Routines to read BYTE, WORD, LONG */

BYTE fgetByte( FILE *inFilePtr )
	{
	return( getc( inFilePtr ) );
	}

WORD fgetWord( FILE *inFilePtr )
	{
	WORD value;

	value = ( ( WORD ) getc( inFilePtr ) ) << 8;
	value |= ( WORD ) getc( inFilePtr );
	return( value );
	}

LONG fgetLong( FILE *inFilePtr )
	{
	LONG value;

	value = ( ( LONG ) getc( inFilePtr ) ) << 24;
	value |= ( ( LONG ) getc( inFilePtr ) ) << 16;
	value |= ( ( LONG ) getc( inFilePtr ) ) << 8;
	value |= ( LONG ) getc( inFilePtr );
	return( value );
	}

/* Routines to write BYTE, WORD, LONG */

void fputByte( FILE *outFilePtr, BYTE data )
	{
	putc( data, outFilePtr );
	}

void fputWord( FILE *outFilePtr, WORD data )
	{
	putc( ( BYTE ) ( data >> 8 ), outFilePtr );
	putc( ( BYTE ) data, outFilePtr );
	}

void fputLong( FILE *outFilePtr, LONG data )
	{
	putc( ( BYTE ) ( data >> 24 ), outFilePtr );
	putc( ( BYTE ) ( data >> 16 ), outFilePtr );
	putc( ( BYTE ) ( data >> 8 ), outFilePtr );
	putc( ( BYTE ) data, outFilePtr );
	}

/* Read an MPI */

void readMPI( FILE *inFilePtr, int *mpiLen, BYTE *mpReg )
	{
	BYTE *regPtr;
	int length;

	*mpiLen = fgetWord( inFilePtr );
	length = bits2bytes( *mpiLen );
	regPtr = mpReg;
	while( length-- )
		*regPtr++ = fgetByte( inFilePtr );
	}

/* Write an MPI */

void writeMPI( FILE *outFilePtr, int mpiLen, BYTE *mpReg )
	{
	int length = bits2bytes( mpiLen );

	fputWord( outFilePtr, mpiLen );
	while( length-- )
		fputByte( outFilePtr, *mpReg++ );
	}

/* Store an MPI in an in-memory buffer */

int storeMPI( BYTE *buffer, int mpiLen, BYTE *mpReg )
	{
	int length = bits2bytes( mpiLen );

	*buffer++ = ( BYTE ) ( mpiLen >> 8 );
	*buffer++ = ( BYTE ) mpiLen;
	memcpy( buffer, mpReg, length );
	return( sizeof( WORD ) + length );
	}

/* IDEA support code needed by convertPrivateKey().  Note that the IV must
   be declared static since the IDEA code sets up a pointer to it and uses
   it for storage (ick!) */

void getPassword( void )
	{
	puts( "Please enter password for this private key" );
	printf( "Password: " );
	fflush( stdout );
	gets( password );
	}

void initCrypt( void )
	{
	MD5_CTX	mdContext;
	static BYTE iv[ IDEA_IV_SIZE ];

	/* Get the password from the (l)user */
	getPassword();

	/* Reduce it to 128 bits using MD5 */
	MD5Init( &mdContext );
	MD5Update( &mdContext, ( BYTE * ) password, strlen( password ) );
	MD5Final( &mdContext );

	/* Set up IDEA key */
	memset( iv, 0, IDEA_IV_SIZE );
	initcfb_idea( iv, mdContext.digest, TRUE );
	ideacfb( ideaIV, 8 );
	}

/* Skip to the start of the next key packet block */

void skipToKeyPacket( FILE *inFilePtr, const int length )
	{
	BYTE ctb;

	/* First skip the rest of the current packet if necessary */
	if( length )
		fseek( inFilePtr, length, SEEK_CUR );

	/* Now skip any following userID packets */
	while( ( ctb = fgetByte( inFilePtr ) ) == PGP_CTB_USERID )
		fseek( inFilePtr, fgetByte( inFilePtr ), SEEK_CUR );

	/* Finally, put back the last CTB we read */
	ungetc( ctb, inFilePtr );
	}

/* Convert a private key packet from PGP 1.0 to PGP 2.0 format */

WORD checksum( BYTE *data, int length )
	{
	WORD checkSum = ( ( BYTE ) ( length >> 8 ) ) + ( ( BYTE ) length );

	length = bits2bytes( length );
	while( length-- )
		checkSum += *data++;
	return( checkSum );
	}

void convertPrivateKey( FILE *inFilePtr, FILE *outFilePtr )
	{
	char userID[ 256 ];
	BOOLEAN isEncrypted;
	LONG timeStamp;
	WORD checkSum, packetChecksum, validityPeriod;
	int length, i, packetLength;
	struct tm *localTime;
	BYTE *outBufPtr = outBuffer, *mdcIV, *ivPtr, ctb;

	/* Skip CTB, packet length, and version byte */
	if( fgetc( inFilePtr ) == EOF )
		return;
	packetLength = fgetWord( inFilePtr );
	if( fgetByte( inFilePtr ) != PGP_VERSION_BYTE )
		{
		/* Unknown version number, skip this packet */
		puts( "Skipping unknown packet type..." );
		skipToKeyPacket( inFilePtr, packetLength - sizeof( BYTE ) );
		return;
		}
	packetLength -= sizeof( BYTE );

	/* Read timestamp, validity period */
	timeStamp = fgetLong( inFilePtr );
	validityPeriod = fgetWord( inFilePtr );
	packetLength -= sizeof( LONG ) + sizeof( WORD );

	/* Read public key components */
	if( fgetByte( inFilePtr ) != PKE_ALGORITHM_RSA )
		{
		/* Unknown PKE algorithm type, skip this packet */
		puts( "Skipping unknown PKE algorithm packet..." );
		skipToKeyPacket( inFilePtr, packetLength - sizeof( BYTE ) );
		return;
		}
	readMPI( inFilePtr, &nLen, n );
	readMPI( inFilePtr, &eLen, e );
	packetLength -= sizeof( BYTE ) + sizeof( WORD ) + bits2bytes( nLen ) + \
									 sizeof( WORD ) + bits2bytes( eLen );

	/* Handle decryption info for secret components if necessary */
	if( ( ctb = fgetByte( inFilePtr ) ) == CKE_ALGORITHM_MDC )
		{
		puts( "Key has already been converted, skipping packet..." );
		skipToKeyPacket( inFilePtr, packetLength - sizeof( BYTE ) );
		return;
		}
	isEncrypted = ( ctb == CKE_ALGORITHM_IDEA );
	if( isEncrypted )
		for( i = 0; i < IDEA_IV_SIZE; i++ )
			ideaIV[ i ] = fgetc( inFilePtr );
	packetLength -= sizeof( BYTE ) + ( isEncrypted ) ? IDEA_IV_SIZE : 0;

	/* Read in private key components and checksum */
	readMPI( inFilePtr, &dLen, d );
	readMPI( inFilePtr, &pLen, p );
	readMPI( inFilePtr, &qLen, q );
	readMPI( inFilePtr, &uLen, u );
	packetLength -= sizeof( WORD ) + bits2bytes( dLen ) + \
					sizeof( WORD ) + bits2bytes( pLen ) + \
					sizeof( WORD ) + bits2bytes( qLen ) + \
					sizeof( WORD ) + bits2bytes( uLen );
	packetChecksum = fgetWord( inFilePtr );
	packetLength -= sizeof( WORD );

	/* Read the userID packet */
	if( ( ctb = fgetByte( inFilePtr ) ) != PGP_CTB_USERID )
		{
		/* Check whether we may have found a keyring trust packet (PGP bug) */
		if( ctb == PGP_CTB_TRUST )
			{
			/* Remember it for later */
			fgetByte( inFilePtr );	/* Skip length */
			fgetByte( inFilePtr );	/* Skip null trust info */
			ctb = fgetByte( inFilePtr );
			}

		/* If we still don't have a userID CTB, complain */
		if( ctb != PGP_CTB_USERID )
			{
			puts( "Can't find userID packet after key packet, skipping..." );
			skipToKeyPacket( inFilePtr,  ( int ) fgetByte( inFilePtr ) );
			return;
			}
		}
	length = fgetByte( inFilePtr );
	for( i = 0; i < length; i++ )
		userID[ i ] = fgetByte( inFilePtr );
	userID[ i ] = '\0';

	/* Check whether we may have found a keyring trust packet (another bug) */
	if( ( ctb = fgetByte( inFilePtr ) ) == PGP_CTB_TRUST )
		{
		/* Remember it for later */
		fgetByte( inFilePtr );	/* Skip length */
		fgetByte( inFilePtr );	/* Skip null trust info */
		}
	else
		ungetc( ctb, inFilePtr );

	/* Display the key and ask the user if they want to convert it */
	localTime = localtime( ( time_t * ) &timeStamp );
	printf( "%d bits, date %02d/%02d/%02d, userID %s\n", nLen, \
			localTime->tm_mday, localTime->tm_mon, localTime->tm_year, \
			userID );
	printf( "Add information for this key so HPACK can use it (y/n) " );
	if( toupper( getchar() ) == 'N' )
		{
#ifdef __MSDOS__
		putchar( '\n' );
#else
		getchar();	/* Fix getchar() problem */
#endif /* __MSDOS__ */
		puts( "Skipping key..." );
		return;
		}

#ifdef __MSDOS__
	putchar( '\n' );
#else
	getchar();	/* Try and fix some of getchar()'s problems */
#endif /* __MSDOS__ */

	/* Decrypt the secret-key fields if necessary */
	if( isEncrypted )
		for( i = 0; ; i++ )
			{
			/* Attempt to decrypt the secret-key fields */
			initCrypt();
			ideacfb( d, bits2bytes( dLen ) );
			ideacfb( p, bits2bytes( pLen ) );
			ideacfb( q, bits2bytes( qLen ) );
			ideacfb( u, bits2bytes( uLen ) );

			/* Make sure all was OK */
			checkSum = checksum( d, dLen );
			checkSum += checksum( p, pLen );
			checkSum += checksum( q, qLen );
			checkSum += checksum( u, uLen );
			if( checkSum != packetChecksum )
				{
				/* If they still haven't got it right after 3 attempts,
				   give up */
				if( i == 3 )
					{
					puts( "Can't decrypt key packet, skipping..." );
					return;
					}

				puts( "Incorrect checksum, possibly due to incorrect password" );
				}
			else
				break;
			}
	else
		/* Get the password for encryption, since we didn't need to get one
		   for decryption */
		getPassword();

	/* Write the PGP 2.0 header information */
	fputByte( outFilePtr, PGP_CTB_SECKEY );
	fputWord( outFilePtr, sizeof( BYTE ) + \
						  sizeof( LONG ) + sizeof( WORD ) + sizeof( BYTE ) + \
						  sizeof( WORD ) + bits2bytes( nLen ) + \
						  sizeof( WORD ) + bits2bytes( eLen ) + \
						  sizeof( BYTE ) + IV_SIZE + \
						  sizeof( WORD ) + bits2bytes( dLen ) + \
						  sizeof( WORD ) + bits2bytes( pLen ) + \
						  sizeof( WORD ) + bits2bytes( qLen ) + \
						  sizeof( WORD ) + bits2bytes( uLen ) + \
						  sizeof( WORD ) );
	fputByte( outFilePtr, PGP_VERSION_BYTE );

	/* Write timestamps, algorithm byte */
	fputLong( outFilePtr, timeStamp );
	fputWord( outFilePtr, validityPeriod );
	fputByte( outFilePtr, PKE_ALGORITHM_RSA );

	/* Write public key components */
	writeMPI( outFilePtr, nLen, n );
	writeMPI( outFilePtr, eLen, e );

	/* Write secret key components */
	fputByte( outFilePtr, CKE_ALGORITHM_MDC );
	mdcIV = ivPtr = getIV();
	for( i = 0; i < IV_SIZE; i++ )
		fputByte( outFilePtr, *ivPtr++ );
	outBufPtr += storeMPI( outBufPtr, dLen, d );
	outBufPtr += storeMPI( outBufPtr, pLen, p );
	outBufPtr += storeMPI( outBufPtr, qLen, q );
	outBufPtr += storeMPI( outBufPtr, uLen, u );
	length = ( int ) ( outBufPtr - outBuffer );

	/* Write secret key checksum */
	crc16 = 0;
	crc16buffer( outBuffer, length );
	outBuffer[ length++ ] = ( BYTE ) ( crc16 >> 8 );
	outBuffer[ length++ ] = ( BYTE ) crc16;

	/* Encrypt the secret key components and write them */
	initKey( ( BYTE * ) password, strlen( password ), mdcIV );
	encryptCFB( outBuffer, length );
	fwrite( outBuffer, length, 1, outFilePtr );

	/* Write the userID packet for the preceding key */
	fputByte( outFilePtr, PGP_CTB_USERID );
	length = strlen( userID );
	fputByte( outFilePtr, length );
	for( i = 0; i < length; i++ )
		fputByte( outFilePtr, userID[ i ] );

	/* Zap encryption information */
	memset( password, 0, 80 );
	close_idea();

	keysConverted = TRUE;
	}

void main( const int argc, const char *argv[] )
	{
	FILE *inFilePtr, *outFilePtr;
	char tempFileName[ 256 ];
	long fileSize;
	int ch;

	puts( "HPACK keyring format converter\n" );
	if( argc == 3 )
		{
		if( ( inFilePtr = fopen( argv[ 1 ], "rb" ) ) == NULL )
			{
			perror( argv[ 1 ] );
			exit( ERROR );
			}
		strcpy( tempFileName, argv[ 2 ] );
		strcpy( tempFileName + strlen( tempFileName ) - 1, "_" );
		if( ( outFilePtr = fopen( tempFileName, "wb" ) ) == NULL )
			{
			perror( tempFileName );
			exit( ERROR );
			}
		}
	else
		{
		puts( "Usage: keycvt <input keyfile> <output keyfile>" );
		puts( "\nThe keyfiles are the PGP secret keyrings.  The source keyring may contain" );
		puts( " one or more secret keys.  For each key you will be asked whether you" );
		puts( " wish to add an HPACK-readable version to the destination keyring." );
		puts( " Adding the HPACK-readable version will add an extra key packet which" );
		puts( " is used by HPACK.  In normal usage the input and output keyfiles are" );
		puts( " the same file." );
		exit( ERROR );
		}

	/* Set up CRC table */
	initCRC16();

	/* Find out whether we're handling a public or private keyring */
	ch = fgetc( inFilePtr );
	ungetc( ch, inFilePtr );
	if( ch != PGP_CTB_SECKEY )
		{
		printf( "%s doesn't appear to be a PGP secret key file.\n", argv[ 1 ] );
		exit( ERROR );
		}
	else
		{
		/* Convert all the keys in the keyring */
		keysConverted = FALSE;
		while( !feof( inFilePtr ) )
			{
			convertPrivateKey( inFilePtr, outFilePtr );
			putchar( '\n' );
			}
		}

	/* Check before we add the new packets to the output keyring */
	fclose( inFilePtr );
	fclose( outFilePtr );
	if( keysConverted )
		{
		printf( "Finished processing keys. Add new key information to output keyring (y/n) " );
		fflush( stdout );
		if( toupper( getchar() ) == 'N' )
			{
			/* Clean up and exit */
			unlink( tempFileName );
			exit( ERROR );
			}

		/* Append the new packets to the end of the output keyring */
		if( ( inFilePtr = fopen( tempFileName, "rb" ) ) == NULL )
			{
			puts( "Cannot open temporary work file" );
			exit( ERROR );
			}
		if( ( outFilePtr = fopen( argv[ 2 ], "ab" ) ) == NULL )
			{
			perror( argv[ 2 ] );
			exit( ERROR );
			}
		fseek( inFilePtr, 0L, SEEK_END );
		fileSize = ftell( inFilePtr );
		fseek( inFilePtr, 0L, SEEK_SET );
		while( fileSize-- )
			fputc( fgetc( inFilePtr ), outFilePtr );
		fclose( inFilePtr );
		fclose( outFilePtr );
		}
	else
		puts( "No keys converted, output file left unchanged." );

	/* Finish up */
	unlink( tempFileName );
	puts( "\nDone" );
	}
