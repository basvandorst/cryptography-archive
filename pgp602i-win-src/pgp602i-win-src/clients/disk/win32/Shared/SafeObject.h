///////////////////////////////////////////////////////////////////////////////
// SafeObject.h
//
// Declaration and implementation of class SafeObject
///////////////////////////////////////////////////////////////////////////////

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_SafeObject_h	// [
#define Included_SafeObject_h

#include "DualErr.h"
#include "WaitObjectClasses.h"


///////////////////
// Class SafeObject
///////////////////

// SafeObject is the base class for all thread-safe objects in the project.
// It provides a mutex and two functions for entering and leaving that
// mutex, intended to be placed around accesses to the object. Lock/Unlock
// fake 'constness' so that const member functions of derived classes can use
// them.

class SafeObject
{
public:
	DualErr mInitErr;

			SafeObject();

	void	Lock() const;				// claim object for use
	void	Unlock() const;				// release object

private:
	mutable PGPdiskMutex mAccessMutex;
};


/////////////////////////////////////////////////////////////
// Class SafeObject public inline member function definitions
/////////////////////////////////////////////////////////////

// The SafeObject constructor creates the mutex.

inline
SafeObject::SafeObject()
{
	mInitErr = mAccessMutex.mInitErr;
}

// Lock is called to lock the object for access by one thread.

inline 
void 
SafeObject::Lock() const
{
	mAccessMutex.Enter();
}

// Unlock releases a locked object.

inline 
void 
SafeObject::Unlock() const
{
	mAccessMutex.Leave();
}

#endif	// ] Included_SafeObject_h
