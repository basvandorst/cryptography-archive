/*_____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
_____________________________________________________________________________*/

#include <string.h>

#include "pgpBase.h"
#include "pgpMem.h"

#include "pgpFileUtilities.h"

#include "pgpWordWrap.h"


#define MIN(a, b )		( (a) <= (b) ? (a) : (b) )


#define IsWordBreakChar( c) \
	( (c) == ' ' || (c) == '\t' || (c) == '\n' || (c) == '\r' )

#define IsEOLChar( c )	( (c) == '\r' || (c) == '\n' )


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



	PGPError
pgpWordWrapIO(
	PGPIORef		in,			/* should be positioned at start */
	PGPIORef		out,		/* should be positioned at start */
	PGPUInt16		wrapLength,
	const char *	lineEnd		/* "\r", "\r\n", "\n" */
	)
{
	PGPError		err	= kPGPError_NoErr;
	PGPUInt16		charsOnCurrentLine	= 0;
	#define	kWordWrapBufferSize		512UL
	char			buffer[ kWordWrapBufferSize ];
	PGPUInt16		charsInBuffer		= 0;
	PGPUInt16		lineEndLength		= strlen( lineEnd );
	char			c;
	
	pgpAssert( wrapLength + 1 <= kWordWrapBufferSize );
	if ( wrapLength + 1 > kWordWrapBufferSize )
		return( -1 );
	
	while( IsntPGPError( err = PGPIORead( in, 1, &c, NULL ) )  )
	{
		/* accumulate characters into our temp buffer */
		pgpAssert( charsInBuffer < kWordWrapBufferSize );
		buffer[ charsInBuffer ]	= c;
		++charsInBuffer;
		
		if ( IsEOLChar( c ) )
		{
			charsOnCurrentLine	= 0;
			
			/* if the buffer can't hold another lines worth, then write it */
			/* out now */
			if ( charsInBuffer + wrapLength + 1 > kWordWrapBufferSize )
			{
				err	= PGPIOWrite( out, charsInBuffer, buffer );
				if ( IsPGPError( err ) )
					break;
				charsInBuffer	= 0;
			}
		}
		else
		{
			++charsOnCurrentLine;
			if ( charsOnCurrentLine > wrapLength )
			{
				PGPUInt16	breakIndex	= charsInBuffer;
				
				breakIndex	= FindBreakChar( buffer, charsInBuffer );
			
				err	= PGPIOWrite( out, breakIndex, buffer );
				if ( IsPGPError( err ) )
					break;
					
				err	= PGPIOWrite( out, lineEndLength, lineEnd );
				if ( IsPGPError( err ) )
					break;
				
				/* careful: we are skipping the breaking char */
				pgpAssert( breakIndex <= charsInBuffer );
				if ( breakIndex < charsInBuffer - 1 )
				{
					/* characters remain in the buffer after the break */
					/* character */
					charsInBuffer	-= breakIndex + 1;
					pgpCopyMemory( &buffer[ breakIndex + 1 ], &buffer[ 0 ],
								charsInBuffer);
				}
				else
				{
					charsInBuffer	= 0;	
				}
				
				charsOnCurrentLine	= charsInBuffer;
			}
		}
		pgpAssert( err != kPGPError_EOF );
	}
	/* normal to reach EOF while reading */
	if ( err == kPGPError_EOF )
		err	= kPGPError_NoErr;
	
	/* flush any characters that remain in buffer */
	if ( charsInBuffer != 0 && IsntPGPError( err ) )
	{
		err	= PGPIOWrite( out, charsInBuffer, buffer );
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













