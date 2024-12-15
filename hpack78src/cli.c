/****************************************************************************
*																			*
*							HPACK Multi-System Archiver						*
*							===========================						*
*																			*
*								HPACK CLI Interface							*
*							  CLI.C  Updated 06/05/92						*
*																			*
* This program is protected by copyright and as such any use or copying of	*
*  this code for your own purposes directly or indirectly is highly uncool	*
*  					and if you do so there will be....trubble.				*
* 				And remember: We know where your kids go to school.			*
*																			*
*		Copyright 1989 - 1992  Peter C.Gutmann.  All rights reserved		*
*																			*
****************************************************************************/

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#ifdef __MAC__
  #include "defs.h"
  #include "arcdir.h"
  #include "choice.h"
  #include "error.h"
  #include "filesys.h"
  #include "flags.h"
  #include "frontend.h"
  #include "hpacklib.h"
  #include "hpaktext.h"
  #include "system.h"
  #include "wildcard.h"
  #include "crypt.h"
  #include "fastio.h"
#else
  #include "defs.h"
  #include "arcdir.h"
  #include "choice.h"
  #include "error.h"
  #include "filesys.h"
  #include "flags.h"
  #include "frontend.h"
  #include "hpacklib.h"
  #include "system.h"
  #include "wildcard.h"
  #include "crypt/crypt.h"
  #include "io/fastio.h"
  #include "language/hpaktext.h"
#endif /* __MAC__ */

/* Prototypes for functions in VIEWFILE.C */

void setDateFormat( void );

/* Prototypes for functions in SCRIPT.C */

void addFilespec( char *fileSpec );
void freeFilespecs( void );
void processListFile( const char *listFileName );

/* Prototypes for compression functions */

void initPack( const BOOLEAN initAllBuffers );
void endPack( void );

/* Prototypes for functions in VIEWFILE.C */

void showTotals( void );

/* The default match string, which matches all files.  This must be done
   as a static array since if given as a constant string some compilers
   may put it into the code segment which may cause problems when FILESYS.C
   attempts any conversion on it */

char WILD_MATCH_ALL[] = "*";

/* The following are defined in ARCHIVE.C */

extern BOOLEAN overWriteEntry;	/* Whether we overwrite the existing file
								   entry or add a new one in addFileHeader()
								   - used by FRESHEN, REPLACE, UPDATE */
#ifdef __MSDOS__
  void ndoc( const char **strPtr );
#endif /* __MSDOS__ */
#ifdef __OS2__
  BOOLEAN queryFilesystemType( char *path );
#endif /* __OS2__ */
#ifdef __MAC__
  #define ARCHIVE_PATH	TRUE
  #define FILE_PATH		FALSE

  void setVolumeRef( char *path, const BOOLEAN isArchivePath );
  int stripVolumeName( char *path );
#endif /* __MAC__ */
#ifdef __VMS__
  int translateVMSretVal( const int retVal );
#endif /* __VMS__ */

#if defined( __ATARI__ ) || defined( __MSDOS__ ) || defined( __OS2__ )

/* The drive the archive is on (needed for multidisk archives) */

extern BYTE archiveDrive;
#endif /* __ATARI__ || __MSDOS__ || __OS2__ */

/****************************************************************************
*																			*
*							Handle Command-line Switches					*
*																			*
****************************************************************************/

/* Handle the command for the archive.  This can also be done in a switch
   statement; however the compiler probably produces code equivalent to the
   following, and it is far more compact in source form */

#define NO_CMDS	9

static void doCommand( const char *theCommand )
	{
	int i;

	choice = toupper( *theCommand );
	for( i = 0; ( i < NO_CMDS ) && ( "ADFPVXRUT"[ i ] != choice ); i++ );
	if( i == NO_CMDS || theCommand[ 1 ] )
		error( UNKNOWN_COMMAND, theCommand );
	}

/* Get a byte as two hex digits - used by several sections of doArg() */

#ifdef __MSDOS__
  BYTE getHexByte( char **strPtr );
#else
BYTE getHexByte( char **strPtr )
	{
	char ch;
	BYTE retVal;

	retVal = toupper( **strPtr );
	retVal -= ( retVal >= 'A' ) ? 'A' - 10 : '0';
	*strPtr++;
	if( isxdigit( **strPtr ) )
		{
		retVal <<= 4;
		ch = toupper( **strPtr );
		ch -= ( ch >= 'A' ) ? ( 'A' - 10 ) : '0';
		retVal |= ch;
		}
	return( retVal );
	}
#endif /* __MSDOS__ */

