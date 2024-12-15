//////////////////////////////////////////////////////////////////////////////
// DcbManager.cpp
//
// Implementation of class DcbManager.
//////////////////////////////////////////////////////////////////////////////

// $Id: DcbManager.cpp,v 1.5 1999/02/13 04:24:35 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include <vtoolscp.h>

#include "Required.h"
#include "FatUtils.h"
#include "UtilityFunctions.h"

#include "DcbManager.h"
#include "Globals.h"
#include "Wrappers.h"


////////////
// Constants
////////////

static LPCSTR	kVendorId	= "PGP     ";				// vendor ID
static LPCSTR	kProductId	= "PGPdisk         ";		// product ID
static LPCSTR	kRevLevel	= "0001";					// revision level

const PGPUInt32	kSizeVendorId		= 8;	// must be 8 bytes
const PGPUInt32	kSizeProductId		= 16;	// must be 16 bytes
const PGPUInt32	kSizeRevLevel		= 4;	// must be 4 bytes


///////////////////////////////////////////
// Class DcbManager public member functions
///////////////////////////////////////////

// NewDcb allocates, initializes, and returns a DCB.

DualErr 
DcbManager::NewDcb(PDCB *ppDcb)
{
	DualErr	derr;

	if (IsNull(*ppDcb = new DCB))
		derr = DualErr(kPGDMinorError_OutOfMemory);
		
	if (derr.IsntError())
	{
		InitDcb(*ppDcb);

		if (!IspInsertCalldown(*ppDcb, Driver->PGPDISK_RequestHandlerStub, 
				Driver->mTheDDB, 0, (*ppDcb)->DCB_cmn.DCB_dmd_flags, 
				Driver->mLoadGroupNum))
		{
			derr = DualErr(kPGDMinorError_IspInsertCalldownFailed);		
		}
	}

	return derr;
}

// DeleteDcb deletes a DCB.

void 
DcbManager::DeleteDcb(PDCB pDcb)
{
	pgpAssertAddrValid(pDcb);
	delete pDcb;
}


////////////////////////////////////////////
// Class DcbManager private member functions
////////////////////////////////////////////

void 
DcbManager::InitDcb(PDCB pDcb)
{
	pgpAssertAddrValid(pDcb, DCB);
	pgpClearMemory(pDcb, sizeof(DCB));

	pDcb->DCB_cmn.DCB_physical_dcb		= (ULONG) pDcb;
	pDcb->DCB_cmn.DCB_drive_lttr_equiv	= 0xFF;
	pDcb->DCB_cmn.DCB_TSD_Flags			= DCB_TSD_NO_USER_INT13 |
											DCB_TSD_BAD_MBR;
	pDcb->DCB_cmn.DCB_vrp_ptr			= NULL;
	pDcb->DCB_cmn.DCB_device_flags		= DCB_DEV_LOGICAL;

	pDcb->DCB_cmn.DCB_apparent_blk_shift	= SimpleLog2(kDefaultBlockSize);
	pDcb->DCB_cmn.DCB_partition_type		= kBigFat16PartId;
	pDcb->DCB_cmn.DCB_sig					= 0x4342;
	pDcb->DCB_cmn.DCB_device_type			= DCB_type_disk;

	pDcb->DCB_cmn.DCB_user_drvlet	= 0xffff;
	pDcb->DCB_max_xfer_len			= kMaxHeads*kMaxSpt*kMaxCyls;
	pDcb->DCB_actual_sector_cnt[0]	= kMaxHeads*kMaxSpt*kMaxCyls;
	pDcb->DCB_actual_sector_cnt[1]	= 0;
	pDcb->DCB_actual_blk_size		= kDefaultBlockSize;
	pDcb->DCB_actual_head_cnt		= kMaxHeads;
	pDcb->DCB_actual_cyl_cnt		= kMaxCyls;
	pDcb->DCB_actual_spt			= kMaxSpt;
	pDcb->DCB_bus_type				= DCB_BUS_ESDI;

	pgpCopyMemory(kVendorId, pDcb->DCB_vendor_id, kSizeVendorId);
	pgpCopyMemory(kProductId, pDcb->DCB_product_id, kSizeProductId);
	pgpCopyMemory(kRevLevel, pDcb->DCB_rev_level, kSizeRevLevel);

	// Fill in the BDD fields so older drivers won't become confused.
	pDcb->DCB_bdd.DCB_BDD_BD_Major_Version	= 0;
	pDcb->DCB_bdd.DCB_BDD_BD_Minor_Version	= 0;
	pDcb->DCB_bdd.DCB_BDD_Device_SubType	= BDT_FIXED_DISK;
	pDcb->DCB_bdd.DCB_BDD_flags				= BDF_VERSION_002;

	pDcb->DCB_bdd.DCB_apparent_sector_cnt[0]	= kMaxHeads*kMaxSpt*kMaxCyls;
	pDcb->DCB_bdd.DCB_apparent_sector_cnt[1]	= 0;
	pDcb->DCB_bdd.DCB_apparent_blk_size			= kDefaultBlockSize;
	pDcb->DCB_bdd.DCB_apparent_head_cnt			= kMaxHeads;
	pDcb->DCB_bdd.DCB_apparent_cyl_cnt			= kMaxCyls;
	pDcb->DCB_bdd.DCB_apparent_spt				= kMaxSpt;
}
