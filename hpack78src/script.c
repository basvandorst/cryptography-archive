/****************************************************************************
*																			*
*							HPACK Multi-System Archiver						*
*							===========================						*
*																			*
*							 Script File Handling Code						*
*							SCRIPT.C  Updated 20/07/92						*
*																			*
* This program is protected by copyright and as such any use or copying of	*
*  this code for your own purposes directly or indirectly is highly uncool	*
*  					and if you do so there will be....trubble.				*
* 				And remember: We know where your kids go to school.			*
*																			*
*		Copyright 1991 - 1992  Peter C.Gutmann.  All rights reserved		*
*																			*
****************************************************************************/

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#ifdef __MAC__
  #include "defs.h"
  #include "choice.h"
  #include "error.h"
  #include "filesys.h"
  #include "flags.h"
  #include "frontend.h"
  #include "hpacklib.h"
  #include "hpaktext.h"
  #include "system.h"
  #include "wildcard.h"
  #include "fastio.h"
  #include "hpackio.h"
#else
  #include "defs.h"
  #include "choice.h"
  #include "error.h"
  #include "filesys.h"
  #include "flags.h"
  #include "frontend.h"
  #include "hpacklib.h"
  #include "system.h"
  #include "wildcard.h"
  #include "io/fastio.h"
  #include "io/hpackio.h"
  #include "language/hpaktext.h"
#endif /* __MAC__ */

/****************************************************************************
*																			*
*						Build/Free List of Files to Handle					*
*																			*
****************************************************************************/

/* The start of the list of fileSpecs */

FILEPATHINFO *filePathListStart = NULL;

/* Add a filespec to the list of files to handle */

