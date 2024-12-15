/****************************************************************************
*																			*
*							HPACK Multi-System Archiver						*
*							===========================						*
*																			*
*						  Error Handling Routines Header					*
*							ERROR.H  Updated 14/03/92						*
*																			*
* This program is protected by copyright and as such any use or copying of	*
*  this code for your own purposes directly or indirectly is highly uncool	*
*  					and if you do so there will be....trubble.				*
* 				And remember: We know where your kids go to school.			*
*																			*
*		Copyright 1989 - 1992  Peter C.Gutmann.  All rights reserved		*
*																			*
****************************************************************************/

#ifdef __MAC__
  #include "hpackio.h"
#else
  #include "io/hpackio.h"
#endif /* __MAC__ */

/* The possible error messages.  When changing these remember to change
   the errors[] table in HPAKTEXT.C as well */

typedef struct {
			   int errorLevel;			/* The error exit level */
			   char *msg;				/* The error text itself */
			   } ERROR_INFO;

extern ERROR_INFO err00, err01, err02, err03, err04, err05, err06, err07;
extern ERROR_INFO err08, err09, err10, err11, err12, err13, err14, err15;
extern ERROR_INFO err16, err17, err18, err19, err20, err21, err22, err23;
extern ERROR_INFO err24, err25, err26, err27, err28, err29, err30, err31;
extern ERROR_INFO err32, err33, err34, err35, err36, err37, err38, err39;
extern ERROR_INFO err40, err41, err42, err43, err44, err45, err46, err47;

#define INTERNAL_ERROR					&err00
#define OUT_OF_MEMORY					&err01
#define OUT_OF_DISK_SPACE				&err02
#define CANNOT_OPEN_ARCHFILE			&err03
#define CANNOT_OPEN_TEMPFILE			&err04
#define PATH_NOT_FOUND					&err05
#define CANNOT_ACCESS_BASEDIR			&err06
#define CANNOT_CREATE_DIR				&err07
#define STOPPED_AT_USER_REQUEST			&err08
#define FILE_ERROR						&err09
#define ARCHIVE_DIRECTORY_CORRUPTED		&err10

#define PATH_s_TOO_LONG					&err11
#define PATH_s__TOO_LONG				&err12
#define PATH_ss_TOO_LONG				&err13
#define PATH_ss__TOO_LONG				&err14
#define PATH_s_s_TOO_LONG				&err15
#define CANNOT_OVERRIDE_BASEPATH		&err16
#define TOO_MANY_LEVELS_NESTING			&err17
#define N_ERRORS_DETECTED_IN_SCRIPTFILE	&err18
#define NOT_HPACK_ARCHIVE				&err19
#define NO_FILES_ON_DISK				&err20
#define NO_FILES_IN_ARCHIVE				&err21
#define NO_ARCHIVES						&err22
#define BAD_KEYFILE						&err23
#define UNKNOWN_COMMAND					&err24
#define UNKNOWN_DIR_OPTION				&err25
#define UNKNOWN_OVERWRITE_OPTION		&err26
#define UNKNOWN_VIEW_OPTION				&err27
#define UNKNOWN_OPTION					&err28

#define MISSING_USERID					&err29
#define CANNOT_FIND_SECRET_KEY_FOR_s	&err30
#define CANNOT_FIND_SECRET_KEY			&err31
#define CANNOT_FIND_PUBLIC_KEY_FOR_s	&err32
#define CANNOT_READ_RANDOM_SEEDFILE		&err33
#define PASSWORDS_NOT_SAME				&err34
#define CANNOT_CHANGE_DEL_ARCH			&err35
#define CANNOT_CHANGE_MULTIPART_ARCH	&err36
#define CANNOT_CHANGE_ENCRYPTED_ARCH	&err37
#define CANNOT_CHANGE_UNENCRYPTED_ARCH	&err38
#define CANNOT_CHANGE_UNIFIED_ARCH		&err39
#define LONG_ARG_NOT_SUPPORTED			&err40
#define BAD_WILDCARD_FORMAT				&err41
#define WILDCARD_TOO_COMPLEX			&err42
#define CANNOT_USE_WILDCARDS_s			&err43
#define CANNOT_USE_BOTH_CKE_PKE			&err44
#define CANNOT_PROCESS_CRYPT_ARCH		&err45

/* Some global vars */

extern char errorFileName[];
extern FD errorFD;
extern char dirFileName[];
extern FD dirFileFD;
extern char secFileName[];
extern FD secFileFD;
extern int secInfoLen;
extern long oldArcEnd;
extern void *oldHdrlistEnd;				/* Actually FILEHDRLIST * */
extern int oldDirEnd;
extern unsigned int oldDirNameEnd;
extern FD archiveFD;
#ifdef __MAC__
  #define RESOURCE_TMPNAME		"HPACK temporary file"
  extern FD resourceTmpFD;
#endif /* __MAC__ */

/* Prototypes for functions in ERROR.C */

void error( ERROR_INFO *errInfo, ... );
void fileError( void );
void closeFiles( void );
