/*____________________________________________________________________________
	MacStrings.c
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: MacStrings.c,v 1.18 1997/09/18 01:34:56 lloyd Exp $
____________________________________________________________________________*/
#include <ctype.h>
#include <limits.h>
#include <string.h>

#include <Files.h>
#include <TextUtils.h>

#include "MacNumerics.h"
#include "MacStrings.h"
#include "MacMemory.h"

/*____________________________________________________________________________
	NumVersionToString: Convert a Macintosh numeric version to a Pascal string
____________________________________________________________________________*/

	void
NumVersionToString(const NumVersion *version, StringPtr versionString)
{
	Str255	tempString;
	UInt32	majorRev;
	UInt32	minorRev;
	UInt32	bugFixRev;
	uchar	stageChar;
	
	pgpAssertAddrValid( versionString, StringPtr );
	
	versionString[0] = 0;
	
	BCDToDecimal( version->majorRev, &majorRev );
	BCDToDecimal( version->minorAndBugRev >> 4, &minorRev );
	BCDToDecimal( version->minorAndBugRev & 0x0F, &bugFixRev );

	NumToString( majorRev, versionString );
	AppendPString( "\p.", versionString );
	NumToString( minorRev, tempString );
	AppendPString( tempString, versionString );
	
	if( bugFixRev != 0 )
	{
		AppendPString( "\p.", versionString );
		NumToString( bugFixRev, tempString );
		AppendPString( tempString, versionString );
	}
		
	switch( version->stage )
	{
		case developStage:
			stageChar = 'd';
			break;
			
		case alphaStage:
			stageChar = 'a';
			break;

		case betaStage:
			stageChar = 'b';
			break;
		
		default:
			stageChar = 0;
			break;
	}
	
	if( stageChar != 0 )
	{
		UInt32	nonReleaseRev;
		
		/* Append the stage character to the version string */
		versionString[0] += 1;
		versionString[versionString[0]] = stageChar;
	
		BCDToDecimal( version->nonRelRev, &nonReleaseRev );

		NumToString( nonReleaseRev, tempString );
		AppendPString( tempString, versionString );
	}
}

/*____________________________________________________________________________
	VersionToString: Convert a Macintosh BCD version UInt16 to a Pascal string
____________________________________________________________________________*/

	void
VersionToString(UInt16 version, StringPtr versionString)
{
	Str255	tempString;
	UInt32	majorRev;
	UInt32	minorRev;
	UInt32	bugFixRev;
	
	pgpAssertAddrValid( versionString, StringPtr );
	
	versionString[0] = 0;
	
	BCDToDecimal( version >> 8, &majorRev );
	BCDToDecimal( ( version >> 4 ) & 0x000F, &minorRev );
	BCDToDecimal( version & 0x000F, &bugFixRev );

	NumToString( majorRev, versionString );
	AppendPString( "\p.", versionString );
	NumToString( minorRev, tempString );
	AppendPString( tempString, versionString );
	
	if( bugFixRev != 0 )
	{
		AppendPString( "\p.", versionString );
		NumToString( bugFixRev, tempString );
		AppendPString( tempString, versionString );
	}
}

/*____________________________________________________________________________
	CopyPString: Copy a Pascal string
____________________________________________________________________________*/

	void
CopyPString(ConstStr255Param srcStr, StringPtr destStr)
{
	pgpAssertAddrValidMsg( srcStr, uchar,
		"CopyPString: invalid source string" );
	pgpAssertAddrValidMsg( destStr, uchar,
		"CopyPString: invalid destination string" );
	
	BlockMoveData( srcStr, destStr, 1UL + srcStr[ 0 ] );
}

/*____________________________________________________________________________
	AppendPString: Append one Pascal string to the end of another
____________________________________________________________________________*/
	
#define kMaxStringLength	(UInt16)255
	void
AppendPString(ConstStr255Param appendStr, StringPtr destStr)
{
	AppendPStringMax( appendStr, destStr, kMaxStringLength );
}

/*____________________________________________________________________________
	CopyPString: Copy a Pascal string up to a maximum number of characters
____________________________________________________________________________*/

	void
CopyPStringMax(
	ConstStr255Param 	srcStr,
	StringPtr 			destStr,
	UInt32				maxBytesToCopy)	/* Does not include length PGPByte */
{
	UInt32	lengthToCopy;
	
	pgpAssertAddrValidMsg( srcStr, uchar,
		"CopyPStringMax: invalid source string" );
	pgpAssertAddrValidMsg( destStr, uchar,
		"CopyPStringMax: invalid destination string" );
	
	lengthToCopy = srcStr[0];
	if( lengthToCopy > maxBytesToCopy )
		lengthToCopy = maxBytesToCopy;
		
	BlockMoveData( srcStr, destStr, 1UL + lengthToCopy );
}

