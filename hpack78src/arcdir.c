/****************************************************************************
*																			*
*						  HPACK Multi-System Archiver						*
*						  ===========================						*
*																			*
*					   Archive Directory Handling Routines					*
*						  ARCDIR.C  Updated 10/06/92						*
*																			*
* This program is protected by copyright and as such any use or copying of	*
*  this code for your own purposes directly or indirectly is highly uncool	*
*  				  and if you do so there will be....trubble.				*
* 			  And remember: We know where your kids go to school.			*
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
  #include "error.h"
  #include "flags.h"
  #include "hpacklib.h"
  #include "system.h"
  #include "tags.h"
  #include "fastio.h"
#else
  #include "defs.h"
  #include "arcdir.h"
  #include "error.h"
  #include "flags.h"
  #include "hpacklib.h"
  #include "system.h"
  #include "tags.h"
  #include "io/fastio.h"
#endif /* __MAC__ */

#if defined( __MSDOS__ ) && !defined( GUI )

/* Prototypes for memory management functions */

void markMem( void );
void releaseMem( void );
void ungetMem( void );

#endif /* __MSDOS__ && !GUI */

/* The following are declared in ARCDIRIO.C */

extern PARTSIZELIST *partSizeStartPtr, *partSizeCurrPtr;
extern BOOLEAN arcdirCorrupted;
extern int cryptDirDataLength, cryptFileDataLength;

/* The following are declared in FRONTEND.C */

extern char archiveFileName[];

/* The data structure for the name table for fileNames */

typedef struct NT {
				  char *namePtr;		/* Pointer to the name */
				  WORD dirIndex;		/* Directory index of this filename */
				  WORD hType;			/* Type of this file */
				  struct NT *next;		/* The next entry in this list */
				  } NT_ENTRY;

/* The size of the memory buffer for archive information */

#ifdef ARCHIVE_TYPE
  #define MEM_BUFSIZE	1000	/* More modest memory usage in test vers. */
#else
  #define MEM_BUFSIZE	8000
#endif /* ARCHIVE_TYPE */

/* The size of the hash table.  Must be a power of 2 */

#define HASHTABLE_SIZE	4096

/****************************************************************************
*																			*
*								Global Variables							*
*																			*
****************************************************************************/

FILEHDRLIST *fileHdrStartPtr, *fileHdrCurrPtr;
		/* Start and current position in the list of file headers */

/* vvv This'll be the first thing up against the wall when the rewrite comes */
DIRHDRLIST **dirHdrList;		/* Table to hold directory structure */
int currDirHdrListIndex;		/* Current pos.in list of dir.headers */
int currEntry;					/* Current index into directory array */
int lastEntry;					/* Last used directory in dir.array */

static NT_ENTRY **hashTable;	/* The table for hashing into nameTable */

long fileDataOffset;			/* Offset of current file data from start of
								   archive */
long dirDataOffset;				/* Offset of current directory data from start
								   of directory data area */

WORD fileLinkNo, dirLinkNo;		/* Current maximum dir and file link ID */

/****************************************************************************
*																			*
*						Archive Directory Memory Usage						*
*																			*
****************************************************************************/

/* Struct sizes are as follows:

	FILEHDR			19 bytes		DIRHDR		 	11 bytes
	FILEHDRLIST		43 bytes		DIRHDRLIST		43 bytes
	NT_ENTRY		12 bytes					  ----------
				  ----------						54 bytes + dirName
					65 bytes + fileName

   Data structure sizes are as follows:

	fileHdrList		entries limited by memory
	dirHdrList		8000 bytes, 2000 entries
	hashTable		16K bytes, 4096 entries (fixed size)
	nameTable		entries limited by memory
	--------------------------------------------------------
	Total :		  24,384 bytes */

/****************************************************************************
*																			*
*							General Work Functions							*
*																			*
****************************************************************************/

/* Add a file header to the lists of file headers */

static char *fileNamePtr;

void addFileHeader( FILEHDR *theHeader, WORD hType, BYTE extraInfo, WORD linkID )
	{
	FILEHDRLIST *fileHdrPrevPtr;
	WORD dirNo;

	/* Link the header into the list */
	if( fileHdrStartPtr == NULL )
		fileHdrCurrPtr = fileHdrStartPtr = ( FILEHDRLIST * ) hmalloc( sizeof( FILEHDRLIST ) );
	else
		{
		fileHdrCurrPtr->next = ( FILEHDRLIST * ) hmalloc( sizeof( FILEHDRLIST ) );
		fileHdrCurrPtr = fileHdrCurrPtr->next;
		}
	if( fileHdrCurrPtr == NULL )
		error( OUT_OF_MEMORY );
	fileHdrCurrPtr->next = NULL;
	fileHdrCurrPtr->data = *theHeader;
	fileHdrCurrPtr->offset = fileDataOffset;
	fileHdrCurrPtr->hType = hType;
	fileHdrCurrPtr->fileName = fileNamePtr;
	fileDataOffset += theHeader->dataLen + theHeader->auxDataLen;
	fileHdrCurrPtr->tagged = FALSE;

	/* Allocate room for extraInfo if necessary */
	if( extraInfo )
		{
		if( ( fileHdrCurrPtr->extraInfo = \
				( BYTE * ) hmalloc( getExtraInfoLen( extraInfo ) + 1 ) ) == NULL )
			error( OUT_OF_MEMORY );
		*fileHdrCurrPtr->extraInfo = extraInfo;
		}
	else
		fileHdrCurrPtr->extraInfo = NULL;

	/* Update the head and tail pointers and link in the new header to the
	   list of files in this directory */
	if( ( dirNo = theHeader->dirIndex ) > lastEntry )
		{
		/* Bad entry, move to ROOT_DIR */
		dirNo = theHeader->dirIndex = ROOT_DIR;
		arcdirCorrupted = TRUE;
		}
	if( ( fileHdrPrevPtr = dirHdrList[ dirNo ]->fileInDirListTail ) == NULL )
		/* Set up new list if necessary */
		dirHdrList[ dirNo ]->fileInDirListHead = fileHdrCurrPtr;
	else
		fileHdrPrevPtr->nextFile = fileHdrCurrPtr;
	dirHdrList[ dirNo ]->fileInDirListTail = fileHdrCurrPtr;
	fileHdrCurrPtr->prevFile = fileHdrPrevPtr;
	fileHdrCurrPtr->nextFile = NULL;

	/* Link the new file to the list of linked files if necessary */
	if( extraInfo & EXTRA_LINKED )
		{
		/* Find new link and add it */
/*		addLinkInfo( NO_LINK ); */
/*		Don't process links yet */
		fileHdrCurrPtr->linkID = linkID;
		fileHdrCurrPtr->nextLink = fileHdrCurrPtr->prevLink = NULL;
		}
	else
		{
		fileHdrCurrPtr->linkID = NO_LINK;
		fileHdrCurrPtr->nextLink = fileHdrCurrPtr->prevLink = NULL;
		}
	}

