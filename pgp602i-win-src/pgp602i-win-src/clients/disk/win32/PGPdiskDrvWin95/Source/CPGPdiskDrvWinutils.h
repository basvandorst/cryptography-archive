//////////////////////////////////////////////////////////////////////////////
// CPGPdiskDrvWinutils.h
//
// Declarations for CPGPdiskDrvWinutils.cpp.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPGPdiskDrvWinutils.h,v 1.1.2.8 1998/08/08 01:22:07 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CPGPdiskDrvWinutils_h	// [
#define Included_CPGPdiskDrvWinutils_h

#include "WaitObjectClasses.h"


////////////
// Constants
////////////

const PGPUInt16 kSizeShared = 0x1000;	// bytes shared memory


////////
// Types
////////

// Lock level enumeration.

enum LockLevel {kLock_None, kLock_L1, kLock_L2, kLock_L3, kLock_L0, 
	kLock_L0MR};

// Registers is used to store the input and output register values for calls
// to the interrupt execution routines.

typedef struct Registers
{
	PGPUInt32	EAX;			// general register EAX
	PGPUInt32	EBX;			// general register EBX
	PGPUInt32	ECX;			// general register ECX
    PGPUInt32	EDX;			// general register EDX
	PGPUInt32	EDI;			// general register EDI
	PGPUInt32	ESI;			// general register ESI

	PGPUInt16	DS;				// segment register DS
	PGPUInt16	ES;				// segment register ES

	PGPUInt16	Flags;			// lower word of EFLAGS register

} Registers;

// SharedMem is used to store both the linear Ring0 and the Ring3 selector:
// offset pointer to a given block of memory allocated at Ring0.

typedef struct SharedMem
{
	PGPUInt32		nBytes;		// size of the memory block
	void			*linear;	// R0 linear address of memory block
	PGPUInt32		segment;	// segment or selector
	PGPUInt32		offset;		// offset
	PGPdiskMutex	mutex;		// mutex for access to shared memory

} SharedMem;

#endif	// ] Included_CPGPdiskDrvWinutils_h
