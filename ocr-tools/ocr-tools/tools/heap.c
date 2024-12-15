/*
 * heap.c -- Simple priority queue.  Takes pointers to cost values
 * (presumably the first field in a larger structure) and returns
 * them in increasing order of cost.
 *
 * Copyright (C) 1997 Pretty Good Privacy, Inc.
 *
 * Written by Colin Plumb and Mark H. Weaver
 *
 * $Id: heap.c,v 1.2 1997/07/05 02:55:23 colin Exp $
 */

#include <stdio.h>	/* For fprintf(stderr, "Out of memory") */
#include <stdlib.h>	/* For malloc() & co. */

#include "heap.h"

#define HeapParent(i)			((i) / 2)
#define HeapLeftChild(i)		((i) * 2)
#define HeapRightChild(i)		((i) * 2 + 1)
#define HeapElem(h, i)			(h)->elems[i]
#define HeapMinElem(h)			HeapElem(h, 1)
#define HeapElemCost(e)			(*(e))
#define HeapCost(h, i)			HeapElemCost(HeapElem(h, i))
#define HeapSize(h)				((h)->numElems)

static void
SiftDown(Heap const *heap, HeapCost *e)
{
	HeapIndex size = HeapSize(heap), parent = 1, child;
	HeapCost cparent = HeapElemCost(e), cchild;

	for (;;) {
		child = 2*parent;
		if (child > size)
			break;
		cchild = HeapCost(heap, child);
		if (child < size && cchild > HeapCost(heap, child+1)) {
			cchild = HeapCost(heap, child+1);
			child++;
		}
		if (cparent <= cchild)
			break;	/* Stop sifting down */
		HeapElem(heap, parent) = HeapElem(heap, child);
		parent = child;
	}
	HeapElem(heap, parent) = e;
}

/* Debug tool: verify heap property */
void
HeapVerify(Heap *heap)
{
	HeapIndex i;

	for (i = 2; i <= HeapSize(heap); i++)
		if (HeapCost(heap, i) < HeapCost(heap, HeapParent(i)))
			fprintf(stderr, "DEBUG: VerifyHeap failed at elem %d\n", i);
}

/* Remove and return the minimum cost from the heap. */
HeapCost *
HeapGetMin(Heap *heap)
{
	HeapIndex lastElem = HeapSize(heap);
	HeapCost *retval;

	if (!lastElem)
		return NULL;
	retval = HeapMinElem(heap);
	HeapSize(heap) = lastElem-1;
	SiftDown(heap, HeapElem(heap, lastElem));
	return retval;
}

/* Helper - set heap size, reallocating if needed */
static void
HeapResize(Heap *heap, HeapIndex newNumElems)
{
	if (newNumElems >= heap->elemsAllocated) {
		HeapIndex newAllocSize = heap->elemsAllocated * 2;

		if (newAllocSize <= newNumElems)
			newAllocSize = newNumElems + 1;
		heap->elems = (HeapCost **)realloc((void *)heap->elems,
									  sizeof(*heap->elems) * newAllocSize);
		if (heap->elems == NULL) {
			fprintf(stderr, "Fatal error: Out of memory growing heap\n");
			exit(1);
		}
		heap->elemsAllocated = newAllocSize;
	}
	heap->numElems = newNumElems;
}

/* Add an element to the heap */
void
HeapInsert(Heap *heap, HeapCost *newElem)
{
	HeapIndex parent, i = ++HeapSize(heap);
	HeapCost cost = HeapElemCost(newElem);

	HeapResize(heap, i);
	/* Sift up until parent = 0 */
	while ((parent = HeapParent(i)) && HeapCost(heap, parent) > cost) {
		HeapElem(heap, i) = HeapElem(heap, parent);
		i = parent;
	}
	heap->elems[i] = newElem;
}

/* Initialize a new heap */
void
HeapInit(Heap *heap, HeapIndex initSize)
{
	initSize++;	/* Add one for temporary element */
	if (initSize < 1)
		initSize = 1;
	heap->elems = (HeapCost **)malloc(initSize * sizeof(*heap->elems));
	if (heap->elems == NULL) {
		fprintf(stderr, "Fatal error: Out of memory creating heap\n");
		exit(1);
	}
	heap->elemsAllocated = initSize;
	heap->numElems = 0;
}

/* Free up a heap's resources. */
void
HeapDestroy(Heap *heap)
{
	free((void *)heap->elems);
	heap->elemsAllocated = 0;
	heap->numElems = 0;
	heap->elems = NULL;
}

/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
