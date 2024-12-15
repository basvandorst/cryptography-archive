//////////////////////////////////////////////////////////////////////////////
// CPGPdiskDrvErrors.h
//
// Ddeclarations for CPGPdiskDrvErrors.cpp.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPGPdiskDrvErrors.h,v 1.1.2.7 1998/07/06 08:58:08 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CPGPdiskDrvErrors_h	// [
#define Included_CPGPdiskDrvErrors_h

#include "DualErr.h"
#include "Packets.h"


////////////
// Constants
////////////

const PGPUInt32 kNumErrPackets = 30;	// # of prealloc'd error packets


////////
// Types
////////

// An ErrorCell is a simple holding place for error data.

typedef struct ErrorCell
{
	PGPBoolean		isInUse;	// cell in use?
	PGDMajorError	perr;		// major error
	DualErr			derr;		// minor error
	PGPUInt8		drive;		// drive data field
	LPSTR			errString;	// error string

} ErrorCell;

#endif	// ] Included_CPGPdiskDrvErrors_h
