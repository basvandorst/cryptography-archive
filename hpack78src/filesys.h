/****************************************************************************
*																			*
*							HPACK Multi-System Archiver						*
*							===========================						*
*																			*
*					Filesystem-Specific Support Routines Header				*
*							FILESYS.H  Updated 12/10/91						*
*																			*
* This program is protected by copyright and as such any use or copying of	*
*  this code for your own purposes directly or indirectly is highly uncool	*
*  					and if you do so there will be....trubble.				*
* 				And remember: We know where your kids go to school.			*
*																			*
*		Copyright 1990, 1991 Peter C.Gutmann.  All rights reserved			*
*																			*
****************************************************************************/

#ifndef _FILESYS_DEFINED

#define _FILESYS_DEFINED

#include "arcdir.h"

char *buildInternalPath( FILEHDRLIST *fileInfoPtr );
char *buildExternalPath( FILEHDRLIST *fileInfoPtr );
BOOLEAN isTruncated( void );
char *getPath( WORD dirIndex );
int extractPath( char *fullPath, char * pathName );
char *findFilenameStart( char *filePath );
BOOLEAN dirExists( char *pathName );
void makeDirTree( void );
void findFileType( FILEHDRLIST *fileInfoPtr );
void addTypeAssociation( char *assocData );
void freeTypeAssociations( void );

#endif /* _FILESYS_DEFINED */
