/****************************************************************************
*																			*
*							HPACK Multi-System Archiver						*
*							===========================						*
*																			*
*					   Display the Contents of an Archive					*
*						  VIEWFILE.C  Updated 12/07/92						*
*																			*
* This program is protected by copyright and as such any use or copying of	*
*  this code for your own purposes directly or indirectly is highly uncool	*
*  					and if you do so there will be....trubble.				*
* 				And remember: We know where your kids go to school.			*
*																			*
*		Copyright 1989 - 1992  Peter C.Gutmann.  All rights reserved		*
*																			*
****************************************************************************/

#include <stdlib.h>
#include <string.h>
#ifdef __MAC__
  #include "defs.h"
  #include "arcdir.h"
  #include "choice.h"
  #include "error.h"
  #include "flags.h"
  #include "frontend.h"
  #include "hpacklib.h"
  #include "hpaktext.h"
  #include "system.h"
  #include "wildcard.h"
  #include "fastio.h"
  #include "hpackio.h"
  #include "tags.h"
#else
  #include "defs.h"
  #include "arcdir.h"
  #include "choice.h"
  #include "error.h"
  #include "flags.h"
  #include "frontend.h"
  #include "hpacklib.h"
  #include "system.h"
  #include "wildcard.h"
  #include "io/fastio.h"
  #include "io/hpackio.h"
  #include "language/hpaktext.h"
  #include "tags.h"
#endif /* __MAC__ */

/* Prototypes for functions in ARCHIVE.C */

int extractData( const WORD dataInfo, const BYTE extraInfo, \
				 LONG dataLen, const LONG fileLen, const BOOLEAN isFile );

#ifdef __MSDOS__

/* Prototypes for functions in MISC.ASM */

int getRatio( long dataLen, long compressedLen );
#endif /* __MSDOS__ */

/****************************************************************************
*																			*
*							Display Contents of Archive						*
*																			*
****************************************************************************/

/* The names of the OS's. */

const char *systemName[] = { "MSDOS", "UNIX ", "Amiga", " Mac ",
							 " Arc ", "OS/2 ", "IIgs ", "Atari",
							 " VMS ", "Prime", "?????" };

/* What format to print the date in */

enum { DATE_MDY, DATE_DMY, DATE_YMD, DATE_MYD, DATE_DYM, DATE_YDM };

int dateFormat;		/* Which of the above date formats to use */

/* Determine the format to print the date in */

void setDateFormat( void )
	{
	dateFormat = getCountry();
	}

#ifdef __MSDOS__
  void extractDate( const LONG time, int *time1, int *time2, int *time3, \
									 int *hours, int *minutes, int *seconds );
#else

/* Constants for time handling functions */

#define SECS_PER_DAY	86400L
#define DAYS_PER_YEAR	365
#define MONTHS_PER_YEAR	12
#define FEB29			60		/* Days to Feb.29 from start of year */
#define AD2000			30		/* Year count to 2000AD */

/* Lookup table for month lengths */

