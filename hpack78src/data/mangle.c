/* Heavy BFI converter to mangle source code for non-ANSI compilers.
   Don't even try to run this on code which doesn't follow the HPACK
   formatting conventions - it will probably hang since I couldn't be
   bothered doing all sorts of fancy checking.  It does the job and no
   more */

#include <ctype.h>
#include <defs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char inBuffer[ 255 ];
FILE *outFile;

BOOLEAN doVoid = FALSE, doElif = FALSE, doUnindent = FALSE;
BOOLEAN doCont = FALSE, doStrcat = FALSE, doVoidPtr = FALSE;
BOOLEAN doConst = FALSE, doPragma = FALSE, doPrototype = FALSE;
BOOLEAN doFunction = FALSE, doMaxProto = FALSE;
BOOLEAN doEnum = FALSE;

/* Output count spaces if necessary */

static void spaces( int count )
	{
	int i = 0;

	if( doUnindent )
		return;
	while( count-- )
		putc( inBuffer[ i++ ], outFile );
	}

/* Unprotoize a string (eg "int foo, char *bar" -> "int, char *") */

static void unprotoize( char *string, int length )
	{
	int commaIndex = 0, nameStart;

	/* Check for pure void function - the de-voider hasn't hacked it yet */
	if( !strncmp( string, "void )", 6 ) )
		{
		memmove( string - 1, string + 5, strlen( string ) );
		return;
		}

	while( commaIndex < length )
		{
		/* Find end of next name in string */
		for( ; commaIndex < length && \
			 string[ commaIndex ] != ',' && string[ commaIndex ] != ')';
			 commaIndex++ );
		if( string[ commaIndex ] == ')' )
			commaIndex--;		/* Special case: last arg */

		/* Find start of next name in string */
		for( nameStart = commaIndex - 1; \
			 nameStart && !( string[ nameStart ] == ' ' || string[ nameStart ] == '*' ); \
			 nameStart-- );
		while( string[ nameStart ] == '*' )
			nameStart++;				/* Don't zap '*' */

		/* Zap out name. NB take advantage of multiple fenceposts from above */
		memmove( string + nameStart, string + commaIndex, strlen( string + commaIndex ) + 1 );
		length -= commaIndex - nameStart;
		}
	}

/* Unfunctionize a string (eg int foo( int a, int b ) -> int foo( a, b ) int a; int b;) */

char argTable[ 20 ][ 40 ];	/* Table to hold args */
int maxArg = 0;				/* Current high-water mark in argTable */

static void unfunctionize( char *string, int length )
	{
	int startIndex = 0, commaIndex = 0, nameStart, lastCommaIndex, offset = 0;
	char *strPos;

	/* Check if it's a pure void fn. (the de-voider hasn't attacked it yet) */
	if( !strncmp( string, "void )", 6 ) )
		return;

	maxArg = 0;
	while( startIndex < length )
		{
		/* Find end of next name in string */
		for( ; commaIndex < length && \
			 string[ commaIndex ] != ',' && string[ commaIndex ] != ')';
			 commaIndex++ );
		if( string[ commaIndex ] == ')' )
			commaIndex--;		/* Special case: last arg */

		/* Find start of next name in string */
		for( nameStart = commaIndex - 1; \
			 nameStart && !( string[ nameStart ] == ' ' || string[ nameStart ] == '*' ); \
			 nameStart-- );

		/* Find start of type in string */
		for( lastCommaIndex = nameStart - 1; \
			 lastCommaIndex && string[ lastCommaIndex ] != ','; \
			 lastCommaIndex-- );
		if( string[ lastCommaIndex ] == ',' )
			lastCommaIndex += 2;

		/* Check for special case of ellipses */
		if( string[ lastCommaIndex ] == '.' )
			{
			memmove( string + lastCommaIndex - 2, string + nameStart, strlen( string + commaIndex ) );
			break;
			}

		/* Store declaration for later */
		*argTable[ maxArg ] = '\t';
		strncpy( argTable[ maxArg ] + 1, string + lastCommaIndex, commaIndex - lastCommaIndex );
		strcpy( argTable[ maxArg++ ] + commaIndex - lastCommaIndex + 1, ";\n" );

		/* Zap out type. NB take advantage of multiple fenceposts from above */
		if( string[ commaIndex - 1 ] == ']' )
			{
			memmove( string + commaIndex - 2, string + commaIndex, strlen( string + commaIndex ) + 1 );
			offset = 2;
			}
		memmove( string + startIndex, string + nameStart + 1, strlen( string + nameStart ) );
		length -= ( nameStart + 1 ) - lastCommaIndex + offset;
		startIndex += commaIndex - nameStart + 1 - offset;
		commaIndex -= offset;
		offset = 0;
		}

	/* Zap any 'const's' and 'void *'s in the argTable if necessary */
	if( doConst )
		for( nameStart = 0; nameStart < maxArg; nameStart++ )
			if( !strncmp( argTable[ nameStart ] + 1, "const ", 6 ) )
				memmove( argTable[ nameStart ] + 1, argTable[ nameStart ] + 7, strlen( argTable[ nameStart ] ) );
	if( doVoidPtr )
		for( nameStart = 0; nameStart < maxArg; nameStart++ )
			if( ( strPos = strstr( argTable[ nameStart ] + 1, "void *" ) ) != NULL )
				strncpy( strPos, "char", 4 );
	}

