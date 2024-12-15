/****************************************************************************
*																			*
*							HPACK Multi-System Archiver						*
*							===========================						*
*																			*
*					 	Output Intercept Handling Routines					*
*							DISPLAY.C  Updated 25/10/91						*
*																			*
* This program is protected by copyright and as such any use or copying of	*
*  this code for your own purposes directly or indirectly is highly uncool	*
*  					and if you do so there will be....trubble.				*
* 				And remember: We know where your kids go to school.			*
*																			*
*			Copyright 1991  Peter C.Gutmann.  All rights reserved			*
*																			*
****************************************************************************/

#include <string.h>
#include "defs.h"
#include "frontend.h"
#include "hpacklib.h"

/* Maximum possible screen width */

#define MAX_WIDTH	150

#if defined( __MSDOS__ ) && !defined( GUI )

/* Prototypes for functions in BIOSCALL.ASM */

BYTE getAttribute( void );
void writeChar( const char ch, const BYTE attribute );

/* Text mode information */

#define NORMAL_ATTR		0x07	/* Normal text */
#define BLINK_MASK		0x80	/* Mask for blink bit */
#define INTENSITY_MASK	0x08	/* Mask for intensity bit */

#endif /* __MSDOS__ && !GUI */

/* Command settings */

BOOLEAN doJustify, doCenter;
int leftMargin, rightMargin, tempIndent;

/* Internal status info */

#ifndef GUI
  int lineNo;
#endif /* !GUI */
int outPos, outWords;
char outBuffer[ MAX_WIDTH ];	/* Max possible screen width */
BOOLEAN breakFollows;
#if defined( __MSDOS__ ) && !defined( GUI )
  BYTE outAttrBuffer[ MAX_WIDTH ];
  BYTE attribute;
  BOOLEAN putSpace;				/* Whether there is a space queued */
#endif /* __MSDOS__ && !GUI */

/* Prototypes for various functions */

static void putWord( char *word, int length );

/* Get an optional numeric parameter */

static void getParam( char *str, int *theValue, const int defaultValue, \
					 const int minValue, const int maxValue )
	{
	char ch;
	int value = 0;
	BOOLEAN sign = FALSE, absolute = TRUE;

	/* Skip whitespace */
	while( ( ch = *str ) == ' ' || ch == '\t' )
		str++;

	/* Check for sign */
	if( ( ch = *str ) == '-' || ch == '+' )
		{
		str++;
		absolute = FALSE;	/* This is a delta not an absolute value */
		sign = ch - '+';	/* sign = ~( ch - '+' + 1 ); value *= sign */
		}

	/* Get digit */
	while( *str > '0' && *str < '9' )
		value = ( value * 10 ) + ( *str++ - '0' );

	/* Adjust sign */
	if( sign )
		value = -value;

	/* Do range check */
	if( value )
		if( absolute )
			*theValue = value;
		else
			*theValue += value;
	else
		*theValue = defaultValue;
	if( *theValue < minValue )
		*theValue = minValue;
	if( *theValue > maxValue )
		*theValue = maxValue;
	}

/* Force a line break */

static void doBreak( void )
	{
	BOOLEAN justifyValue = doJustify;

	doJustify = FALSE;		/* Make sure we don't justify half-full line */
	putWord( "", rightMargin );
	doJustify = justifyValue;
	outPos = leftMargin + tempIndent;
	tempIndent = 0;
	}

/* Process a formatting command.  Note that we can't do an immediate break in
   some cases when the command has an implied .br since it may be followed
   by further implied-.br commands, so we set the breakFollows flag which
   forces a break when we next output text */

#ifdef BIG_ENDIAN
  #define BOLDFACE		0x626F		/* 'bo' */
  #define PAGE_BREAK	0x6270		/* 'bp' */
  #define LINE_BREAK	0x6272		/* 'br' */
  #define CENTER		0x6365		/* 'ce' */
  #define FILL			0x6669		/* 'fi' */
  #define FLASH			0x666C		/* 'fl' */
  #define INDENT		0x6E6E		/* 'in' */
  #define ITALIC		0x6974		/* 'it' */
  #define INVERSE		0x6976		/* 'iv' */
  #define NOFILL		0x6E66		/* 'nf' */
  #define NORMAL		0x6E6F		/* 'no' */
  #define RIGHT_MARGIN	0x726D		/* 'rm' */
  #define TEMP_INDENT	0x7469		/* 'ti' */
#else
  #define BOLDFACE		0x6F62		/* 'bo' */
  #define PAGE_BREAK	0x7062		/* 'bp' */
  #define LINE_BREAK	0x7262		/* 'br' */
  #define CENTER		0x6563		/* 'ce' */
  #define FILL			0x6966		/* 'fi' */
  #define FLASH			0x6C66		/* 'fl' */
  #define INDENT		0x6E6E		/* 'in' */
  #define ITALIC		0x7469		/* 'it' */
  #define INVERSE		0x7669		/* 'iv' */
  #define NOFILL		0x666E		/* 'nf' */
  #define NORMAL		0x6F6E		/* 'no' */
  #define RIGHT_MARGIN	0x6D72		/* 'rm' */
  #define TEMP_INDENT	0x6974		/* 'ti' */