int monthLen[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

/* Extract the date fields from the time value.  A custom version is used
   since there is some confusion with different versions of localtime()
   making strange adjustments for what they think is local time */

void extractDate( LONG time, int *time1, int *time2, int *time3, \
							 int *hours, int *minutes, int *seconds )
	{
	int days, months, years, leapYearOffset = 0;

	/* Extract date/time */
	days = ( int ) ( time / SECS_PER_DAY );
	time %= SECS_PER_DAY;

	/* Extract hours, minutes, seconds */
	*seconds = ( int ) ( time % 60 );
	time /= 60;
	*hours = ( int ) ( time / 60 );
	*minutes = ( int ) ( time % 60 );

	/* Extract days, months, years */
	years = days / DAYS_PER_YEAR;
	days = ( ( days % DAYS_PER_YEAR ) + 1 ) - ( years / 4 );

	/* Check for underflow due to leap years */
	if( days <= 0 )
		{
		/* Move back 1 year and adjust days to December */
		years--;
		days += DAYS_PER_YEAR;
		}

	/* Check for leap year extra day - need to adjust if year count (adjusted
	   to base of 1972) is divisible by 4, if it's not 2000AD, and if it's
	   past Feb.29 */
	if( !( ( years + 2 ) & 3 ) && years != AD2000 && days >= FEB29 )
		{
		days--;				/* Subtract day for Feb.29 */
		leapYearOffset++;	/* Remember extra day later on */
		}

	/* Evaluate month days */
	months = MONTHS_PER_YEAR - 1;
	while( days <= monthLen[ months ] )
		months--;
	days -= monthLen[ months ] + leapYearOffset;
	months++;		/* Convert offset from 0 -> 1 */
	years = ( years + 70 ) % 100;

	/* Reorder fields depending on country */
	switch( dateFormat )
		{
		case DATE_DMY:
			/* Use DD/MM/YY format */
			*time1 = days;
			*time2 = months;
			*time3 = years;
			break;

		case DATE_YMD:
			/* Use YY/MM/DD format */
			*time1 = years;
			*time2 = months;
			*time3 = days;
			break;

		default:
			/* Use MM/DD/YY format */
			*time1 = months;
			*time2 = days;
			*time3 = years;
		}
	}
#endif /* __MSDOS__ */

/* Format a filename to fit the screen size */

static char *formatFilename( char *fileName, BOOLEAN isUnicode )
	{
	int fileNameFieldLen = screenWidth - 53;	/* Width of fileName field */
	int fileNameLen, maxFileNameLen;
	static char formattedFileName[ 128 ];

	/* If it's a Unicode filename, at the moment just return an indicator of
	   this fact (virtually no OS's support this yet) */
	if( isUnicode )
		return( "<Unicode>" );

	/* Get filename, truncate if overly long, and add end marker */
	fileNameLen = strlen( fileName );
	maxFileNameLen = ( fileNameLen > fileNameFieldLen ) ? fileNameFieldLen : fileNameLen;
	strncpy( formattedFileName, fileName, maxFileNameLen );
	if( fileNameLen > fileNameFieldLen )
		strcpy( &formattedFileName[ fileNameFieldLen - 2 ], ".." );
	else
		formattedFileName[ maxFileNameLen ] = '\0';

	return( formattedFileName );
	}

/* Display information on one file */

static void showFileInfo( const FILEHDRLIST *fileInfoPtr, LONG *totalLength, \
						  LONG *totalSize )
	{
	const FILEHDR *theHeader = &fileInfoPtr->data;
	int time1, time2, time3, hours, minutes, seconds;
	BYTE cryptInfo = ( theHeader->archiveInfo & ARCH_EXTRAINFO ) ? \
			*fileInfoPtr->extraInfo & ( EXTRA_SECURED | EXTRA_ENCRYPTED ) : 0;
	int ratio;

	/* Handle file, data lengths and compression ratios.  The check for
	   fileLen > dataLen is necessary since we may end up expanding the
	   data when we append the checksum to the end */
	*totalLength += theHeader->fileLen;
	*totalSize += theHeader->dataLen;
#ifdef __MSDOS__
	ratio = getRatio( theHeader->fileLen, theHeader->dataLen );
#else
	if( theHeader->fileLen && theHeader->fileLen > theHeader->dataLen )
		ratio = 100 - ( int ) ( ( 100 * theHeader->dataLen ) / theHeader->fileLen );
	else
		ratio = 0;
#endif /* __MSDOS__ */

	/* Set up the date fields */
	extractDate( theHeader->fileTime, &time1, &time2, &time3, \
									  &hours, &minutes, &seconds );

	hprintf( VIEWFILE_MAIN_DISPLAY, \
			 ( ( theHeader->archiveInfo & ARCH_SYSTEM ) >= OS_UNKNOWN ) ? \
				systemName[ OS_UNKNOWN ] : \
				systemName[ theHeader->archiveInfo & ARCH_SYSTEM ], \
			 theHeader->fileLen, theHeader->dataLen, ratio, \
			 time1, time2, time3, hours, minutes, seconds, \
			 ( cryptInfo == ( EXTRA_ENCRYPTED | EXTRA_SECURED ) ) ? '#' : \
				( cryptInfo == EXTRA_ENCRYPTED ) ? '*' : \
				( cryptInfo == EXTRA_SECURED ) ? '-' : ' ', \
			 formatFilename( fileInfoPtr->fileName, \
							 ( fileInfoPtr->extraInfo != NULL ) && \
							 ( *fileInfoPtr->extraInfo & EXTRA_UNICODE ) ) );
	}

/* The size of the directory stack.  We should never have more than 50
   directories in one pathname */

#define DIRSTACK_SIZE	50

/* Display information on one directory */

void showDirPath( WORD dirNo )
	{
	WORD dirStack[ DIRSTACK_SIZE ];
	int time1, time2, time3, hours, minutes, seconds;
	int stackIndex = 0;
	char *dirName;

	/* Print the directory info if necessary */
	if( dirNo )
		{
		hprintf( MESG_DIRECTORY );
		extractDate( getDirTime( dirNo ), &time1, &time2, &time3, &hours, &minutes, &seconds );

		/* Get chain of directories from root to current directory.  Some of
		   this code is duplicated in getPath(), but we can't use getPath()
		   since it munges the pathName into an OS-compatible format as it goes */
		do
			dirStack[ stackIndex++ ] = dirNo;
		while( ( dirNo = getParent( dirNo ) ) && stackIndex <= DIRSTACK_SIZE );

		/* Now print full path to current directory */
		while( stackIndex )
			{
			dirName = getDirName( dirStack[ --stackIndex ] );
			hprintf( "/%s", dirName );
			}

		/* Finally, print the directory date */
		hprintf( MESG_DIRECTORY_TIME, time1, time2, time3, hours, minutes, seconds );
		}

	/* Make sure we don't get a "Nothing to do" error if we elect to view
	   only (empty) directories */
	if( viewFlags & VIEW_DIRS )
		archiveChanged = TRUE;
	}

/* Static vars for tracking totals when multiple archives are used */

static LONG grandTotalLength = 0L, grandTotalSize = 0L;
static int grandTotalFiles = 0, totalArchives = 0;

/* Display a directory of files within an archive */

void listFiles( void )
	{
	FILEHDRLIST *fileInfoPtr;
	DIRHDRLIST *dirInfoPtr;
	FILEHDR theHeader;
	LONG totalLength = 0L, totalSize = 0L;
	int fileCount = 0, totalRatio;
	int time1, time2, time3, hours, minutes, seconds;
	WORD oldFlags, hType, count;
	BOOLEAN showFile, dirNameShown;

	/* Turn on stealth mode to disable printing of any extraneous noise
	   while extracting data.  This is safe since the view options don't
	   check for stealth mode */
	flags |= STEALTH_MODE;

	/* Print a newline if there is an archive listing preceding this one */
	if( totalArchives )
		hputchar( '\n' );

	hputs( VIEWFILE_TITLE );

	for( count = getFirstDir(); count != END_MARKER; count = getNextDir() )
		{
		/* First handle any special entries (if there are any and provided
		   it's not a multipart archive) */
		if( ( fileInfoPtr = getFirstFileEntry( count ) ) != NULL && \
			!( flags & MULTIPART_ARCH ) )
			do
				{
				/* Check whether this is an archive comment file, unless
				   we've been asked to diaply comment files only */
				if( !( flags & ARCH_COMMENT ) && \
					( ( hType = fileInfoPtr->hType ) == TYPE_COMMENT_TEXT || \
						hType == TYPE_COMMENT_ANSI ) )
					{
					/* Move to the comment data */
					theHeader = fileInfoPtr->data;
					vlseek( fileInfoPtr->offset, SEEK_SET );
					resetFastIn();
					oldFlags = flags;

					/* Set up any handlers we need for it */
					switch( hType )
						{
						case TYPE_COMMENT_TEXT:
							/* Set up system for text output */
							setOutputIntercept( OUT_FMT_TEXT );
							if( !( flags & XLATE_OUTPUT ) )
								{
								/* Turn on smart translation if none is
								   specified */
								flags |= XLATE_OUTPUT;
								xlateFlags = XLATE_SMART;
								}
							break;

						case TYPE_COMMENT_ANSI:
							/* Output it raw (assumes ANSI driver) */
							setOutputFD( STDOUT );
							break;

						default:
							/* Don't know what to do with it, send it to the
							   bit bucket */
							setOutputIntercept( OUT_NONE );
						}

					/* Call extractData() to move the comment to wherever
					   it's meant to go */
					extractData( theHeader.archiveInfo, 0, theHeader.dataLen, \
								 theHeader.fileLen, FALSE );

					/* Reset status of output handlers */
					flags = oldFlags;
					resetOutputIntercept();
					archiveChanged = TRUE;
					}
				}
			while( ( fileInfoPtr = getNextFileEntry() ) != NULL );

		dirNameShown = FALSE;	/* Haven't printed dir.name yet */

		/* First print any subdirectories */
		if( !( viewFlags & VIEW_FILES ) )
			if( ( dirInfoPtr = getFirstDirEntry( count ) ) != NULL )
				do
					if( getDirTaggedStatus( dirInfoPtr->dirIndex ) )
						{
						/* Print the path to the directory if necessary */
						if( !dirNameShown )
							{
							showDirPath( count );
							dirNameShown = TRUE;
							}

						extractDate( dirInfoPtr->data.dirTime, &time1, &time2, &time3, &hours, &minutes, &seconds );
						hprintf( VIEWFILE_SUBDIRECTORY, time1, time2, time3, \
								 hours, minutes, seconds, \
								 formatFilename( dirInfoPtr->dirName, \
												 dirInfoPtr->data.dirInfo & DIR_UNICODE ) );
						archiveChanged = TRUE;
						}
				while( ( dirInfoPtr = getNextDirEntry() ) != NULL );

		/* Then print the files in the directory */
		if( !( viewFlags & VIEW_DIRS ) )
			{
			/* Sort the files if necessary */
			if( viewFlags & VIEW_SORTED )
				sortFiles( count );

			if( ( fileInfoPtr = getFirstFileEntry( count ) ) != NULL )
				do
					{
					/* Exclude either special-format files by default, or non-comment
					   files if asked to display only comment files */
					if( flags & ARCH_COMMENT )
						showFile = ( hType = fileInfoPtr->hType ) == TYPE_COMMENT_TEXT || \
									 hType == TYPE_COMMENT_ANSI;
					else
						showFile = ( !( fileInfoPtr->data.archiveInfo & ARCH_SPECIAL ) );

					if( showFile && fileInfoPtr->tagged )
						{
						/* Print the path to the directory if necessary */
						if( !dirNameShown )
							{
							showDirPath( count );
							dirNameShown = TRUE;
							}

						archiveChanged = TRUE;	/* We've accessed the archive */
						showFileInfo( fileInfoPtr, &totalLength, &totalSize );
						fileCount++;
						}
					}
				while( ( fileInfoPtr = getNextFileEntry() ) != NULL );
			}
		}

	/* Print summary of file info */
#ifdef __MSDOS__
	totalRatio = getRatio( totalLength, totalSize );
#else
	if( totalLength && totalLength > totalSize )
		totalRatio = 100 - ( int ) ( ( 100 * totalSize ) / totalLength );
	else
		totalRatio = 0;
#endif /* __MSDOS__ */

	hprintf( VIEWFILE_TRAILER, totalLength, totalSize, ( int ) totalRatio, \
			 fileCount, ( fileCount == 1 ) ? MESG_SINGULAR_FILES : \
			 								 MESG_PLURAL_FILES );

	/* Update grand totals */
	grandTotalLength += totalLength;
	grandTotalSize += totalSize;
	grandTotalFiles += fileCount;
	totalArchives++;
	}

/* Print summary of archives viewed if necessary */

void showTotals( void )
	{
	if( totalArchives > 1 )
		hprintf( VIEWFILE_GRAND_TOTAL, \
				 grandTotalLength, grandTotalSize, totalArchives, \
				 grandTotalFiles, \
				 ( grandTotalFiles == 1 ) ? MESG_SINGULAR_FILES : \
				 							MESG_PLURAL_FILES );
	}