void addFilespec( char *fileSpec )
	{
	FILEPATHINFO *theNode, *prevNode, *newNode;
	FILENAMEINFO *fileNamePtr, *prevFileNamePtr;
	char fileCode[ MATCH_DEST_LEN ], pathCode[ MATCH_DEST_LEN ];
	char pathName[ MAX_PATH ];
	int pathNameEnd, nonMatch, fileCodeLen, pathCodeLen;
	BOOLEAN fileHasWildcards, pathHasWildcards;
#if defined( __AMIGA__ ) || defined( __ARC__ ) || defined( __ATARI__ ) || \
	defined( __MAC__ ) || defined( __MSDOS__ ) || defined( __OS2__ ) || \
	defined( __VMS__ )
	int count;
#endif /* __AMIGA__ || __ARC__ || __ATARI__ || __MAC__ || __MSDOS__ || __OS2__ || __VMS__ */
#if defined( __ARC__ ) || defined( __VMS__ )
	int nodeNameEnd = 0;
#endif /* __ARC__ || __VMS__ */

	/* Check the path is of a legal length */
	if( strlen( fileSpec ) > MAX_PATH - 1 )
		error( PATH_s_TOO_LONG, fileSpec );

	/* Extract the pathname and convert it into an OS-compatible format */
	pathNameEnd = extractPath( fileSpec, pathName );

	/* Compile the file spec.and path spec.into a form acceptable by the
	   wildcard-matching finite-state machine */
	fileCodeLen = compileString( fileSpec + pathNameEnd, fileCode );
	pathCodeLen = compileString( pathName, pathCode );
	fileHasWildcards = strHasWildcards( fileSpec + pathNameEnd );
	pathHasWildcards = strHasWildcards( pathName );

	/* Complain if it's an external path with wildcards.  This is the case
	   for Add and the add portion of an Update */
	if( pathHasWildcards && ( choice == ADD || choice == UPDATE ) )
		error( CANNOT_USE_WILDCARDS_s, pathName );

	/* Now sort the file names by path as they are added, in order to make disk
	   accesses more efficient.  This can be done by a simple insertion sort
	   since there are only a small number of names to sort, and it can be done
	   on the fly.  Note that there is one case in which the files won't be
	   sorted:  If there drive specs, normal files/paths, and files/paths
	   beginning with letters lower than ':' then they will be put before the
	   drive specs, with the rest of the normal files/paths being placed
	   after the drive specs.  However the occurrence of filenames containing
	   these characters is virtually unheard-of so no special actions are
	   taken for them */
	theNode = filePathListStart;
	if( filePathListStart == NULL )
		{
		/* Create the initial list node */
		if( ( filePathListStart = newNode = ( FILEPATHINFO * ) hmalloc( sizeof( FILEPATHINFO ) ) ) == NULL )
			error( OUT_OF_MEMORY );
		}
	else
		{
		/* Find the correct position to insert the new pathname */
		prevNode = filePathListStart;
		while( theNode != NULL && ( nonMatch = strcmp( pathName, theNode->filePath ) ) > 0 )
			{
			prevNode = theNode;
			theNode = theNode->next;
			}

		/* Don't insert the pathname if it's already in the list */
		if( !nonMatch )
			{
			/* Find the end of the list of fileNames and add a new node */
			for( prevFileNamePtr = fileNamePtr = theNode->fileNames; \
				 fileNamePtr != NULL; prevFileNamePtr = fileNamePtr, \
				 fileNamePtr = fileNamePtr->next )
				/* Leave now if the fileName is already in the list */
				if( !strncmp( fileNamePtr->fileName, fileCode, fileCodeLen ) )
					return;
			fileNamePtr = prevFileNamePtr;

			if( ( fileNamePtr->next = ( FILENAMEINFO * ) hmalloc( sizeof( FILENAMEINFO ) ) ) == NULL )
				error( OUT_OF_MEMORY );
			fileNamePtr = fileNamePtr->next;
			goto nodeExists;
			}

		/* Create the new node and link it into the list */
		if( ( newNode = ( FILEPATHINFO * ) hmalloc( sizeof( FILEPATHINFO ) ) ) == NULL )
			error( OUT_OF_MEMORY );
		if( prevNode == filePathListStart )
			{
			/* Insert at start of list */
			filePathListStart = newNode;
			theNode = prevNode;
			}
		else
			/* Insert in middle/end of list */
			prevNode->next = newNode;
		}

	newNode->next = theNode;

	/* Seperate out the node/path component and make sure we're not trying
	   to override an existing base path */
#if defined( __ATARI__ ) || defined( __MSDOS__ ) || defined( __OS2__ )
	/* Look for a drive spec in the filename */
	newNode->device = NULL;
	if( ( ( count = ( *pathName && pathName[ 1 ] == ':' ) ? \
				toupper( *pathName ) : 0 ) && *basePath ) || \
		( *basePath && basePath[ 1 ] == ':' && basePath[ 2 ] && \
				*pathName == SLASH ) )
		error( CANNOT_OVERRIDE_BASEPATH, basePath );

	/* If there is a drive spec, save it */
	if( count )
		{
		if( ( newNode->device = ( char * ) hmalloc( 3 ) ) == NULL )
			error( OUT_OF_MEMORY );
		newNode->device[ 0 ] = count;
		newNode->device[ 1 ] = ':';
		newNode->device[ 2 ] = '\0';
		}
#elif defined( __AMIGA__ ) || defined( __MAC__ )
	/* Look for a drive spec in the pathname */
	newNode->device = NULL;
	for( count = 0; count < pathNameEnd; count++ )
		if( pathName[ count ] == ':' )
			{
			/* Save the drive spec */
			if( ( newNode->device = ( char * ) hmalloc( count + 2 ) ) == NULL )
				error( OUT_OF_MEMORY );
			strncpy( newNode->device, pathName, count + 1 );
			newNode->device[ count + 1 ] = '\0';
			break;	/* Exit while loop */
			}

	if( *basePath && *pathName == SLASH )
		error( CANNOT_OVERRIDE_BASEPATH, basePath );
#elif defined( __ARC__ )
	/* Look for a node name in the pathname */
	newNode->node = NULL;
	for( count = 1; count < pathNameEnd; count++ )
		if( pathName[ count - 1 ] == ':' && pathName[ count ] == ':' )
			{
			/* Save the node name */
			if( ( newNode->node = ( char * ) hmalloc( count + 2 ) ) == NULL )
				error( OUT_OF_MEMORY );
			strncpy( newNode->node, pathName, ++count );
			newNode->node[ count ] = '\0';
			nodeNameEnd = count;
			break;	/* Exit while loop */
			}

	/* Archimedes drives are handled funny */
	newNode->device = NULL;
#elif defined( __UNIX__ )
	if( *basePath && *pathName == SLASH )
		error( CANNOT_OVERRIDE_BASEPATH, basePath );
#elif defined( __VMS__ )
	/* Look for a node name in the pathname */
	newNode->node = NULL;
	for( count = 1; count < pathNameEnd; count++ )
		if( pathName[ count - 1 ] == ':' && pathName[ count ] == ':' )
			{
			/* Save the node name */
			if( ( newNode->node = ( char * ) hmalloc( count + 2 ) ) == NULL )
				error( OUT_OF_MEMORY );
			strncpy( newNode->node, pathName, ++count );
			newNode->node[ count ] = '\0';
			nodeNameEnd = count;
			break;	/* Exit while loop */
			}

	/* Look for a device name in the pathname */
	newNode->device = NULL;
	for( count = nodeNameEnd; count < pathNameEnd; count++ )
		if( pathName[ count ] == ':' )
			{
			/* Save the device name */
			count -= nodeNameEnd;
			if( ( newNode->device = ( char * ) hmalloc( count + 2 ) ) == NULL )
				error( OUT_OF_MEMORY );
			strncpy( newNode->device, pathName + nodeNameEnd, ++count );
			newNode->device[ count ] = '\0';
			break;	/* Exit while loop */
			}

	if( *basePath && *pathName == SLASH )
		error( CANNOT_OVERRIDE_BASEPATH, basePath );
#endif /* Various OS-specific device handling */

	if( ( newNode->filePath = ( char * ) hmalloc( pathCodeLen ) ) == NULL )
		error( OUT_OF_MEMORY );
	memcpy( newNode->filePath, pathCode, pathCodeLen );
	newNode->hasWildcards = pathHasWildcards;

	if( ( newNode->fileNames = ( FILENAMEINFO * ) hmalloc( sizeof( FILENAMEINFO ) ) ) == NULL )
		error( OUT_OF_MEMORY );
	fileNamePtr = newNode->fileNames;

nodeExists:
	fileNamePtr->next = NULL;

	/* Add the compiled filespec to the list of filespecs */
	if( ( fileNamePtr->fileName = ( char * ) hmalloc( fileCodeLen ) ) == NULL )
		error( OUT_OF_MEMORY );
	memcpy( fileNamePtr->fileName, fileCode, fileCodeLen );
	fileNamePtr->hasWildcards = fileHasWildcards;
	}