#endif /* BIG_ENDIAN */

static void processCommand( char *command )
	{
	int commandVal = *( ( int * ) command );

	switch( commandVal )
		{
#if defined( __MSDOS__ ) && !defined( GUI )
		case BOLDFACE:
			/* Boldface text */
			attribute |= INTENSITY_MASK;
			break;
#endif /* __MSDOS__ && !GUI */

		case PAGE_BREAK:
			/* Page break */
			doBreak();
#ifndef GUI
			while( lineNo )
				/* Print lots of blank lines */
				doBreak();
#endif /* !GUI */

		case LINE_BREAK:
			/* Line break */
			doBreak();
			break;

		case CENTER:
			/* Center text */
			doBreak();
			doCenter = TRUE;
			break;

		case FILL:
			/* Justify text */
			breakFollows = doJustify = TRUE;
			break;

#if defined( __MSDOS__ ) && !defined( GUI )
		case FLASH:
			/* Flashing text */
			attribute |= BLINK_MASK;
			break;
#endif /* __MSDOS__ && !GUI */

		case INDENT:
			/* Set left margin */
			breakFollows = TRUE;
			getParam( command + 2, &leftMargin, 0, -leftMargin, rightMargin );
			break;

#if defined( __MSDOS__ ) && !defined( GUI )
		case ITALIC:
			/* Italic text */
			attribute |= INTENSITY_MASK;	/* Treat as italic text */
			break;

		case INVERSE:
			/* Inverse text */
			attribute = ( attribute & 0x88 ) | ( ( attribute & 0x70 ) >> 4 ) | \
											   ( ( attribute & 0x07 ) << 4 );
			break;
#endif /* __MSDOS__ && !GUI */

		case NOFILL:
			/* Don't justify text */
			breakFollows = TRUE;
			doJustify = FALSE;
			break;

#if defined( __MSDOS__ ) && !defined( GUI )
		case NORMAL:
			/* Normal text mode */
			attribute = NORMAL_ATTR;
			break;
#endif /* __MSDOS__ && !GUI */

		case RIGHT_MARGIN:
			/* Set right margin */
			breakFollows = TRUE;
			rightMargin = screenWidth - rightMargin;
			getParam( command + 2, &rightMargin, 0, 0, screenWidth - leftMargin - 1 );
			rightMargin = screenWidth - rightMargin;
			break;

		case TEMP_INDENT:
			/* Temporary indent */
			breakFollows = TRUE;
			getParam( command + 2, &tempIndent, -leftMargin, -leftMargin, rightMargin );
			break;

		}
	}

/* Output a word with all sorts of fancy formatting */