/* Remove a file header from the lists of file headers */

void deleteFileHeader( FILEHDRLIST *theHeader )
	{
	FILEHDRLIST *nextPtr = theHeader->nextFile, *prevPtr = theHeader->prevFile;
	WORD dirNo = theHeader->data.dirIndex;

	/* Unlink this header from the list of files in this directory */
	if( prevPtr == NULL )
		{
		/* Delete from start of list */
		if( dirHdrList[ dirNo ]->fileInDirListTail == theHeader )
			/* Only header in list, set list to empty */
			dirHdrList[ dirNo ]->fileInDirListTail = NULL;
		dirHdrList[ dirNo ]->fileInDirListHead = nextPtr;
		}
	else
		/* Delete from middle/end of list */
		prevPtr->nextFile = nextPtr;
	if( nextPtr != NULL )
		nextPtr->prevFile = prevPtr;

	/* Free the header and associated extraInfo and fileName */
	hfree( theHeader->fileName );
	if( theHeader->data.archiveInfo & ARCH_EXTRAINFO )
		hfree( theHeader->extraInfo );
	hfree( theHeader );
	}

/* Add a fileName to the fileNameList, checking for duplicate names */

static WORD oldHashValue;
static NT_ENTRY *nameTablePtr;

BOOLEAN addFileName( const WORD dirIndex, const WORD hType, const char *fileName )
	{
	int ch = TRUE;
	WORD hashValue = hType;
	NT_ENTRY *tblEntry;
	int fileNameLen = 0;
	BOOLEAN isUnicode = FALSE;
	WORD *wordPtr = ( WORD * ) mrglBuffer;

	/* Check whether we're reading a Unicode string */
	if( ( fileName == NULL ) && ( fileHdrCurrPtr->extraInfo != NULL ) && \
		( *fileHdrCurrPtr->extraInfo & EXTRA_UNICODE ) )
		isUnicode = TRUE;

	/* Insert the string in the name table.  A possible optimization at this
	   point is that if the filename is already in the filename list to not
	   insert it a second time but merely point to the first filename.  The
	   problem with this is that if the first filename is hfree()'d at a later
	   point the second reference will point to hyperspace */
	while( ch )
		{
		/* Get the next char in the name from the appropriate location.
		   Hashing the full name is probably a waste for > 5-odd chars,
		   but we still need to grovel through them all to get them into the
		   fileName list */
		if( fileName == NULL )
			{
			/* Read in char with sanity check:  With an archive which has been
			   corrupted just right we may go into an infinite loop of reading
			   EOF's or garbage here */
			if( isUnicode )
				{
				if( ( ch = fgetWord() ) == FEOF || fileNameLen >= DIRBUFSIZE / sizeof( WORD ) )
					{
					arcdirCorrupted = TRUE;
					return( FALSE );
					}
				}
			else
				{
				if( ( ch = fgetByte() ) == FEOF || fileNameLen >= DIRBUFSIZE )
					{
					arcdirCorrupted = TRUE;
					return( FALSE );
					}
				}
			}
		else
			ch = *fileName++;

#if defined( __AMIGA__ ) || defined( __ARC__ ) || defined( __MAC__ ) || \
	defined( __OS2__ ) || defined( __UNIX__ )
		/* Check if we want to smash case */
		if( sysSpecFlags & SYSPEC_FORCELOWER )
			ch = tolower( ch );
#endif /* __AMIGA__ || __ARC__ || __MAC__ || __OS2__ || __UNIX__ */

		if( ch )	/* Don't hash the final '\0' */
			hashValue = ( hashValue << 1 ) ^ ch;
		if( isUnicode )
			wordPtr[ fileNameLen++ ] = ch;
		else
			mrglBuffer[ fileNameLen++ ] = ch;
		}
	hashValue &= HASHTABLE_SIZE - 1;				/* A faster MOD */

	/* Search the name table, with chaining in case of a collision */
	tblEntry = hashTable[ hashValue ];
	while( tblEntry != NULL )
		if( !strcmp( tblEntry->namePtr, ( char * ) mrglBuffer ) && \
					 tblEntry->dirIndex == dirIndex && tblEntry->hType == hType )
			return( /* tblEntry */ TRUE );	/* Already exists, exit */
		else
			tblEntry = tblEntry->next;

	/* Now add the new entry to the start of the list pointed to by hashtable
	   after remembering the current state for possible future use.  The chains
	   are organised in LRU order for no obvious reason */
	oldHashValue = hashValue;
	if( ( fileNamePtr = ( char * ) hmalloc( fileNameLen ) ) == NULL )
		error( OUT_OF_MEMORY );
	strcpy( fileNamePtr, ( char * ) mrglBuffer );
	if( ( nameTablePtr = ( NT_ENTRY * ) hmalloc( sizeof( NT_ENTRY ) ) ) == NULL )
		error( OUT_OF_MEMORY );
	nameTablePtr->namePtr = fileNamePtr;
	nameTablePtr->dirIndex = dirIndex;
	nameTablePtr->hType = hType;
	nameTablePtr->next = tblEntry;
	hashTable[ hashValue ] = nameTablePtr;

	/* Link the fileName to the header now if we can */
	if( fileName == NULL )
		fileHdrCurrPtr->fileName = fileNamePtr;

	return( FALSE );
	}

/* Delete the last filename entered */

void deleteLastFileName( void )
	{
	hashTable[ oldHashValue ] = nameTablePtr->next;
#if defined( __MSDOS__ ) && !defined( GUI )
	/* Undo the last hmalloc.  This works because there are no hmalloc()'s
	   between the addFileName() and deleteFileName() calls in FRONTEND.C */
	ungetMem();		/* Undo fileName malloc */
	ungetMem();		/* Undo nameTable malloc */
#else
	hfree( fileNamePtr );
#endif /* __MSDOS__ && !GUI */
	}

/* Add a section size for a multipart archive */

void addPartSize( const long segmentEnd )
	{
	/* Link the new part size into the list */
	if( partSizeStartPtr == NULL )
		partSizeCurrPtr = partSizeStartPtr = ( PARTSIZELIST * ) hmalloc( sizeof( PARTSIZELIST ) );
	else
		{
		partSizeCurrPtr->next = ( PARTSIZELIST * ) hmalloc( sizeof( PARTSIZELIST ) );
		partSizeCurrPtr = partSizeCurrPtr->next;
		}
	if( partSizeCurrPtr == NULL )
		error( OUT_OF_MEMORY );
	partSizeCurrPtr->next = NULL;
	partSizeCurrPtr->segEnd = segmentEnd;
	}

