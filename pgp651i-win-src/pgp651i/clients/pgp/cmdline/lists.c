/*____________________________________________________________________________
    lists.c

    Copyright(C) 1998,1999 Network Associates, Inc.
    All rights reserved.

	PGP 6.5 Command Line 

    manage simple lists. null-terminated string lists for passphrases or
    file names and (void *) lists for arbitrary other uses.

    $Id: lists.c,v 1.8.6.1 1999/06/14 19:01:57 build Exp $
____________________________________________________________________________*/

#include <string.h>

#include "pgpBase.h"
#include "pgpContext.h"
#include "pgpDebug.h"

#include "lists.h"
#include "config.h"

#define kPGPPassphraseListMagic 0x4C685050 /* 'PPhL' */
#define kPGPFileNameListMagic   0x4C6D4E46 /* 'FNmL' */
#define kPGPPointerListMagic    0x4C696F50 /* 'PoiL' */

/*
   Problem:
   . Don't let the structures become corrupt if the functions
     are called in a signal handler.

   Simplifying assumptions:
   . The main application does not run multithreaded, or if it
     does, there will be no concurrent access to the same lists.
   . The signal handler never returns to the main application.
   . The signal handler only cares about tearing down lists and
     disposing of memory.
 */

struct pgpPassphraseList {
    PGPUInt32 magic;
    PGPMemoryMgrRef mmgr;
    struct pgpPassphraseListNode *first;
    struct pgpPassphraseListNode *current;
    struct pgpPassphraseListNode **tail;
};

struct pgpPassphraseListNode {
    struct pgpPassphraseListNode *next;
    char passphrase[ kMaxPassPhraseLength+1 ];
};


/* These lists store names of lexical sections
   (decoded filenames) before emitting output. */

struct pgpFileNameList {
    PGPUInt32 magic;
    PGPMemoryMgrRef mmgr;
    struct pgpFileNameListNode *first;
    struct pgpFileNameListNode *current;
    struct pgpFileNameListNode **tail;
};

struct pgpFileNameListNode {
    struct pgpFileNameListNode *next;
    char filename[ MAX_PATH+1 ];
};


struct pgpPointerList {
    PGPUInt32 magic;
    PGPMemoryMgrRef mmgr;
    struct pgpPointerListNode *first;
    struct pgpPointerListNode *current;
    struct pgpPointerListNode **tail;
};

struct pgpPointerListNode {
    struct pgpPointerListNode *next;
    void *data;
};


PGPError pgpNewPassphraseList( PGPContextRef context,
        pgpPassphraseListRef *listPtr )
{
    PGPMemoryMgrRef mmgr = PGPGetContextMemoryMgr( context );
    pgpPassphraseListRef myList = PGPNewSecureData( mmgr, sizeof(struct
            pgpPassphraseList), 0 );
    if(myList == NULL)
        return kPGPError_OutOfMemory;

    myList->magic = kPGPPassphraseListMagic;
    myList->mmgr = mmgr;
    myList->first = NULL;
    myList->current = NULL;
    myList->tail = &myList->first;
    *listPtr=myList;
    return kPGPError_NoErr;
}

PGPError pgpNewFileNameList( PGPContextRef context, pgpFileNameListRef
        *listPtr )
{
    PGPMemoryMgrRef mmgr = PGPGetContextMemoryMgr( context );
    pgpFileNameListRef myList = PGPNewData( mmgr, sizeof(struct
            pgpFileNameList), 0 );

    if(myList == NULL)
        return kPGPError_OutOfMemory;

    myList->magic = kPGPFileNameListMagic;
    myList->mmgr = mmgr;
    myList->first = NULL;
    myList->current = NULL;
    myList->tail = &myList->first;
    *listPtr=myList;
    return kPGPError_NoErr;
}

PGPError pgpNewPointerList( PGPContextRef context,
        pgpPointerListRef *listPtr )

{
    PGPMemoryMgrRef mmgr = PGPGetContextMemoryMgr( context );
    pgpPointerListRef myList = PGPNewData( mmgr, sizeof(struct
            pgpPointerList), 0 );

    if(myList == NULL)
        return kPGPError_OutOfMemory;

    myList->magic = kPGPPointerListMagic;
    myList->mmgr = mmgr;
    myList->first = NULL;
    myList->current = NULL;
    myList->tail = &myList->first;
    *listPtr=myList;
    return kPGPError_NoErr;
}


PGPError pgpCountPassphraseList( pgpPassphraseListRef list,
        PGPUInt32 *count )

