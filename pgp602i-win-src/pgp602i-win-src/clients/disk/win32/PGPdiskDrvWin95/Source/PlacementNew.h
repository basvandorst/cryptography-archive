//////////////////////////////////////////////////////////////////////////////
// PlacementNew.h
//
// Definition of placement new.
//////////////////////////////////////////////////////////////////////////////

// $Id: PlacementNew.h,v 1.1.2.4 1998/07/06 08:58:26 nryan Exp $

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