/* Determine which archive part a given data offset is in */

int getPartNumber( const long offset )
	{
	PARTSIZELIST *partSizePtr;
	int index;

	/* Find the archive segment in which the offset value falls */
	for( partSizePtr = partSizeStartPtr, index = 0; \
		 partSizePtr != NULL && offset >= partSizePtr->segEnd; \
		 partSizePtr = partSizePtr->next, index++ );

	return( index );
	}

/* Determine the end of a given segment */

long getPartSize( int partNumber )
	{
	PARTSIZELIST *partSizePtr;

	for( partSizePtr = partSizeStartPtr; partNumber; partNumber-- )
		partSizePtr = partSizePtr->next;
	return( ( partSizePtr == NULL ) ? endPosition : partSizePtr->segEnd );
	}

/* Get a given part of an archive */

void readArcTrailer( void );

void getPart( const int thePart )
	{
	/* Ask for the wanted archive part */
	hclose( archiveFD );
	archiveFD = IO_ERROR;		/* Mark it as invalid */
	multipartWait( 0, thePart );
	while( ( archiveFD = hopen( archiveFileName, O_RDONLY | S_DENYWR | A_RANDSEQ ) ) == ERROR )
		multipartWait( 0, thePart );
	setInputFD( archiveFD );
	readArcTrailer();

	/* Make sure we've got the right part */
	while( thePart != currPart )
		{
		hclose( archiveFD );
		archiveFD = IO_ERROR;	/* Mark it as invalid */
		multipartWait( 0, thePart );
		while( ( archiveFD = hopen( archiveFileName, O_RDONLY | S_DENYWR | A_RANDSEQ ) ) == ERROR )
			multipartWait( 0, thePart );
		setInputFD( archiveFD );
		readArcTrailer();
		}
	}

/* Set up the vars used */

void initArcDir( void )
	{
	if( ( hashTable = ( NT_ENTRY ** ) hmalloc( HASHTABLE_SIZE * sizeof( NT_ENTRY * ) ) ) == NULL || \
		( dirHdrList = ( DIRHDRLIST ** ) hmalloc( MEM_BUFSIZE ) ) == NULL )
		error( OUT_OF_MEMORY );

	/* Initially there is no list of headers - this initial reset is necessary
	   to stop resetArcDir() trying to free nonexistant header lists */
	fileHdrStartPtr = NULL;
	partSizeStartPtr = NULL;

#if !defined( __MSDOS__ ) || defined( GUI )
	/* Set dirHdrList to empty so we don't try and free it in freeHdrLists() */
	dirHdrList[ ROOT_DIR ] = NULL;
#endif /* !__MSDOS__ || GUI */
	}

#if !defined( __MSDOS__ ) || defined( GUI )

/* Free the mem used in the list of headers + strings */

static void freeHdrLists( void )
	{
	FILEHDRLIST *fileHdrCursor = fileHdrStartPtr, *fileHdrHeaderPtr;
	PARTSIZELIST *partSizeCursor = partSizeStartPtr, *partSizeHeaderPtr;
	NT_ENTRY *nameTblCursor, *nameTblPtr;
	WORD theEntry, nextEntry;

	/* Free the fileHdr list */
	while( fileHdrCursor != NULL )
		{
		fileHdrHeaderPtr = fileHdrCursor;
		fileHdrCursor = fileHdrCursor->next;
		hfree( fileHdrHeaderPtr->fileName );
		if( fileHdrHeaderPtr->data.archiveInfo & ARCH_EXTRAINFO )
			hfree( fileHdrHeaderPtr->extraInfo );
		hfree( fileHdrHeaderPtr );
		}

	/* Free the dirHdr list */
	if( dirHdrList[ ROOT_DIR ] != NULL )
		for( theEntry = dirHdrList[ ROOT_DIR ]->next; theEntry != END_MARKER; \
			 theEntry = nextEntry )
			{
			nextEntry = dirHdrList[ theEntry ]->next;
			hfree( dirHdrList[ theEntry ]->dirName );
			hfree( dirHdrList[ theEntry ] );
			}

	/* Free the name table lists.  We must be careful not to free the
	   associated fileNames since they have already been freed as part of
	   the fileHdr freeing process */
	for( theEntry = 0; theEntry < HASHTABLE_SIZE; theEntry++ )
		{
		nameTblCursor = hashTable[ theEntry ];
		while( nameTblCursor != NULL )
			{
			nameTblPtr = nameTblCursor;
			nameTblCursor = nameTblCursor->next;
			hfree( nameTblPtr );
			}
		}

	/* Free the archive part size list */
	while( partSizeCursor != NULL )
		{
		partSizeHeaderPtr = partSizeCursor;
		partSizeCursor = partSizeCursor->next;
		hfree( partSizeHeaderPtr );
		}

	/* Reset header list pointer */
	fileHdrStartPtr = NULL;
	partSizeStartPtr = NULL;
	}

void endArcDir( void )
	{
	freeHdrLists();
	hfree( dirHdrList );
	hfree( hashTable );
	}
#else

void resetArcDirMem( void )
	{
	releaseMem();	/* Ah, the joys of a single-user OS */
	}
#endif /* !__MSDOS__ || GUI */

/* Reset various pointers and arrays for each new archive handled */

static void resetLinkCache( void );

