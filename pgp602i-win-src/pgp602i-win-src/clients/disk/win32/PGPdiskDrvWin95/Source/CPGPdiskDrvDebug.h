//////////////////////////////////////////////////////////////////////////////
// CPGPdiskDrvDebug.h
//
// Declarations for CPGPdiskDrvDebug.cpp.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPGPdiskDrvDebug.h,v 1.1.2.3 1998/07/06 08:58:06 nryan Exp $

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