/*____________________________________________________________________________
	AppendPString: Append one Pascal string to the end of another
____________________________________________________________________________*/
	
	void
AppendPStringMax(
	ConstStr255Param 	appendStr,
	StringPtr 			destStr,
	UInt32 				maxBytesToAppend)
{
	UInt32	lengthToAppend;

	pgpAssertAddrValidMsg( appendStr, uchar,
		"AppendPStringMax: invalid appendStr string" );
	pgpAssertAddrValidMsg( destStr, uchar,
		"AppendPStringMax: invalid destination string" );

	/*
	** if the new length would exceed the maximum string length, then
	** just append as much as possible
	*/
	
	lengthToAppend	= appendStr[0];
	if ( lengthToAppend + (UInt32)destStr[0] > kMaxStringLength )
		lengthToAppend	= kMaxStringLength - destStr[0];

	if( lengthToAppend > maxBytesToAppend )
		lengthToAppend = maxBytesToAppend;
		
	BlockMoveData( &appendStr[1], &destStr[destStr[0]+1], lengthToAppend);
	destStr[0]	+= lengthToAppend;
}

/*____________________________________________________________________________
	DeleteFromPString: Delete 'count' characters from a pascal string,
	beginning at 'offset' >= 1
____________________________________________________________________________*/

	void
DeleteFromPString(uchar *string, uchar offset, uchar count)
{
	uchar	trailing;
	
	pgpAssertAddrValid( string, uchar );

	if ( (offset < 1) ||
		(UInt16) offset + (UInt16) count - 1  > (UInt16) string[0]  )
	{
		pgpDebugMsg( "DeleteFromPString: Invalid offset/count combination" );
		return;
	}
	
	trailing = string[0] - (offset -1) - count;
	BlockMoveData( &string[offset + count], &string[offset], trailing );

	string[0]	-= count;
}

/*____________________________________________________________________________
	InsertInPString: Insert one pascal string into another beginning at
	'offset' >= 1
____________________________________________________________________________*/
				
	void
InsertInPString(const uchar *source, uchar offset, uchar *dest)
{
	uchar	temp[256];
	UInt32	sourceLength	= source[0];
	UInt32	destLength		= dest[0];
	
	pgpAssertAddrValid( source, uchar );
	pgpAssertAddrValid( dest, uchar );

	if( ( destLength + sourceLength > 255 ) ||
		( (UInt16) offset > destLength + 1 ) )
	{
		pgpDebugMsg( "InsertInPString: Resulting string too big" );
		return;
	}
	
	temp[0]	= destLength + 1 - offset;
	BlockMoveData( &dest[offset], &temp[1], temp[0]);
	
	dest[0]	= offset - 1;
	AppendPString( source, dest );
	AppendPString( temp, dest );
}

/*____________________________________________________________________________
	PrintPString: Insert a pascal string into another at the best location,
	which is as follows:

	The first occurrence of the sequence %s. If no %s sequences are found, 
	then any '%' character followed by a digit 0-9.
	The first occurrence of the lowest %digit sequence is replaced first.
____________________________________________________________________________*/

	void
PrintPString(uchar *dest, const uchar *formatStr, const uchar *stringToPut)
{
	uchar	*cur;
	uchar	*last;
	uchar	*insertAt;
	uchar	minChar;
	
	pgpAssertAddrValid( dest, uchar );
	pgpAssertAddrValid( formatStr, uchar );
	pgpAssertAddrValid( stringToPut, uchar );

	CopyPString( formatStr, dest );
	
	cur 		= &dest[1];
	last 		= &dest[dest[0]];
	insertAt	= nil;
	minChar		= '9' + 1;
	
	while( cur <= last )
	{
		if( *cur++ == '%' )
		{
			register uchar	curChar = *cur;
			
			if( curChar == 's' )
			{
				insertAt = cur;
				break;
			}
			else if( curChar >= '0' && curChar <= '9' )
			{
				if( curChar < minChar )
				{
					minChar 	= curChar;
					insertAt 	= cur;
				}
			}
		}
	}

	if( insertAt != nil )
	{
		DeleteFromPString( dest, insertAt - dest - 1, 2 );
		InsertInPString( stringToPut, insertAt - dest - 1, dest );
	}
}


	Boolean	
