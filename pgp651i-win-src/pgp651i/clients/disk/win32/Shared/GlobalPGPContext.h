//////////////////////////////////////////////////////////////////////////////
// GlobalPGPContext.h
//
// Functions for creating, deleting, and returning the global PGPContextRef.
//////////////////////////////////////////////////////////////////////////////

// $Id: GlobalPGPContext.h,v 1.1 1999/02/26 04:09:59 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_GlobalPGPContext_h	// [
#define Included_GlobalPGPContext_h

#include "DualErr.h"


/////////////////////
// Exported Functions
/////////////////////

DualErr			CreateGlobalPGPContext();
void			FreeGlobalPGPContext();
PGPContextRef	GetGlobalPGPContext();

#endif	// ] Included_GlobalPGPContext_h
