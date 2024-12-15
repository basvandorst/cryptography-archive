/*
 * Copyright (C) 1996, 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
#include <windows.h>
#include "assert.h"
#include "config.h"
#include "pgpTypes.h"
#include "pgpMem.h"
#include "pgpErr.h"

#include "BlockUtils.h"

#define SUCCESS		0
#define ERR_NO_MEM	1

	const char *
FindMatchingBytes(
	const char *	buffer,
	ulong			length,
	const char *	findIt,
	ulong			findItLength
	)
{
	const char *	result	= NULL;
	
	pgpAssert( length >= 1 ); // allow 'length' to be 0
	
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




/*_________________________________________________________________
	Find a block of the form:
	
	linestart
	...
	lineEnd
	
	Return the offset at which it begins and its length.
	Length includes the begin/end lines and the trailing carriage
	return/line feed.
___________________________________________________________________*/
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
	
	cur	= (const char *)inBuffer;
	inBufferLast	= ( cur + inBufferSize ) -1;

#define IsEOL( c )	( (c) == '\r' || (c) == '\n' )
	
	while ( cur <= inBufferLast )
	{
		beginning = FindMatchingBytes(cur, inBufferSize,
										(const char *)startLine,
										strlen( startLine ) );

		if (!beginning )
			break;
		if ( (cur == inBuffer)  || IsEOL( *(cur - 1) ) )
			break;

		// string doesn't start at beginning of line--skip it
		cur	= beginning + strlen( startLine );
	}

	if ( beginning )
	{
		ulong	bytesRemaining;

		*encryptedBlockStart = beginning - (const char *)inBuffer;

		cur	= beginning + strlen(startLine );

		bytesRemaining	= inBufferSize - *encryptedBlockStart;

		while ( cur <= inBufferLast )
		{
			end	= FindMatchingBytes(
						cur,
						bytesRemaining,
						(const char *)endLine,
						strlen( endLine ) );
			if ( !end )
				break;

			if ( IsEOL( *(end - 1 ) ) )
			{
				// advance past line end, which could be <CR><LF> or <LF>
				// or <CR>
				cur	= end + strlen( endLine );

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
			}

			cur	= end + strlen( endLine );
		}
	}
	
	return( foundBlock );
}

/*_________________________________________________________________
	Find a block of the form:
	
-----BEGIN PGP SIGNED MESSAGE-----
 ...
-----BEGIN PGP SIGNATURE-----
 ...
-----END PGP SIGNATURE-----
	
	Return the offset at which it begins and its length.
	Length includes the begin/end lines and the trailing carriage
	return/line feed.
___________________________________________________________________*/
	Boolean
FindSignedBlock(
	const void *	inBuffer,
	ulong			inBufferSize,
	ulong *			encryptedBlockStart,
	ulong *			encryptedBlockSize )
{
	const char		kBeginString[]	= "-----BEGIN PGP SIGNED MESSAGE-----";
	const char		kEndString[]	= "-----END PGP SIGNATURE-----";
	
	return( FindBlock( inBuffer, inBufferSize, kBeginString, kEndString,
						encryptedBlockStart, encryptedBlockSize ) );
}


/*________________________________________________________________
	Find a block of the form:
	
	-----BEGIN PGP MESSAGE-----
	.....
	-----END PGP MESSAGE-----
	
	Return the offset at which it begins and its length.
	Length includes the begin/end lines and the trailing carriage
	return/line feed.
__________________________________________________________________*/
	Boolean
FindEncryptedBlock(
	const void *	inBuffer,
	ulong			inBufferSize,
	ulong *			encryptedBlockStart,
	ulong *			encryptedBlockSize )
{
	const char		kBeginString[]	= "-----BEGIN PGP MESSAGE-----";
	const char		kEndString[]	= "-----END PGP MESSAGE-----";
	
	return( FindBlock( inBuffer, inBufferSize, kBeginString, kEndString,
						encryptedBlockStart, encryptedBlockSize ) );
}


/*_________________________________________________________________
	Find a block of the form:
	
	-----BEGIN PGP KEY-----
	.....
	-----END PGP KEY-----
	
	Return the offset at which it begins and its length.
	Length includes the begin/end lines and the trailing carriage
	return/line feed.
__________________________________________________________________*/
	Boolean
