/****************************************************************************
 *
 * Copyright (c) 1998, Network Associates, Inc. and its affiliated Companies
 *
 ****************************************************************************/
 
/*****************************************************************
*
* Memory routines that wrap around the C library routines with
* debugging help included
*
*****************************************************************/

/* NOTE: currently not used, could be updated and used as the default
   memory mgmt routines when then are not provided by the user */

#include "tc.h"
#include "memory.h"
#include "debug.h"

#ifdef DEBUG

static int CreateBlockInfo(TC_POINTER, int);
static void FreeBlockInfo(BYTE *);
static int sizeofBlock(TC_POINTER);
static int ValidPointer(TC_POINTER, unsigned int);

#endif

int
NewMemory(TC_POINTER *p, unsigned int size)
{
    TC_POINTER    *localp = p;

    ASSERT(size != 0); 

    *localp = (TC_POINTER)malloc(size);

    #ifdef DEBUG
    if (*localp != (void *)0) {
	(void)memset(*localp, MALLOC_GARBAGE, size);
	/* if unable to create block info., fake a total memory failure */
	if (CreateBlockInfo(*localp, size) != OKAY) {
	    free(*localp);
	    *localp = (void *)0;
	}
    }
    #endif

    return(*p != (void *)0 ? 0 : TC_E_NOMEMORY);

} /* NewMemory */

void 
FillMemory(TC_POINTER p, unsigned char b, unsigned int size)
{
    ASSERT(ValidPointer(p,size));
    
    (void)memset(p,b,size);

} /* Fill Memory */


void 
FreeMemory(TC_POINTER p)
{
    if (p != (void *)0) {

	#ifdef DEBUG
	(void)memset(p,FREE_GARBAGE,sizeofBlock(p));
	FreeBlockInfo(p);
	#endif

	free(p);
	/* set p to NULL here so that if its used again
	   we can catch it in a debug situation? */
	
    }
} /* FreeMemory */

#ifdef DEBUG

/* Private data/functions */
static BLOCKINFO_P BlockListHead = (BLOCKINFO_P)0;

static BLOCKINFO_P
GetBlockInfo(TC_POINTER pointer)
{
    BLOCKINFO_P	    blockInfo;
    BYTE_P	    start;
    BYTE_P	    end;

    for (blockInfo = BlockListHead; 
         blockInfo != (BLOCKINFO_P)0; 
         blockInfo = blockInfo->next) {

	start = blockInfo->block;
	end = blockInfo->block + blockInfo->size - 1;

	if (pointer >= start && pointer <= end)
	    break;
    }

    ASSERT(blockInfo != (BLOCKINFO_P)0);

    return(blockInfo);

} /* GetBlockInfo */


static int
CreateBlockInfo(TC_POINTER new, int size)
{
    BLOCKINFO_P	blockInfo;

    ASSERT(new != (BYTE *)0 && size != 0);

    blockInfo = (BLOCKINFO_P)malloc(sizeof(BLOCKINFO_T));
    if (blockInfo != (BLOCKINFO_P)0) {
	blockInfo->block = new;
	blockInfo->size = size;
	blockInfo->next = BlockListHead;
	BlockListHead = blockInfo;
    }

    return(blockInfo != (BLOCKINFO_P)0 ? OKAY : GERR_FAIL_MALLOC);

} /* CreateBlockInfo */


static void
FreeBlockInfo(TC_POINTER block)
{
    BLOCKINFO_P	current, prev;

    prev = (BLOCKINFO_P)0;
    for (current = BlockListHead; 
         current != (BLOCKINFO_P)0;
         current = current->next) {
	if (current->block == block) {
	    if (prev == (BLOCKINFO_P)0)
		BlockListHead = current->next;
	    else
		prev->next = current->next;
	    break;
	}
	prev = current;
    }

    ASSERT(current != (BLOCKINFO_P)0);
    (void)memset(current, BLOCK_GARBAGE, sizeof(BLOCKINFO_T));
    free(current);

} /* FreeBlockInfo */


static int
sizeofBlock(TC_POINTER p)
{
    BLOCKINFO_P	blockInfo;

    blockInfo = GetBlockInfo(p);
    ASSERT(p == blockInfo->block);

    return(blockInfo->size);

} /* sizeofBlock */


static int
ValidPointer(TC_POINTER p, unsigned int size)
{
    BLOCKINFO_P	blockInfo;
    TC_POINTER     localp = (TC_POINTER)p;

    ASSERT(p != (void *)0 && size !=0);

    blockInfo = GetBlockInfo(localp);

    ASSERT((localp + size) <= (blockInfo->block + blockInfo->size));

    return(OKAY);

} /* ValidPointer */

#endif  /* DEBUG */