/* Get all switches and point at the next arg to process */

/* Flags used/not used:
   Used:     ABCDEF..I.KLMNO..RSTUVWX.Z
   Not used: ......GH.J.....PQ.......Y. */

static void doArg( char *argv[], int *count )
	{
	char *strPtr, ch;
	BYTE lineEndChar;
	int length = 0;

	*basePath = '\0';	/* Default is no output directory */

	while( TRUE )
		{
		/* Get next arg */
		strPtr = ( char * ) argv[ *count ];
		if( *strPtr != '-' )
			break;

		/* Check for '--' as end of arg list */
		if( *++strPtr == '-' )
			{
			/* This was the last arg */
			( *count )++;
			break;
			}

		/* Process individual args */
		while( *strPtr )
			{
			ch = *strPtr++;				/* Avoid toupper() side-effects */
			switch( toupper( ch ) )
				{
				case '0':
					flags |= STORE_ONLY;
					break;

				case 'A':
					flags |= STORE_ATTR;
					break;

				case 'B':
					/* Copy the argument across */
					while( *strPtr )
						{
						basePath[ length ] = caseConvert( *strPtr );
#if defined( __ATARI__ ) || defined( __OS2__ )
						if( basePath[ length ] == '\\' )
							basePath[ length ] = SLASH;
#endif /* __ATARI__ || __OS2__ */
						strPtr++;
						if( ++length >= MAX_PATH - 1 )
							error( PATH_s__TOO_LONG, basePath );
						}
					basePath[ length ] = '\0';

					/* Append SLASH if necessary */
#if defined( __AMIGA__ ) || defined( __ARC__ ) || defined( __ATARI__ ) || \
	defined( __MAC__ ) || defined( __MSDOS__ ) || defined( __OS2__ ) || \
	defined( __VMS__ )
					if( length && ( ch = basePath[ length - 1 ] ) != SLASH && \
															   ch != ':' )
#else
					if( length && ( ch = basePath[ length - 1 ] ) != SLASH )
#endif /* __AMIGA__ || __ARC__ || __ATARI__ || __MAC__ || __MSDOS__ || __OS2__ || __VMS__ */
						{
						strcat( basePath, SLASH_STR );
						length++;
						}
					basePathLen = length;

					/* Make sure we don't try and use wildcards in the path */
					if( strHasWildcards( basePath ) )
						error( CANNOT_USE_WILDCARDS_s, basePath );
					break;

				case 'C':
					flags |= CRYPT;

					ch = *strPtr++;
					switch( toupper( ch ) )
						{
						case 'A':
							cryptFlags |= CRYPT_CKE_ALL;
							break;

						case 'I':
							cryptFlags |= CRYPT_CKE;
							break;

						case 'P':
							ch = *strPtr++;
							ch = toupper( ch );
							if( ch == 'S' )
								{
								cryptFlags |= CRYPT_SEC | CRYPT_PKE_ALL;
								secUserID = strPtr;
								}
							else
								{
								if( ch == 'A' )
									cryptFlags |= CRYPT_PKE_ALL;
								else
									cryptFlags |= CRYPT_PKE;
								mainUserID = strPtr;
								}

							/* Check for existence of userID, and skip it */
							if( !*strPtr )
								error( MISSING_USERID );
							while( *strPtr )
								strPtr++;		/* Skip to end of arg */
							break;

						case 'S':
							cryptFlags |= CRYPT_SEC | CRYPT_CKE_ALL;
							break;

						default:
							cryptFlags |= CRYPT_CKE_ALL;
							strPtr--;
						}

					/* Make sure we're not trying to encrypt using two
					   methods */
					ch = cryptFlags & ( CRYPT_CKE | CRYPT_PKE | CRYPT_CKE_ALL | CRYPT_PKE_ALL );
					if( ch != CRYPT_CKE && ch != CRYPT_PKE && \
						ch != CRYPT_CKE_ALL && ch != CRYPT_PKE_ALL )
						error( CANNOT_USE_BOTH_CKE_PKE );
					break;

				case 'D':
					ch = *strPtr++;
					switch( toupper( ch ) )
						{
						case 'A':
							dirFlags |= DIR_ALLPATHS;
							flags |= STORE_PATH;
							break;

						case 'C':
							dirFlags |= DIR_CONTAIN;
							break;

						case 'M':
							dirFlags |= DIR_MKDIR;
							break;

						case 'N':
							dirFlags |= DIR_NOCREATE;
							flags |= STORE_PATH;
							break;

						case 'R':
							dirFlags |= DIR_RMDIR;
							break;

						case 'V':
							dirFlags |= DIR_MVDIR;
							break;

						default:
							flags |= STORE_PATH;
							strPtr--;
						}
					break;

				case 'E':
					flags |= ERROR_RECOVER;
					break;

				case 'F':
					/* Only apply the move command if it would make sense
					   (protect the user from performing a move on Display
					   or Test) */
					if( choice == ADD || choice == FRESHEN || choice == EXTRACT || \
						choice == REPLACE || choice == UPDATE )
						flags |= MOVE_FILES;
					break;

				case 'I':
					flags |= INTERACTIVE;
					break;

				case 'K':
					flags |= OVERWRITE_SRC;
					if( choice == FRESHEN || choice == REPLACE || choice == UPDATE )
						error( CANNOT_CHANGE_DEL_ARCH );
					break;

				case 'L':
					ch = *strPtr++;
					if( toupper( ch ) == 'I' )
						cryptFlags |= CRYPT_SIGN;
					else
						{
						strPtr--;	/* Back up arg.pointer */
						cryptFlags |= CRYPT_SIGN_ALL;
						}
					signerID = strPtr;	/* Point to ID of signer */

					/* Check for existence of userID, and skip it */
					if( !*strPtr )
						error( MISSING_USERID );
					while( *strPtr )
						strPtr++;		/* Skip to end of arg */
					break;

				case 'M':
					flags |= MULTIPART_ARCH;
					if( choice == DELETE || choice == FRESHEN || \
						choice == REPLACE || choice == UPDATE || \
						( choice == EXTRACT && ( flags & MOVE_FILES ) ) )
						error( CANNOT_CHANGE_MULTIPART_ARCH );
					if( choice == ADD )
						multipartFlags |= MULTIPART_WRITE;
					break;

				case 'N':
#ifdef __MSDOS__
					ndoc( &strPtr );
#endif /* __MSDOS__ */
					break;

				case 'O':
					ch = *strPtr++;
					switch( toupper( ch ) )
						{
						case 'A':
							overwriteFlags |= OVERWRITE_ALL;
							break;

						case 'N':
							overwriteFlags |= OVERWRITE_NONE;
							break;

						case 'P':
							overwriteFlags |= OVERWRITE_PROMPT;
							break;

						case 'S':
							overwriteFlags |= OVERWRITE_SMART;
							break;

						default:
							error( UNKNOWN_OVERWRITE_OPTION, *--strPtr  );
						}
					break;

				case 'R':
					flags |= RECURSE_SUBDIR;
					break;

				case 'S':
					/* Stuart if you fclose( STDOUT ) to implement this I'll
					   dump ten thousand frogs on you */
					flags |= STEALTH_MODE;
					break;

				case 'T':
					flags |= TOUCH_FILES;
					break;

				case 'U':
					flags |= BLOCK_MODE;
					break;

				case 'V':
					ch = *strPtr++;
					switch( toupper( ch ) )
						{
						case 'D':
							viewFlags |= VIEW_DIRS;
							break;

						case 'F':
							viewFlags |= VIEW_FILES;
							break;

						case 'S':
							viewFlags |= VIEW_SORTED;
							break;

						default:
							error( UNKNOWN_VIEW_OPTION, *--strPtr  );
						}
					break;

				case 'W':
					flags |= ARCH_COMMENT;
					ch = *strPtr++;
					switch( toupper( ch ) )
						{
						case 'A':
							/* ANSI text comment */
							commentType = TYPE_COMMENT_ANSI;
							break;

						case 'G':
							/* GIF graphics comment */
							commentType = TYPE_COMMENT_GIF;
							break;

						default:
							/* Default: Text comment */
							commentType = TYPE_COMMENT_TEXT;
							strPtr--;	/* Correct strPtr value */
						}
					break;

				case 'X':
					/* Only apply outupt translation if it would make sense */
					if( choice == EXTRACT || choice == DISPLAY )
						flags |= XLATE_OUTPUT;

					/* Determine which kind of translation is required */
					ch = *strPtr++;
					switch( toupper( ch ) )
						{
						case 'A':
							xlateFlags |= XLATE_EOL;
							lineEndChar = '\0';
							break;

#if !defined( __ATARI__ ) && !defined( __MSDOS__ ) && !defined( __OS2__ )
						case 'C':
							xlateFlags |= XLATE_EOL;
							lineEndChar = '\r' | 0x80;
							break;
#endif /* !( __ATARI__ || __MSDOS__ || __OS2__ ) */

						case 'E':
							xlateFlags |= XLATE_EBCDIC;
							break;

#if !defined( __AMIGA__ ) && !defined( __ARC__ ) && !defined( __UNIX__ )
						case 'L':
							xlateFlags |= XLATE_EOL;
							lineEndChar = '\n';
							break;
#endif /* !( __AMIGA__ || __ARC__ || __UNIX__ ) */

						case 'P':
							xlateFlags |= XLATE_PRIME;
							break;

#ifndef __MAC__
						case 'R':
							xlateFlags |= XLATE_EOL;
							lineEndChar = '\r';
							break;
#endif /* !__MAC__ */

						case 'S':
							xlateFlags = XLATE_SMART;
							break;

						case 'X':
							/* Get line-end-char in hex */
							xlateFlags |= XLATE_EOL;
							lineEndChar = getHexByte( &strPtr );
							break;

						default:
							/* Default: Smart translate - ignore
							   lineEndChar setting */
							xlateFlags = XLATE_SMART;
							strPtr--;	/* Correct strPtr value */
						}

					/* Set up the translation system if necessary */
					initTranslationSystem( lineEndChar );

					break;

				case 'Z':
#if defined( __MSDOS__ )
					if( toupper( *strPtr ) == 'S' )
						/* Check for device filenames on extract */
						sysSpecFlags |= SYSPEC_CHECKSAFE;
					else
						error( UNKNOWN_OPTION, *--strPtr );

					strPtr++;			/* Skip arg */
#elif defined( __AMIGA__ ) || defined( __OS2__ )
					if( matchString( strPtr, "lower", NO_WILDCARDS ) )
						{
						/* Force lower case on file and dir names if it
						   would make sense */
						if( choice == VIEW || choice == EXTRACT || \
							choice == TEST || choice == DISPLAY )
							sysSpecFlags |= SYSPEC_FORCELOWER;
						}

					while( *strPtr )
						strPtr++;		/* Skip to end of arg */
#elif defined( __ARC__ )
					if( matchString( strPtr, "invert", NO_WILDCARDS ) )
						{
						/* Invert filename extensions into directories if it
						   would make sense */
						if( choice == EXTRACT )
							sysSpecFlags |= SYSPEC_INVERTDIR;
						}
					else
						if( matchString( strPtr, "lower", NO_WILDCARDS ) )
							{
							/* Force lower case on file and dir names if it
							   would make sense */
							if( choice == VIEW || choice == EXTRACT || \
								choice == TEST || choice == DISPLAY )
								sysSpecFlags |= SYSPEC_FORCELOWER;
							}
						else
							if( matchString( strPtr, "type", NO_WILDCARDS ) )
								/* Add file-type association data */
								addTypeAssociation( strPtr + 5 );
							else
								error( UNKNOWN_OPTION, *--strPtr );

					while( *strPtr )
						strPtr++;		/* Skip to end of arg */
#elif defined( __IIGS__ ) || defined( __MAC__ )
					if( matchString( strPtr, "lower", NO_WILDCARDS ) )
						{
						/* Force lower case on file and dir names if it
						   would make sense */
						if( choice == VIEW || choice == EXTRACT || \
							choice == TEST || choice == DISPLAY )
							sysSpecFlags |= SYSPEC_FORCELOWER;
						}
					else
						if( matchString( strPtr, "type", NO_WILDCARDS ) )
							/* Add file-type association data */
							addTypeAssociation( strPtr + 5 );
						else
							error( UNKNOWN_OPTION, *--strPtr );

					while( *strPtr )
						strPtr++;		/* Skip to end of arg */
#elif defined( __UNIX__ )
					if( matchString( strPtr, "lower", NO_WILDCARDS ) )
						{
						/* Force lower case on file and dir names if it
						   would make sense */
						if( choice == VIEW || choice == EXTRACT || \
							choice == TEST || choice == DISPLAY )
							sysSpecFlags |= SYSPEC_FORCELOWER;
						}
					else
						if( matchString( strPtr, "noumask", NO_WILDCARDS ) )
							/* Ignore umask for file and dir attributes */
							sysSpecFlags |= SYSPEC_NOUMASK;
						else
							if( matchString( strPtr, "device", NO_WILDCARDS ) )
								{
								/* Treat archive name as device for multipart
								   archive to device */
								sysSpecFlags |= SYSPEC_DEVICE;
								flags |= OVERWRITE_SRC;
								}
							else
								error( UNKNOWN_OPTION, *--strPtr );

					while( *strPtr )
						strPtr++;		/* Skip to end of arg */
#elif defined( __VMS__ )
					if( matchString( strPtr, "rsx", NO_WILDCARDS ) )
						/* Translate file/dir names to RSX-11 type format */
						sysSpecFlags |= SYSPEC_RSX11;
					else
						error( UNKNOWN_OPTION, *--strPtr );

					while( *strPtr )
						strPtr++;		/* Skip to end of arg */
#else
					error( LONG_ARG_NOT_SUPPORTED );
#endif /* Various system-specific options */
					break;

				default:
					error( UNKNOWN_OPTION, *--strPtr );
				}
			}
		( *count )++;
		}

	/* If we're performing public-key encryption with a secondary userID,
	   make sure the user has also entered a primary userID */
	if( !( cryptFlags ^ ( CRYPT_SEC | CRYPT_PKE_ALL ) ) && mainUserID == NULL )
		error( MISSING_USERID );

#ifdef __OS2__
	/* Set the flag which controls filename truncation depending on whether
	   we are extracting to an HPFS or FAT filesystem */
	destIsHPFS = queryFilesystemType( basePath );
#endif /* __OS2__ */
#ifdef __MAC__
	/* Remember the vRefNum for the path from/to which we are munging files */
	setVolumeRef( basePath, FILE_PATH );
#endif /* __MAC__ */
	}

