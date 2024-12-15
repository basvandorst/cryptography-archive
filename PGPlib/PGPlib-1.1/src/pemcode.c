/*********************************************************************
 * Filename:      pemcode.c
 * Description:   
 * Author:        Donated to PGPlib by Peter Gutman 
 *		<pgut001@cs.auckland.ac.nz> in an e-mail received 
 *		Wed, 4 Jun 1997 14:26:47 +0200 (MET DST)
 *
 * Modified at:   Thu Aug 14 20:32:33 1997
 * Modified by:   Tage Stabell-Kulo <tage@acm.org>
 * 	
 * 	Copyright University of Tromsø (Norway)
 * 	See the file COPYING for details
 ********************************************************************/


/* Encode/decode data from binary to the printable format specified in
   RFC 1113 (this should really be called base64 encoding, the names used
   here show the age of this code) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The padding value used to pad odd output bytes */

#define PEM_PAD		'='

/* Non-PEM character values */

#define PERR		0xFF			/* Illegal char */
#define PEOF		0x7F			/* PEM-EOF (padding char) */

/* Binary <-> PEM translation tables */

static char binToAscii[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static unsigned char asciiToBin[] =
{ 
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, 0x3E, PERR, PERR, PERR, 0x3F,
    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
    0x3C, 0x3D, PERR, PERR, PERR, PEOF, PERR, PERR,
    PERR, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
    0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
    0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
    0x17, 0x18, 0x19, PERR, PERR, PERR, PERR, PERR,
    PERR, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20,
    0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30,
    0x31, 0x32, 0x33, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR
};

/* Basic single-char en/decode functions */

#define encode(data)	binToAscii[ data ]
#define decode(data)	asciiToBin[ data ]

/****************************************************************************
*									*
* PEM Encoding Routines							*
*									*
****************************************************************************/

/* Encode a block of binary data into the PEM format.  Return the total
   number of output bytes */

int
pemEncode( char *outBuffer, void *inBuffer, int count )
{
    int srcIndex = 0, destIndex = 0;
    int remainder = count % 3;
    unsigned char *inBufferPtr = inBuffer;

    while( srcIndex < count )
    {
	outBuffer[ destIndex++ ] = encode( inBufferPtr[ srcIndex ] >> 2 );
	outBuffer[ destIndex++ ] = encode( ( ( inBufferPtr[ srcIndex ] << 4 ) & 0x30 ) |
					   ( ( inBufferPtr[ srcIndex + 1 ] >> 4 ) & 0x0F ) );
	srcIndex++;
	outBuffer[ destIndex++ ] = encode( ( ( inBufferPtr[ srcIndex ] << 2 ) & 0x3C ) |
					   ( ( inBufferPtr[ srcIndex + 1 ] >> 6 ) & 0x03 ) );
	srcIndex++;
	outBuffer[ destIndex++ ] = encode( inBufferPtr[ srcIndex++ ] & 0x3F );
    }

    /* Go back and add padding if we've encoded too many characters */
    if( remainder == 2 )
	/* There were only 2 bytes in the last group */
	outBuffer[ destIndex - 1 ] = PEM_PAD;
    else
	if( remainder == 1 )
	    /* There was only 1 byte in the last group */
	    outBuffer[ destIndex - 2 ] = outBuffer[ destIndex - 1 ] = PEM_PAD;
    outBuffer[ destIndex ] = '\0';

    /* Return count of encoded bytes */
    return( destIndex );
}

/* Encode a block of binary data in base64 format and write it to a file with
 * line breaks every 72 characters.  Returns the total number of output bytes.
 * The code maintains no state, so output must be processed in chunks of 72
 *  (after encoding) bytes 
 */

void 
pemEncodeFile( FILE *outFile, void *inBuffer, long length )
{
    unsigned int index = 0;
    int remainder = 3 - ( ( int ) length % 3 ), byteCount = 0;
    unsigned char *inBufferPtr = inBuffer;

/*	fwrite( "-- BEGIN SECURED DATA --\n", 1, 25, outFile ); */
    while( index < length )
    {
	putc( encode( inBufferPtr[ index ] >> 2 ), outFile );
	putc( encode( ( ( inBufferPtr[ index ] << 4 ) & 0x30 ) |
		      ( ( inBufferPtr[ index + 1 ] >> 4 ) & 0x0F ) ),
	      outFile );
	index++;
	if( index > length - 2 )
	{
	    /* Special handling for last chars in file */
	    if( remainder == 2 )
		putc( PEM_PAD, outFile );
	    else
	    {
		putc( encode( ( ( inBufferPtr[ index ] << 2 ) & 0x3C ) |
			      ( ( inBufferPtr[ index + 1 ] >> 6 ) & 0x03 ) ),
		      outFile );
		index++;
	    }
	    if( remainder >= 1 )
		putc( PEM_PAD, outFile );
	    else
		putc( encode( inBufferPtr[ index++ ] & 0x3F ), outFile );
	}
	else
	{
	    putc( encode( ( ( inBufferPtr[ index ] << 2 ) & 0x3C ) |
			  ( ( inBufferPtr[ index + 1 ] >> 6 ) & 0x03 ) ),
		  outFile );
	    index++;
	    putc( encode( inBufferPtr[ index++ ] & 0x3F ), outFile );
	}

	/* Output a NL if necessary */
	byteCount += 4;
	if( byteCount >= 72 )
	{
	    byteCount = 0;
	    putc( '\n', outFile );
	}
    }
    if( byteCount )
	putc( '\n', outFile );
/*	fwrite( "-- END SECURED DATA --\n", 1, 23, outFile ); */
}

/****************************************************************************
*									*
*PEM Decoding Routines							*
*									*
****************************************************************************/

/* Decode a block of binary data from the PEM format.  Return the total
   number of decoded bytes */

int 
pemDecode( void *outBuffer, char *inBuffer, int count )
{
    int srcIndex = 0, destIndex = 0;
    int remainder = 0;
    unsigned char c0, c1, c2, c3;
    unsigned char *outBufferPtr = outBuffer;

    /* Find out how many bytes we have to decode and how much padding there
       is */
    if( count && inBuffer[ count - 1 ] == PEM_PAD )
	remainder++;
    if( ( count - 1 ) && ( inBuffer[ count - 2 ] == PEM_PAD ) )
	remainder++;

    while( srcIndex < count )
    {
	/* Decode a block of data from the input buffer */
	c0 = decode( inBuffer[ srcIndex++ ] );
	c1 = decode( inBuffer[ srcIndex++ ] );
	c2 = decode( inBuffer[ srcIndex++ ] );
	c3 = decode( inBuffer[ srcIndex++ ] );
	if( ( c0 | c1 | c2 | c3 ) == PERR )
	    return( 0 );

	/* Copy the decoded data to the output buffer */
	outBufferPtr[ destIndex++ ] = ( c0 << 2 ) | ( c1 >> 4 );
	outBufferPtr[ destIndex++ ] = ( c1 << 4 ) | ( c2 >> 2);
	outBufferPtr[ destIndex++ ] = ( c2 << 6 ) | ( c3 );
    }

    /* Return count of decoded bytes */
    return( destIndex - remainder );
}

/* Decode a block of binary data from the base64 format up to a given maximum
 * number of decoded bytes, with line breaks every 72 characters.  Returns
 * the total number of decoded bytes.  The code maintains no state, so input
 * must be processed in chunks of 72 bytes 
 */

long 
pemDecodeFile( void *outBuffer, FILE *inFile, long count )
{
    unsigned int index = 0;
    unsigned char c0, c1, c2, c3;
    unsigned char *outBufferPtr = outBuffer;
    int byteCount = 0;
#if 0
    char buffer[ 100 ];

    /* Find the start of the secured data */
    while( !feof( inFile ) )
    {
	fgets( buffer, 100, inFile );
	if( !(strncmp( buffer, "-- BEGIN SECURED DATA --", 24 ) ) )
	    break;
    }
    if( feof( inFile ) )
	return( 0 );
#endif /* 0 */

    while( !feof( inFile ) && count > 0 )
    {
	/* Get the first char and see if we've hit the end of the message */
	c0 = getc( inFile );
	if( c0 == '-' )
	    break;

	/* Decode a block of data from the input file */
	c0 = decode( c0 );
	c1 = decode( getc( inFile ) );
	c2 = decode( getc( inFile ) );
	c3 = decode( getc( inFile ) );
	if( ( c0 | c1 | c2 | c3 ) == PERR && !feof( inFile ) )
	    return( 0 );
	count -= 3;

	/* Copy the decoded data to the output buffer */
	outBufferPtr[ index++ ] = ( c0 << 2 ) | ( c1 >> 4 );
	if( c2 != PEOF )
	    outBufferPtr[ index++ ] = ( c1 << 4 ) | ( c2 >> 2);
	if( c3 != PEOF )
	    outBufferPtr[ index++ ] = ( c2 << 6 ) | ( c3 );

	byteCount += 4;
	if( byteCount >= 72 || c2 == PEOF || c3 == PEOF )
	{
	    int ch;

	    /* Skip the newline, with special-case handling of CRLF vs CR vs
	       LF */
	    fgetc( inFile );
	    if( ( ch = fgetc( inFile ) ) != '\n' )
		ungetc( ch, inFile );
	    byteCount = 0;
	}
    }
#if 0
    if( feof( inFile ) )
	return( 0 );

    /* Check for the end-of-data marker */
    fgets( buffer, 100, inFile );
    if( ( strncmp( buffer, "- END SECURED DATA --", 21 ) ) )
	return( 0 );
#endif /* 0 */

    /* Return count of decoded bytes */
    return( index );
}

