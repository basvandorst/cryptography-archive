//////////////////////////////////////////////////////////////////////////////
// SimpleQueue.h
//
// Declaration and implementation of class SimpleQueue.
//////////////////////////////////////////////////////////////////////////////

// $Id: SimpleQueue.h,v 1.3 1998/12/14 19:01:32 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_SimpleQueue_h	// [
#define Included_SimpleQueue_h

#include "DualErr.h"
#include "UtilityFunctions.h"


////////////
// Constants
////////////

const PGPUInt32	kMaxQueueSize = 100;		// max elems in queue


////////////////////
// Class SimpleQueue
////////////////////

// SimpleQueue is a simple circular queue for 32-bit integers.

class SimpleQueue
{
public:
					SimpleQueue();
					~SimpleQueue();

	SimpleQueue &	operator=(const SimpleQueue &from);

	PGPBoolean		IsEmpty();

	PGPBoolean		Push(PGPUInt32 num);
	PGPBoolean		Pop(PGPUInt32 *pNum);

private:
	PGPUInt32	mQueueElems[kMaxQueueSize];	// elements stored here
	PGPUInt32	mQueueStart, mQueueEnd;		// start and end indices
	PGPUInt32	mNumElems;					// population
};


////////////////////////////////////////////////////
// Class SecureString public inline member functions
////////////////////////////////////////////////////

// The SimpleQueue default constructor.

inline 
SimpleQueue::SimpleQueue()
{
	mQueueStart = mQueueEnd = mNumElems = 0;
}

// The SimpleQueue destructor.

inline 
SimpleQueue::~SimpleQueue()
{
}

// operator= copies one SimpleQueue to another.

inline 
SimpleQueue & 
SimpleQueue::operator=(const SimpleQueue &from)
{
	PGPUInt32 i = 0;

	if (this == &from)
		return (* this);

	for (i=0; i<kMaxQueueSize; i++)
	{
		mQueueElems[i] = from.mQueueElems[i];
	}

	mQueueStart	= from.mQueueStart;
	mQueueEnd	= from.mQueueEnd;
	mNumElems	= from.mNumElems;

	return (* this);
}

// IsEmpty returns TRUE if the queue is empty, FALSE otherwise.

inline 
PGPBoolean 
SimpleQueue::IsEmpty()
{
	return (mNumElems == 0);
}

// Push pushes a new number on top of the queue. TRUE if successful, FALSE
// otherwise.

inline 
PGPBoolean 
SimpleQueue::Push(PGPUInt32 num)
{
	if (mNumElems == kMaxQueueSize)
		return FALSE;

	mQueueElems[mQueueEnd] = num;

	mNumElems++;
	mQueueEnd++;

	if (mQueueEnd == kMaxQueueSize)
		mQueueEnd = 0;

	return TRUE;
}

// Pop pops a number from the bottom of the queue. TRUE if successful, FALSE
// otherwise.

inline 
PGPBoolean 
SimpleQueue::Pop(PGPUInt32 *pNum)
{
	if (mNumElems == 0)
		return FALSE;

	(* pNum) = mQueueElems[mQueueStart];

	mNumElems--;
	mQueueStart++;

	if (mQueueStart == kMaxQueueSize)
		mQueueStart = 0;

	return TRUE;
}

#endif	// ] Included_SimpleQueue_h