void resetArcDir( void )
	{
	DIRHDRLIST *rootDirPtr;
	int theEntry;

	/* Clear out the hash table */
	for( theEntry = 0; theEntry < HASHTABLE_SIZE; theEntry++ )
		hashTable[ theEntry ] = NULL;

	/* Set up initial table indices */
	currDirHdrListIndex = 1;			/* Room for ROOT_DIR header */
	currEntry = lastEntry = ROOT_DIR;   /* Entries added off root dir */
	currPart = lastPart = 0;			/* Only one part to archive */

	/* Free the various lists of headers */
#if !defined( __MSDOS__ ) || defined( GUI )
	freeHdrLists();
#else
	fileHdrStartPtr = NULL;
	partSizeStartPtr = NULL;	/* Reset header list pointers */
	markMem();	/* Cut back the heap to this point later on */
#endif /* !__MSDOS__ || GUI */
	fileDataOffset = dirDataOffset = 0L;	/* Begin at start of archive */
	fileLinkNo = dirLinkNo = NO_LINK + 1;	/* Set up initial magic link no. */
	resetLinkCache();			/* Clear link cache */

	/* Create a dummy header for the root directory */
	if( ( rootDirPtr = dirHdrList[ ROOT_DIR ] = \
		( DIRHDRLIST * ) hmalloc( sizeof( DIRHDRLIST ) ) ) == NULL )
		error( OUT_OF_MEMORY );
	rootDirPtr->dirIndex = ROOT_DIR;
	rootDirPtr->next = END_MARKER;
	rootDirPtr->data.dirInfo = 0;
	rootDirPtr->data.dirTime = 0L;
	rootDirPtr->data.dataLen = 0L;
	rootDirPtr->data.parentIndex = ROOT_DIR;
	rootDirPtr->fileInDirListHead = rootDirPtr->fileInDirListTail = NULL;
	rootDirPtr->dirInDirListHead = rootDirPtr->dirInDirListTail = NULL;
	rootDirPtr->nextDir = rootDirPtr->prevDir = NULL;
	rootDirPtr->nextLink = rootDirPtr->prevLink = NULL;
	rootDirPtr->dirName = ( char * ) hmalloc( 1 );	/* No need for NULL chk */
	*rootDirPtr->dirName = '\0';	/* The dir with no name */
	rootDirPtr->offset = 0L;
	rootDirPtr->hType = DIRTYPE_NORMAL;
	rootDirPtr->linkID = NO_LINK;
	rootDirPtr->tagged = FALSE;

	/* Reset the directory data buffer */
	dirBufCount = 0;

	/* Initially we have no crypt information */
	cryptFileDataLength = cryptDirDataLength = 0;
	}

/* Return the current state of the archive directory data structures.
   doFixEverything can safely be initialized statically since as soon
   as it is set to FALSE we exit due to the error processing */

BOOLEAN doFixEverything = TRUE;

void getArcdirState( FILEHDRLIST **hdrListEnd, int *dirEnd )
	{
	*hdrListEnd = fileHdrCurrPtr;
	*dirEnd = currEntry;
	}

/* Set the current archive directory structures.  The doFixEverything flag
   must be set to false at this point to make sure we don't call
   fixEverything() when performing error recovery, since calling
   fixEverything() will rebuild the directory tree into an invalid (read:
   stuffed) state */

void setArcdirState( FILEHDRLIST *hdrListEnd, const int dirEnd )
	{
#if !defined( __MSDOS__ ) || defined( GUI )
	FILEHDRLIST *fileHdrCursor = hdrListEnd->next, *fileHdrPtr;
#endif /* !__MSDOS__ || GUI */

	if( hdrListEnd != NULL )	/* This may be NULL if no files */
		{
#if !defined( __MSDOS__ ) || defined( GUI )
		/* Remove all file headers after the cutoff point */
		while( fileHdrCursor != NULL )
			{
			fileHdrPtr = fileHdrCursor;
			fileHdrCursor = fileHdrCursor->next;
			deleteFileHeader( fileHdrPtr );
			}
#endif /* !__MSDOS__ || GUI */
		hdrListEnd->next = NULL;
		}
	dirHdrList[ dirEnd ]->next = END_MARKER;
	doFixEverything = FALSE;
	}

/* Get the length of the compressed data in the archive */

long getCoprDataLength( void )
	{
	return( fileDataOffset );
	}

/****************************************************************************
*																			*
*					File/Directory Link Handling Functions					*
*																			*
****************************************************************************/

/* The link cache management code.  This is used to speed lookups for
   link ID's.  If we have a cache miss (only under exceptional circumstances),
   a linear search of the file list is done */

#ifdef __MSDOS__
  #define CACHE_SIZE		16
#else
  #define CACHE_SIZE		64
#endif /* __MSDOS__ */

typedef struct {
			   WORD linkID;			/* The link ID */
			   void *linkChain;		/* The link chain it points to */
			   } CACHE_ENTRY;

CACHE_ENTRY linkCache[ CACHE_SIZE ];
int cacheIndex;
BOOLEAN cacheFull;

/* Clear the link cache */

static void resetLinkCache( void )
	{
	int i;

	for( i = 0; i < CACHE_SIZE; i++ )
		linkCache[ i ].linkID = NO_LINK;
	cacheIndex = 0;
	cacheFull = FALSE;
	}

/* Look up a link ID in the link cache */

static int linkCacheLookup( const WORD linkID )
	{
	int i;

	/* Search cache for matching link ID */
	for( i = 0; i < cacheIndex; i++ )
		if( linkCache[ i ].linkID == linkID )
			return( i );

	/* No match found in cache */
	return( ERROR );
	}

/* Update the cache entries */

static void updateLinkCache( int cachePos )
	{
	int newPos = cachePos >> 1, i;
	CACHE_ENTRY theEntry = linkCache[ cachePos ];

	/* Move all the cache entries back one and move the entry to update
	   to the newly vacated position */
	for( i = cachePos; i > newPos; i-- )
		linkCache[ i ] = linkCache[ i - 1 ];
	linkCache[ newPos ] = theEntry;
	}

/* Enter a link ID in the cache */

static int addLinkToCache( WORD linkID, void *linkChain )
	{
	if( cacheFull )
		{
		/* Overwrite the LRU entry */
		linkCache[ CACHE_SIZE - 1 ].linkID = linkID;
		linkCache[ CACHE_SIZE - 1 ].linkChain = linkChain;
		return( CACHE_SIZE - 1 );
		}
	else
		{
		/* Just add the entry in the next available position */
		linkCache[ cacheIndex ].linkID = linkID;
		linkCache[ cacheIndex++ ].linkChain = linkChain;
		if( cacheIndex == CACHE_SIZE )
			cacheFull = TRUE;
		return( cacheIndex - 1 );
		}
	}

/* Find the link chain corresponding to a given file/directory link ID */

FILEHDRLIST *findFileLinkChain( WORD linkID )
	{
	FILEHDRLIST *fileHdrCursor;
	int index;

	linkID &= ~LINK_ANCHOR;		/* Mask out anchor bit */

	/* Try and find the linkID in the cache */
	if( ( index = linkCacheLookup( linkID ) ) == ERROR )
		{
		/* Not in cache, need to do linear search of file header list.
		   Note that we always find a match since we've just added the
		   header with this linkID to the chain */
		for( fileHdrCursor = fileHdrStartPtr; fileHdrCursor != NULL; \
			 fileHdrCursor = fileHdrCursor->next )
			if( fileHdrCursor->linkID == linkID )
				break;

		index = addLinkToCache( linkID, fileHdrCursor );
		}
	else
		fileHdrCursor = ( FILEHDRLIST * ) linkCache[ index ].linkChain;

	/* Update the position of the cache entry */
	updateLinkCache( index );
	return( fileHdrCursor );
	}