/* Turn an enumerated type into a series of #defines */

void doUnenum( char *string, int length, const BOOLEAN hasTypedef )
	{
	int index = 0, endIndex = 0, value = 0;
	char ch;

	if( hasTypedef )
		{
		string += 15;
		length -= 15;		/* Skip "typedef enum { " */
		}
	else
		{
		string += 7;
		length -= 7;		/* Skip "enum { " */
		}

	while( index < length )
		{
		/* Find next enumeration */
		while( ( ch = string[ endIndex ] ) != ' ' && ch != '\t' && ch )
			endIndex++;
		string[ ( string[ endIndex - 1 ] == ',' ) ? endIndex - 1 : endIndex ] = '\0';

		/* Check if it has a value */
		if( string[ endIndex + 1 ] == '=' )
			{
			endIndex += 3;
			value = atoi( string + endIndex );
			while( ( ch = string[ endIndex ] ) != ' ' && ch != '\t' )
				endIndex++;
			}

		fprintf( outFile, "#define %s\t%d\n", string + index, value++ );

		index = ++endIndex;			/* Skip space */
		}

	/* Check if we have to declare an equivalent integer type */
	if( hasTypedef )
		{
		while( string[ endIndex ] != ';' )
			endIndex++;
		string[ endIndex ] = '\0';
		fprintf( outFile, "\n#define %s\tint\n", string + index + 2 );	/* +2 = "} " */
		}
	}

