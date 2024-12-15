/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: pgpStrings.c,v 1.8.12.1 1998/11/12 03:18:43 heller Exp $
____________________________________________________________________________*/

#include "pgpPFLConfig.h"

#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "pgpMem.h"
#include "pgpStrings.h"

#if PGP_MACINTOSH
#include <NumberFormatting.h>

#include "MacStrings.h"
#endif


/*____________________________________________________________________________
	Return a pointer to the file name extension in the name.  The pointer
	does NOT include the period.
	
	File name extensions must be 3 chars or less and not be the whole name.
	
	Return NULL if extension not found
____________________________________________________________________________*/
#define kMaxExtensionLength		3	/* not including period */
	const char *
PGPGetFileNameExtension( const char *	name )
{
	const char *extension;
	
	extension	= strrchr( name, '.' );
	if ( IsntNull( extension ) )
	{
		/* must not be whole name, or be longer than ".xxx" */
		if ( extension == name )
			extension	= NULL;
		else if ( strlen( extension ) > (kMaxExtensionLength + 1)  )
			extension	= NULL;
		else
			++extension;	/* skip over period */
	}
	
	return( extension );
}

	const char *
PGPGetEndOfBaseFileName(
	const char *	name )
{
	const char *	result;

	result = PGPGetFileNameExtension( name );
	if ( IsNull( result ) )
		result = name + strlen( name );
	else
		result--;
	
	return result;
}

/*____________________________________________________________________________
	Form a new file name based on the base name, a number.
	
	Example (with number = 99, separator = "-", maxSize = 13):
	Foo.tmp			=> Foo-99.tmp
	Foo				=> Foo-99
	TooLongName.tmp => TooLo-99.tmp
	
	'maxSize' includes trailing null.  If necessary, first the base name
	will be truncated, then the separator, and then the extension, in
	order to fit in maxSize.
	
	returns an error if new name won't fit in buffer
____________________________________________________________________________*/
	PGPError
PGPNewNumberFileName(
	const char *	origName,
	const char *	separator,
	PGPUInt32		number,
	PGPSize			maxSize,	/* including NULL */
	char *			outName )
{
	char			numString[ 32 ];
	const char *	extension		= NULL;
	PGPSize			baseLength		= 0;
	PGPSize			separatorLength	= 0;
	PGPSize			extensionLength	= 0;
	PGPSize			numLength		= 0;

	PGPValidateParam( origName != outName );
	/* require minimum of 8.3 */
	PGPValidateParam( maxSize >= ( 8 + 1 + 3 + 1 ) );

#if PGP_MACINTOSH
	/* avoid linking in sprintf */
	NumToString( number, (unsigned char *)numString );
	PToCString( (unsigned char *)numString, numString );
#else
	sprintf( numString, "%ld", (long)number );
#endif
	
	extension	= PGPGetFileNameExtension( origName );
	if ( IsntNull( extension ) )
	{
		extensionLength	= strlen( extension );
		pgpAssert( strlen( extension ) <= maxSize - 1 );
	}
	else
	{
		extensionLength	= 0;
	}
	
	baseLength	= strlen( origName ) - extensionLength;
	if ( extensionLength != 0 )
	{
		if ( baseLength != 0 )
			--baseLength;	/* account for "." */
	}

	separatorLength	= strlen( separator );
	
	numLength	= strlen( numString );
	while ( TRUE )
	{
		PGPSize	totalLength;
	
		totalLength	= baseLength + separatorLength + numLength;
		if ( extensionLength != 0 )
		{
			++totalLength; /* for "." */
			totalLength	+= extensionLength;
		}
		++totalLength;	/* for trailing NULL */
		if ( totalLength <= maxSize )
			break;
			
		/* reduce the size of various name parts */
		/* truncate the base name first */
		if ( baseLength != 0 )
			--baseLength;
		else if ( separatorLength != 0 )
			--separatorLength;
		else if ( extensionLength != 0 )
			--extensionLength;
		else
		{
			pgpDebugMsg( "impossible" );
		}
	}
	
	/* now form the complete name */
	{
		char *cur	= outName;
		
		pgpCopyMemory( origName, cur, baseLength );
		cur	+= baseLength;
		pgpCopyMemory( separator, cur, separatorLength );
		cur	+= separatorLength;
		pgpCopyMemory( numString, cur, numLength );
		cur	+= numLength;
		if ( extensionLength != 0 )
		{
			*cur++	= '.';
			pgpCopyMemory( extension, cur, extensionLength );
			cur	+= extensionLength;
		}
		*cur	= '\0';
	}
	
	pgpAssert( strlen( outName ) + 1 <= maxSize );
	
	return( kPGPError_NoErr );
}

