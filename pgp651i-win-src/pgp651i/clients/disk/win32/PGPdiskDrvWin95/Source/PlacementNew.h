//////////////////////////////////////////////////////////////////////////////
// PlacementNew.h
//
// Definition of placement new.
//////////////////////////////////////////////////////////////////////////////

// $Id: PlacementNew.h,v 1.4 1998/12/14 19:00:23 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_PlacementNew_h	// [
#define Included_PlacementNew_h


////////////////
// Placement New
////////////////

inline 
void * 
__cdecl 
operator new(size_t, void *pMem)
{
	return pMem;			// all too easy!
}

#endif // ] Included_PlacementNew_h
