//////////////////////////////////////////////////////////////////////////////
// VolFile.cpp
//
// Implementation of class VolFile.
//////////////////////////////////////////////////////////////////////////////

// $Id: VolFile.cpp,v 1.6 1998/12/14 18:58:51 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "UtilityFunctions.h"

#include "Globals.h"
#include "VolFile.h"


////////////////////////////////////////
// Class VolFile public member functions
////////////////////////////////////////

// The VolFile constructor attaches a drive and a pathname for an already
// mounted VolFile.

VolFile::VolFile(LPCSTR path, PGPUInt8 drive) : File(), Volume(drive)
{
	if (IsntNull(path))
		mInitErr = File::SetPath(path);
}

// Mount asks the driver to mount the volume file specified by 'path'. If
// 'pMNT' exists, it must use this packet passed to it from a derived class,
// instead of its own packet. This allows extensions to be made to the MNT
// structure without breaking base classes.

DualErr 
VolFile::Mount(
	LPCSTR		path, 
	PGPUInt8	drive, 
	PGPBoolean	mountReadOnly, 
	PAD_Mount	useThisPMNT)
{
	AD_Mount	MNT, *pMNT;
	DualErr		derr;

	pMNT = (useThisPMNT ? useThisPMNT : &MNT);

	pgpAssertAddrValid(pMNT, AD_Mount);
	pgpAssert(Unmounted());
	pgpAssert(!Opened());
	
	derr = SetPath(path);

	// Set the fields we are responsible for and call down.
	if (derr.IsntError())
	{
		pMNT->path		= path;
		pMNT->sizePath	= strlen(path) + 1;
		pMNT->drive		= drive;

		derr = Volume::Mount(mountReadOnly, pMNT);
	}

	if (derr.IsError())
	{
		ClearPath();
	}

	return derr;
}

// Unmount asks the driver to unmount a volume. If 'pUMNT' exists, it must
// use this packet passed to it from a derived class, instead of its own
// packet. This allows extensions to be made to the UMNT structure without
// breaking base classes.

DualErr 
VolFile::Unmount(PGPBoolean isThisEmergency, PAD_Unmount useThisPUNMNT)
{
	AD_Unmount	UNMNT, *pUNMNT;
	DualErr		derr;

	pUNMNT = (useThisPUNMNT ? useThisPUNMNT : &UNMNT);

	pgpAssertAddrValid(pUNMNT, AD_Unmount);
	pgpAssert(Mounted());

	// We aren't responsible for any fields, so just call down.
	derr = Volume::Unmount(isThisEmergency, pUNMNT);

	if (derr.IsntError())
	{
		ClearPath();
	}

	return derr;
}
