//////////////////////////////////////////////////////////////////////////////
// PGPdiskContainer.cpp
//
// Implementation of class PGPdiskContainer.
//////////////////////////////////////////////////////////////////////////////

// $Id: PGPdiskContainer.cpp,v 1.9 1999/03/31 23:51:09 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#if defined(PGPDISK_MFC)

#include "StdAfx.h"

#elif defined(PGPDISK_95DRIVER)

#include <vtoolscp.h>

#elif defined(PGPDISK_NTDRIVER)

#define	__w64
#include <vdw.h>

#else
#error Define PGPDISK_MFC, PGPDISK_95DRIVER, or PGPDISK_NTDRIVER.
#endif	// PGPDISK_MFC

#include "Required.h"
#include "PGPdiskContainer.h"
#include "PGPdiskHighLevelUtils.h"
#include "UtilityFunctions.h"

#include "PGPdisk.h"

#if defined(PGPDISK_95DRIVER) || defined(PGPDISK_NTDRIVER)
#include "Globals.h"
#endif	// PGPDISK_95DRIVER || PGPDISK_NTDRIVER


/////////////////////////////////////////////////
// Class PGPdiskContainer public member functions
/////////////////////////////////////////////////

// The PGPdiskContainer constructor.

PGPdiskContainer::PGPdiskContainer()
{
	mNumPGPdisks = 0;

	ClearContainer();
}

// GetSize returns the number of PGPdisks in the container.

PGPUInt32 
PGPdiskContainer::GetNumPGPdisks()
{
	return mNumPGPdisks;
}

// EnumPGPdisks returns the nth PGPdisk in the container.

PGPdisk * 
PGPdiskContainer::EnumPGPdisks(PGPUInt32 n)
{
	PGPBoolean	foundPGPdisk	= FALSE;
	PGPdisk		*pPGD;
	PGPUInt32	i				= 0;
	PGPUInt32	j				= 0;

	while (i < kMaxDrives)
	{
		if (mPGPdiskArray[i])
		{
			if (j++ == n)
			{
				foundPGPdisk = TRUE;
				pPGD = mPGPdiskArray[i];
				break;
			}
		}

		i++;
	}

	if (foundPGPdisk)
		return pPGD;
	else
		return NULL;
}

// IsLocalDriveAPGPdiskHost returns TRUE if specified local drive hosts a
// PGPdisk, FALSE otherwise.

PGPBoolean 
PGPdiskContainer::IsLocalDriveAPGPdiskHost(PGPUInt8 drive)
{
	pgpAssert(IsLegalDriveNumber(drive));

	return (mLocalHosts[drive] > 0);
}

// FindPGPdisk(PGPUInt8 drive) finds and returns the PGPdisk specified by
// 'drive'.

PGPdisk * 
PGPdiskContainer::FindPGPdisk(PGPUInt8 drive)
{
	pgpAssert(IsLegalDriveNumber(drive));

	return mPGPdiskArray[drive];
}

// FindPGPdisk(LPCSTR path) finds and returns the PGPdisk whose pathname is
// 'path'. Note how it compares the session Id, not the exact path.

PGPdisk * 
PGPdiskContainer::FindPGPdisk(LPCSTR path)
{
	PGPdisk		*pPGD;
	PGPUInt32	i	= 0;
	PGPUInt64	sessionId;

	pgpAssertStrValid(path);

	sessionId = GetPGPdiskUniqueSessionId(path);

	pPGD = EnumPGPdisks(i);

	while (IsntNull(pPGD))
	{
		if ((pPGD->GetUniqueSessionId() == sessionId) && 
			pPGD->BestGuessComparePaths(path))
		{
			break;
		}

		pPGD = EnumPGPdisks(i++);
	}

	return pPGD;
}

#if defined(PGPDISK_95DRIVER)

// FindPGPdisk(PDCB pDcb) finds and returns the PGPdisk whose DCB is 'pDcb'.

PGPdisk * 
PGPdiskContainer::FindPGPdisk(PDCB pDcb)
{
	PGPUInt32	i	= 0;
	PGPdisk		*pPGD;

	pgpAssertAddrValid(pDcb);

	while (pPGD = EnumPGPdisks(i++))
	{
		if (pPGD->GetDcb() == pDcb)
			break;
	}

	return pPGD;
}

// FlipAllContexts tells each PGPdisk in the container to flip the bits on
// each of its contexts.