{
    struct pgpPassphraseListNode *p;
    PGPUInt32 i;

    pgpAssertAddrValid( list, struct pgpPassphraseList );
    pgpAssertAddrValid( count, PGPUInt32 );
    if( list->magic != kPGPPassphraseListMagic)
        return kPGPError_BadParams;

    for( i=0, p = list->first; p != NULL; i++,p=p->next)
        ;
    *count = i;
    return kPGPError_NoErr;
}

PGPError pgpCountFileNameList( pgpFileNameListRef list, PGPUInt32 *count )
{
    struct pgpFileNameListNode *p;
    PGPUInt32 i;

    pgpAssertAddrValid( list, struct pgpFileNameList );
    pgpAssertAddrValid( count, PGPUInt32 );
    if( list->magic != kPGPFileNameListMagic)
        return kPGPError_BadParams;

    for( i=0, p = list->first; p != NULL; i++,p=p->next)
        ;
    *count = i;
    return kPGPError_NoErr;
}

PGPError pgpCountPointerList( pgpPointerListRef list, PGPUInt32 *count )
{
    struct pgpPointerListNode *p;
    PGPUInt32 i;

    pgpAssertAddrValid( list, struct pgpPointerList );
    pgpAssertAddrValid( count, PGPUInt32 );
    if( list->magic != kPGPPointerListMagic)
        return kPGPError_BadParams;

    for( i=0, p = list->first; p != NULL; i++,p=p->next)
        ;
    *count = i;
    return kPGPError_NoErr;
}


PGPError pgpAppendToPassphraseList( pgpPassphraseListRef list, char *pass )
{
    struct pgpPassphraseListNode **nodePtr;
    struct pgpPassphraseListNode *pitem;

    pgpAssertAddrValid( list, struct pgpPassphraseList );
    pgpAssertAddrValid( pass, char );
    if( list->magic != kPGPPassphraseListMagic)
        return kPGPError_BadParams;
    if(pass[0]=='\0')
        return kPGPError_BadParams;

    pitem = PGPNewSecureData( list->mmgr, sizeof(struct
            pgpPassphraseListNode), 0 );
    if(pitem == NULL)
        return kPGPError_OutOfMemory;

    /* always tack onto the tail of the list..*/
    nodePtr = list->tail;
    pgpAssert( *nodePtr == NULL );

    strncpy( pitem->passphrase, pass, kMaxPassPhraseLength );
    pitem->passphrase[ kMaxPassPhraseLength ] = '\0';
    pitem->next = NULL;

    (*nodePtr) = pitem;
    list->tail = &pitem->next;

    return kPGPError_NoErr;
}

PGPError pgpAppendToFileNameList( pgpFileNameListRef list, char *name )
{
    struct pgpFileNameListNode **nodePtr;
    struct pgpFileNameListNode *fitem;

    pgpAssertAddrValid( name, char );
    pgpAssertAddrValid( list, struct pgpFileNameList );
    if(name[0]=='\0')
        return kPGPError_BadParams;

    fitem = PGPNewData( list->mmgr, sizeof(struct pgpFileNameListNode), 0 );
    if(fitem == NULL)
        return kPGPError_OutOfMemory;

    /* always tack onto the tail of the list..*/
    nodePtr = list->tail;
    pgpAssert( *nodePtr == NULL );

    strncpy( fitem->filename, name, kMaxPassPhraseLength );
    fitem->filename[ kMaxPassPhraseLength ] = '\0';
    fitem->next = NULL;

    (*nodePtr) = fitem;
    list->tail = &fitem->next;

    return kPGPError_NoErr;
}

PGPError pgpAppendToPointerList( pgpPointerListRef list, void *data )
{
    struct pgpPointerListNode **nodePtr;
    struct pgpPointerListNode *item;

    pgpAssertAddrValid( data, char );
    pgpAssertAddrValid( list, struct pgpPointerList );

    item = PGPNewData( list->mmgr, sizeof(struct pgpPointerListNode), 0 );
    if(item == NULL)
        return kPGPError_OutOfMemory;

    /* always tack onto the tail of the list..*/
    nodePtr = list->tail;
    pgpAssert( *nodePtr == NULL );

    item->data = data;
    item->next = NULL;

    (*nodePtr) = item;
    list->tail = &item->next;

    return kPGPError_NoErr;
}