/****************************************************************************
*																			*
*								Main Program Code							*
*																			*
****************************************************************************/

#if !defined( __IIGS__ ) && !defined( __MAC__ ) && !defined( __MSDOS__ )

/* Handle program interrupt */

#include <signal.h>

static void progBreak( int dummyValue )
	{
	dummyValue = dummyValue;	/* Get rid of used but not defined warning */
	error( STOPPED_AT_USER_REQUEST );
	}
#endif /* !( __IIGS__ || __MAC__ || __MSDOS__ ) */

#ifdef __MAC__					/* Needed for command-line processing */
  #include <console.h>
#endif /* __MAC__ */

/* The main program */

int main( int argc, char *argv[] )
	{
	int no, count = 1, lastSlash;
	FILEINFO archiveInfo;
	char archivePath[ MAX_PATH ], fileCode[ MATCH_DEST_LEN ], ch;
	BOOLEAN createNew, nothingDone = TRUE, hasFileSpec = FALSE;
	BOOLEAN gotPassword = FALSE, hasWildCards;

#ifdef __MAC__
	/* Get command-line parameters */
	argc = ccommand( &argv );
#endif /* __MAC__ */

	/* Spew forth some propaganda */
	showTitle();

#if !defined( __IIGS__ ) && !defined( __MAC__ ) && !defined( __MSDOS__ )
	/* Trap program break */
  #ifdef __UNIX__
	if( signal( SIGINT, SIG_IGN ) == SIG_IGN )
		/* If we're running in the background, turn on stealth mode so we
		   don't swamp the (l)user with noise */
		flags |= STEALTH_MODE;
	else
  #endif /* __UNIX__ */
	signal( SIGINT, progBreak );
#endif /* !( __IIGS__ || __MAC__ || __MSDOS__ ) */

	/* Perform general initialization */
#if defined( __MSDOS__ )
	initMem();		/* Init mem.mgr - must come before all other inits */
#endif /* __MSDOS__ */
	initFastIO();
#ifndef __OS2__
	initExtraInfo();
#endif /* __OS2__ */
	setDateFormat();
	getScreenSize();

	/* Only initialise the compressor/decompressor if we have to.
	   Unfortunately we need to initialise the decompressor for the VIEW
	   command since there may be compressed archive comments in the data */
	if( ( argc > 1 ) && ( ch = toupper( *argv[ 1 ] ) ) != DELETE )
		/* Only allocate the memory for the compressor if we have to */
		initPack( ch != EXTRACT && ch != TEST && ch != DISPLAY );

	initArcDir();	/* Must be last init in MSDOS version */

	/* If there are not enough command-line args, exit with a help message */
	if( argc < 3 )
		showHelp();

	/* First process command */
	doCommand( argv[ 1 ] );
	count++;
	doArg( argv, &count );
	createNew = ( choice == ADD && ( flags & OVERWRITE_SRC ) ) ? TRUE : FALSE;
										/* Common subexpr.elimination */

	/* Allow room in path for '.xxx\0' suffix */
	if( strlen( argv[ count ] ) > MAX_PATH - 5 )
		error( PATH_s_TOO_LONG, argv[ count ] );
	else
		strcpy( archiveFileName, argv[ count++ ] );

	/* Extract the pathname and convert it into an OS-compatible format */
	lastSlash = extractPath( archiveFileName, archivePath );
#if defined( __ATARI__ ) || defined( __MSDOS__ ) || defined( __OS2__ )
	archiveDrive = ( archiveFileName[ 1 ] == ':' ) ? toupper( *archiveFileName ) - 'A' + 1 : 0;
#endif /* __ATARI__ || __MSDOS__ || __OS2__ */

	/* Either append archive file suffix to the filespec or force the
	   existing suffix to HPAK_MATCH_EXT, and point the variable no to the
	   end of the filename component */
	for( no = strlen( archiveFileName ); \
		 no >= lastSlash && archiveFileName[ no ] != '.';
		 no-- );
#ifdef __UNIX__
	if( !( sysSpecFlags & SYSPEC_DEVICE ) )
		/* Only force the HPACK extension if it's a normal file */
#endif /* __UNIX__ */
	if( archiveFileName[ no ] != '.' )
		{
		no = strlen( archiveFileName );
		strcat( archiveFileName, HPAK_MATCH_EXT );
		}
	else
		strcpy( archiveFileName + no, HPAK_MATCH_EXT );

#ifdef __OS2__
	/* Make sure we get the case-mangling right */
	isHPFS = queryFilesystemType( archiveFileName );
#endif /* __OS2__ */
#ifdef __MAC__
	/* Remember the vRefNum for the archive path.  In addition we strip off
	   the volume name, since if we're creating a multidisk archive the
	   volume name may change with each disk */
	setVolumeRef( archiveFileName, ARCHIVE_PATH );
	no -= stripVolumeName( archivePath );
	lastSlash -= stripVolumeName( archiveFileName );
#endif /* __MAC__ */

	/* Compile the filespec into a form acceptable by the wildcard-matching
	   finite-state machine */
	compileString( archiveFileName + lastSlash, fileCode );

	/* Now add names of files to be processed to the fileName list */
	while( count < argc )
		{
		if( *argv[ count ] == '@' )
			{
			/* Assemble the listFile name + path into the dirBuffer (we can't
			   use the mrglBuffer since processListFile() ovrewrites it) */
			strcpy( ( char * ) dirBuffer, argv[ count++ ] + 1 );
			lastSlash = extractPath( ( char * ) dirBuffer, ( char * ) dirBuffer + 80 );
			compileString( ( char * ) dirBuffer + lastSlash, ( char * ) dirBuffer + 80 );
			hasWildCards = strHasWildcards( ( char * ) dirBuffer + 80 );
#if defined( __ATARI__ ) || defined( __MSDOS__ ) || defined( __MAC__ ) || defined( __OS2__ )
			if( lastSlash && ( ch = dirBuffer[ lastSlash - 1 ] ) != SLASH && \
															  ch != ':' )
#else
			if( lastSlash && ( ch = dirBuffer[ lastSlash - 1 ] ) != SLASH )
#endif /* __ATARI__ || __MAC__ || __MSDOS__ || __OS2__ */
				/* Add a slash if necessary */
				dirBuffer[ lastSlash - 1 ] = SLASH;
			strcpy( ( char * ) dirBuffer + lastSlash, MATCH_ALL );

			/* Check each file in the directory, passing those that match to
			   processListFile() */
			if( findFirst( ( char * ) dirBuffer, FILES, &archiveInfo ) )
				{
				do
					{
					strcpy( ( char * ) dirBuffer + lastSlash, archiveInfo.fName );
					if( matchString( ( char * ) dirBuffer + 80, archiveInfo.fName, hasWildCards ) )
						processListFile( ( char * ) dirBuffer );
					}
				while( findNext( &archiveInfo ) );
				findEnd( &archiveInfo );
				}
			}
		else
			addFilespec( ( char * ) argv[ count++ ] );
		hasFileSpec = TRUE;
		}

	/* Default is all files if no name is given */
	if( !hasFileSpec )
		addFilespec( WILD_MATCH_ALL );

	/* See if we can find a matching archive.  The only case where a non-
	   match is not an error is if we are using the ADD command:  In this
	   case we create a new archive */
#if defined( __ATARI__ ) || defined( __MSDOS__ ) || defined( __OS2__ )
	if( lastSlash && ( ch = archivePath[ lastSlash - 1 ] ) != SLASH && \
														ch != ':' )
#else
	if( lastSlash && ( ch = archivePath[ lastSlash - 1 ] ) != SLASH )
#endif /* __ATARI__ || __MSDOS__ || __OS2__ */
		/* Add a slash if necessary */
		archivePath[ lastSlash - 1 ] = SLASH;
	strcpy( archivePath + lastSlash, MATCH_ARCHIVE );
	if( !findFirst( archivePath, FILES, &archiveInfo ) )
		if( choice == ADD && !hasWildcards( archiveFileName + lastSlash, no - lastSlash ) )
			/* No existing archive found, force the creation of a new one */
			createNew = TRUE;
		else
			error( NO_ARCHIVES );

	/* Set up the encryption system */
	initCrypt();

	/* Now process each archive which matches the given filespec */
again:
	do
		if( createNew || matchString( fileCode, archiveInfo.fName, TRUE ) )
			{
			if( createNew )
				{
#ifdef __UNIX__
				if( !( sysSpecFlags & SYSPEC_DEVICE ) )
					/* Only force the HPACK extension if it's a normal file */
#endif /* __UNIX__ */
				/* Set the extension to the proper form */
				strcpy( archiveFileName + no, HPAK_EXT );
				strcpy( errorFileName, archiveFileName );
				}
			else
				{
				/* Set archivePath as well as archiveFileName to the archive's
				   name, since when processing multiple archives archiveFileName
				   will be stomped */
				strcpy( archivePath + lastSlash, archiveInfo.fName );
				strcpy( archiveFileName, archivePath );
				if( addArchiveName( archiveInfo.fName ) )
					/* Make sure we don't try to process the same archive
					   twice (see the comment for addArchiveName) */
					continue;
				}

			hprintfs( MESG_ARCHIVE_IS_s, archiveFileName );

			/* Reset error stats and arcdir system */
			archiveFD = errorFD = dirFileFD = secFileFD = IO_ERROR;
			oldArcEnd = 0L;
			resetArcDir();
			overWriteEntry = FALSE;
			if( flags & BLOCK_MODE )
				firstFile = TRUE;

			/* Open archive file with various types of mungeing depending on the
			   command type */
			if( choice == ADD || choice == UPDATE )
				{
				/* Create a new archive if none exists or overwrite an existing
				   one if asked for */
				if( createNew )
					{
					errorFD = archiveFD = hcreat( archiveFileName, CREAT_ATTR );
					setOutputFD( archiveFD );
					}
				else
					if( ( archiveFD = hopen( archiveFileName, O_RDWR | S_DENYRDWR | A_SEQ ) ) != IO_ERROR )
						{
						setInputFD( archiveFD );
						setOutputFD( archiveFD );
						readArcDir( SAVE_DIR_DATA );

						/* If we are adding to an existing archive and run out
						   of disk space, we must cut back to the size of the
						   old archive.  We do this by remembering the old state
						   of the archive and restoring it to this state if
						   necessary */
						if( fileHdrStartPtr != NULL )
							/* Only set oldArcEnd if there are files in the archive */
							oldArcEnd = fileHdrCurrPtr->offset + fileHdrCurrPtr->data.dataLen + \
										fileHdrCurrPtr->data.auxDataLen;
						getArcdirState( ( FILEHDRLIST ** ) &oldHdrlistEnd, &oldDirEnd );

						/* Move past existing data */
						hlseek( archiveFD, oldArcEnd + HPACK_ID_SIZE, SEEK_SET );
						}
				}
			else
				{
				/* Try to open the archive for read/write (in case we need to
				   truncate X/Ymodem padding bytes.  If that fails, open it
				   for read-only */
				if( ( archiveFD = hopen( archiveFileName, O_RDWR | S_DENYWR | A_RANDSEQ ) ) == IO_ERROR )
					archiveFD = hopen( archiveFileName, O_RDONLY | S_DENYWR | A_RANDSEQ );
				setInputFD( archiveFD );
				if( archiveFD != IO_ERROR )
					readArcDir( choice != VIEW && choice != EXTRACT && \
								choice != TEST && choice != DISPLAY  );

				/* The password will have been obtained as part the process
				   of reading the archive directory */
				gotPassword = TRUE;
				}

			/* Make sure the open was successful */
			if( archiveFD == IO_ERROR )
				error( CANNOT_OPEN_ARCHFILE, archiveFileName );

			/* We've found an archive to ADD things to, so signal the fact
			   that we don't need to make a second pass with the createNew
			   flag set */
			nothingDone = FALSE;

			/* Set up the password(s) if necessary */
			if( !gotPassword && ( cryptFlags & ( CRYPT_CKE | CRYPT_CKE_ALL ) ) )
				{
				initPassword( MAIN_KEY );
				if( cryptFlags & CRYPT_SEC )
					initPassword( SECONDARY_KEY );

				/* Flag the fact that we no longer need to get the password
				   later on if there are more archives */
				gotPassword = TRUE;
				}

			/* Now munge files to/from archive */
			handleArchive();

			/* The UPDATE command is just a combination of the ADD and FRESHEN
			   commands.  We handle it by first adding new files in one pass
			   (but not complaining when we strike duplicates like ADD does),
			   and then using the freshen command to update duplicates in a
			   second pass.  This leads to a slight problem in that when files
			   are added they will then subsequently match on the freshen pass;
			   hopefully the extra overhead from checking whether they should
			   be freshened will not be too great */
			if( choice == UPDATE )
				{
				choice = FRESHEN;
				oldArcEnd = 0L;	/* Make sure we don't try any ADD-style recovery
								   since errorFD is now the temp file FD */
				vlseek( 0L, SEEK_SET );	/* Go back to start of archive */
				handleArchive();
				}

			/* The EXTRACT command with MOVE_FILES option is just a
			   combination of the standard EXTRACT and DELETE commands.  We
			   handle this by making a second pass which deletes the files.
			   This could be done in one step by combining the EXTRACT and
			   DELETE, but doing it this way is much easier since the two
			   normally involve several mutually exclusive alternatives */
			if( choice == EXTRACT && ( flags & MOVE_FILES ) )
				{
				choice = DELETE;
				oldArcEnd = 0L;			/* Don't try any recovery */
				vlseek( 0L, SEEK_SET );	/* Go back to start of archive */
				handleArchive();
				}

			/* Perform cleanup functions for this file */
			if( choice != DELETE && choice != FRESHEN && choice != REPLACE && \
				!( choice == EXTRACT && ( flags & MOVE_FILES ) ) )
				/* If DELETE, FRESHEN, or REPLACE, archive file is already closed */
				{
				hclose( archiveFD );
				archiveFD = IO_ERROR;	/* Mark it as invalid */
				}

			/* Perform any updating of extra file information on the archive
			   if we've changed it, unless it's a multipart archive which
			   leads to all sorts of complications */
			if( archiveChanged && !( flags & MULTIPART_ARCH ) && \
				!( choice == VIEW || choice == TEST || choice == EXTRACT || choice == DISPLAY ) )
				setExtraInfo( archiveFileName );

			/* Now that we've finished all processing, delete any files we've
			   added if the MOVE_FILES option was used */
			if( archiveChanged && ( flags & MOVE_FILES ) )
				wipeFilePaths();

#if defined( __MSDOS__ )
			/* Reset arcDir filesystem in memory */
			resetArcDirMem();
#endif /* __MSDOS__ */

			/* Only go through the loop once if we are either creating a new
			   archive or processing a multipart archive */
			if( createNew || flags & MULTIPART_ARCH )
				break;
			}
	while( findNext( &archiveInfo ) );

	/* There were no files found to ADD to, so we execute the above loop one
	   more time and create a new archive */
	if( choice == ADD && nothingDone )
		{
		createNew = TRUE;
		goto again;
		}

	/* Finish findFirst() functions */
	findEnd( &archiveInfo );

	/* Complain if we didn't end up doing anything */
	if( !archiveChanged )
		if( choice == FRESHEN )
			{
			/* If we were freshening an archive then the nonexistance of
			   files to change isn't necessarily a problem */
			hputs( MESG_ARCH_IS_UPTODATE );
			closeFiles();
			}
		else
			/* There was nothing to do to the archive, treat as an error
			   condition */
			if( nothingDone )
				/* Couldn't find a matching archive */
				error( NO_ARCHIVES );
			else
				if( choice == ADD || choice == REPLACE )
					/* Couldn't find a matching file to add */
					error( NO_FILES_ON_DISK );
				else
					/* Couldn't find a matching file in the archive */
					error( NO_FILES_IN_ARCHIVE );

	/* Clean up before exiting */
	if( choice == VIEW )
		showTotals();
	endCrypt();
#if defined( __MSDOS__ )
	endMem();		/* The mem.mgr.takes care of all memory freeing */
#else
	if( choice != DELETE )
		endPack();
	endArcDir();
	endExtraInfo();
	endFastIO();
	freeFilespecs();
	freeArchiveNames();
#endif /* __MSDOS__ */

	/* Obesa Cantavit */
	hputs( MESG_DONE );
#ifdef __VMS__
	return( translateVMSretVal( OK ) );
#else
	return( OK );
#endif /* __VMS__ */
	}