void 
PGPdiskContainer::FlipAllContexts()
{
	PGPUInt32	i	= 0;
	PGPdisk		*pPGD;

	while (pPGD = EnumPGPdisks(i++))
	{
		pPGD->FlipContexts();
	}
}

// ValidateAllCipherContexts tells each PGPdisk in the container to verify
// that its cipher contexts are not damaged.

void 
PGPdiskContainer::ValidateAllCipherContexts()
{
	DualErr		derr;
	PGPUInt32	i	= 0;
	PGPdisk		*pPGD;

	while (pPGD = EnumPGPdisks(i++))
	{
		derr = pPGD->ValidateContexts();

		if (derr.IsError())
		{
			DualErr derr;

			PGPUInt8 drive = pPGD->GetDrive();

			// Emergency unmount NOW.
			derr = Driver->UnmountPGPdisk(drive, TRUE);

			if (derr.IsntError())
			{
				Driver->ReportError(kPGDMajorError_InvalidCipherContext, 
					DualErr::NoError, drive);
			}
		}
	}
}

#elif defined(PGPDISK_NTDRIVER)

// FlipAllContexts tells each PGPdisk in the container to flip the bits on
// each of its contexts.

void 
PGPdiskContainer::FlipAllContexts()
{
	PGPUInt32	i	= 0;
	PGPdisk		*pPGD;

	while (pPGD = EnumPGPdisks(i++))
	{
		pPGD->FlipContexts();
	}
}

// ValidateAllCipherContexts tells each PGPdisk in the container to verify
// that its cipher contexts are not damaged.

void 
PGPdiskContainer::ValidateAllCipherContexts()
{
	DualErr		derr;
	PGPUInt32	i	= 0;
	PGPdisk		*pPGD;

	while (pPGD = EnumPGPdisks(i++))
	{
		derr = pPGD->ValidateContexts();

		if (derr.IsError())
		{
			DualErr derr;

			PGPUInt8 drive = pPGD->GetDrive();

			// Emergency unmount NOW.
			derr = Interface->UnmountPGPdisk(drive, TRUE);

			if (derr.IsntError())
			{
				Interface->ReportError(kPGDMajorError_InvalidCipherContext, 
					DualErr::NoError, drive);
			}
		}
	}
}

#endif	// ] PGPDISK_95DRIVER

// AddPGPdisk adds a mounted PGPdisk to the container.

void 
PGPdiskContainer::AddPGPdisk(PGPdisk *pPGD)
{
	PGPUInt8 drive;

	pgpAssertAddrValid(pPGD, PGPdisk);
	pgpAssert(pPGD->Mounted());

	drive = pPGD->GetDrive();

	pgpAssert(IsLegalDriveNumber(drive));
	pgpAssert(IsNull(mPGPdiskArray[drive]));

	mPGPdiskArray[drive] = pPGD;
	mNumPGPdisks++;

	if (!pPGD->IsHostNetworked())
		mLocalHosts[pPGD->GetLocalHostDrive()]++;
}

// RemovePGPdisk looks for the passed PGPdisk pointer in the container and
// removes it if it was found.

void 
PGPdiskContainer::RemovePGPdisk(PGPdisk *pPGD)
{
	PGPUInt8 drive;

	pgpAssertAddrValid(pPGD, PGPdisk);
	pgpAssert(pPGD->Mounted());

	drive = pPGD->GetDrive();
	pgpAssert(IsLegalDriveNumber(drive));
	pgpAssert(IsntNull(mPGPdiskArray[drive]));

	mPGPdiskArray[drive] = NULL;
	mNumPGPdisks--;

	if (!pPGD->IsHostNetworked())
		mLocalHosts[pPGD->GetLocalHostDrive()]--;
}

// ClearContainer purges all the PGPdisks from the container.

void 
PGPdiskContainer::ClearContainer()
{
	for (PGPUInt32 i=0; i<kMaxDrives; i++)
	{
		mLocalHosts[i] = 0;
		mPGPdiskArray[i] = NULL;
	}

	mNumPGPdisks = 0;
}

// ClearContainerWithDelete purges all the PGPdisks from the container and
// deletes the objects.

void 
PGPdiskContainer::ClearContainerWithDelete()
{
	for (PGPUInt32 i=0; i<kMaxDrives; i++)
	{
		mLocalHosts[i] = 0;

		if (IsntNull(mPGPdiskArray[i]))
		{
			pgpAssertAddrValid(mPGPdiskArray[i], PGPdisk);

			delete mPGPdiskArray[i];
			mPGPdiskArray[i] = NULL;
		}
	}

	mNumPGPdisks = 0;
}
