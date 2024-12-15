//////////////////////////////////////////////////////////////////////////////
// PGPdiskContainer.h
//
// Declaration of class PGPdiskContainer.
//////////////////////////////////////////////////////////////////////////////

// $Id: PGPdiskContainer.h,v 1.1.2.17 1998/08/04 02:10:15 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_PGPdiskContainer_h	// [
#define Included_PGPdiskContainer_h

#include "DualErr.h"


////////
// Types
////////

class PGPdisk;


/////////////////////////
// Class PGPdiskContainer
/////////////////////////

// Class PGPdiskContainer objects serve as containers for mounted PGPdisks.
// You can attach a PGPdisk by pointer and detach it, as well as search the
// list of PGPdisks for one matching a specific mounted drive number or path.
// This class is thread-safe.

class PGPdiskContainer
{
public:
				PGPdiskContainer();
				~PGPdiskContainer() { };

	PGPUInt32	GetNumPGPdisks();
	PGPdisk	*	EnumPGPdisks(PGPUInt32 n);

	PGPBoolean	IsLocalDriveAPGPdiskHost(PGPUInt8 drive);

	PGPdisk *	FindPGPdisk(PGPUInt8 drive);
	PGPdisk *	FindPGPdisk(LPCSTR path);

#if defined(PGPDISK_95DRIVER)
	PGPdisk *	FindPGPdisk(PDCB pDcb);
#endif	// PGPDISK_95DRIVER

#if defined(PGPDISK_95DRIVER) || defined(PGPDISK_NTDRIVER)

	void		FlipAllContexts();
	void		ValidateAllCipherContexts();

#endif	// PGPDISK_95DRIVER || PGPDISK_NTDRIVER

	void		AddPGPdisk(PGPdisk *pPGD);
	void		RemovePGPdisk(PGPdisk *pPGD);

	void		ClearContainer();
	void		ClearContainerWithDelete();

private:
	PGPdisk		*mPGPdiskArray[kMaxDrives];	// cross-ref drives&PGPdisks
	PGPUInt32	mNumPGPdisks;				// population of the array

	PGPUInt32	mLocalHosts[kMaxDrives];	// local vol host PGPdisk?
};

#endif	// ] Included_PGPdiskContainer_h
