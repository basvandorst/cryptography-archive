/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: pgpHex.c,v 1.2 1997/10/10 01:07:12 lloyd Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

#include <string.h>

#include "pgpHex.h"




	PGPBoolean
pgpIsValidHexChar( char	theChar )
{
	return( (theChar >= '0' && theChar <= '9') ||
		( theChar >= 'a' && theChar <= 'f' ) ||
		( theChar >= 'A' && theChar <= 'F' ) );
}



	PGPByte
pgpHexCharToNibble( char theChar )
{
	if ( theChar >= '0' && theChar <= '9' )
		return( theChar - '0' );
	if ( theChar >= 'a' && theChar <= 'f' )
		return( ( theChar - 'a' ) + 10 );
		
	return( ( theChar - 'A' ) + 10 );
}




	void
pgpBytesToHex(
	PGPByte const *	keyBytes,
	PGPSize			numBytes,
	PGPBoolean		add0x,
	char *			outString )
{
	static const char 	sHexDigits[] = "0123456789ABCDEF";
	char *				curString	= outString;
	static const char	sPrefixStr[] = "0x";
	PGPSize				keyByteIndex;
	
	if ( add0x )
	{
		strcpy( curString, sPrefixStr );
		curString	+= sizeof( sPrefixStr ) - 1;
	}
	
	for( keyByteIndex = 0; keyByteIndex < numBytes; ++keyByteIndex )
	{
		PGPByte	keyByte	= keyBytes[ keyByteIndex ];
		
		// output high nibble, then low nibble
		*curString++	= sHexDigits[ ( keyByte >> 4 ) & 0x0F ];
		*curString++	= sHexDigits[ keyByte & 0x0F ];
	}
	
	*curString++	= '\0';
}


/*____________________________________________________________________________
	Convert a stream of hex data into binary data. Since it's just a stream,
	byte order is irrelevant.
____________________________________________________________________________*/
	void
pgpHexToBytes(
	const char *	hex,
	PGPUInt32		numBinaryBytes,
	PGPByte *		buffer )
{
	PGPUInt32		counter;
	const char *	cur;
	PGPByte *		outCur;
	
	/* skip leading "0x", if present */
	if ( hex[ 0 ] == '0' && 
		(( hex[ 1 ] == 'x' ) || ( hex[ 1 ] == 'X' )) )
	{
		hex	+= 2;
	}
		
	counter	= numBinaryBytes;
	cur		= hex;
	outCur	= buffer;
	while ( counter-- != 0 )
	{
		PGPByte		theByte;
		
		if ( ! pgpIsValidHexChar( *cur ) )
			break;
		theByte	= pgpHexCharToNibble( *cur ) << 4;
		++cur;
		
		if ( ! pgpIsValidHexChar( *cur ) )
			break;
		theByte	|= pgpHexCharToNibble( *cur );
		++cur;
		
		*outCur++	= theByte;
	}
}

	PGPUInt32
pgpHexToPGPUInt32( const char *hex )
{
	PGPByte		buffer[ 4 ];
	PGPUInt32	result;
	
	pgpHexToBytes( hex, 4, buffer );
	
	result	= buffer[ 0 ];
	result	= ( result << 8 ) | buffer[ 1 ];
	result	= ( result << 8 ) | buffer[ 2 ];
	result	= ( result << 8 ) | buffer[ 3 ];
	
	return( result );
}
























/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/