FindPGPKeyBlock(
	const void *	inBuffer,
	ulong			inBufferSize,
	ulong *			encryptedBlockStart,
	ulong *			encryptedBlockSize )
{
	const char		kBeginString[]	= "-----BEGIN PGP PUBLIC KEY BLOCK-----";
	const char		kEndString[]	= "-----END PGP PUBLIC KEY BLOCK-----";
	
	return( FindBlock( inBuffer, inBufferSize, kBeginString, kEndString,
						encryptedBlockStart, encryptedBlockSize ) );
}


unsigned long WrapBuffer(char **WrappedBuffer,
						 char *OriginalBuffer,
						 unsigned short WordWrapThreshold)
{
	unsigned long ReturnCode = SUCCESS;
	char *StartOfCurrentBlock, *EndOfCurrentBlock, *StartOfCurrentLine;
	unsigned long CharsOnCurrentLine = 0,
					WrappedBufferLength = 1,
					CurrentBlockLength = 0;
	unsigned short SpaceOffset = 1;

	assert(WrappedBuffer);
	assert(OriginalBuffer);

	--WordWrapThreshold;

	//Do an initial, one char Allocation:
	*WrappedBuffer = (char *) pgpAlloc(WrappedBufferLength * sizeof(char));

	if(*WrappedBuffer)
	{
		**WrappedBuffer = '\0';
		StartOfCurrentBlock=StartOfCurrentLine=EndOfCurrentBlock=OriginalBuffer;

		//Blocks run until we hit something that needs to be word-wrapped
		//(ie > 78 chars on a line).  Blocks can be multiple lines.

		//Run until the end of the original buffer:
		while(EndOfCurrentBlock &&
			  *EndOfCurrentBlock &&
			  ReturnCode == SUCCESS)
		{
			if((*EndOfCurrentBlock == '\r') ||
			   (*EndOfCurrentBlock == '\n'))
			{
				CharsOnCurrentLine = 0;
				StartOfCurrentLine=EndOfCurrentBlock+1;
			}
			else
			{
				if(WordWrapThreshold && CharsOnCurrentLine >= WordWrapThreshold)
				{
					//Uh-oh!  Time to wrap!  Back up to the last space...
					while((EndOfCurrentBlock > StartOfCurrentLine) &&
						  (*EndOfCurrentBlock != ' ' &&
						   *EndOfCurrentBlock != '\t' &&
						   *EndOfCurrentBlock != '\n' &&
						   *EndOfCurrentBlock != '\r' &&
						   *EndOfCurrentBlock != '-'))
						  --EndOfCurrentBlock;

					//We only get here if there WERE no spaces!
					if(EndOfCurrentBlock == StartOfCurrentLine)
					{
						EndOfCurrentBlock =
						 StartOfCurrentLine + WordWrapThreshold;
						SpaceOffset=1; //SpaceOffset = 0;
					}
					else
						SpaceOffset = 1;

					//Allocate the new block space (+2 is one for the newline
					//and one for the return):
					CurrentBlockLength = (EndOfCurrentBlock -
											StartOfCurrentBlock);
					CurrentBlockLength++;

					WrappedBufferLength += (CurrentBlockLength + 2);

					pgpRealloc((void **) WrappedBuffer, WrappedBufferLength);
					if(*WrappedBuffer)
					{
						strncat(*WrappedBuffer, StartOfCurrentBlock,
								CurrentBlockLength);
						//Force NULL termination:
						*((*WrappedBuffer) + WrappedBufferLength - 2) = '\0';
						strcat(*WrappedBuffer, "\r\n");

						//Start the next block, skipping the space:
						StartOfCurrentBlock = StartOfCurrentLine=
							EndOfCurrentBlock + SpaceOffset;
					}
					else
						ReturnCode = ERR_NO_MEM;

					CharsOnCurrentLine = 0;
				}
				else
					++CharsOnCurrentLine;
			}
			++EndOfCurrentBlock;
		}
		//They did wrap; we need to clear up any trailing block.
		if(EndOfCurrentBlock > StartOfCurrentBlock)
		{
			CurrentBlockLength = (EndOfCurrentBlock - StartOfCurrentBlock);
			WrappedBufferLength += CurrentBlockLength + 1;

			pgpRealloc((void **) WrappedBuffer, WrappedBufferLength);

			if(*WrappedBuffer)
			{
				strncat(*WrappedBuffer, StartOfCurrentBlock,
						CurrentBlockLength);
				//Force NULL termination:
				*((*WrappedBuffer) + WrappedBufferLength - 2) = '\0';
			}
			else
   	   	   	   	   ReturnCode = ERR_NO_MEM;
   	   	   }
   	   }
   	   else
   	   	   ReturnCode = ERR_NO_MEM;

   	   return(ReturnCode);
   }
