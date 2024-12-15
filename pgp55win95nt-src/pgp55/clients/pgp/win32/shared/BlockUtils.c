/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: BlockUtils.c,v 1.5 1997/09/24 00:27:03 dgal Exp $
____________________________________________________________________________*/

// System Headers
#include <windows.h>
#include <assert.h>


// CDK Headers
#include "pgpMem.h"

// Project Headers
#include "BlockUtils.h"

	const char *
FindMatchingBytes(
	const char *	buffer,
	ulong			length,
	const char *	findIt,
	ulong			findItLength
	)
{
	const char *	result	= NULL;
	
	assert( length >= 1 ); // allow 'length' to be 0
	
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




/*______________________________________________________________
	Find a block of the form:
	
	linestart
	...
	lineEnd
	
	Return the offset at which it begins and its length.
	Length includes the begin/end lines and the trailing 
	carriage return/line feed.
______________________________________________________________*/
	Boolean
FindBlock(
	const void *		inBuffer,
	ulong				inBufferSize,
	const char*			startLine,
	const char*			endLine,
	ulong *				encryptedBlockStart,
	ulong *				encryptedBlockSize )
{
	Boolean			foundBlock	= FALSE;
	const char *	cur;
	const char *	beginning	= NULL;
	const char *	end	= NULL;
	const char		kCarriageReturnChar	= 13;
	const char		kLineFeedChar		= 10;
	const char *	inBufferLast;
	int				nReplyNum = 0;
	
	cur	= (const char *)inBuffer;
	inBufferLast	= ( cur + inBufferSize ) -1;

#define IsEOL( c )	( (c) == '\r' || (c) == '\n' )
	
	while ( cur <= inBufferLast )
	{
		beginning = FindMatchingBytes(	cur, 
										inBufferSize, 
										(const char *)startLine, 
										strlen( startLine ) );

		nReplyNum = 0;

		if (!beginning )
			break;

		while ((beginning != inBuffer) && !IsEOL( *(beginning - 1) ) )
		{
			beginning--;
			nReplyNum++;
		}

/*
		if (cur == inBuffer) || IsEOL( *(cur - 1) ) )
*/
			break;

/*
		// string doesn't start at beginning of line--skip it
		cur	= beginning + strlen( startLine );
*/
	}

	if ( beginning ) 
	{
		ulong	bytesRemaining;

		*encryptedBlockStart = beginning - (const char *)inBuffer;

		cur	= beginning + strlen(startLine ) + nReplyNum;

		bytesRemaining	= inBufferSize - *encryptedBlockStart;

		while ( cur <= inBufferLast )
		{
			end	= FindMatchingBytes(	cur, 
										bytesRemaining,
										(const char *)endLine,
										strlen( endLine ) );
			if ( !end ) 
				break;

			nReplyNum = 0;
			while (!IsEOL( *(end - 1) ) )
			{
				end--;
				nReplyNum++;
			}
/*
			if ( IsEOL( *(end - 1 ) ) )
			{
*/
				// advance past line end, 
				// which could be <CR><LF> or <LF> or <CR>
				cur	= end + strlen( endLine ) + nReplyNum;

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

				foundBlock	= TRUE;
				break;
/*
			}

			cur	= end + strlen( endLine );
*/
		}
	}
	
	return( foundBlock );
}

/*______________________________________________________________
	Find a block of the form:
	
-----BEGIN PGP SIGNED MESSAGE-----
...
-----BEGIN PGP SIGNATURE-----
...
-----END PGP SIGNATURE-----
	
	Return the offset at which it begins and its length.
	Length includes the begin/end lines and the trailing 
	carriage return/line feed.
______________________________________________________________*/
	Boolean
FindSignedBlock(
	const void *	inBuffer,
	ulong			inBufferSize,
	ulong *			encryptedBlockStart,
	ulong *			encryptedBlockSize )
{
	const char		kBeginString[]	= "-----BEGIN PGP SIGNED MESSAGE-----";
	const char		kEndString[]	= "-----END PGP SIGNATURE-----";
	
	return( FindBlock(	inBuffer, 
						inBufferSize, 
						kBeginString, 
						kEndString, 
						encryptedBlockStart, 
						encryptedBlockSize ) );
}


/*______________________________________________________________
	Find a block of the form:
	
	-----BEGIN PGP MESSAGE-----
	.....
	-----END PGP MESSAGE-----
	
	Return the offset at which it begins and its length.
	Length includes the begin/end lines and the trailing 
	carriage return/line feed.
______________________________________________________________*/
	Boolean
FindEncryptedBlock(
	const void *	inBuffer,
	ulong			inBufferSize,
	ulong *			encryptedBlockStart,
	ulong *			encryptedBlockSize )
{
	const char		kBeginString[]	= "-----BEGIN PGP MESSAGE-----";
	const char		kEndString[]	= "-----END PGP MESSAGE-----";
	
	return( FindBlock(	inBuffer, 
						inBufferSize, 
						kBeginString, 
						kEndString, 
						encryptedBlockStart, 
						encryptedBlockSize ) );
}


/*______________________________________________________________
	Find a block of the form:
	
	-----BEGIN PGP KEY-----
	.....
	-----END PGP KEY-----
	
	Return the offset at which it begins and its length.
	Length includes the begin/end lines and the trailing 
	carriage return/line feed.
______________________________________________________________*/
	Boolean
FindPGPKeyBlock(
	const void *	inBuffer,
	ulong			inBufferSize,
	ulong *			encryptedBlockStart,
	ulong *			encryptedBlockSize )
{
	const char		kBeginString[]	= "-----BEGIN PGP PUBLIC KEY BLOCK-----";
	const char		kEndString[]	= "-----END PGP PUBLIC KEY BLOCK-----";
	
	return( FindBlock(	inBuffer, 
						inBufferSize, 
						kBeginString, 
						kEndString, 
						encryptedBlockStart, 
						encryptedBlockSize ) );
}