DIRHDRLIST *findDirLinkChain( WORD linkID )
	{
	DIRHDRLIST *dirHdrCursor;
	WORD theEntry;
	int index;

	linkID &= ~LINK_ANCHOR;		/* Mask out anchor bit */

	/* Try and find the linkID in the cache */
	if( ( index = linkCacheLookup( linkID ) ) == ERROR )
		{
		/* Not in cache, need to do linear search of dir header list.
		   Note that we always find a match since we've just added the
		   header with this linkID to the chain */
		for( theEntry = dirHdrList[ ROOT_DIR ]->next; theEntry != END_MARKER; \
			 theEntry = dirHdrList[ theEntry ]->next )
			if( dirHdrList[ theEntry ]->linkID == linkID )
				break;
		dirHdrCursor = dirHdrList[ theEntry ];

		index = addLinkToCache( linkID, dirHdrCursor );
		}
	else
		dirHdrCursor = ( DIRHDRLIST * ) linkCache[ index ].linkChain;

	/* Update the position of the cache entry */
	updateLinkCache( index );
	return( dirHdrCursor );
	}

#ifdef __UNIX__

/* Unix-specific link-management code.  It would be nice to be able to
   cache link lookups here too but the inodes are usually unique so
   the cache won't work - we need to use a slow linear search.

   All links found in this manner are hard links, and will have the anchor-
   node bit set.  Soft links are dependant links, and are handled
   seperately */

/* Check if the file link ID's are identical.  The file link ID's are the
   device number and inode */

FILEHDRLIST *linkFileHdr;

BOOLEAN checkForLink( const LONG fileLinkID )
	{
	/* Step along the chain of headers checking if the linkID's are identical */
	for( linkFileHdr = fileHdrStartPtr; linkFileHdr != NULL; \
		 linkFileHdr = linkFileHdr->next )
		if( linkFileHdr->fileLinkID == fileLinkID )
			return( TRUE );

	return( FALSE );
	}

/* Add the link ID to a file header */

void addLinkInfo( const LONG linkID )
	{
	/* Step along the chain of headers checking if the linkID's are identical */
	for( linkFileHdr = fileHdrStartPtr; linkFileHdr != NULL; \
		 linkFileHdr = linkFileHdr->next )
		if( linkFileHdr->linkID == linkID )
			{
			/* Check if there is already a link chain set up for this link ID */
			if( linkFileHdr->fileLinkID == NO_LINK )
				{
				/* No, set the link number for the two linked files */
				linkFileHdr->fileLinkID = fileHdrCurrPtr->fileLinkID = \
										  fileLinkNo | LINK_ANCHOR;
				fileLinkNo++;
				}
			else
				{
				/* Go to the end of the link chain and add the link number */
				while( linkFileHdr->nextLink != NULL )
					linkFileHdr = linkFileHdr->nextLink;
				fileHdrCurrPtr->fileLinkID = linkFileHdr->fileLinkID;
				}

			/* Add the header to the link chain */
			linkFileHdr->nextLink = fileHdrCurrPtr;
			fileHdrCurrPtr->prevLink = linkFileHdr;
			fileHdrCurrPtr->nextLink = NULL;

			return;
			}

	/* No link found, set up header in isolation */
	fileHdrCurrPtr->nextLink = NULL;
	fileHdrCurrPtr->prevLink = NULL;
	}
#endif /* __UNIX__ */

/****************************************************************************
*																			*
*						Archive Directory Handling Functions				*
*																			*
****************************************************************************/

/* Pointers into the the fileHdrList and dirHdrList, used by the
   getFirst/NextFileEntry() and getFirst/NextDirEntry() routines */

static FILEHDRLIST *filePtr;
static DIRHDRLIST *dirPtr;

/* The current directory being handled by getFirstDir/getNextDir() */

static WORD currDir;

/* Add a directory header to the directory-by-directory list of its parent
   directory */

void addDirHdrToList( DIRHDRLIST *theHeader )
	{
	int dirNo = theHeader->data.parentIndex;
	DIRHDRLIST *dirHdrPrevPtr;

	if( dirNo > lastEntry )
		{
		/* Bad entry, move to ROOT_DIR */
		dirNo = theHeader->data.parentIndex = ROOT_DIR;
		arcdirCorrupted = TRUE;
		}

	/* Update the dir head and tail pointers and link in the new header */
	if( ( dirHdrPrevPtr = dirHdrList[ dirNo ]->dirInDirListTail ) == NULL )
		/* Set up new list if necessary */
		dirHdrList[ dirNo ]->dirInDirListHead = theHeader;
	else
		dirHdrPrevPtr->nextDir = theHeader;
	dirHdrList[ dirNo ]->dirInDirListTail = theHeader;
	theHeader->prevDir = dirHdrPrevPtr;
	theHeader->nextDir = NULL;

	/* Clear directory links */
	theHeader->nextLink = theHeader->prevLink = NULL;
	}

/* Add a directory name to the list of directory names */

void addDirName( char *dirName, const LONG dirTime )
	{
	DIRHDRLIST *theHeader;

	if( ( theHeader = ( DIRHDRLIST * ) hmalloc( sizeof( DIRHDRLIST ) ) ) == NULL )
		error( OUT_OF_MEMORY );
	dirHdrList[ lastEntry + 1 ] = theHeader;
	currDirHdrListIndex++;
	if( currDirHdrListIndex >= MEM_BUFSIZE )
		error( OUT_OF_MEMORY );

	/* Link in new header */
	dirHdrList[ lastEntry ]->next = lastEntry + 1;
	theHeader->data.dirInfo = 0;
	theHeader->data.parentIndex = currEntry;
	theHeader->data.dirTime = dirTime;
	theHeader->data.dataLen = 0L;
	theHeader->fileInDirListHead = theHeader->fileInDirListTail = NULL;
	theHeader->dirInDirListHead = theHeader->dirInDirListTail = NULL;
	theHeader->nextLink = theHeader->prevLink = NULL;
	theHeader->offset = dirDataOffset;
	theHeader->next = END_MARKER;
	theHeader->dirIndex = lastEntry + 1;
	theHeader->hType = DIRTYPE_NORMAL;
	theHeader->linkID = NO_LINK;
	theHeader->tagged = FALSE;

	/* Add the header to the directory-by-directory list */
	lastEntry++;
	currEntry = lastEntry;
	addDirHdrToList( theHeader );

#if defined( __AMIGA__ ) || defined( __ARC__ ) || defined( __MAC__ ) || \
	defined( __OS2__ ) || defined( __UNIX__ )
	/* Check if we want to smash case */
	if( sysSpecFlags & SYSPEC_FORCELOWER )
		strlwr( dirName );
#endif /* __AMIGA__ || __ARC__ || __MAC__ || __OS2__ || __UNIX__ */

	/* Add the directory name */
	if( ( theHeader->dirName = ( char * ) hmalloc( strlen( dirName ) + 1 ) ) == NULL )
		error( OUT_OF_MEMORY );
	strcpy( theHeader->dirName, dirName );
	}

