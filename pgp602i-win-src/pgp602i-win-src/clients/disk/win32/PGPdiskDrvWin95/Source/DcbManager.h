//////////////////////////////////////////////////////////////////////////////
// DcbManager.h
//
// Declaration of class DcbManager.
//////////////////////////////////////////////////////////////////////////////

// $Id: DcbManager.h,v 1.1.2.4 1998/07/06 08:58:16 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_DcbManager_h	// [
#define Included_DcbManager_h

#include "DualErr.h"


////////
// Types
////////

// DcbWrapper is used by the DcbManager to keep track of which DCBs are in
// use.

typedef struct DcbWrapper
{
	PGPBoolean	isInUse;		// wrapper in use?
	PGPBoolean	isClaimed;		// DCB claimed?
	PDCB		pDcb;			// wrapped DCB

} DcbWrapper;


///////////////////
// Class DcbManager
///////////////////

// Class DcbManager keeps track of the DCBs allocated by the IOS subsystem for
// the driver's use. The client adds DCBs to the container as they are
// recieved during IOS registration, and can claim and release DCBs later on
// as they are needed. A DCB describes a volume either mounted or unmounted,
// and the same DCB can be used at different times to represent different
// volumes. This class is Ring-0 thread-safe.

class DcbManager
{
public:
				DcbManager();

	PGPUInt32	GetNumDcbs();

	PDCB		EnumClaimedDcbs(PGPUInt32 n);

	void		AddDcb(PDCB pDcb);
	PDCB		ClaimDcb();
	void		ReleaseDcb(PDCB pDcb);

private:
	DcbWrapper	mDcbArray[kMaxDrives];	// cross-ref unit #s and DCBs
	PGPUInt32	mNumDcbs;				// population of the array

	void InitDcb(PDCB pDcb);
};

#endif	// ] Included_DcbManager_h
