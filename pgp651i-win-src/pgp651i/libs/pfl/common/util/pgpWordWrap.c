/*_____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

  $Id: pgpWordWrap.c,v 1.11 1999/04/12 20:21:03 dgal Exp $
_____________________________________________________________________________*/

#include <string.h>

#include "pgpBase.h"
#include "pgpMem.h"

#include "pgpFileUtilities.h"

#include "pgpWordWrap.h"
#include "pgpStrings.h"

#define	kWordWrapBufferSize		4096UL
#define kReplyCharBufferSize	64UL

#define MIN(a, b )		( (a) <= (b) ? (a) : (b) )


#define IsWordBreakChar( c) \
	( (c) == ' ' || (c) == '\t' || (c) == '\n' || (c) == '\r' )

#define IsEOLChar( c )	( (c) == '\r' || (c) == '\n' )

#define IsReplyChar( c ) \
	( (c) == ' ' || (c) == '>' || (c) == ':' || (c) == '|' )

	static PGPUInt32
FindBreakChar( const char *buffer, PGPUInt32 numChars )
{
	PGPUInt32	breakIndex	= numChars;
	PGPInt32	index;

	/* time to line break...find the last breaking character */
	for ( index = numChars - 1; index >= 0; --index )
	{
		if ( IsWordBreakChar( buffer[ index ] ) )
		{
			breakIndex	= index;
			break;
		}
	}
	return( breakIndex );
}



	static PGPError
WriteParagraph(
	PGPIORef		out,
	PGPUInt16		charsInBuffer,
	char *			buffer,
	PGPUInt16		charsInReplyBuffer,
	char *			replyBuffer,
	PGPUInt16		wrapLength,
	const char *	lineEnd		/* "\r", "\r\n", "\n" */
	)
{
	PGPError		err	= kPGPError_NoErr;
	PGPUInt16		charsOnCurrentLine	= 0;
	PGPUInt16		charsOut = 0;
	PGPUInt16		lineEndLength		= strlen( lineEnd );
	PGPUInt16		breakIndex;

	while (charsOut < charsInBuffer)
	{
		charsOnCurrentLine = wrapLength - charsInReplyBuffer;
		
		if ((charsOnCurrentLine + charsOut) > charsInBuffer)
		{
			charsOnCurrentLine = charsInBuffer - charsOut;
			breakIndex = charsOnCurrentLine;
		}
		else
		{
			breakIndex	= FindBreakChar( &(buffer[charsOut]), 
							charsOnCurrentLine );
		}
		
		if (charsInReplyBuffer)
		{
			err	= PGPIOWrite( out, charsInReplyBuffer, replyBuffer );
			if ( IsPGPError( err ) )
				break;
		}
		
		err	= PGPIOWrite( out, breakIndex, &(buffer[charsOut]) );
		if ( IsPGPError( err ) )
			break;
		
		err	= PGPIOWrite( out, lineEndLength, lineEnd );
		if ( IsPGPError( err ) )
			break;
		
		charsOut += breakIndex + 1;
	}

	return err;
}


	PGPError
