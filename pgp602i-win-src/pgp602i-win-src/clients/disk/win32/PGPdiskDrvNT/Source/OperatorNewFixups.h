//////////////////////////////////////////////////////////////////////////////
// OperatorNewFixups.h
//
// Extra definitions for operator new.
//////////////////////////////////////////////////////////////////////////////

// $Id: OperatorNewFixups.h,v 1.1.2.3 1998/07/06 08:57:52 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_OperatorNewFixups_h	// [
#define Included_OperatorNewFixups_h


//////////
// Externs
//////////

extern ULONG __Pool_Tag__;


/////////////////////////////
// New without pool specifier
/////////////////////////////

// Why not provide a default specifier and save everyone from rewriting their
// shared code Vireo? What the dil-e-o?

inline 
void * 
__cdecl 
operator new(unsigned int nSize)
{
#if DBG
		return ExAllocatePoolWithTag(NonPagedPool, nSize, __Pool_Tag__);
#else
		return ExAllocatePool(NonPagedPool, nSize);
#endif
};


////////////////
// Placement New
////////////////

inline 
void * 
__cdecl 
operator new(unsigned int nSize, void *pMem)
{
	return pMem;			// all too easy!
}

#endif // ] Included_OperatorNewFixups_h
