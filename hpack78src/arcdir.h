/****************************************************************************
*																			*
*							HPACK Multi-System Archiver						*
*							===========================						*
*																			*
*				Header File for the Directory Handling Routines				*
*							ARCDIR.H  Updated 14/03/92						*
*																			*
* This program is protected by copyright and as such any use or copying of	*
*  this code for your own purposes directly or indirectly is highly uncool	*
*  					and if you do so there will be....trubble.				*
* 				And remember: We know where your kids go to school.			*
*																			*
*		Copyright 1989 - 1992  Peter C.Gutmann.  All rights reserved		*
*																			*
****************************************************************************/

#ifndef _ARCDIR_DEFINED

#define _ARCDIR_DEFINED

#ifdef __MAC__
  #include "filehdr.h"
  #include "hpackio.h"
#else
  #include "filehdr.h"
  #include "io/hpackio.h"
#endif /* __MAC__ */

/* The size of a directory header as written to disk.  This must be defined
   in terms of its constituent fields since some compilers will pad the
   struct fields out for alignment reasons */

#define DIRHDRSIZE	( sizeof( LONG ) + sizeof( WORD ) + sizeof( WORD ) )

/* The data structure for the list of file headers.  This contains the
   global information which is identical for all versions of HPACK, plus
   any mission-critical extra information needed by different OS's */

typedef struct FH {
				  /* Information used on all systems */
				  struct FH *next;		/* The next header in the list */
				  struct FH *nextFile;	/* Next file in this directory */
				  struct FH *prevFile;	/* Prev file in this directory */
				  struct FH *nextLink;	/* The next linked file */
				  struct FH *prevLink;	/* The previous linked file */
				  FILEHDR data;			/* The file data itself */
				  char *fileName;		/* The filename */
				  BYTE *extraInfo;		/* Any extra information needed */
				  long offset;			/* Offset of data from start of archive */
				  WORD hType;			/* The type of the file (within HPACK) */
				  WORD linkID;			/* The magic number of this link */
				  BOOLEAN tagged;		/* Whether this file has been selected */
#if defined( __UNIX__ )
				  /* Information used to handle hard links on Unix systems */
				  LONG fileLinkID;		/* The dev.no/inode used for links */
#elif defined( __MAC__ )
				  /* Information on file types/creators on Macintoshes */
				  LONG type;			/* The file's type */
				  LONG creator;			/* The file's creator */
#elif defined( __ARC__ )
				  /* Information on the file type on Archimedes' */
				  WORD type;			/* The file's type */
#elif defined( __IIGS__ )
				  /* Information on the file types on Apple IIgs's */
				  WORD type;			/* The file's type */
				  LONG auxType;			/* The file's auxiliary type */
#endif /* Various OS-specific extra information */
				  } FILEHDRLIST;

/* The data structures for the list of directories */

typedef struct DH {
				  WORD dirIndex;		/* The index of this directory */
				  WORD next;			/* The next header in the list */
/*				  struct DH *next;		// The next header in the list */
				  struct DH *nextDir;	/* Next dir in this directory */
				  struct DH *prevDir;	/* Prev dir in this directory */
				  struct DH *nextLink;	/* The next linked directory */
				  struct DH *prevLink;	/* The previous linked directory */
				  DIRHDR data;			/* The directory data */
				  char *dirName;		/* The directory name */
				  long offset;			/* Offset of data from start of dir
										   directory data area */
				  FILEHDRLIST *fileInDirListHead;	/* Start of the list of
										   files in this directory */
				  FILEHDRLIST *fileInDirListTail;	/* End of the list of
										   files in this directory */
				  struct DH *dirInDirListHead;		/* Start of the list of
										   dirs in this directory */
				  struct DH *dirInDirListTail;		/* End of the list of
										   dirs in this directory */
				  WORD hType;			/* The type of the directory (within HPACK) */
				  WORD linkID;			/* The magic number of this link */
				  BOOLEAN tagged;		/* Whether this dir.has been selected */
				  } DIRHDRLIST;

/* The data structure for the list of archive parts for a multi-part archive */

typedef struct PL {
				  struct PL *next;		/* The next header in the list */
				  long segEnd;			/* Extent of this segment */
				  } PARTSIZELIST;

/* The end marker for various lists */

#define END_MARKER		0xFFFF

/* The directory index number for the root directory */

#define ROOT_DIR		0

/* The magic number for a non-linked file */

#define NO_LINK			0

/* The types of path match which matchPath() can return */