pgpWordWrapIO(
	PGPIORef		in,			/* should be positioned at start */
	PGPIORef		out,		/* should be positioned at start */
	PGPUInt16		wrapLength,
	const char *	lineEnd		/* "\r", "\r\n", "\n" */
	)
{
	PGPError		err	= kPGPError_NoErr;
	char			buffer[ kWordWrapBufferSize ];
	char			reply[ kReplyCharBufferSize ];
	char			replyBuffer[ kReplyCharBufferSize ];
	PGPUInt16		charsInBuffer		= 0;
	PGPUInt16		charsInReply		= 0;
	PGPUInt16		charsInReplyBuffer	= 0;
	PGPUInt16		lineEndLength		= strlen( lineEnd );
	PGPUInt16		charsInLineEnd		= 0;
	char			c;
	char			lastChar;
	PGPBoolean		isReply;
	PGPBoolean		gotLineEnd = FALSE;
	PGPBoolean		newParagraph = FALSE;
	PGPBoolean		userParagraph = FALSE;
	PGPBoolean		blankReplyLine = FALSE;
	PGPBoolean		indentation = FALSE;
	
	pgpAssert( wrapLength + 1 <= kWordWrapBufferSize );
	if ( wrapLength + 1 > kWordWrapBufferSize )
		return( -1 );
	
	reply[0] = 0;
	isReply = TRUE;
	lastChar = 0;

	/* We're now dealing with paragraphs instead of lines. As characters
	   are read in, there's three possible cases. What we want to do is
	   accumulate a paragraph, then write it out, prepending any reply
	   characters and word-wrapping appropriately. */

	while( IsntPGPError( err = PGPIORead( in, 1, &c, NULL ) )  )
	{
		/* accumulate characters into our temp buffer */
		pgpAssert( charsInBuffer < kWordWrapBufferSize );

		/* Check for indentation, i.e. leading spaces starting a line,
		   or two or more leading spaces after the last reply character */

		indentation = (c == ' ') && ((lastChar == ' ') || IsEOLChar(lastChar));

		/* Case #1: It's a reply character at the beginning of a line */

		if ( IsReplyChar( c ) && isReply && !indentation &&
			(charsInReply < kReplyCharBufferSize) )
		{
			charsInLineEnd = 0;

			/* Check for new paragraph */

			if (c != reply[charsInReply])
			{
				/* Reply char pattern doesn't match previous pattern,
				   therefore it's a new paragraph */

				reply[charsInReply] = c;

				/* If there's anything in the buffer, write it out before 
				   starting a new paragraph */
	
				if (charsInBuffer)
				{
					err = WriteParagraph(out, charsInBuffer, buffer, 
							charsInReplyBuffer, replyBuffer, wrapLength, 
							lineEnd);

					if ( IsPGPError( err ) )
						break;

					charsInBuffer = 0;
					gotLineEnd = FALSE;
					newParagraph = FALSE;
				}

				/* Save the current line's reply characters */

				pgpCopyMemory(reply, replyBuffer, charsInReply + 1);
				charsInReplyBuffer = charsInReply + 1;
			}

			charsInReply++;
		}

		/* Case #2: It's an End-Of-Line character */

		else if ( IsEOLChar( c ) )
		{
			/* If this line began with reply chars, then this might
			   be a new paragraph. */

			if (charsInReply)
			{
				/* Did the line before this one have reply chars? */

				if (charsInReplyBuffer)
				{
					/* If the last line's reply chars don't match this
					   line's, then we have a new paragraph. The only
					   reason it didn't trigger in case #1 is that
					   the EOL came right after the reply characters */

					if ((charsInReplyBuffer != charsInReply) ||
						(pgpCompareStringsIgnoreCaseN(reply, replyBuffer, 
							charsInReply)))
					{
						err = WriteParagraph(out, charsInBuffer, buffer, 
								charsInReplyBuffer, replyBuffer, wrapLength, 
								lineEnd);

						if (IsntPGPError(err))
							err	= PGPIOWrite( out, charsInReply, reply );

						if ( IsPGPError( err ) )
							break;
						
						charsInBuffer = 0;
					}

					/* Otherwise, this is a line with reply characters
					   only. We'll have to write this line out separately,
					   else it will get folded into the reformatted
					   paragraph. */

					else if (isReply)
						blankReplyLine = TRUE;
				}

				/* Otherwise, this is a line with reply characters
				   only. We'll have to write this line out separately,
				   else it will get folded into the reformatted
				   paragraph. */

				else if (isReply)
					blankReplyLine = TRUE;

				/* Save the current line's reply characters */

				pgpCopyMemory(reply, replyBuffer, charsInReply);
				charsInReplyBuffer = charsInReply;
			}

			userParagraph = FALSE;

			/* Do we have a line end? */

			if (c == lineEnd[charsInLineEnd])
			{
				charsInLineEnd++;
				if (charsInLineEnd == lineEndLength)
				{
					/* Do we have a new paragraph? */

					if (gotLineEnd)
						newParagraph = TRUE;

					/* Did the user end the line before word-wrap? */

					if (charsInBuffer && !newParagraph &&
						((charsInBuffer + charsInReplyBuffer) < wrapLength))
					{
						userParagraph = TRUE;
					}

					/* If there were no reply characters, we'll also
					   treat this as a new paragraph */

					if (charsInBuffer && !charsInReplyBuffer && !newParagraph)
						userParagraph = TRUE;

					gotLineEnd = TRUE;
					charsInLineEnd = 0;
				}
			}

			/* If the buffer can't hold another lines worth, then write it
			   out now. Also write it out if a new paragraph is beginning.
			   A new paragraph is defined as consecutive line ends,
			   or if the user terminated the line before the word-wrap. */

			if ( ( charsInBuffer + wrapLength + 1 > kWordWrapBufferSize ) ||
				 newParagraph || userParagraph)
			{
				err = WriteParagraph(out, charsInBuffer, buffer, 
						charsInReplyBuffer, replyBuffer, wrapLength, 
						lineEnd);

				/* Write out any lines that have reply characters only */

				if (IsntPGPError(err) && blankReplyLine)
				{
					err	= PGPIOWrite( out, charsInReplyBuffer, replyBuffer );
					blankReplyLine = FALSE;
				}

				/* If this was a new paragraph due to consecutive EOL's,
				   write out the second EOL */

				if (IsntPGPError(err) && !userParagraph)
					err	= PGPIOWrite( out, lineEndLength, lineEnd );
		
				if ( IsPGPError( err ) )
					break;

				charsInBuffer = 0;
				charsInReplyBuffer = 0;
				reply[0] = 0;
				newParagraph = FALSE;
			}

			/* Add a space to the buffer as we accumulate lines for the
			   paragraph */

			else if (gotLineEnd && charsInBuffer)
			{
				buffer[charsInBuffer] = ' ';
				charsInBuffer++;
			}

			charsInReply = 0;
			isReply = TRUE;
		}

		/* Case #3: It's a non-reply character or a reply character
		   that occurred after a non-reply character */

		else
		{
			/* Did we have an EOL recently? */

			if (gotLineEnd)
			{
				/* If so, we should check to see if this is a new paragraph.
				   It's possible to miss the checks in cases #1 and #2,
				   because the previous reply could have been ">>", and
				   the current reply could be ">", for example. */

				/* Also, if the user indented the line, it's definitely
				   a new paragraph */

				if ((charsInReplyBuffer != charsInReply) || indentation ||
					(pgpCompareStringsIgnoreCaseN(reply, replyBuffer, 
						charsInReply)))
				{
					err = WriteParagraph(out, charsInBuffer, buffer, 
							charsInReplyBuffer, replyBuffer, wrapLength, 
							lineEnd);

					if ( IsPGPError( err ) )
						break;

					charsInBuffer = 0;
					charsInReplyBuffer = 0;
				}
			}

			buffer[ charsInBuffer ]	= c;
			++charsInBuffer;
			charsInLineEnd = 0;
			isReply = FALSE;
			gotLineEnd = FALSE;
			newParagraph = FALSE;
		}

		lastChar = c;
		pgpAssert( err != kPGPError_EOF );
	}
	/* normal to reach EOF while reading */
	if ( err == kPGPError_EOF )
		err	= kPGPError_NoErr;
	
	/* flush any characters that remain in buffer */
	if ( charsInBuffer != 0 && IsntPGPError( err ) )
	{
		err = WriteParagraph(out, charsInBuffer, buffer, 
				charsInReplyBuffer, replyBuffer, wrapLength, 
				lineEnd);
	}
	
	return( err );
}


	PGPError
