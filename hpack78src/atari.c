/****************************************************************************
*																			*
*							HPACK Multi-System Archiver						*
*							===========================						*
*																			*
*						   	  Atari-Specific Routines						*
*							 ATARI.C  Updated 26/09/92						*
*																			*
* This program is protected by copyright and as such any use or copying of	*
*  this code for your own purposes directly or indirectly is highly uncool	*
*  					and if you do so there will be....trubble.				*
* 				And remember: We know where your kids go to school.			*
*																			*
*			Copyright 1992  Peter C Gutmann.  All rights reserved			*
*																			*
****************************************************************************/

/* Note: This code predates the full Atari ST port - see the note in the
		 readme file for the full story */

#include <osbind.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "defs.h"
#include "frontend.h"
#include "system.h"
#include "io/hpackio.h"
#include "io/fastio.h"

/* GemDOS <-> Unix date format conversion routines */

void unixToDos( LONG unixTime, WORD *dosTime, WORD *dosDate )
	{
	struct tm *tmtime;

	/* Break the Unix time into its component values */
	tmtime = localtime( ( time_t * ) &unixTime );

	/* Build up the equivalent GemDOS values, with conversion for months
	   starting at 0 and years since 1980 */
	*dosTime = ( tmtime->tm_sec >> 1 ) | ( tmtime->tm_min << 5 ) | \
			   ( tmtime->tm_hour << 11 );
	*dosDate = ( tmtime->tm_mday ) | ( ( tmtime->tm_mon + 1 ) << 5 ) | \
			   ( ( tmtime->tm_year - 80 ) << 9 );
	}

LONG dosToUnix( WORD dosDate, WORD dosTime )
	{
	struct tm timeBuf;

	/* Break up the GemDOS time into its component values, with conversion
	   for months starting at 0 and years since 1980 */
	timeBuf.tm_sec = ( dosTime & 0x001F ) << 1;
	timeBuf.tm_min = ( dosTime >> 5 ) & 0x003F;
	timeBuf.tm_hour = ( dosTime >> 11 );
	timeBuf.tm_mday = dosDate & 0x001F;
	timeBuf.tm_mon = ( ( dosDate >> 5 ) & 0x000F ) - 1;
	timeBuf.tm_year = ( dosDate >> 9 ) + 80;
	timeBuf.tm_isdst = -1;					/* Daylight saving indeterminate */
	timeBuf.tm_wday = 0;
	timeBuf.tm_yday = 0;

	return( mktime( &timeBuf ) );
	}

/****************************************************************************
*																			*
*								SYSTEM Functions							*
*																			*
****************************************************************************/

/* Set file time */

#define SETDATE		0

void setFileTime( const char *fileName, const LONG time )
	{
	FD theFD;
	WORD dosDate, dosTime;
	BYTE timeBuf[ 4 ];

	/* Convert Unix time to correct format for Ftimedat(), with the GemDOS
	   date in the first two bytes and the time in the second two bytes of a
	   buffer */
	unixToDos( time, &dosDate, &dosTime );
	mputWord( timeBuf, dosDate );
	mputWord( timeBuf + 2, dosTime );

	/* Open the file and set its timestamp */
	if( ( theFD = hopen( fileName, O_RDONLY ) ) == ERROR )
		return;
	Ftimedat( timeBuf, theFD, SETDATE );
	hclose( theFD );
	}

/* Find the first/next file in a directory.  This is simply a wrapper for
   the standard findFirst/Next() call which un-dosifies the returned
   information */

#if 0
int findfirst (const char *pathname, struct dta *info, int attribute);
int findnext (struct dta *info);
struct dta {char attrib /* attribute found */
            time_t time; /* date and time of file */
            size_t length; /* length of file */
            char filename[13];
           };
#endif /* 0 */

BOOLEAN findFirst( const char *filePath, const ATTR fileAttr, FILEINFO *fileInfo )
	{
	return( ( findfirst( filePath, ( struct dta * ) fileInfo, fileAttr ) != IO_ERROR ) ? TRUE : FALSE );
	}

BOOLEAN findNext( FILEINFO *fileInfo )
	{
	return( ( findnext( ( struct dta * ) fileInfo ) != IO_ERROR ) ? TRUE : FALSE );
	}

#if 0		/* May or may not need the following */

/* Case-insensitive string comparisons */

int strnicmp( const char *src, const char *dest, int length )
	{
	char srcCh, destCh;
	char *srcPtr = (char *) src, *destPtr = (char *) dest;

	while( length-- )
		{

		/* Need to be careful with toupper() side-effects */
		srcCh = *srcPtr++;
		srcCh = toupper( srcCh );
		destCh = *destPtr++;
		destCh = toupper( destCh );

		if( srcCh != destCh )
			return( srcCh - destCh );
		}

	return( 0 );
	}

int stricmp( const char *src, const char *dest )
	{
	int srcLen = strlen( src ), destLen = strlen( dest );

	return( strnicmp( src, dest, ( srcLen > destLen ) ? srcLen : destLen ) );
	}

/* Lowercase a string */

void strlwr( char *string )
	{
	while( *string )
		{
		*string = tolower( *string );
		string++;
		}
	}
#endif /* 0 */

void getScreenSize( void )
	{
	/* Assume a model 33 for now - do something with getrez() ?? */
	screenWidth  = 80;
	screenHeight = 24;
	}

int getCountry( void )
	{
	/* Default to US - there should be an XBIOS call to do it */
	return( 0 );
	}
