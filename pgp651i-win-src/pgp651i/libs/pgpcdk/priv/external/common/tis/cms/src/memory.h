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
*
* Example use:
*	if ((status = NewMemory((TC_POINTER *)&ptrval, size)) != 0)
*	    return(status);
*	...
*	FillMemory((TC_POINTER), 0, size);
*	...
*	FreeMemory((TC_POINTER)ptrval);
*
*
*****************************************************************/

#ifndef _INCL_MEMORY_H

#include <stdlib.h>

typedef unsigned char *TC_POINTER;

int NewMemory(TC_POINTER *, unsigned int);
void FillMemory(TC_POINTER, unsigned char, unsigned int);
void FreeMemory(TC_POINTER);

/*void ClearMemoryRefs(void);*/
/*void NoteMemoryRef(TC_POINTER);*/
/*void CheckMemoryRefs(void);*/

#define FREE_GARBAGE	0xA3
#define MALLOC_GARBAGE  0xB3
#define BLOCK_GARBAGE   0xC3

#ifdef DEBUG

typedef struct _blockinfo {
    struct _blockinfo *next;
    BYTE	      *block;
    unsigned int      size;
    int		      referenced; /*flag, ever referenced? */
} BLOCKINFO_T, *BLOCKINFO_P;

#endif

#define _INCL_MEMORY_H
#endif
