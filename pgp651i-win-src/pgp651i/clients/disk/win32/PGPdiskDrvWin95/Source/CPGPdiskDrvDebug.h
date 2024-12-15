//////////////////////////////////////////////////////////////////////////////
// CPGPdiskDrvDebug.h
//
// Declarations for CPGPdiskDrvDebug.cpp.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPGPdiskDrvDebug.h,v 1.4 1998/12/14 19:00:04 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CPGPdiskDrvDebug_h	// [
#define Included_CPGPdiskDrvDebug_h


////////
// Types
////////

// NameAssoc is used by the debug routines to associate strings to constants.

typedef struct NameAssoc
{
	PGPUInt32	func;
	LPCSTR		name;

} NameAssoc;

#endif	// ] Included_CPGPdiskDrvDebug_h
