/*	config.c  - config file parser by Peter Gutmann
	Parses config file for PGP

	Modified 24 Jun 92 - HAJK
	Misc fixes for VAX C restrictions.

*/

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "usuals.h"
#include "fileio.h"
#include "pgp.h"
#include "config.h"
#include "charset.h"

static int lookup( char *key, int keyLength, char *keyWords[], int range );
static int extractToken( char *buffer, int *endIndex, int *length );
static int getaString( char *buffer, int *endIndex );
static int getAssignment( char *buffer, int *endIndex, INPUT_TYPE settingType );
static void processAssignment( int intrinsicIndex );

/* The external config variables we can set here are referenced in pgp.h */

/* Return values */

#define ERROR	-1
#define OK		0

/* The types of error we check for */

enum { NO_ERROR, ILLEGAL_CHAR_ERROR, LINELENGTH_ERROR };

#define CPM_EOF			0x1A	/* ^Z = CPM EOF char */

#define MAX_ERRORS		3		/* Max.no.errors before we give up */

#define LINEBUF_SIZE	100		/* Size of input buffer */

static int line;				/* The line on which an error occurred */
static int errCount;			/* Total error count */
static boolean hasError;		/* Whether this line has an error in it */

/* The settings parsed out by getAssignment() */

static char str[ LINEBUF_SIZE ];
static int value;
static boolean flag;
static char *errtag;	/* prefix for printing error messages */
static char optstr[100];	/* option being processed */

/* A .CFG file roughly follows the format used in the world-famous HPACK
   archiver and is as follows:

	- Leading spaces/tabs (whitespace) are ignored.

	- Lines with a '#' as the first non-whitespace character are treated as
	  comment lines.

	- All other lines are treated as config options for the program.

	- Lines may be terminated by either linefeeds, carriage returns, or
	  carriage return/linefeed pairs (the latter being the DOS default
	  method of storing text files).

	- Config options have the form:

	  <option> '=' <setting>

	  where <setting> may be 'on', 'off', a numeric value, or a string
	  value.

	- If strings have spaces or the '#' character inside them they must be
	  surrounded by quote marks '"' */

/* Intrinsic variables */

#define NO_INTRINSICS		(sizeof(intrinsics) / sizeof(intrinsics[0]))

enum
{	ARMOR, COMPRESS, SHOWPASS, KEEPBINARY, LANGUAGE,
	MYNAME, TEXTMODE, TMP, TZFIX, VERBOSE, BAKRING,
	ARMORLINES, COMPLETES_NEEDED, MARGINALS_NEEDED, PAGER,
	CERT_DEPTH, CHARSET, CLEAR, SELF_ENCRYPT,
	INTERACTIVE, PKCS_COMPAT,
	/* options below this line can only be used as command line
	 * "long" options */
#define CONFIG_INTRINSICS	BATCHMODE
	BATCHMODE, FORCE
};

static char *intrinsics[] =
{	"ARMOR", "COMPRESS", "SHOWPASS", "KEEPBINARY", "LANGUAGE",
	"MYNAME", "TEXTMODE", "TMP", "TZFIX", "VERBOSE", "BAKRING",
	"ARMORLINES", "COMPLETES_NEEDED", "MARGINALS_NEEDED", "PAGER",
	"CERT_DEPTH", "CHARSET", "CLEARSIG", "ENCRYPTTOSELF", 
	"INTERACTIVE", "PKCS_COMPAT",
	/* command line only */
	"BATCHMODE", "FORCE",
};

static INPUT_TYPE intrinsicType[] =
{	BOOL, BOOL, BOOL, BOOL, STRING,
	STRING, BOOL, STRING, NUMERIC, NUMERIC, STRING,
	NUMERIC, NUMERIC, NUMERIC, STRING,
	NUMERIC, STRING, BOOL, BOOL,
	BOOL, NUMERIC,
	/* command line only */
	BOOL, BOOL,
};

/* Possible settings for variables */

#define NO_SETTINGS			2

static char *settings[] = { "OFF", "ON" };


/* Search a list of keywords for a match */

