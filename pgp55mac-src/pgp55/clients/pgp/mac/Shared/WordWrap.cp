/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#include <Timer.h>
#include <stdio.h>
#include <string.h>
#include <FSp_fopen.h>

#include "pgpMem.h"
#include "MacFiles.h"

#include "WordWrap.h"

no longer used; use pfl/common/util/pgpWordWrap.c

#define MIN(a, b )		( (a) <= (b) ? (a) : (b) )


	inline Boolean
IsWordBreakChar( char c )
{
	return( c == ' ' ||
			c == '\t' ||
			c == '\n' ||
			c == '\r' );
}

	inline Boolean
IsEOLChar( char c )
{
	return( c == '\r' || c == '\n' );
}


	static ushort
FindBreakChar( const char *buffer, ulong numChars )
{
	ushort	breakIndex	= numChars;
	
	// time to line break...find the last breaking character
	for ( short index = numChars - 1; index >= 0; --index )
	{
		if ( IsWordBreakChar( buffer[ index ] ) )
		{
			breakIndex	= index;
			break;
		}
	}
	return( breakIndex );
}


	static OSErr
WordWrapFiles(
	FILE *			in,			// should be positioned at start
	FILE *			out,		// should be positioned at start
	ushort			wrapLength,
	const char *	lineEnd		// "\r", "\r\n", "\n"
	)
{
	OSErr			err	= noErr;
	int				c;
	ushort			charsOnCurrentLine	= 0;
	const ushort	kBufferSize			= 512;
	char			buffer[ kBufferSize ];
	ushort			charsInBuffer		= 0;
	ushort			lineEndLength		= strlen( lineEnd );
	
	pgpAssert( wrapLength + 1 <= kBufferSize );
	if ( wrapLength + 1 > kBufferSize )
		return( -1 );
	
	while(  (c = getc( in ) ) != EOF )
	{
		// accumulate characters into our temp buffer
		pgpAssert( charsInBuffer < kBufferSize );
		buffer[ charsInBuffer ]	= c;
		++charsInBuffer;
		
		if ( IsEOLChar( c ) )
		{
			charsOnCurrentLine	= 0;
			
			// if the buffer can't hold another lines worth, then write it
			// out now
			if ( charsInBuffer + wrapLength + 1 > kBufferSize )
			{
				if ( fwrite( buffer, sizeof(char), charsInBuffer, out ) !=
							charsInBuffer )
				{
					err	= -1;
					break;
				}
				charsInBuffer	= 0;
			}
		}
		else
		{
			++charsOnCurrentLine;
			if ( charsOnCurrentLine > wrapLength )
			{
				ushort	breakIndex	= charsInBuffer;
				
				breakIndex	= FindBreakChar( buffer, charsInBuffer );
			
				if ( fwrite( buffer, sizeof(char), breakIndex, out ) !=
							breakIndex )
				{
					err	= -1;
					break;
				}
				if ( fwrite( lineEnd, sizeof(char), lineEndLength, out ) !=
							lineEndLength )
				{
					err	= -1;
					break;
				}
				
				// careful: we are skipping the breaking char
				pgpAssert( breakIndex <= charsInBuffer );
				if ( breakIndex < charsInBuffer - 1 )
				{
					// characters remain in the buffer after the break
					// character
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
	}
	
	// flush any characters that remain in buffer
	if ( charsInBuffer != 0 )
	{
		if ( fwrite( buffer, sizeof(char), charsInBuffer, out ) !=
					charsInBuffer )
		{
			err	= -1;
		}
	}
	
	return( err );
}


	OSErr
FSpWordWrapFile(
	const FSSpec *	inSpec,
	const FSSpec *	outSpec,
	ushort			wrapLength )
{
	AssertSpecIsValid( inSpec, "FSpWordWrapFile" );
	AssertSpecIsValid( outSpec, "FSpWordWrapFile" );
	pgpAssert( ! FSpEqual( inSpec, outSpec ) );
	pgpAssert( wrapLength != 0 );
	
	OSErr		err;
	FILE *		in	= NULL;
	FILE *		out	= NULL;
	CInfoPBRec	cpb;
		
	(void)FSpGetCatInfo( inSpec, &cpb );

	err	= FSpCreate( outSpec, cpbFileCreator( &cpb ), cpbFileType( &cpb ),
				smSystemScript );
	if ( IsntErr( err ) )
	{
		in	= FSp_fopen( inSpec, "rb" );
		if ( IsntNull( in ) )
			out	= FSp_fopen( outSpec, "wb" );
	}
	
	if ( IsntNull( in ) && IsntNull( out ) )
	{
		err	= WordWrapFiles( in, out, wrapLength, "\r" );
	}
	else
	{
		err	= memFullErr;
	}
	
	if ( IsntNull( in ) )
		fclose( in );
	if ( IsntNull( out ) )
		fclose( out );
	
	return( err );
}






