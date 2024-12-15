//////////////////////////////////////////////////////////////////////////////
// DcbManager.cpp
//
// Implementation of class DcbManager.
//////////////////////////////////////////////////////////////////////////////

// $Id: DcbManager.cpp,v 1.1.2.4 1998/07/06 08:58:15 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include <vtoolscp.h>

#include "Required.h"
#include "FatUtils.h"
#include "UtilityFunctions.h"

#include "DcbManager.h"
#include "Wrappers.h"

#include LOCKED_CODE_SEGMENT
#include LOCKED_DATA_SEGMENT


///////////////////////////////////////////
// Class DcbManager public member functions
///////////////////////////////////////////

// The DcbManager constructor.

DcbManager::DcbManager()
{
	for (PGPUInt32 i=0; i<kMaxDrives; i++)
	{
		mDcbArray[i].isInUse = FALSE;
	}
}

// GetNumDcbs returns the number of DCBs in the container.

PGPUInt32 
DcbManager::GetNumDcbs()
{
	return mNumDcbs;
}

// EnumClaimedDcbs is simply an enumeration function for DCBs that have been
// claimed for use.

PDCB 
DcbManager::EnumClaimedDcbs(PGPUInt32 n)
{
	PDCB		pDcb	= NULL;
	PGPUInt32	i		= 0;
	PGPUInt32	j		= 0;

	while (i < kMaxDrives)
	{
		if (mDcbArray[i].isInUse && mDcbArray[i].isClaimed)
		{
			if (j++ == n)
			{
				pDcb = mDcbArray[i].pDcb;
				break;
			}
		}

		i++;
	}

	return pDcb;
}

// AddDcb adds a newly allocated DCB to the container and initializes it.

void 
DcbManager::AddDcb(PDCB pDcb)
{
	PGPUInt8 unitOnCtl;

	pgpAssertAddrValid(pDcb, PDCB);

	unitOnCtl = pDcb->DCB_unit_on_ctl;
	pgpAssert(IsLegalDriveNumber(unitOnCtl));

	mDcbArray[unitOnCtl].isInUse	= TRUE;
	mDcbArray[unitOnCtl].isClaimed	= FALSE;
	mDcbArray[unitOnCtl].pDcb		= pDcb;

	mNumDcbs++;
	InitDcb(pDcb);
}

// ClaimDcb finds an unclaimed DCB, marks it as being claimed, and returns a
// pointer to it for use by the client.

PDCB 
DcbManager::ClaimDcb()
{
	PDCB pDcb = NULL;

	for (PGPUInt32 i=0; i<kMaxDrives; i++)
	{
		if (mDcbArray[i].isInUse && !mDcbArray[i].isClaimed)
		{
			mDcbArray[i].isClaimed = TRUE;
			pDcb = mDcbArray[i].pDcb;

			break;
		}
	}

	return pDcb;
}

// ReleaseDcb takes a DCB that has been previously claimed and marks it as
// being not in use.

void 
DcbManager::ReleaseDcb(PDCB pDcb)
{
	PGPUInt8 unitOnCtl;

	pgpAssertAddrValid(pDcb, PDCB);

	unitOnCtl = pDcb->DCB_unit_on_ctl;
	pgpAssert(IsLegalDriveNumber(unitOnCtl));

	pgpAssert(mDcbArray[unitOnCtl].isInUse && mDcbArray[unitOnCtl].isClaimed);
	mDcbArray[unitOnCtl].isClaimed = FALSE;
}


////////////////////////////////////////////
// Class DcbManager private member functions
////////////////////////////////////////////

// InitDcb sets the DCB's fields so it is initialized enough that it won't
// crash the IOS, but virgin enough so Windows won't try to mount it itself.
// Later on when we claim this DCB we can fill it in with data corresponding
// to a PGPdisk and mount it.

void 
DcbManager::InitDcb(PDCB pDcb)
{
	pgpAssertAddrValid(pDcb, DCB);

	pDcb->DCB_cmn.DCB_drive_lttr_equiv	= 0xFF;
	pDcb->DCB_cmn.DCB_TSD_Flags			= DCB_TSD_NO_USER_INT13 |
											DCB_TSD_BAD_MBR;
	pDcb->DCB_cmn.DCB_vrp_ptr			= NULL;
	pDcb->DCB_cmn.DCB_device_flags		= DCB_DEV_PHYSICAL | 
											DCB_DEV_WRITEABLE;
	pDcb->DCB_cmn.DCB_device_flags2		= 0;
	pDcb->DCB_cmn.DCB_Partition_Start	= 0;
	pDcb->DCB_cmn.DCB_track_table_ptr	= 0;
	pDcb->DCB_cmn.DCB_bds_ptr			= 0;

	pDcb->DCB_cmn.DCB_apparent_blk_shift	= SimpleLog2(kDefaultBlockSize);
	pDcb->DCB_cmn.DCB_partition_type		= kBigFat16PartId;
	pDcb->DCB_cmn.DCB_device_type			= DCB_type_disk;

	pDcb->DCB_cmn.DCB_cAssoc		= 0;
	pDcb->DCB_cmn.DCB_user_drvlet	= 0xffff;

	pDcb->DCB_max_xfer_len			= kMaxHeads*kMaxSpt*kMaxCyls;
	pDcb->DCB_actual_sector_cnt[0]	= kMaxHeads*kMaxSpt*kMaxCyls;
	pDcb->DCB_actual_sector_cnt[1]	= 0;
	pDcb->DCB_actual_blk_size		= kDefaultBlockSize;
	pDcb->DCB_actual_head_cnt		= kMaxHeads;
	pDcb->DCB_actual_cyl_cnt		= kMaxCyls;
	pDcb->DCB_actual_spt			= kMaxSpt;
	pDcb->DCB_bus_type				= DCB_BUS_ESDI;

	// Fill in the BDD fields so older drivers won't become confused.

	pDcb->DCB_bdd.DCB_BDD_flags					= 0;
	pDcb->DCB_bdd.DCB_apparent_sector_cnt[0]	= kMaxHeads*kMaxSpt*kMaxCyls;
	pDcb->DCB_bdd.DCB_apparent_sector_cnt[1]	= 0;
	pDcb->DCB_bdd.DCB_apparent_blk_size			= kDefaultBlockSize;
	pDcb->DCB_bdd.DCB_apparent_head_cnt			= kMaxHeads;
	pDcb->DCB_bdd.DCB_apparent_cyl_cnt			= kMaxCyls;
	pDcb->DCB_bdd.DCB_apparent_spt				= kMaxSpt;
}