static int lookup( char *key, int keyLength, char *keyWords[], int range )
{
	int indx, pos = 0, matches = 0;

	strncpy(optstr, key, keyLength);
	optstr[keyLength] = '\0';
	/* Make the search case insensitive */
	for( indx = 0; indx < keyLength; indx++ )
		key[ indx ] = to_upper( key[ indx ] );

	for( indx = 0; indx < range; indx++ )
		if( !strncmp( key, keyWords[ indx ], keyLength ) )
		{	if (strlen(keyWords[indx]) == keyLength)
				return indx;	/* exact match */
			pos = indx;
			++matches;
		}
	
	switch (matches)
	{	case 0: fprintf(stderr, "%s: unknown keyword: \"%s\"\n", errtag, optstr); break;
		case 1: return pos;
		default: fprintf(stderr, "%s: \"%s\" is ambiguous\n", errtag, optstr);
	}
	return ERROR;
}

/* Extract a token from a buffer */
static int extractToken( char *buffer, int *endIndex, int *length )
{
	int indx = 0, tokenStart;
	char ch;

	/* Skip whitespace */
	for( ch = buffer[ indx ]; ch && ( ch == ' ' || ch == '\t' ); ch = buffer[ indx ] )
		indx++;
	tokenStart = indx;

	/* Find end of setting */
	while( indx < LINEBUF_SIZE && ( ch = buffer[ indx ] ) != '\0' && ch != ' ' && ch != '\t' )
		indx++;
	*endIndex += indx;
	*length = indx - tokenStart;

	/* Return start position of token in buffer */
	return( tokenStart );
}


/* Get a string constant */
static int getaString( char *buffer, int *endIndex )
	{
	boolean noQuote = FALSE;
	int stringIndex = 0, bufIndex = 1;
	char ch = *buffer;

	/* Skip whitespace */
	while( ch && ( ch == ' ' || ch == '\t' ) )
		ch = buffer[ bufIndex++ ];

	/* Check for non-string */
	if( ch != '\"' )
		{
		*endIndex += bufIndex;

		/* Check for special case of null string */
		if( !ch )
			{
			*str = '\0';
			return( OK );
			}

		/* Use nasty non-rigorous string format */
		noQuote = TRUE;
		}

	/* Get first char of string */
	if( !noQuote )
		ch = buffer[ bufIndex++ ];

	/* Get string into string */
	while( ch && ch != '\"' )
		{
		/* Exit on '#' if using non-rigorous format */
		if( noQuote && ch == '#' )
			break;

		str[ stringIndex++ ] = ch;
		ch = buffer[ bufIndex++ ];
		}

	/* If using the non-rigorous format, stomp trailing spaces */
	if( noQuote )
		while( stringIndex > 0 && str[ stringIndex - 1 ] == ' ' )
			stringIndex--;

	str[ stringIndex++ ] = '\0';
	*endIndex += bufIndex;

	/* Check for missing string terminator */
	if( ch != '\"' && !noQuote )
		{
		if (line)
			fprintf(stderr, "%s: unterminated string in line %d\n", errtag, line );
		else
			fprintf(stderr, "unterminated string: '\"%s'\n", str );
		hasError = TRUE;
		errCount++;
		return( ERROR );
		}

	return( OK );
	}

/* Get an assignment to an intrinsic */
static int getAssignment( char *buffer, int *endIndex, INPUT_TYPE settingType )
{
	int settingIndex = 0, length;

	buffer += extractToken( buffer, endIndex, &length );

	/* Check for an assignment operator */
	if ( *buffer != '=' )
	{
		if (line)
			fprintf(stderr, "%s: expected '=' in line %d\n", errtag, line );
		else
			fprintf(stderr, "%s: expected '=' after \"%s\"\n", errtag, optstr);
		hasError = TRUE;
		errCount++;
		return( ERROR );
	}
	buffer++;		/* Skip '=' */

	buffer += extractToken( buffer, endIndex, &length );

	switch( settingType )
	{
		case BOOL:
			/* Check for known intrinsic - really more general than just
			   checking for TRUE or FALSE */
			if( ( settingIndex = lookup( buffer, length, settings, NO_SETTINGS ) ) == ERROR )
			{
				hasError = TRUE;
				errCount++;
				return( ERROR );
			}

			flag = ( settingIndex == 0 ) ? FALSE : TRUE;
			break;

		case STRING:
			/* Get a string */
			getaString( buffer, &length );
			break;

		case NUMERIC:
			/* Get numeric input.  Error checking is a pain since atoi()
				has no real equivalent of NAN */
			value = atoi( buffer );
			break;
	}

	return( settingIndex );
}