/* Write out directory data tag */

int addDirData( WORD tagID, const int store, LONG dataLength, LONG uncoprLength )
	{
	int headerLen;

	if( dirFileFD == IO_ERROR )
		{
		/* Create a temporary file to hold any relevant directory
		   information if necessary.  We could do this automatically in
		   FRONTEND.C, but by doing it here we only create it if there
		   is a need to */
		strcpy( dirFileName, archiveFileName );
		strcpy( dirFileName + strlen( dirFileName ) - 3, DIRTEMP_EXT );
		if( ( dirFileFD = hcreat( dirFileName, CREAT_ATTR ) ) == IO_ERROR )
			error( CANNOT_OPEN_TEMPFILE );
		}

	/* Write the tag and update the archive directory information */
	headerLen = writeDirTag( tagID, store, dataLength, uncoprLength );
	dataLength += headerLen;
	dirHdrList[ currEntry ]->data.dataLen += dataLength;
	dirDataOffset += dataLength;
	return( headerLen );
	}

/* Move to the start of the directory data in the archive */

void movetoDirData( void )
	{
	int dummy;

	/* Move to start of directory data plus the offset of the current directory */
	vlseek( getCoprDataLength() + dirHdrList[ currDir ]->offset, SEEK_SET );
	resetFastIn();

	/* Process the encryption header if necessary */
	if( ( cryptFlags & ( CRYPT_CKE_ALL | CRYPT_PKE_ALL ) ) && \
			!cryptSetInput( dirDataOffset, &dummy ) )
		error( CANNOT_PROCESS_CRYPT_ARCH );
	}

/* Set the current archive directory */

inline void setArchiveCwd( const WORD dirNo )
	{
	currEntry = currDir = dirNo;
	}

/* Go back up one directory level in the dirNameIndex */

inline void popDir( void )
	{
	currEntry = dirHdrList[ currEntry ]->data.parentIndex;
	}

/* Return the parent of the current directory */

inline WORD getParent( const WORD dirNo )
	{
	return( dirHdrList[ dirNo ]->data.parentIndex );
	}

/* Return the name of the specified directory */

inline char *getDirName( const WORD dirNo )
	{
	return( dirHdrList[ dirNo ]->dirName );
	}

/* Return the creation date of the specified directory */

inline LONG getDirTime( const WORD dirNo )
	{
	return( dirHdrList[ dirNo ]->data.dirTime );
	}

/* Set the creation date of the specified directory */

inline void setDirTime( const WORD dirNo, const LONG dirTime )
	{
	dirHdrList[ dirNo ]->data.dirTime = dirTime;
	}

/* Return the length of the specified directories data */

inline LONG getDirAuxDatalen( const WORD dirNo )
	{
	return( dirHdrList[ dirNo ]->data.dataLen );
	}

/* Get the tagged status of the specified directory */

inline BOOLEAN getDirTaggedStatus( const WORD dirNo )
	{
	return( dirHdrList[ dirNo ]->tagged );
	}

/* Set the tagged status of the specified directory */

inline void setDirTaggedStatus( const WORD dirNo )
	{
	dirHdrList[ dirNo ]->tagged = TRUE;
	}

/* Sort the files in a directory by name.  This routine uses a modified
   insertion sort whose runtime is proportional to the number of files out of
   order.  This algorithm runs in constant time when the files are mostly in
   order, which is usually the case.  In contrast a more advanced algorithm
   like quicksort would run in O( n^2 ) time and require O( n ) stack space
   for recursion in a similar situation.  The best algorithm is actually a
   mergesort, but the code is more than twice as long */

void sortFiles( const WORD dirNo )
	{
	FILEHDRLIST *oldStartPtr = dirHdrList[ dirNo ]->fileInDirListHead;
	FILEHDRLIST *newStartPtr, *newEndPtr;
	FILEHDRLIST *oldCurrent, *newCurrent, *prev;
	char *fileName;

	newEndPtr = newStartPtr = oldStartPtr;
	if( newStartPtr != NULL )
		{
		/* Build first node of new list */
		oldCurrent = oldStartPtr->nextFile;
		newStartPtr->nextFile = NULL;

		/* Copy the old list across to the new list, sorting it as we go */
		while( oldCurrent != NULL )
			{
			fileName = oldCurrent->fileName;
			if( strcmp( newEndPtr->fileName, fileName ) < 0 )
				{
				/* Filename is in order, just append node to end of new list */
				newCurrent = oldCurrent;
				oldCurrent = oldCurrent->nextFile;	/* Go to next node in old list */

				/* Link node to end of list and set newEnd ptr */
				newEndPtr->nextFile = newCurrent;
				newCurrent->prevFile = newEndPtr;
				newCurrent->nextFile = NULL;
				newEndPtr = newCurrent;
				}
			else
				{
				/* Find correct position in new list to insert node from old list */
				prev = newCurrent = newStartPtr;
				while( ( newCurrent != NULL ) && strcmp( newCurrent->fileName, fileName ) < 0 )
					{
					prev = newCurrent;
					newCurrent = newCurrent->nextFile;
					}

				/* Remember end of new list for shortcut add */
				if( newCurrent == NULL )
					newEndPtr = oldCurrent;

				/* Link node from old list into new list */
				if( prev == newCurrent )
					{
					/* At start of list, set node to new list start */
					newStartPtr = oldCurrent;
					newStartPtr->prevFile = NULL;
					}
				else
					{
					/* Add link to previous node */
					prev->nextFile = oldCurrent;
					oldCurrent->prevFile = prev;
					}

				/* Add link to next node */
				prev = oldCurrent->nextFile;	/* Use prev for temporary storage */
				oldCurrent->nextFile = newCurrent;
				newCurrent->prevFile = oldCurrent;
				oldCurrent = prev;
				}
			}

		/* Set fileInDirListHead to point to start of sorted list */
		dirHdrList[ dirNo ]->fileInDirListHead = newStartPtr;
		}
	}

/* Return the first entry in the file-in-directory list, or NULL if no files */

inline FILEHDRLIST *getFirstFileEntry( const WORD dirNo )
	{
	return( filePtr = dirHdrList[ dirNo ]->fileInDirListHead );
	}

/* Return the next entry in the file-in-directory list, or NULL if none */

inline FILEHDRLIST *getNextFileEntry( void )
	{
	return( filePtr = filePtr->nextFile );
	}