void main( int argc, char *argv[] )
	{
	FILE *inFile;
	int index, endIndex, elifCount = 0;
	BOOLEAN inElifBlock = FALSE, hasHash = FALSE, isMacroCont = FALSE;
	char *strPtr, ch;

	/* Check args */
	if( argc != 3 )
		{
		puts( "Usage: MANGLE -cefiknprstvw <warez>" );
		exit( ERROR );
		}

	/* Handle options */
	if( !strcmp( argv[ 1 ], "-ward" ) )
		doCont = doElif = doUnindent = doConst = doVoidPtr = doStrcat = \
		doVoid = doPragma = doPrototype = doFunction = doMaxProto = \
		doEnum = TRUE;
	if( strchr( argv[ 1 ], 'c' ) )
		doCont = TRUE;
	if( strchr( argv[ 1 ], 'e' ) )
		doElif = TRUE;
	if( strchr( argv[ 1 ], 'f' ) )
		doFunction = TRUE;
	if( strchr( argv[ 1 ], 'i' ) )
		doUnindent = TRUE;
	if( strchr( argv[ 1 ], 'k' ) )
		doConst = TRUE;
	if( strchr( argv[ 1 ], 'n' ) )
		doEnum = TRUE;
	if( strchr( argv[ 1 ], 'p' ) )
		doVoidPtr = TRUE;
	if( strchr( argv[ 1 ], 'r' ) )
		doPragma = TRUE;
	if( strchr( argv[ 1 ], 's' ) )
		doStrcat = TRUE;
	if( strchr( argv[ 1 ], 't' ) )
		doPrototype = TRUE;
	if( strchr( argv[ 1 ], 'v' ) )
		doVoid = TRUE;
	if( strchr( argv[ 1 ], 'w' ) )
		doMaxProto = TRUE;

	/* Yo dude, got any warez? */
	if( ( inFile = fopen( argv[ 2 ], "r" ) ) == NULL )
		{
		printf( "No warez: %s\n", argv[ 2 ] );
		exit( ERROR );
		}
#ifdef TEST_VERSION
	outFile = stdout;
#else
	if( ( outFile = fopen( "tmp.$$$", "w" ) ) == NULL )
		{
		printf( "Can't open output file" );
		exit( ERROR );
		}
#endif /* TEST_VERSION */

#if defined( __MSDOS__ ) && !defined( TEST_VERSION )
	/* DOS will happily reopen the same file multiple times, so we have to
	   create a lockfile... */
	{
	char lockFilename[ 80 ];
	FILE *lockFilePtr;
	int i;
	char ch;

	strcpy( lockFilename, argv[ 2 ] );
	for( i = strlen( lockFilename ); \
		 i && lockFilename[ i ] != '.' && lockFilename[ i ] != '\\';
		 i-- );
	if( lockFilename[ i ] != '.' )
		strcpy( lockFilename + strlen( lockFilename ), ".~~~" );
	else
		{
		/* Preserve 1-letter suffix */
		if( ( ch = lockFilename[ i + 1 ] ) == '~' )
			{
			/* This may be a lockfile we're processing */
			fclose( outFile );
			unlink( "tmp.$$$" );
			exit( ERROR );
			}
		strcpy( lockFilename + i, ".~~~" );
		lockFilename[ i + 3 ] = ch;
		}

	if( ( lockFilePtr = fopen( lockFilename, "r" ) ) != NULL )
		{
		/* Don't process same file twice */
		fclose( lockFilePtr );
		fclose( outFile );
		unlink( "tmp.$$$" );
		exit( ERROR );
		}

	/* Create the lockfile */
	lockFilePtr = fopen( lockFilename, "w" );
	fclose( lockFilePtr );
	}
#endif /* __MSDOS__ && !TEST_VERSION */

	/* Mangle the file one line at a time */
	while( fgets( inBuffer, 250, inFile ) != NULL )
		{
		if( inBuffer[ strlen( inBuffer ) - 1 ] == '\n' )
			inBuffer[ strlen( inBuffer ) - 1 ] = '\0';

		/* Skip blank lines now */
		if( !*inBuffer )
			{
			putc( '\n', outFile );
			continue;
			}

		for( index = 0; \
			 index < 250 && ( ch = inBuffer[ index ] ) == '\0' || ch == ' ' || ch == '\t'; \
			 index++ );

		/* Skip garbage at start of line */
		while( !strncmp( inBuffer + index, "inline ", 7 ) || \
			   !strncmp( inBuffer + index, "static ", 7 ) || \
			   !strncmp( inBuffer + index, "extern ", 7 ) )
			index += 7;

		/* Check for preprocessor stuff */
		if( inBuffer[ index ] == '#' )
			{
			/* Check for #pragma */
			if( doPragma && !strncmp( inBuffer + index, "#pragma", 7 ) )
				/* Skip it */
				continue;

			hasHash = TRUE;			/* Tassy green */
			if( doElif && !strncmp( inBuffer + index, "#elif", 5 ) )
				{
				inElifBlock = TRUE;
				elifCount++;
				memmove( inBuffer + index, inBuffer + index + 2, strlen( inBuffer ) );
				spaces( index );
				fputs( "#else\n", outFile );
				strncpy( inBuffer + index, "#if ", 4 );
				}
			else
				if( inElifBlock && !strncmp( inBuffer + index, "#endif", 6 ) )
					{
					while( elifCount-- )
						{
						spaces( index );
						fputs( "#endif\n", outFile );
						}
					elifCount = 0;
					inElifBlock = FALSE;
					}

			/* Get rid of indenting of '#...' */
			spaces( index );
			fputs( inBuffer + index, outFile );
			putc( '\n', outFile );
			continue;
			}

		/* Check for function prototypes.  We check for:
		   <noise> + <type> + opt-'*' + <single_word> + '(' + ... + ')'
		   and delete the '...' part */
		if( doPrototype )
			{
			int subIndex = index;

			/* Do serious BFI search for return type */
			if( !strncmp( inBuffer + index, "int ", 4 ) || \
				!strncmp( inBuffer + index, "char ", 5 ) || \
				!strncmp( inBuffer + index, "long ", 5 ) || \
				!strncmp( inBuffer + index, "void ", 5 ) || \
				!strncmp( inBuffer + index, "BYTE ", 5 ) || \
				!strncmp( inBuffer + index, "WORD ", 5 ) || \
				!strncmp( inBuffer + index, "LONG ", 5 ) || \
				!strncmp( inBuffer + index, "BOOLEAN ", 8 ) || \
				!strncmp( inBuffer + index, "DATA_STATUS ", 12 ) || \
				!strncmp( inBuffer + index, "DIR ", 4 ) || \
				!strncmp( inBuffer + index, "DIRECT ", 7 ) || \
				!strncmp( inBuffer + index, "ATTR ", 5 ) || \
				!strncmp( inBuffer + index, "FILEHDRLIST ", 12 ) || \
				!strncmp( inBuffer + index, "DIRHDRLIST ", 11 ) )
				{
				/* Skip to start of function name */
				while( inBuffer[ subIndex++ ] != ' ' );
				if( inBuffer[ subIndex ] == '*' )
					subIndex++;

				while( ( ch = inBuffer[ subIndex++ ] ) != '\0' && \
					   ( ( ch >= 'A' && ch <= 'Z' ) || \
						 ( ch >= 'a' && ch <= 'z' ) || \
						 ( ch >= '0' && ch <= '9' ) || ch == '_' ) );

				if( ch == '(' )
					{
					int endIndex = subIndex, subSubIndex;

					/* We've matched <noise> <type> opt-'*' <single-word> */
					while( ( ch = inBuffer[ endIndex++ ] ) != '\0' && \
						   ch != ')' && ch != '\\' );

					/* Check for multi-line function */
					if( ch == '\\' )
						{
						fgets( inBuffer + endIndex - 1, 250, inFile );
						inBuffer[ strlen( inBuffer ) - 1 ] = '\0';
						for( subSubIndex = endIndex; \
							 inBuffer[ subSubIndex ] == ' ' || \
							 inBuffer[ subSubIndex ] == '\t';
							 subSubIndex++ );
						memmove( inBuffer + endIndex - 1, inBuffer + subSubIndex, strlen( inBuffer + subSubIndex ) + 1 );
						for( endIndex = subSubIndex; \
							 inBuffer[ endIndex ] && inBuffer[ endIndex ] != ')'; \
							 endIndex++ );
						endIndex++;			/* Skip ')' */
						}

					if( inBuffer[ endIndex ] == ';' )
						{
						/* It's a prototype (finally!), zap it */
						if( doPrototype )
							if( doMaxProto )
								memmove( inBuffer + subIndex, inBuffer + endIndex - 1, \
										 strlen( inBuffer + endIndex ) + 2 );
							else
								unprotoize( inBuffer + subIndex + 1, endIndex - subIndex - 3 );
						}
					else
						{
						/* It's a function, convert to K&R style */
						if( doFunction )
							unfunctionize( inBuffer + subIndex + 1, endIndex - subIndex - 3 );
						}
					}
				}
			}

		/* Check for 'const's */
		if( doConst )
			while( ( strPtr = strstr( inBuffer + index, "const" ) ) != NULL )
				strcpy( strPtr, strPtr + 6 );

		/* Check for 'void' return types */
		if( !strncmp( inBuffer + index, "void *", 6 ) )
			{
			/* Change 'void *' return to 'char *' */
			if( doVoidPtr )
				strncpy( inBuffer + index, "char", 4 );
			}
		else
			if( doVoid && !strncmp( inBuffer + index, "void", 4 ) )
				{
				/* Change void return to int */
				memmove( inBuffer + index, inBuffer + index + 1, strlen( inBuffer ) );
				strncpy( inBuffer + index, "int", 3 );
				}

		/* Change 'void *' into 'char *' */
		if( ( strPtr = strstr( inBuffer + index, "void *" ) ) != NULL )
			{
			if( doVoidPtr )
				strncpy( strPtr, "char", 4 );
			}
		else
			/* Change foo( void ) into foo() */
			if( doVoid && ( strPtr = strstr( inBuffer + index, "void" ) ) != NULL )
				memmove( strPtr - 1, strPtr + 5, strlen( strPtr ) );

		/* Check for enums */
		if( doEnum )
			if( !strncmp( inBuffer + index, "enum ", 5 ) || \
				!strncmp( inBuffer + index, "typedef enum ", 13 ) )
				{
				int endIndex = index, subSubIndex;
				BOOLEAN hasTypedef = strncmp( inBuffer + index, "enum ", 5 );

				/* Find end of declaration or continuation char */
				while( ( ch = inBuffer[ endIndex ] ) != '\0' && \
					   ch != '}' && ch != '\\' )
					endIndex++;

				/* Check for multi-line declaration */
				if( ch == '\\' )
					{
					fgets( inBuffer + endIndex, 250, inFile );
					inBuffer[ strlen( inBuffer ) ] = '\0';
					for( subSubIndex = endIndex; \
						 inBuffer[ subSubIndex ] == ' ' || \
						 inBuffer[ subSubIndex ] == '\t';
						 subSubIndex++ );
					memmove( inBuffer + endIndex, inBuffer + subSubIndex, strlen( inBuffer + subSubIndex ) + 1 );
					for( endIndex = subSubIndex; \
						 inBuffer[ endIndex ] && inBuffer[ endIndex ] != '}'; \
						 endIndex++ );
					}

				doUnenum( inBuffer + index, endIndex, hasTypedef );
				continue;
				}

		/* Check for continuation character strings */
		for( endIndex = strlen( inBuffer );
			 endIndex && ( ch = inBuffer[ endIndex ] ) == ' ' || ch == '\t';
			 endIndex-- );
		if( endIndex )
			endIndex--;
		if( inBuffer[ endIndex ] == '\\' )
			{
			/* Check for macro with continuation chars */
			if( hasHash )
				isMacroCont = TRUE;
			if( isMacroCont )
				{
				/* Leave it alone */
				fputs( inBuffer, outFile );
				putc( '\n', outFile );
				continue;
				}

			/* Check for string continuation */
			if( endIndex > 2 && inBuffer[ endIndex - 2 ] == '\"' )
				{
				if( doStrcat )
					{
					endIndex -= 2;
					fgets( inBuffer + endIndex, 250, inFile );
					inBuffer[ endIndex + strlen( inBuffer + endIndex ) - 1 ] = '\0';
					for( index = endIndex; inBuffer[ index ] != '\"'; \
						 index++ );
					memmove( inBuffer + endIndex, inBuffer + index + 1, strlen( inBuffer ) );
					}
				}
			else
				if( doCont )
					/* Zap '\' at end of line */
					inBuffer[ endIndex ] = '\0';
			}

		hasHash = FALSE;		/* Been raided.... */
		isMacroCont = FALSE;
		fputs( inBuffer, outFile );
		putc( '\n', outFile );

		/* If there are function args floating around, output them now */
		if( maxArg )
			{
			for( index = 0; index < maxArg; index++ )
				fputs( argTable[ index ], outFile );
			maxArg = 0;
			}
		}

	/* Close files */
	fclose( inFile );
	fclose( outFile );

#ifndef TEST_VERSION
	/* Delete old file and rename new one */
	unlink( argv[ 2 ] );
	rename( "tmp.$$$", argv[ 2 ] );
#endif /* TEST_VERSION */
	}
