//////////////////////////////////////////////////////////////////////////////
// LookasideList.h
//
// Declaration and definition of class LookasideList.
//////////////////////////////////////////////////////////////////////////////

// $Id: LookasideList.h,v 1.1.2.4 1998/07/06 08:57:50 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_LookasideList_h	// [
#define Included_LookasideList_h

#include "DualErr.h"


//////////////////////
// Class LookasideList
//////////////////////

template <class T> class LookasideList
{
public:
	DualErr mInitErr;

			LookasideList(PGPUInt16 depth);
			~LookasideList();

	DualErr	Allocate(T **ppObj);
	void	Free(T *pObj);

private:
	PNPAGED_LOOKASIDE_LIST mPListHeader;
};


/////////////////////////////////////////////////////
// Class LookasideList inline public member functions
/////////////////////////////////////////////////////

// The LookasideList constructor.

template <class T> 
inline 
LookasideList<T>::LookasideList(PGPUInt16 depth)
{
	// Get memory for list header.
	mPListHeader = new NPAGED_LOOKASIDE_LIST;

	if (IsNull(mPListHeader))
		mInitErr = DualErr(kPGDMinorError_OutOfMemory);

	if (mInitErr.IsntError())
	{
		ExInitializeNPagedLookasideList(mPListHeader, NULL, NULL, NULL, 
			sizeof(T), kPGPdiskMemPoolTag, depth);
	}
}

// The LookasideList destructor.

template <class T> 
inline 
LookasideList<T>::~LookasideList()
{
	if (IsntNull(mPListHeader))
	{
		ExDeleteNPagedLookasideList(mPListHeader);

		delete mPListHeader;
		mPListHeader = NULL;
	}
}

// Allocate allocates an object from the lookaside list.

template <class T> 
inline 
DualErr	
LookasideList<T>::Allocate(T **ppObj)
{
	DualErr	derr;
	T		*pObj;

	pgpAssertAddrValid(ppObj, T *);
	pgpAssertAddrValid(mPListHeader, NPAGED_LOOKASIDE_LIST);

	pObj = (T *) ExAllocateFromNPagedLookasideList(mPListHeader);

	if (IsntNull(pObj))
		(* ppObj) = pObj;
	else
		derr = DualErr(kPGDMinorError_OutOfMemory);

	return derr;
}

// Free returns an object to the lookaside list.

template <class T> 
inline 
void 
LookasideList<T>::Free(T *pObj)
{
	pgpAssertAddrValid(pObj, T);
	pgpAssertAddrValid(mPListHeader, NPAGED_LOOKASIDE_LIST);

	ExFreeToNPagedLookasideList(mPListHeader, pObj);
}

#endif	// ] Included_LookasideList_h