#if !defined( __MSDOS__ ) || defined( GUI )

/* Free up the memory used by the list of filespecs.  See the comment in
   freeArchiveNames() for why we use this complex double-stepping way of
   freeing the headers */

void freeFilespecs( void )
	{
	FILEPATHINFO *pathHeaderCursor = filePathListStart, *pathHeaderPtr;
	FILENAMEINFO *nameHeaderCursor, *nameHeaderPtr;

	/* Free the nodes of the filePath list */
	while( pathHeaderCursor != NULL )
		{
		/* Free the nodes of the fileName list */
		nameHeaderCursor = pathHeaderCursor->fileNames;
		while( nameHeaderCursor != NULL )
			{
			nameHeaderPtr = nameHeaderCursor;
			nameHeaderCursor = nameHeaderCursor->next;
			hfree( nameHeaderPtr->fileName );
			hfree( ( void * ) nameHeaderPtr );	/* ThinkC needs this */
			}

		pathHeaderPtr = pathHeaderCursor;
		pathHeaderCursor = pathHeaderCursor->next;
		hfree( pathHeaderPtr->filePath );
		hfree( ( void * ) pathHeaderPtr );		/* ThinkC again */
		}
	}
#endif /* !__MSDOS__ || GUI */

/****************************************************************************
*																			*
*								Process a List-File							*
*																			*
****************************************************************************/

/* The types of error we check for */

enum { NO_ERROR, ILLEGAL_CHAR_ERROR, PATH_ERROR };

#define CPM_EOF	0x1A		/* ^Z = CPM EOF char */

#define MAX_ERRORS	10		/* Max.no.errors before we give up */

