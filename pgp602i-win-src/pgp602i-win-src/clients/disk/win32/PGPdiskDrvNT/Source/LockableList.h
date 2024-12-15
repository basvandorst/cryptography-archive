//////////////////////////////////////////////////////////////////////////////
// LockableList.h
//
// Declaration and definition of class LockableList.
//////////////////////////////////////////////////////////////////////////////

// $Id: LockableList.h,v 1.1.2.5 1998/07/06 08:57:50 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_Lockable_h	// [
#define Included_Lockable_h

#include "DualErr.h"


/////////////////////
// Class LockableList
/////////////////////

template <class T> class LockableList : public KList<T>
{
public:
			LockableList();
			~LockableList();
	
	void	PublicLock();
	void	PublicUnlock();

private:
	KIRQL      mOldIrql;
	KSPIN_LOCK mSpinLock;
};


////////////////////////////////////////////////////
// Class LockableList inline public member functions
////////////////////////////////////////////////////

// The LockableList constructor.

template <class T> 
inline 
LockableList<T>::LockableList() : KList<T>()
{
}

// The LockableList destructor.

template <class T> 
inline 
LockableList<T>::~LockableList()
{
}

// PublicLock locks the list.

template <class T> 
void 
LockableList<T>::PublicLock() 
{
	KIRQL oldIrql;

	KeAcquireSpinLock(&mSpinLock, &oldIrql);

	mOldIrql = oldIrql;
}

// PublicUnlock unlocks the list.

template <class T> 
void 
LockableList<T>::PublicUnlock() 
{
	KeReleaseSpinLock(&mSpinLock, mOldIrql);
}

#endif	// ] Included_Lockable_h