/* Process an assignment */

static void processAssignment( int intrinsicIndex )
	{
	if( !hasError )
		switch( intrinsicIndex )
			{
			case ARMOR:
				emit_radix_64 = flag;
				break;

			case COMPRESS:
				compress_enabled = flag;
				break;

			case SHOWPASS:
				showpass = flag;
				break;

			case KEEPBINARY:
				keepctx = flag;
				break;

			case LANGUAGE:
				strncpy(language, str, 15);
				break;

			case BAKRING:
				strcpy(floppyring, str);
				break;

			case MYNAME:
				strcpy(my_name, str);
				break;

			case TEXTMODE:
				if( flag )
					literal_mode = MODE_TEXT;
				else
					literal_mode = MODE_BINARY;
				break;

			case TMP:
				/* directory pathname to store temp files */
				settmpdir(str);
				break;

			case TZFIX:
				/* How many hours to add to time() to get GMT. */
				/* Compute seconds from hours to shift to GMT: */
				timeshift = 3600L * (long) value;
				break;

			case VERBOSE:
				switch (value)
				{
					case 0: quietmode = TRUE; verbose = FALSE; break;
					case 1: quietmode = FALSE; verbose = FALSE; break;
					case 2: quietmode = FALSE; verbose = TRUE; break;
					default: quietmode = FALSE; verbose = FALSE;
				}
				break;

			case ARMORLINES:
				pem_lines = value;
				break;

			case MARGINALS_NEEDED:
				marg_min = value;
				if (marg_min < 1)
					marg_min = 1;
				break;

			case COMPLETES_NEEDED:
				compl_min = value;
				if (compl_min < 1)
					compl_min = 1;
				if (compl_min > 4)
					compl_min = 4;
				break;

			case CERT_DEPTH:
				max_cert_depth = value;
				if (max_cert_depth < 0)
					max_cert_depth = 0;
				if (max_cert_depth > 8)
					max_cert_depth = 8;
				break;

			case PAGER:
				strcpy(pager, str);
				break;

			case CHARSET:
				strcpy(charset, str);
				break;

			case CLEAR:
				clear_signatures = flag;
				break;
				
			case SELF_ENCRYPT:
				encrypt_to_self = flag;
				break;
				
			case INTERACTIVE:
				interactive_add = flag;
				break;
				
			case BATCHMODE: batchmode = flag; break;
			case FORCE: force_flag = flag; break;
			case PKCS_COMPAT: pkcs_compat = value; break;
			}
	}

/* Process an option on a line by itself.  This expects options which are
   taken from the command-line, and is less finicky about errors than the
   config-file version */

int processConfigLine( char *option )
	{
	int indx, intrinsicIndex;
	char ch;

	/* Give it a pseudo-linenumber of 0 */
	line = 0;

	errtag = "pgp";
	errCount = 0;
	for( indx = 0;
		 indx < LINEBUF_SIZE && ( ch = option[ indx ] ) != '\0' &&
				ch != ' ' && ch != '\t' && ch != '=';
		 indx++ );
	if( ( intrinsicIndex = lookup( ( char * ) option, indx, intrinsics, NO_INTRINSICS ) ) == ERROR )
		return -1;
	if (option[indx] == '\0' && intrinsicType[intrinsicIndex] == BOOL)
	{	/* boolean option, no '=' means TRUE */
		flag = TRUE;
		processAssignment(intrinsicIndex);
	}
	else /* Get the value to set to, either as a string, a
		    numeric value, or a boolean flag */
		if (getAssignment( ( char * ) option + indx, &indx, intrinsicType[ intrinsicIndex ] ) != ERROR)
			processAssignment( intrinsicIndex );
	return(errCount ? -1 : 0);
}