void processListFile( const char *listFileName )
	{
	FD listFileFD;
	int ch = 0, theCh;
	int errType, errPos, errCount = 0, line = 1;
	BOOLEAN firstError = TRUE, seenCR = FALSE;
#ifdef GUI
	char string[ 10 ];
#endif /* GUI */

	if( ( listFileFD = hopen( listFileName, O_RDONLY | S_DENYWR | A_SEQ ) ) == IO_ERROR )
		{
#ifdef GUI
		alert( ALERT_CANNOT_OPEN_SCRIPTFILE, listFileName );
#else
		hprintf( WARN_CANNOT_OPEN_SCRIPTFILE_s, listFileName );
#endif /* GUI */
		return;
		}
#ifndef GUI
	hprintfs( MESG_PROCESSING_SCRIPTFILE_s, listFileName );
#endif /* !GUI */
	setInputFD( listFileFD );
	resetFastIn();

	/* Process each line in the listFile */
	while( ch != FEOF )
		{
		/* If we've just seen a CR and now we see a LF, skip to next line */
		if( ( ch = fgetByte() ) == '\n' && seenCR )
			ch = fgetByte();
		seenCR = FALSE;

		/* Skip whitespace */
		while( ( ch == ' ' || ch == '\t' ) && ch != FEOF )
			ch = fgetByte();

		/* Get a line into the mrglBuffer */
		mrglBufCount = 0;
		errType = NO_ERROR;
		while( ch != '\r' && ch != '\n' && ch != CPM_EOF && ch != FEOF )
			{
			/* Check for an illegal char in the data */
			if( ( ch < ' ' || ch > '~' ) && ch != '\r' && ch != '\n' && \
				ch != CPM_EOF && ch != FEOF )
				{
				if( errType == NO_ERROR )
					/* Save position of first illegal char */
					errPos = mrglBufCount;
				errType = ILLEGAL_CHAR_ERROR;
				}
#if defined( __ATARI__ ) || defined( __MSDOS__ ) || defined( __OS2__ )
			if( ch == '\\' )
				ch = SLASH;		/* Get dir.seperator in correct format */
#endif /* __ATARI__ || __MSDOS__ || __OS2__ */

			/* Make sure the path is of the correct length.  Note that the
			   code is ordered so that a PATH_ERROR takes precedence over
			   an ILLEGAL_CHAR_ERROR */
			if( mrglBufCount > MAX_PATH )
				errType = PATH_ERROR;
			else
				mrglBuffer[ mrglBufCount++ ] = ch;

			if( ( ch = fgetByte() ) == '#' )
				{
				/* Skip comment section and trailing whitespace */
				while( ch != '\r' && ch != '\n' && ch != CPM_EOF && ch != FEOF )
					ch = fgetByte();
				break;
				}
			}

		/* Remember we've just passed a CR if necessary */
		if( ch == '\r' )
			seenCR = TRUE;

		/* Skip trailing whitespace and add der terminador */
		while( mrglBufCount && \
			   ( ( theCh = mrglBuffer[ mrglBufCount - 1 ] ) == ' ' || theCh == '\t' ) )
			mrglBufCount--;
		mrglBuffer[ mrglBufCount ] = '\0';

		/* Process the line unless its a blank or comment line */
		if( mrglBufCount && *mrglBuffer != '#' )
			{
			/* At the first error we must print a newline to get the text
			   away from the HPACK title message unless we've already
			   printed one as part of the listFile message */
			if( ( errType != NO_ERROR ) && firstError )
				{
				firstError = FALSE;
#ifndef GUI
				hputchar( '\n' );
#endif /* !GUI */
				}

			switch( errType )
				{
				case PATH_ERROR:
					mrglBuffer[ screenWidth - strlen( MESG_PATH_s__TOO_LONG_LINE_d ) - 3 ] = '\0';
								/* Truncate to fit screen size (3 = '%s' + 1) */
#ifdef GUI
					itoa( line, string, 10 );
					alert( ALERT_PATH_TOO_LONG_LINE, string );
#else
					hprintf( MESG_PATH_s__TOO_LONG_LINE_d, mrglBuffer, line );
#endif /* GUI */
					errCount++;
					break;

				case ILLEGAL_CHAR_ERROR:
#ifdef GUI
					itoa( line, string, 10 );
					alert( ALERT_BAD_CHAR_IN_FILENAME_LINE, string );
					if( errPos );	/* Get rid of "Not used" warning */
#else
					hprintf( "> %s\n  ", mrglBuffer );
					while( errPos-- )
						hputchar( ' ' );
					hprintf( MESG_BAD_CHAR_IN_FILENAME_LINE_d, line );
#endif /* GUI */
					errCount++;
					break;

				default:
					addFilespec( ( char * ) mrglBuffer );
				}
			}
		else
			/* It may be a script language command */
			if( strlen( ( char * ) mrglBuffer ) > 2 && mrglBuffer[ 1 ] != ' ' )
				{
				/* At the moment just complain about it */
#ifdef GUI
				alert( ALERT_UNKNOWN_SCRIPT_COMMAND, ( char * ) mrglBuffer );
#else
				hprintf( WARN_UNKNOWN_SCRIPT_COMMAND_s, mrglBuffer );
#endif /* GUI */
				errCount++;
				}

		/* Handle special-case of ^Z if listFile came off an MSDOS system */
		if( ch == CPM_EOF )
			ch = FEOF;

		/* Exit if there are too many errors */
		if( errCount >= MAX_ERRORS )
			break;

		line++;
		}
	hclose( listFileFD );

	/* Exit if there were errors */
	if( errCount )
		error( N_ERRORS_DETECTED_IN_SCRIPTFILE, ( errCount >= MAX_ERRORS ) ? \
			   MESG_MAXIMUM_LEVEL_OF : "", errCount );
	}