#ifdef GUI

/* Return the previous entry in the file-in-directory list, or NULL if none */

inline FILEHDRLIST *getPrevFileEntry( void )
	{
	return( filePtr = filePtr->prevFile );
	}
#endif /* GUI */

/* Return the first entry in the directory-in-directory list, or NULL if no files */

inline DIRHDRLIST *getFirstDirEntry( const WORD dirNo )
	{
	return( dirPtr = dirHdrList[ dirNo ]->dirInDirListHead );
	}

/* Return the next entry in the directory-in-directory list, or NULL if none */

inline DIRHDRLIST *getNextDirEntry( void )
	{
	return( dirPtr = dirPtr->nextDir );
	}

#ifdef GUI

/* Return the previous entry in the directory-in-directory list, or NULL if none */

inline DIRHDRLIST *getPrevDirEntry( void )
	{
	return( dirPtr = dirPtr->prevDir );
	}
#endif /* GUI */

/* Return the first directory in the directory tree */

inline WORD getFirstDir( void )
	{
	currDir = ROOT_DIR;
	return( ROOT_DIR );
	}

/* Return the next directory in the directory tree */

inline WORD getNextDir( void )
	{
	currDir = dirHdrList[ currDir ]->next;
	return( currDir );
	}

/* Determine whether a given path is contained in the archives directory
   structure.  It would be easier to do the matching backwards since a
   directory can only ever have one parent and so we don't have to search a
   list of directory names; however this won't work because we don't know in
   advance the directory index of the bottom directory */

int matchPath( const char *pathName, const int pathLen, WORD *pathDirIndex )
	{
	BOOLEAN matched, hasSlash = ( *pathName == SLASH ) ? TRUE : FALSE;
	int startIndex = hasSlash, endIndex, segmentLen;
	WORD currDirIndex = ROOT_DIR;
	DIRHDRLIST *currDirPtr, *prevDirPtr;
	char *dirName;

	*pathDirIndex = ROOT_DIR;	/* Initially we're at the root directory */

	/* Check for the trivial case of the path being to the archive's root
	   directory */
	if( !pathLen )
		return( PATH_FULLMATCH );

	while( TRUE )
		{
		/* Extract a directory name out of the pathname */
		endIndex = startIndex;
		while( endIndex < pathLen && pathName[ endIndex ] != SLASH )
			endIndex++;

		/* Now search through the list of subdirectories in this directory
		   trying to find a match */
		currDirPtr = dirHdrList[ currDirIndex ]->dirInDirListHead;
		segmentLen = endIndex - startIndex;
		matched = FALSE;
		if( currDirPtr != NULL )
			do
				{
				dirName = currDirPtr->dirName;

				/* We must not only check that the two directory names are
				   equal but also that they are of the same length, since
				   we could get an erroneous match if one directory name is
				   merely a prefix of another */
				if( !caseStrncmp( pathName + startIndex, dirName, segmentLen ) && \
								segmentLen == strlen( dirName ) )
					{
					matched = TRUE;
					break;
					}
				}
			while( ( currDirPtr = currDirPtr->nextDir ) != NULL );

		if( endIndex == pathLen || !matched )
			/* We've reached the end of the pathname */
			break;

		startIndex = endIndex + 1;		/* +1 skips SLASH */
		prevDirPtr = currDirPtr;
		currDirIndex = currDirPtr->dirIndex;
		}

	/* Determine the nature of the match */
	if( matched )
		{
		/* Full match: currDirPtr contains the directory index */
		*pathDirIndex = currDirPtr->dirIndex;
		return( PATH_FULLMATCH );
		}
	else
		if( startIndex == hasSlash )
			/* No match at all */
			return( PATH_NOMATCH );
		else
			{
			/* Partial match: prevDirPtr contains last matched directory */
			*pathDirIndex = prevDirPtr->dirIndex;
			return( PATH_PARTIALMATCH );
			}
	}

/* Clean up the archive directory tree by first turning it into a correct
   depth-first traversal with contiguous directory indices (note that since
   this procedure rearranges the order of the directories within the archive
   it will destroy the correspondence of the dirData to the directory
   entries), and then traversing the file and directory lists and setting the
   link ID's to contiguous values */

#define MAX_DIR_NESTING	50	/* Shouldn't get > 50 nested directories */

void fixEverything( void )
	{
	BOOLEAN skipFind = FALSE, moreDirs = TRUE, isAnchorNode;
	FILEHDRLIST *fileInfo, *fileHdrCursor, *savedFilePtr = filePtr;
	DIRHDRLIST *dirPtr, *dirPtrStack[ MAX_DIR_NESTING ];
	WORD dirStack[ MAX_DIR_NESTING ], dirNo;
	int currentLevel = 0, currCount = 0, lastCount = 0;

	/* First, clean up the directory structure */
	if( ( dirPtr = dirHdrList[ ROOT_DIR ]->dirInDirListHead ) == NULL )
		moreDirs = FALSE;
	else
		skipFind = TRUE;

	lastEntry = ROOT_DIR;
	while( TRUE )
#pragma warn -pia
		if( moreDirs && ( skipFind || ( dirPtr = dirPtr->nextDir ) != NULL ) )
#pragma warn +pia
			{
			skipFind = FALSE;	/* It's 3am - do you know where your brain is? */
			dirPtrStack[ currentLevel ] = dirPtr;
			dirStack[ currentLevel++ ] = currCount;
			if( currentLevel > MAX_DIR_NESTING )
				error( TOO_MANY_LEVELS_NESTING );

			/* Set correct directory number and rebuild chain of directories
			   in correct order */
			dirNo = dirPtr->dirIndex;
			dirHdrList[ dirNo ]->data.parentIndex = currCount;
			lastCount++;
			currCount = lastCount;
			dirHdrList[ lastEntry ]->next = dirNo;
			lastEntry = dirNo;

			/* Process all files in this directory.  We never bother doing
			   ROOT_DIR since all files in this automatically have the correct
			   directory index */
			if( ( fileInfo = getFirstFileEntry( dirNo ) ) != NULL )
				do
					fileInfo->data.dirIndex = lastCount;
				while( ( fileInfo = getNextFileEntry() ) != NULL );

			if( ( dirPtr = dirHdrList[ dirNo ]->dirInDirListHead ) == NULL )
				moreDirs = FALSE;
			else
				skipFind = TRUE;
			}
		else
			if( currentLevel )
				{
				dirPtr = dirPtrStack[ --currentLevel ];
				currCount = dirStack[ currentLevel ];
				moreDirs = TRUE;
				}
			else
				break;

	dirHdrList[ lastEntry ]->next = END_MARKER;
	filePtr = savedFilePtr;

	/* Now clean up directory links if there are any */
	if( dirLinkNo > NO_LINK + 1 )
		{
		/* Reset dirLinkNo to lowest possible link value */
		dirLinkNo = NO_LINK + 1;

		/* Step through the list of dir.headers looking for linked directories */
		for( dirNo = dirHdrList[ ROOT_DIR ]->next; dirNo != END_MARKER; \
			 dirNo = dirHdrList[ dirNo ]->next )
			if( getLinkID( dirHdrList[ dirNo ]->linkID ) != NO_LINK )
				{
				dirPtr = dirHdrList[ dirNo ];

				/* If there are previous links in the chain, set the link ID
				   to that of the previous link.  Otherwise, add a new link ID */
				isAnchorNode = isLinkAnchor( dirPtr->linkID );
				if( dirPtr->prevLink != NULL )
					dirPtr->linkID = dirPtr->prevLink->linkID;
				else
					dirPtr->linkID = dirLinkNo++;

				/* Mark new link ID as an anchor node if necessary */
				if( isAnchorNode )
					dirPtr->linkID |= LINK_ANCHOR;
				}
		}

	/* Finally, clean up any file links */
	if( fileLinkNo > NO_LINK + 1 )
		{
		/* Reset fileLinkNo to lowest possible link value */
		fileLinkNo = NO_LINK + 1;

		/* Step through the list of file headers looking for linked files */
		for( fileHdrCursor = fileHdrStartPtr; fileHdrCursor != NULL; \
			 fileHdrCursor = fileHdrCursor->next )
			if( getLinkID( fileHdrCursor->linkID ) != NO_LINK )
				{
				/* If there are previous links in the chain, set the link ID
				   to that of the previous link.  Otherwise, add a new link ID */
				isAnchorNode = isLinkAnchor( fileHdrCursor->linkID );
				if( fileHdrCursor->prevLink != NULL )
					fileHdrCursor->linkID = fileHdrCursor->prevLink->linkID;
				else
					fileHdrCursor->linkID = fileLinkNo++;

				/* Mark new link ID as an anchor node if necessary */
				if( isAnchorNode )
					fileHdrCursor->linkID |= LINK_ANCHOR;
				}
		}
	}

