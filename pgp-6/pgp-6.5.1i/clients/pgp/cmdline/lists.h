/*____________________________________________________________________________
    lists.h

    Copyright(C) 1998,1999 Network Associates, Inc.
    All rights reserved.

	PGP 6.5 Command Line 

    $Id: lists.h,v 1.3 1999/05/12 21:01:04 sluu Exp $
____________________________________________________________________________*/


#ifndef PGP_LISTS_H
#define PGP_LISTS_H

/* some simple linked lists of strings.. */


#include "pgpBase.h"

/* These lists store passwords for future use. */

typedef struct pgpPassphraseList *pgpPassphraseListRef;


/* These lists store decoded filenames (lexical sections)
   before emitting output. */

typedef struct pgpFileNameList *pgpFileNameListRef;



typedef struct pgpPointerList *pgpPointerListRef;


PGPError pgpNewPassphraseList( PGPContextRef context,
        pgpPassphraseListRef *listPtr );
PGPError pgpCountPassphraseList( pgpPassphraseListRef list,
        PGPUInt32 *count );
PGPError pgpRewindPassphrase( pgpPassphraseListRef list );
PGPError pgpNextPassphrase( pgpPassphraseListRef list, char **passphrase );
PGPError pgpAppendToPassphraseList( pgpPassphraseListRef list, char *p );
PGPError pgpFreePassphraseList( pgpPassphraseListRef list );

PGPError pgpNewFileNameList( PGPContextRef context,
        pgpFileNameListRef *listPtr );
PGPError pgpCountFileNameList( pgpFileNameListRef list, PGPUInt32 *count );
PGPError pgpRewindFileName( pgpFileNameListRef list );
PGPError pgpNextFileName( pgpFileNameListRef list, char **filename );
PGPError pgpAppendToFileNameList( pgpFileNameListRef list, char *name );
PGPError pgpFreeFileNameList( pgpFileNameListRef list );

PGPError pgpNewPointerList( PGPContextRef context,
        pgpPointerListRef *listPtr );
PGPError pgpCountPointerList( pgpPointerListRef list, PGPUInt32 *count );
PGPError pgpRewindPointer( pgpPointerListRef list );
PGPError pgpNextPointer( pgpPointerListRef list, void **data );
PGPError pgpAppendToPointerList( pgpPointerListRef list, void *data );
PGPError pgpRemoveFromPointerList( pgpPointerListRef list, void *data );
PGPError pgpFreePointerList( pgpPointerListRef list );

#endif