pgpWordWrapFileSpecs(
	PFLFileSpecRef	inputSpec,
	PFLFileSpecRef	outputSpec, 
	PGPUInt16		wrapLength,
	const char *	lineEnd )
{
	PGPError		err;
	PGPFileIORef	inRef	= NULL;
#if PGPIO_EOF
	PGPFileOffset	curPos;
#endif

	(void)PFLFileSpecCreate( outputSpec );
	
	err	= PGPOpenFileSpec( inputSpec,
			kPFLFileOpenFlags_ReadOnly, &inRef );
	if ( IsntPGPError( err ) )
	{
		PGPFileIORef	outRef	= NULL;
	
		err	= PGPOpenFileSpec( outputSpec,
			kPFLFileOpenFlags_ReadWrite, &outRef );
		if ( IsntPGPError( err ) )
		{
			err	= pgpWordWrapIO( (PGPIORef)inRef,
				(PGPIORef)outRef, wrapLength, lineEnd );

#if PGPIO_EOF
			PGPIOGetPos( (PGPIORef)outRef, &curPos );
			PGPIOSetEOF( (PGPIORef)outRef, curPos );
#endif
			PGPFreeIO( (PGPIORef)outRef );
		}
		PGPFreeIO( (PGPIORef)inRef );
	}
	
	return( err );
}


#if PGP_MACINTOSH	/* [ */

	PGPError
pgpWordWrapFileFSSpec(
	PGPMemoryMgrRef	memoryMgr,
	const FSSpec *	inFSSpec,
	const FSSpec *	outFSSpec,
	PGPUInt16		wrapLength,
	char const *	lineEnd )
{
	PGPError			err;
	PFLFileSpecRef		inSpec	= NULL;
	
	err	= PFLNewFileSpecFromFSSpec( memoryMgr, inFSSpec, &inSpec );
	if ( IsntPGPError( err ) )
	{
		PFLFileSpecRef		outSpec	= NULL;
		
		err	= PFLNewFileSpecFromFSSpec( memoryMgr, outFSSpec, &outSpec );
		if ( IsntPGPError( err ) )
		{
			err	= pgpWordWrapFileSpecs( inSpec,
				outSpec, wrapLength, lineEnd );
			
			PFLFreeFileSpec( outSpec );
		}
		PFLFreeFileSpec( inSpec );
	}
	
	
	return( err );
}


#endif /* ] PGP_MACINTOSH */