static void putWord( char *word, int length )
	{
	int noBlanks, i;
	static BOOLEAN forwards = TRUE;

	/* See if there is a break in the pipeline */
	if( breakFollows )
		{
		breakFollows = FALSE;
		doBreak();
		}

#if defined( __MSDOS__ ) && !defined( GUI )
	/* Add a space if there's one queued.  We couldn't output this earlier
	   since there may have been an attribute change since then */
	if( putSpace )
		{
		putSpace = FALSE;
		outBuffer[ outPos++ ] = ' ';
		outAttrBuffer[ outPos - 1 ] = attribute;
		}
#endif /* __MSDOS__ && !GUI */

	/* Check whether line is full */
	if( outPos + length >= rightMargin )
		{
		outBuffer[ outPos ] = '\0';

		/* Delete trailing blank */
		if( outBuffer[ outPos - 1 ] == ' ' )
			outBuffer[ --outPos ] = '\0';

		/* Check for special text formatting */
		if( doCenter )
			{
			/* Pad out line with spaces to center text */
			for( i = 0; outBuffer[ i ] == ' '; i++ );
			noBlanks = ( ( rightMargin - leftMargin ) - strlen( outBuffer + i ) ) >> 1;
			while( noBlanks-- )
#ifdef GUI
				putchar( ' ' );
#else
				hputchar( ' ' );
#endif /* GUI */
			doCenter = FALSE;
			}
		else
			if( doJustify && outPos < rightMargin )
				{
				/* Spread out the words on this line.  Note the use of
				   memmove() instead of strcpy() since the source and
				   destination strings overlap */
				noBlanks = ( rightMargin - outPos );
				while( noBlanks )
					{
					if( forwards )
						{
						/* Starting from the beginning of the line, insert
						   an extra space where there is already one */
						for( i = leftMargin + 1; i <= rightMargin; i++ )
							if( outBuffer[ i ] == ' ' )
								{
								memmove( outBuffer + i + 1, outBuffer + i, rightMargin - i );
#if defined( __MSDOS__ ) && !defined( GUI )
								memmove( outAttrBuffer + i + 1, outAttrBuffer + i, rightMargin - i );
#endif /* __MSDOS__ && !GUI */
								noBlanks--;
								i++;	/* Skip new blank */
								if( !noBlanks )
									break;
								}
						}
					else
						/* Starting from the end of the line, insert an extra
						   space where there is already one */
						for( i = outPos - 1; i > leftMargin; i-- )
							if( outBuffer[ i ] == ' ' )
								{
								memmove( outBuffer + i + 1, outBuffer + i, rightMargin - i );
#if defined( __MSDOS__ ) && !defined( GUI )
								memmove( outAttrBuffer + i + 1, outAttrBuffer + i, rightMargin - i );
#endif /* __MSDOS__ && !GUI */
								noBlanks--;
								i--;	/* Skip new blank */
								if( !noBlanks )
									break;
								}

					/* Reverse direction for next round */
					forwards = !forwards;
					}
				}
		outBuffer[ screenWidth + 1 ] = '\0'; /* Make sure we never overrun screen */
#if defined( __MSDOS__ ) && !defined( GUI )
		noBlanks = strlen( outBuffer );
		for( i = 0; i < noBlanks; i++ )
			writeChar( outBuffer[ i ], outAttrBuffer[ i ] );
		if( noBlanks <= screenWidth )
			hputchar( '\n' );
#else
  #ifdef GUI
		printf( "%s\n", outBuffer );
  #else
		if( strlen( outBuffer ) == screenWidth + 1 )
			hprintf( "%s", outBuffer );	/* Line = screen width, no need for NL */
		else
			hprintf( "%s\n", outBuffer );
  #endif /* GUI */
#endif /* __MSDOS__ && !GUI */

		/* Indent next line of text */
		memset( outBuffer, ' ', leftMargin + tempIndent );
#if defined( __MSDOS__ ) && !defined( GUI )
		memset( outAttrBuffer, NORMAL_ATTR, leftMargin + tempIndent );
#endif /* __MSDOS__ && !GUI */
		outPos = leftMargin + tempIndent;
		outWords = 0;

#ifndef GUI
		/* Handle page breaks */
		lineNo++;
		if( lineNo >= screenHeight - 1 )
			{
			hgetch();
			lineNo = 0;
			}
#endif /* GUI */
		}
	strncpy( outBuffer + outPos, word, length );
#if defined( __MSDOS__ ) && !defined( GUI )
	memset( outAttrBuffer + outPos, attribute, length );
#endif /* __MSDOS__ && !GUI */
	outPos += length;
	if( outPos < rightMargin )
#if defined( __MSDOS__ ) && !defined( GUI )
		if( attribute != NORMAL_ATTR )
			/* We can't add the space now since the attribute may change by
			   the time it is due to be inserted, so we queue it for later
			   insertion */
			putSpace = TRUE;
		else
			{
			outBuffer[ outPos++ ] = ' ';
			outAttrBuffer[ outPos - 1 ] = attribute;
			}
#else
		outBuffer[ outPos++ ] = ' ';
#endif /* __MSDOS__ && !GUI */
	outWords++;
	}

/****************************************************************************
*																			*
*						Formatted Output Driver Code						*
*																			*
****************************************************************************/

int lineBufCount;
char lineBuffer[ MAX_WIDTH ];
BOOLEAN wasNewline;

/* Initialise the format routines */

void initOutFormatText( void )
	{
	/* Set up defaults */
	doJustify = FALSE;
	doCenter = FALSE;
	leftMargin = 0;
	rightMargin = screenWidth;

	/* Set up internal status info */
#ifndef GUI
	lineNo = 0;
#endif /* !GUI */
	outPos = 0;
	tempIndent = 0;
	lineBufCount = 0;
	wasNewline = TRUE;	/* At start we've just 'seen' a newline */
	breakFollows = FALSE;
#if defined( __MSDOS__ ) && !defined( GUI )
	attribute = getAttribute();
	putSpace = FALSE;
#endif /* __MSDOS__ && !GUI */
	}

/* Output a char via the format routines */

void outFormatChar( char ch )
	{
	/* Put a word in the line buffer */
	if( ch != '\r' && ch != '\n' && ch != 0x1A && \
		( ( wasNewline && *lineBuffer == '.' ) || ( ch != ' ' && ch != '\t' ) ) )
		{
		if( ch >= ' ' && ch <= '~' )
			lineBuffer[ lineBufCount++ ] = ch;

		/* Force line break if line is overly long */
		if( leftMargin + tempIndent + lineBufCount > rightMargin )
			goto lineBreak;
		}
	else
		{
lineBreak:
		lineBuffer[ lineBufCount ] = '\0';

		/* Process the data in the lineBuffer */
		if( wasNewline && *lineBuffer == '.' )
			processCommand( lineBuffer + 1 );
		else
			if( lineBufCount )
				putWord( lineBuffer, lineBufCount );
		lineBufCount = 0;

		/* Keep track of whether we've just seen a newline, after which
		   dotcommands are valid */
		if( wasNewline )
			wasNewline = FALSE;
		if( ch == '\r' || ch == '\n' || ch == 0x1A )
			wasNewline = TRUE;
		}
	}

/* Shutdown function for formatted text output */

void endOutFormatText( void )
	{
	/* Flush last line of text */
	putWord( "", rightMargin );
	}
