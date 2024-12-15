//////////////////////////////////////////////////////////////////////////////
// VolFile.h
//
// Declaration of class VolFile.
//////////////////////////////////////////////////////////////////////////////

// $Id: VolFile.h,v 1.1.2.6 1998/07/06 08:57:32 nryan Exp $

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

// Class VolFile represents volume files.

class VolFile : public Volume, public File
{
public:
	DualErr	mInitErr;

			VolFile(LPCSTR path = NULL, PGPUInt8 drive = kInvalidDrive);
			~VolFile() { };

	DualErr	Mount(LPCSTR path, PGPUInt8 drive = kInvalidDrive, 
				PGPBoolean mountReadOnly = FALSE, 
				PAD_Mount useThisPMNT = NULL);
	DualErr	Unmount(PGPBoolean isThisEmergency = FALSE, 
				PAD_Unmount useThisPUNMNT = NULL);
};

#endif	// ] Included_VolFile_h