PGPError pgpRemoveFromPointerList( pgpPointerListRef list, void *data )
{
    struct pgpPointerListNode **nodePtr;
    struct pgpPointerListNode *item;
    PGPError err;

    pgpAssertAddrValid( data, char );
    pgpAssertAddrValid( list, struct pgpPointerList );

    /* scan for the item...*/

    nodePtr = &list->first;
    while( *nodePtr && (*nodePtr)->data != data )
        nodePtr = &(*nodePtr)->next;

    err = kPGPError_NoErr;

    if( *nodePtr == NULL ) {
        err = kPGPError_ItemNotFound;
        goto out;
    }
    item = *nodePtr;

    if( list->tail == &item->next )
        list->tail = nodePtr;
    *nodePtr = item->next;
    PGPFreeData(item);

out:
    return err;
}


PGPError pgpRewindPassphrase( pgpPassphraseListRef list )
{
    pgpAssertAddrValid( list, struct pgpPassphraseList );
    if( list->magic != kPGPPassphraseListMagic )
        return kPGPError_BadParams;

    list->current = list->first;
    return kPGPError_NoErr;
}

PGPError pgpRewindFileName( pgpFileNameListRef list )
{
    pgpAssertAddrValid( list, struct pgpFileNameList );
    if( list->magic != kPGPFileNameListMagic)
        return kPGPError_BadParams;

    list->current = list->first;
    return kPGPError_NoErr;
}

PGPError pgpRewindPointer( pgpPointerListRef list )
{
    pgpAssertAddrValid( list, struct pgpPointerList );
    if( list->magic != kPGPPointerListMagic)
        return kPGPError_BadParams;

    list->current = list->first;
    return kPGPError_NoErr;
}



PGPError pgpNextPassphrase( pgpPassphraseListRef list, char **passphrase )
{
    pgpAssertAddrValid( passphrase, char );
    pgpAssertAddrValid( list, struct pgpPassphraseList );
    if( list->magic != kPGPPassphraseListMagic )
        return kPGPError_BadParams;

    if( list->current != NULL)
    {
        *passphrase = list->current->passphrase;
        list->current = list->current->next;
        return kPGPError_NoErr;
    }
    *passphrase = NULL;
    return kPGPError_EndOfIteration;
}

PGPError pgpNextFileName( pgpFileNameListRef list, char **filename )
{
    pgpAssertAddrValid( filename, char );
    pgpAssertAddrValid( list, struct pgpFileNameList );
    if( list->magic != kPGPFileNameListMagic )
        return kPGPError_BadParams;

    if( list->current != NULL)
    {
        *filename = list->current->filename;
        list->current = list->current->next;
        return kPGPError_NoErr;
    }
    *filename = NULL;
    return kPGPError_EndOfIteration;
}

PGPError pgpNextPointer( pgpPointerListRef list, void **data )
{
    pgpAssertAddrValid( data, char );
    pgpAssertAddrValid( list, struct pgpPointerList );
    if( list->magic != kPGPPointerListMagic )
        return kPGPError_BadParams;

    if( list->current != NULL)
    {
        *data = list->current->data;
        list->current = list->current->next;
        return kPGPError_NoErr;
    }
    *data = NULL;
    return kPGPError_EndOfIteration;
}



PGPError pgpFreePassphraseList( pgpPassphraseListRef list )
{
    struct pgpPassphraseListNode *tmp;
    PGPError err = kPGPError_NoErr;
    pgpAssertAddrValid( list, struct pgpPassphraseList );
    if( list->magic != kPGPPassphraseListMagic)
        return kPGPError_BadParams;

    while( list->first ) {

        tmp = list->first->next;
        err = PGPFreeData( list->first );
        if( IsPGPError(err) )
            break;
        list->first = tmp;
    }

    if( IsntPGPError(err))
        err = PGPFreeData( list );
    return err;
}

PGPError pgpFreeFileNameList( pgpFileNameListRef list )
{
    struct pgpFileNameListNode *tmp;
    PGPError err = kPGPError_NoErr;
    pgpAssertAddrValid( list, struct pgpFileNameList );
    if( list->magic != kPGPFileNameListMagic)
        return kPGPError_BadParams;

    while( list->first ) {
        tmp = list->first->next;
        err = PGPFreeData( list->first );
        if( IsPGPError(err) )
            break;
        list->first = tmp;
    }

    if( IsntPGPError(err))
        err = PGPFreeData( list );
    return err;
}

PGPError pgpFreePointerList( pgpPointerListRef list )
{
    struct pgpPointerListNode *tmp;
    PGPError err = kPGPError_NoErr;
    pgpAssertAddrValid( list, struct pgpPointerList );
    if( list->magic != kPGPPointerListMagic)
        return kPGPError_BadParams;

    while( list->first ) {
        tmp = list->first->next;
        err = PGPFreeData( list->first );
        if( IsPGPError(err) )
            break;
        list->first = tmp;
    }

    if( IsntPGPError(err))
        err = PGPFreeData( list );
    return err;
}