PStringsAreEqual(
	ConstStr255Param	str1,
	ConstStr255Param	str2 )
{
	Boolean		equal	= FALSE;
	
	if ( StrLength( str1 ) == StrLength( str2 ) )
	{
		equal	= pgpMemoryEqual( &str1[ 1 ], &str2[ 1 ], StrLength( str1 ) );
	}
	
	return( equal );
}

	Boolean
PStringHasSuffix(
	ConstStr255Param	str,
	ConstStr255Param	suffix,
	Boolean				caseSensitive )
{
	Boolean	hasSuffix = FALSE;
	
	pgpAssertAddrValid( str, uchar );
	pgpAssertAddrValid( suffix, uchar );

	if( suffix[0] < str[0]  )
	{
		/* The name is at lease as long as the suffix, and the last
		character matches.  Search for the suffix. */
	
		Str255	testStr;
		UInt32	suffixLength;
		UInt32	strLength;
		
		suffixLength 	= suffix[0];
		strLength		= str[0];
		
		pgpCopyMemory( &str[strLength - suffixLength + 1], &testStr[1],
					suffixLength );
		testStr[0] = suffixLength;
		
		hasSuffix = EqualString( testStr, suffix, caseSensitive, TRUE );
	}

	return( hasSuffix );
}


/*____________________________________________________________________________
	A temporary string is used so that source and dest strings may be
	the same string.
____________________________________________________________________________*/
	void
CToPString(
	const char	*cString,
	StringPtr	pString)
	{
	UInt32		length;
	UInt8		*curOut;
	Str255		tempStr;
	
	pgpAssertAddrValid( cString, char );
	pgpAssertAddrValid( pString, uchar );
	
	length		= 0;
	curOut		= &tempStr[1];
	#define kMaxPascalStringLength	255
	while ( length < kMaxPascalStringLength )
		{
		if ( (curOut[length] = cString[length]) == 0)
			break;
		++length;
		}
	
	tempStr[0]	= length;
	
	CopyPString( tempStr, pString );
	}
	
	
	void
PToCString(
	ConstStr255Param	pString,
	char *				cString)
	{
	short	length;
	
	pgpAssertAddrValid( pString, uchar );
	pgpAssertAddrValid( cString, char );

	length	= StrLength( pString );
	
	BlockMoveData( &pString[ 1 ], cString, length);
	cString[ length ]	= '\0';
	}




/*	Case insensitive strstr */
	const char *
FindSubStringNoCase(
	const char * inBuffer,
	const char * inSearchStr)
{
	const char *	currBuffPointer = inBuffer;
	
	while (*currBuffPointer != 0)
	{
		const char * compareOne = currBuffPointer;
		const char * compareTwo = inSearchStr;
		
		while ( tolower(*compareOne) == tolower(*compareTwo) )
		{
			compareOne++;
			compareTwo++;
			if (*compareTwo == 0)
			{
				return (char *) currBuffPointer;
			}
		}
		currBuffPointer++;
	}
	return NULL;
}

	char *
CopyCString(
	const char *	src,
	char *			dest)
{
	pgpAssertAddrValid( dest, char );
	pgpAssertAddrValid( dest, char );
	
	while ( ( *dest++ = *src++ ) != '\0' )
	{
	}
		
	return( dest );
}




	OSErr
AppendStringToHandle(
	Handle				theHandle,
	ConstStr255Param	string )
{
	OSErr	err	= noErr;
	
	err	= PtrAndHand( &string[ 1 ], theHandle, StrLength( string ) );
	
	return( err );
}


	void
GetIndCString(
	char 	theString[256],
	ResID 	stringListResID,
	short 	stringListIndex)
{
	Str255	pString;
	
	GetIndString( pString, stringListResID, stringListIndex );
	PToCString( pString, theString );
}




/*____________________________________________________________________________
	Insert strings using PrintPString into baseString.
	
	PrintPString looks for occurences of %1, %2, %3, etc
____________________________________________________________________________*/
	void
InsertPStrings(
	ConstStr255Param	baseString,
	ConstStr255Param	str1,
	ConstStr255Param	str2,
	ConstStr255Param	str3,
	StringPtr			resultString )
{
	Str255	finalMessage;
	
	CopyPString( baseString, finalMessage );
	
	if ( IsntNull( str1 ) )
	{
		PrintPString( finalMessage, finalMessage, str1 );
	}
	
	if ( IsntNull( str2 ) )
	{
		PrintPString( finalMessage, finalMessage, str2 );
	}
	
	if ( IsntNull( str3 ) )
	{
		PrintPString( finalMessage, finalMessage, str3 );
	}
	
	CopyPString( finalMessage, resultString );
}




		
