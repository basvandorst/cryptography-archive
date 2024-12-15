//////////////////////////////////////////////////////////////////////////////
// CPGPdiskDriver.h
//
// Declaration of class CPGPdiskDriver.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPGPdiskDriver.h,v 1.3 1998/12/14 18:59:26 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CPGPdiskDriver_h	// [
#define Included_CPGPdiskDriver_h

#include "DualErr.h"


////////////
// Constants
////////////

const PGPUInt32 kPGPdiskMemPoolTag = 'dPGP';


////////
// Types
////////

class PGPdisk;


///////////////////////
// Class CPGPdiskDriver
///////////////////////
   
class CPGPdiskDriver : public KDriver
{
public:
	DualErr mInitErr;

	virtual NTSTATUS	DriverEntry(PUNICODE_STRING RegistryPath);
	virtual VOID		Unload();

	NTSTATUS			DispatchFilter(KDevice *pDevice, KIrp I, 
							NTSTATUS(KDevice::*func)(KIrp));

private:
	SAFE_DESTRUCTORS

	NTSTATUS DispatchPGPdiskRequest(KIrp I, PGPdisk *pPGD, 
				PGPBoolean *irpNeedsCompletion);
};

#endif // Included_CPGPdiskDriver_h