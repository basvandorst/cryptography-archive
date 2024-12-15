/*
 * heap.h -- Simple priority queue.  Takes pointers to cost values
 * (presumably the first field in a larger structure) and returns
 * them in increasing order of cost.
 *
 * Copyright (C) 1997 Pretty Good Privacy, Inc.
 *
 * Written by Colin Plumb and Mark H. Weaver
 *
 * $Id: heap.h,v 1.6 1997/10/31 04:22:46 mhw Exp $
 */

#ifndef HEAP_H
#define HEAP_H 1

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

typedef int HeapCost;
#define COST_INFINITY INT_MAX
typedef unsigned HeapIndex;

typedef struct Heap {
	HeapCost	**elems;
	HeapIndex	numElems, elemsAllocated;
} Heap;

void HeapInit(Heap *heap, HeapIndex initSize);
void HeapDestroy(Heap *heap);
void HeapInsert(Heap *heap, HeapCost *newElem);
HeapCost *HeapGetMin(Heap *heap);
void HeapVerify(Heap *heap);

#endif

/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
