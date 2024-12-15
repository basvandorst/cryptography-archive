//////////////////////////////////////////////////////////////////////////////
// VolFile.h
//
// Declaration of class VolFile.
//////////////////////////////////////////////////////////////////////////////

// $Id: VolFile.h,v 1.3 1998/12/14 18:59:50 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_VolFile_h	// [
#define Included_VolFile_h

#include "DualErr.h"

#include "File.h"
#include "Volume.h"


////////////////
// Class VolFile
////////////////

// Class VolFile objects represent volumes associated with a file on a host
// drive. These files can be mounted and unmounted at will, and all I/O
// requests will be routed to the appropriate location on these files just
// like they were hard disks themselves.

class VolFile : public File, public Volume
{
public:
				VolFile();
	virtual		~VolFile();

	void		GetGeometry(PDISK_GEOMETRY pGeom);
	void		GetPartitionInfo(PPARTITION_INFORMATION pPartInfo);

	DualErr		Mount(LPCSTR path, LPCSTR deviceName, PGPUInt64 blocksHeader, 
					PGPUInt64 blocksDisk, PGPUInt8 drive = kInvalidDrive, 
					PGPBoolean readOnly = FALSE);
	DualErr		Unmount(PGPBoolean isThisEmergency = FALSE);

	DualErr		Read(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes);
	DualErr		Write(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes);

private:
	PGPBoolean	mFailAllIo;			// fail all input/output?

	PGPUInt64	mBlocksDisk;		// size of volume
	PGPUInt64	mBlocksHeader;		// size of file header
};

#endif	// ] Included_VolFile_h