/* Select/unselect a file */

void selectFile( FILEHDRLIST *fileHeader, BOOLEAN status )
	{
	fileHeader->tagged = status;
	}

/* Select/unselect all subdirectories and files in a directory.
   selectDir() takes a DIRHDRLIST entry as its argument, selectDirNo()
   takes a directory ID */

void selectDirNo( const WORD dirNo, BOOLEAN status )
	{
	selectDir( dirHdrList[ dirNo ], status );
	}

void selectDir( DIRHDRLIST *dirHeader, BOOLEAN status )
	{
	BOOLEAN skipFind = FALSE, moreDirs = TRUE;
	FILEHDRLIST *fileInfo, *savedFilePtr = filePtr;
	DIRHDRLIST *dirPtr, *dirPtrStack[ MAX_DIR_NESTING ];
	int dirNo, currentLevel = 0;

	/* Tag the directory itself and its files */
	dirHeader->tagged = status;
	if( ( fileInfo = getFirstFileEntry( dirHeader->dirIndex ) ) != NULL )
		do
			fileInfo->tagged = status;
		while( ( fileInfo = getNextFileEntry() ) != NULL );

	/* Now tag any subdirectories and their files */
	if( ( dirPtr = dirHeader->dirInDirListHead ) == NULL )
		moreDirs = FALSE;
	else
		skipFind = TRUE;

	while( TRUE )
#pragma warn -pia
		if( moreDirs && ( skipFind || ( dirPtr = dirPtr->nextDir ) != NULL ) )
#pragma warn +pia
			{
			skipFind = FALSE;
			dirPtrStack[ currentLevel++ ] = dirPtr;
			if( currentLevel > MAX_DIR_NESTING )
				error( TOO_MANY_LEVELS_NESTING );
			dirPtr->tagged = status;

			/* Process all files in this directory */
			dirNo = dirPtr->dirIndex;
			if( ( fileInfo = getFirstFileEntry( dirNo ) ) != NULL )
				do
					fileInfo->tagged = status;
				while( ( fileInfo = getNextFileEntry() ) != NULL );

			if( ( dirPtr = dirHdrList[ dirNo ]->dirInDirListHead ) == NULL )
				moreDirs = FALSE;
			else
				skipFind = TRUE;
			}
		else
			if( currentLevel )
				{
				dirPtr = dirPtrStack[ --currentLevel ];
				moreDirs = TRUE;
				}
			else
				break;

	filePtr = savedFilePtr;
	}

/* Select only the immediate contents of a directory */

void selectDirNoContents( const WORD dirNo, BOOLEAN status )
	{
	FILEHDRLIST *fileInfo, *savedFilePtr = filePtr;
	DIRHDRLIST *dirInfo, *savedDirPtr = dirPtr;

	/* Tag the files within the directory */
	if( ( fileInfo = getFirstFileEntry( dirNo ) ) != NULL )
		do
			fileInfo->tagged = status;
		while( ( fileInfo = getNextFileEntry() ) != NULL );
	filePtr = savedFilePtr;

	/* Tag any directories within it */
	if( ( dirInfo = getFirstDirEntry( dirNo ) ) != NULL )
		do
			dirInfo->tagged = status;
		while( ( dirInfo = getNextDirEntry() ) != NULL );
	dirPtr = savedDirPtr;
	}

/* Calculate the size of a header as it will be written to disk */

int computeHeaderSize( const FILEHDR *theHeader, const BYTE extraInfo )
	{
	int headerSize = sizeof( WORD ) + sizeof( LONG );

	/* Evaluate size of main header */
	headerSize += ( theHeader->fileLen > 0xFFFF ) ? \
		sizeof( LONG ) : sizeof( WORD );
	headerSize += ( theHeader->dataLen > 0xFFFF ) ? \
		sizeof( LONG ) : sizeof( WORD );
	headerSize += ( theHeader->dirIndex > 0xFF ) ? sizeof( WORD ) + sizeof( WORD ) : \
				  ( theHeader->auxDataLen > 0xFF ) ? sizeof( BYTE ) + sizeof( WORD ) : \
				  ( theHeader->dirIndex || theHeader->auxDataLen ) ? \
					sizeof( BYTE ) + sizeof( BYTE ) : 0;

	/* Evaluate size of extraInfo information */
	headerSize += getExtraInfoLen( extraInfo );
	if( extraInfo )
		headerSize++;	/* Allow for size of extraInfo byte */

	return( headerSize );
	}