/*____________________________________________________________________________
	Checks to see if PGPNewNumberFileName() could have generated <fileName>
	from <origName> for some number, and returns the number.

	maxSize must be <= the maxSize which was passed to PGPNewNumberFileName()
	and is used to make sure the origName wasn't truncated more than
	necessary.  Passing 0 is fine, but then any filename of the form
	<number>.xxx (with matching extension) will succeed.

	WARNING: This may not work properly if the separator contains a digit,
			 or if the separator is empty and the basename contains a digit.
____________________________________________________________________________*/
	PGPError
PGPVerifyNumberFileName(
	const char *	origName,
	const char *	separator,
	const char *	fileName,
	PGPSize			maxSize,
	PGPBoolean *	outValid,
	PGPUInt32 *		outNumber )
{
	PGPSize			fileNameLength;
	const char *	fileNamePtr;
	const char *	fileNameSep;
	const char *	fileNameNum;
	const char *	origNameEnd;
	const char *	fileNameEnd;
	PGPSize			origNameEndLength;
	PGPSize			fileNameEndLength;
	PGPBoolean		isMaxLength;
	PGPUInt32		number;
	PGPUInt32		oldNumber;

	*outValid = FALSE;
	*outNumber = -1;

	fileNameLength = strlen( fileName );
	isMaxLength = (maxSize <= fileNameLength + 1 );
	origNameEnd = PGPGetEndOfBaseFileName( origName );
	fileNameEnd = PGPGetEndOfBaseFileName( fileName );
	origNameEndLength = strlen( origNameEnd );
	fileNameEndLength = strlen( fileNameEnd );

	/* Check to see if the extensions match */
	if ( fileNameEndLength != 1 &&
		 strncmp( origNameEnd, fileNameEnd, fileNameEndLength ) == 0 )
	{
		/* Scan left from the extension to find the start of the number */
		fileNamePtr = fileNameEnd;
		while ( fileNamePtr >= fileName &&
				fileNamePtr[-1] >= '0' && fileNamePtr[-1] <= '9' )
			fileNamePtr--;
		fileNameNum = fileNamePtr;

		/* Parse the number */
		number = 0;
		if ( *fileNamePtr >= '0' && *fileNamePtr <= '9' )
		{
			while ( *fileNamePtr >= '0' && *fileNamePtr <= '9' )
			{
				oldNumber = number;
				number = ( number * 10 ) + ( *fileNamePtr - '0' );
				if ( oldNumber != ( number - ( *fileNamePtr - '0' ) ) / 10 )
					break;	/* Overflow */
				fileNamePtr++;
			}

			/*
			 * Now we know exactly where the separator must start.
			 * The separator won't be truncated until the rest of the
			 * Basename is completely gone.
			 */
			fileNameSep = fileNameNum - strlen( separator );
			if ( fileNameSep < fileName )
				fileNameSep = fileName;

			/* Check the following conditions for a valid name: */
				/* - short extension implies an empty baseName */
			if ( ( fileNameEndLength == origNameEndLength ||
				   fileNameNum == fileName ) &&

				/* - short baseName implies fileName is max length */
				 ( fileNameSep - fileName == origNameEnd - origName ||
				   fileNameLength + 1 >= maxSize ) &&

				/* - baseName before separator matches */
				 strncmp( origName, fileName, fileNameSep - fileName ) == 0 &&

				/* - separator matches */
				 strncmp( separator, fileNameSep, fileNameNum - fileNameSep )
						== 0 )
			{
				*outValid = TRUE;
				*outNumber = number;
			}
		}
	}
	return kPGPError_NoErr;
}


	int
pgpCompareStringsIgnoreCase(
	const char *	s1,
	const char *	s2 )
{
	while ((*s1 != 0) && (*s2 != 0) && (tolower(*s1) == tolower(*s2))) {
		s1++;
		s2++;
	}
	return (*s1 - *s2);
}



	int
pgpCompareStringsIgnoreCaseN(
	const char *	s1,
	const char *	s2,
	int				n)
{
	while ((n > 0) &&
		(*s1 != 0) && (*s2 != 0) &&
		(tolower(*s1) == tolower(*s2)))
	{
		s1++;
		s2++;
		n--;
	}
	
	return ((n == 0) ? 0 : *s1 - *s2);
}



/*	Case insensitive strstr */
	char *
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
















/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