enum { PATH_NOMATCH, PATH_PARTIALMATCH, PATH_FULLMATCH };

/* Whether we want readArcDir() to save the directory data to a temporary
   file (this is necessary since it will be overwritten during an ADD) */

#define SAVE_DIR_DATA	TRUE

/* The magic ID and temporary name extensions for HPACK archives */

extern char HPACK_ID[];

#define HPACK_ID_SIZE	4			/* 'HPAK' */

extern char TEMP_EXT[], DIRTEMP_EXT[], SECTEMP_EXT[];

/* Bit masks for the fileKludge in the archive trailer */

#define SPECIAL_MULTIPART	0x01	/* Multipart archive */
#define SPECIAL_MULTIEND	0x02	/* Last part of multipart archive */
#define SPECIAL_SECURED		0x04	/* Secured archive */
#define SPECIAL_ENCRYPTED	0x08	/* Encrypted archive */
#define SPECIAL_BLOCK		0x10	/* Block compressed archive */

/* Some global vars, defined in ARCDIR.C */

extern FILEHDRLIST *fileHdrCurrPtr, *fileHdrStartPtr;
extern int currEntry, currPart, lastPart;
extern long segmentEnd, endPosition;

/* Prototypes for functions in ARCDIR.C */

void initArcDir( void );
void endArcDir( void );
void resetArcDir( void );
#ifdef __MSDOS__
  void resetArcDirMem( void );
#endif /* __MSDOS__ */
void readArcDir( const BOOLEAN doSaveDirData );
void writeArcDir( void );
void addFileHeader( FILEHDR *theHeader, WORD hType, BYTE extraInfo, WORD linkID );
void deleteFileHeader( FILEHDRLIST *theHeader );
int writeFileHeader( FILEHDRLIST *theHeader );
BOOLEAN addFileName( const WORD dirIndex, const WORD hType, const char *fileName );
void deleteLastFileName( void );
void addDirName( char *dirName, const LONG dirTime );
int addDirData( WORD tagID, const int store, LONG dataLength, LONG uncoprDataLength );
void movetoDirData( void );
inline void setArchiveCwd( const WORD dirIndex );
inline void popDir( void );
inline WORD getParent( const WORD dirIndex );
void sortFiles( const WORD dirIndex );
FILEHDRLIST *findFileLinkChain( WORD linkID );
DIRHDRLIST *findDirLinkChain( WORD linkID );
inline FILEHDRLIST *getFirstFileEntry( const WORD dirIndex );
inline FILEHDRLIST *getNextFileEntry( void );
inline FILEHDRLIST *getPrevFileEntry( void );
inline DIRHDRLIST *getFirstDirEntry( const WORD dirIndex );
inline DIRHDRLIST *getNextDirEntry( void );
inline DIRHDRLIST *getPrevDirEntry( void );
inline WORD getFirstDir( void );
inline WORD getNextDir( void );
int matchPath( const char *filePath, const int pathLen, WORD *dirIndex );
inline char *getDirName( const WORD dirIndex );
inline LONG getDirTime( const WORD dirIndex );
inline void setDirTime( const WORD dirIndex, const LONG time );
inline LONG getDirAuxDatalen( const WORD dirIndex );
inline BOOLEAN getDirTaggedStatus( const WORD dirIndex );
inline void setDirTaggedStatus( const WORD dirIndex );
void getArcdirState( FILEHDRLIST **hdrListEnd, int *dirListEnd );
void setArcdirState( FILEHDRLIST *hdrListEnd, const int dirListEnd );
long getCoprDataLength( void );
int computeHeaderSize( const FILEHDR *theHeader, const BYTE extraInfo );
void addPartSize( const long partSize );
int getPartNumber( const long offset );
long getPartSize( int partNumber );
void getPart( const int thePart );
void selectFile( FILEHDRLIST *fileHeader, BOOLEAN status );
void selectDir( DIRHDRLIST *dirHeader, BOOLEAN status );
void selectDirNo( const WORD dirNo, BOOLEAN status );
void selectDirNoContents( const WORD dirNo, BOOLEAN status );

/* Prototypes for supplementary functions in ARCDIR.C used only by some OS's */

#if defined( __UNIX__ )
  BOOLEAN checkForLink( const LONG linkID );
  void addLinkInfo( const LONG linkID );
#elif defined( __MAC__ )
  void addTypeInfo( LONG owner, LONG creator );
#endif /* Various OS-specific supplementary functions */

#endif
