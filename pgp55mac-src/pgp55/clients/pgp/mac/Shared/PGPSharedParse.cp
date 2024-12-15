/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#include "pgpMem.h"
#include "PGPSharedParse.h"


/*____________________________________________________________________________
	Find a string in the buffer and return a pointer to it.
	
	Note: the length of the buffer and the length of the search string are
	specified via length, *not* null termination.
___________________________________________________________________________*/
	const char *
FindMatchingBytes(
	const char *	buffer,
	const ulong		length,
	const char *	findIt,
	const ulong		findItLength
	)
{
	pgpAssertAddrValid( buffer, char );
	pgpAssertAddrValid( findIt, char );
	pgpAssert( findItLength >= 1 );
	// allow 'length' to be 0
	
	const char *	result	= nil;
	
	if ( 	length != 0 &&
			findItLength != 0 &&
			findItLength <= length )
	{
		const char *	cur;
		const char *	last;
		const char		searchChar	= findIt[ 0 ];
		
		cur		= buffer;
		last	= cur + ( length - findItLength );
		while ( cur <= last )
		{
			if ( *cur == searchChar )
			{
				if ( pgpMemoryEqual( cur, findIt, findItLength ) )
				{
					result	= cur;
					break;
				}
			}
			
			++cur;
		}
	}
	
	return( result );
}




/*____________________________________________________________________________
	Find a block of the form:
	
	linestart
	...
	lineEnd
	
	Return the offset at which it begins and its length.
	Length includes the begin/end lines and the trailing carriage return/line
	feed.
____________________________________________________________________________*/
	Boolean
FindBlock(
	const void *		inBuffer,
	const ulong			inBufferSize,
	ConstStr255Param	startLine,
	ConstStr255Param	endLine,
	ulong *				encryptedBlockStart,
	ulong *				encryptedBlockSize )
{
	Boolean			foundBlock	= false;
	const char *	cur = nil;
	const char *	beginning	= nil;
	const char *	end	= nil;
	const char		kCarriageReturnChar	= 13;
	const char		kLineFeedChar		= 10;
	const char *	inBufferLast;
	
	cur				= (const char *)inBuffer;
	inBufferLast	= ( cur + inBufferSize ) -1;

#define IsEOL( c )	( (c) == '\r' || (c) == '\n' )
	
	while ( cur <= inBufferLast )
	{ 
		beginning	= FindMatchingBytes( cur, inBufferSize,
						(const char *)&startLine[ 1 ],
						StrLength( startLine ) );
		if ( IsNull( beginning ) )
			break;
		if ( (cur == inBuffer) || IsEOL( *(cur - 1) ) )
			break;
		
		// string doesn't start at beginning of line--skip it
		cur	= beginning + StrLength( startLine );
	}
	
	if ( IsntNull( beginning ) )
	{
		ulong	bytesRemaining;
		
		*encryptedBlockStart	= beginning - (const char *)inBuffer;
		
		cur				= beginning + StrLength( startLine );
		bytesRemaining	= inBufferSize - *encryptedBlockStart;
		
		while ( cur <= inBufferLast )
		{
			end	= FindMatchingBytes( cur, bytesRemaining,
					(const char *)&endLine[ 1 ], StrLength( endLine ) );
			if ( IsNull( end ) )
				break;
				
			if ( IsEOL( *(end - 1 ) ) )
			{
				// advance past line end, which could be <CR><LF> or <LF> or
				// <CR>
				cur	= end + StrLength( endLine );
				if ( *cur == kCarriageReturnChar )
				{
					++cur;
					if ( *cur == kLineFeedChar )
						++cur;
				}
				else if ( *cur == kLineFeedChar )
				{
					++cur;
				}
				
				*encryptedBlockSize	= ( cur - (const char *)inBuffer) -
										*encryptedBlockStart;
				
				foundBlock	= true;
				break;
			}
			
			cur	= end + StrLength( endLine );
		}
	}
	
	return( foundBlock );
}

/*____________________________________________________________________________
	Find a block of the form:
	
-----BEGIN PGP SIGNED MESSAGE-----
...
-----BEGIN PGP SIGNATURE-----
...
-----END PGP SIGNATURE-----
	
	Return the offset at which it begins and its length.
	Length includes the begin/end lines and the trailing carriage return/line
	feed.
_____________________________________________________________________________*/
	Boolean
FindSignedBlock(
	const void *	inBuffer,
	const ulong		inBufferSize,
	ulong *			encryptedBlockStart,
	ulong *			encryptedBlockSize )
{
	const uchar		kPGPSignedBlockBeginString[] =
								"\p-----BEGIN PGP SIGNED MESSAGE-----";
	const uchar		kPGPSignedBlockEndString[]	=
								"\p-----END PGP SIGNATURE-----";
	
	return( FindBlock( inBuffer, inBufferSize,
			kPGPSignedBlockBeginString, kPGPSignedBlockEndString,
			encryptedBlockStart, encryptedBlockSize ) );
}


/*____________________________________________________________________________
	Find a block of the form:
	
	-----BEGIN PGP MESSAGE-----
	.....
	-----END PGP MESSAGE-----
	
	Return the offset at which it begins and its length.
	Length includes the begin/end lines and the trailing carriage return/line
	feed.
_____________________________________________________________________________*/
	Boolean
FindEncryptedBlock(
	const void *	inBuffer,
	const ulong		inBufferSize,
	ulong *			encryptedBlockStart,
	ulong *			encryptedBlockSize )
{
	const uchar		kBeginString[]	= "\p-----BEGIN PGP MESSAGE-----";
	const uchar		kEndString[]	= "\p-----END PGP MESSAGE-----";
	
	return( FindBlock( inBuffer, inBufferSize, kBeginString, kEndString,
				encryptedBlockStart, encryptedBlockSize ) );
}


/*_____________________________________________________________________________
	Find a block of the form:
	
	-----BEGIN PGP KEY-----
	.....
	-----END PGP KEY-----
	
	Return the offset at which it begins and its length.
	Length includes the begin/end lines and the trailing carriage return/line
	feed.
_____________________________________________________________________________*/
	Boolean
FindPGPKeyBlock(
	const void *	inBuffer,
	const ulong		inBufferSize,
	ulong *			encryptedBlockStart,
	ulong *			encryptedBlockSize )
{
	const uchar		kBeginString[]	= "\p-----BEGIN PGP PUBLIC KEY BLOCK-----";
	const uchar		kEndString[]	= "\p-----END PGP PUBLIC KEY BLOCK-----";
	
	return( FindBlock( inBuffer, inBufferSize, kBeginString, kEndString,
				encryptedBlockStart, encryptedBlockSize ) );
}