/* Process a config file */
int processConfigFile( char *configFileName )
{
	FILE *configFilePtr;
	int ch = 0, theCh;
	int errType, errPos = 0, lineBufCount, intrinsicIndex;
	int indx;
	char inBuffer[ LINEBUF_SIZE ];

	line = 1;
	errCount = 0;
	errtag = "config.txt";

	if( ( configFilePtr = fopen( configFileName, FOPRTXT ) ) == NULL )
	{
		fprintf(stderr, "Cannot open configuration file %s\n", configFileName );
		return( OK );	/* treat like empty config file */
	}

	/* Process each line in the configFile */
	while( ch != EOF )
	{
		/* Skip whitespace */
		while( ( ( ch = getc( configFilePtr ) ) == ' ' || ch == '\t' ) && ch != EOF )
		;

		/* Get a line into the inBuffer */
		hasError = FALSE;
		lineBufCount = 0;
		errType = NO_ERROR;
		while( ch != '\r' && ch != '\n' && ch != CPM_EOF && ch != EOF )
		{
			/* Check for an illegal char in the data */
			if( ( ch < ' ' || ch > '~' ) && ch != '\r' && ch != '\n' &&
				ch != ' ' && ch != '\t' && ch != CPM_EOF && ch != EOF )
			{
				if( errType == NO_ERROR )
					/* Save position of first illegal char */
					errPos = lineBufCount;
				errType = ILLEGAL_CHAR_ERROR;
			}

			/* Make sure the path is of the correct length.  Note that the
			   code is ordered so that a LINELENGTH_ERROR takes precedence over
			   an ILLEGAL_CHAR_ERROR */
			if( lineBufCount > LINEBUF_SIZE )
				errType = LINELENGTH_ERROR;
			else
				inBuffer[ lineBufCount++ ] = ch;

			if( ( ch = getc( configFilePtr ) ) == '#' )
			{
				/* Skip comment section and trailing whitespace */
				while( ch != '\r' && ch != '\n' && ch != CPM_EOF && ch != EOF )
					ch = getc( configFilePtr );
				break;
			}
		}

		/* Skip trailing whitespace and add der terminador */
		while(lineBufCount && (( theCh = inBuffer[ lineBufCount - 1 ] ) == ' ' || theCh == '\t' ))
			lineBufCount--;

		inBuffer[ lineBufCount ] = '\0';

		/* Process the line unless its a blank or comment line */
		if( lineBufCount && *inBuffer != '#' )
		{
			switch( errType )
			{
				case LINELENGTH_ERROR:
					fprintf(stderr, "%s: line '%.30s...' too long\n", errtag, inBuffer );
					errCount++;
					break;

				case ILLEGAL_CHAR_ERROR:
					fprintf(stderr, "> %s\n  ", inBuffer );
					fprintf(stderr, "%*s^\n", errPos, ""); 
					fprintf(stderr, "%s: bad character in command on line %d\n", errtag, line );
					errCount++;
					break;

				default:
					for( indx = 0;
						 indx < LINEBUF_SIZE && ( ch = inBuffer[ indx ] ) != '\0'
								&& ch != ' ' && ch != '\t' && ch != '=';
						 indx++ )
						 	;
					if( ( intrinsicIndex = lookup( inBuffer, indx, intrinsics, CONFIG_INTRINSICS ) ) == ERROR )
					{
						errCount++;
					}
					else
					{
						/* Get the value to set to, either as a string, a
						   numeric value, or a boolean flag */
						getAssignment( inBuffer + indx, &indx, intrinsicType[ intrinsicIndex ] );
						processAssignment( intrinsicIndex );
					}
			}
		}

		/* Handle special-case of ^Z if configFile came off an MSDOS system */
		if( ch == CPM_EOF )
			ch = EOF;

		/* Exit if there are too many errors */
		if( errCount >= MAX_ERRORS )
			break;

		line++;
	}

	fclose( configFilePtr );

	/* Exit if there were errors */
	if( errCount )
	{
		fprintf(stderr, "%s: %s%d error(s) detected\n\n", configFileName, ( errCount >= MAX_ERRORS ) ?
				"Maximum level of " : "", errCount );
		return( ERROR );
	}

	return( OK );
}
