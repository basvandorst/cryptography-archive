/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: pgpStrings.h,v 1.6.12.1 1998/11/12 03:18:43 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpStrings_h	/* [ */
#define Included_pgpStrings_h

#include "pgpBase.h"

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif


/*____________________________________________________________________________
	Return a pointer to the file name extension in the name.  The pointer
	does NOT include the period.
	
	File name extensions must be 3 chars or less and not be the whole name.
	
	Return NULL if extension not found
____________________________________________________________________________*/
const char *	PGPGetFileNameExtension( const char * name );

const char *	PGPGetEndOfBaseFileName( const char * name );

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
PGPError	PGPNewNumberFileName( const char * baseName,
				const char * separator, PGPUInt32 number,
				PGPSize maxSize, char *outName );

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
PGPError	PGPVerifyNumberFileName( const char * origName,
				const char * separator, const char * fileName,
				PGPSize maxSize, PGPBoolean * outValid,
				PGPUInt32 * outNumber );

/*____________________________________________________________________________
	Case insensitive string compare
____________________________________________________________________________*/
int		pgpCompareStringsIgnoreCase( const char *str1, const char *str2 );
int		pgpCompareStringsIgnoreCaseN( const char *str1, const char *str2,
			int length);
	
char *	FindSubStringNoCase( const char * mainString,
					const char * subString);


#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpStrings_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/