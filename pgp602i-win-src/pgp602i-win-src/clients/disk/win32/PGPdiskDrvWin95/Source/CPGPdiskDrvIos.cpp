//////////////////////////////////////////////////////////////////////////////
// CPGPdiskDrvIos.cpp
//
// Contains the AEP and IOS request handling routines for the driver.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPGPdiskDrvIos.cpp,v 1.2.2.14.2.1 1998/10/30 22:49:17 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include <vtoolscp.h>

#include "Required.h"
#include "CPGPdiskDrvIos.h"
#include "GenericCallback.h"
#include "Globals.h"
#include "IopProcessor.h"
#include "Volume.h"

#include LOCKED_CODE_SEGMENT
#include LOCKED_DATA_SEGMENT


////////////
// Constants
////////////

// The following three constants are used to initialize three fields of each
// DCB during initialization.

static LPCSTR	kVendorId	= "PGP     ";				// vendor ID
static LPCSTR	kProductId	= "PGPdisk         ";		// product ID
static LPCSTR	kRevLevel	= "0001";					// revision level

const PGPUInt32	kSizeVendorId		= 8;	// must be 8 bytes
const PGPUInt32	kSizeProductId		= 16;	// must be 16 bytes
const PGPUInt32	kSizeRevLevel		= 4;	// must be 4 bytes
const PGPUInt32	kDefaultIopTimeout	= 15;	// seconds for IOP to timeout

const PGPUInt32	kContextToggleSeconds	= 5;
const PGPUInt32	kContextVerifySeconds	= 5;


////////////////////////////////
// AEP message handling routines
////////////////////////////////

// The AepInitialize packet is recieved once during IOS registration for the
// driver. It is responsible for creating a DDB for the driver (which the IOS
// requires).

PGPUInt16 
CPGPdiskDrv::AepInitialize(PAEP_bi_init pAep)
{
	if (mTheDDB = IspCreateDdb(sizeof(IOSDDB), NULL))
		return AEP_SUCCESS;
	else
		return AEP_FAILURE;
}

// AepDeviceInquiry is called a number of times during IOS registration, an
// each time we respond with AEP_SUCCESS the IOS allocates a DCB for us. We
// want as many DCBs as there can be PGPdisks mounted at one time.

PGPUInt16 
CPGPdiskDrv::AepDeviceInquiry(PAEP_inquiry_device pAEP)
{
	PDCB pDcb;

	pDcb = (PDCB) pAEP->AEP_i_d_dcb;
	pgpAssertAddrValid(pDcb, DCB);

	if (pDcb->DCB_unit_on_ctl >= kMaxMountedPGPdisks)
		return AEP_NO_MORE_DEVICES;

	// The following three statements copy several required identification
	// strings into the appropriate fields of the DCB.

	pgpCopyMemory(kVendorId, pDcb->DCB_vendor_id, kSizeVendorId);
	pgpCopyMemory(kProductId, pDcb->DCB_product_id, kSizeProductId);
	pgpCopyMemory(kRevLevel, pDcb->DCB_rev_level, kSizeRevLevel);

	return AEP_SUCCESS;
}

// AepConfigDcb is called the same number of times AepDeviceInquiry is called,
// once for each DCB. The driver takes this opportunity to add the DCB to the
// global list, initialize it, and add the I/O request routine to the DCB's
// calldown table. That means that we will be responsible for any I/O
// involving any volume we mount using this DCB at any time in the future.

PGPUInt16 
CPGPdiskDrv::AepConfigDcb(PAEP_dcb_config pAep)
{
	PDCB pDcb;

	pDcb = (PDCB) pAep->AEP_d_c_dcb;
	pgpAssertAddrValid(pDcb, DCB);

	// Before we configure the DCB we must check to make sure it is actually
	// one of ours. We do this by checking that it is a physical, not logical,
	// DCB, and that the vendor ID matches ours.

	if (IsPhysical(pDcb) &&
		(!strncmp((LPCSTR) pDcb->DCB_vendor_id, kVendorId, kSizeVendorId)))
	{
		// Add the DCB to the global list and initialize it.
		mDcbs.AddDcb(pDcb);

		// Now we insert the driver's IO request handler routines into the
		// calldown table for this DCB. In short, that means we are saying we
		// wish to intercept and possibly handle all IO requests for this
		// potential volume.

		if (!IspInsertCalldown(pDcb, PGPDISK_RequestHandlerStub, 
			(PDDB) pAep->AEP_d_c_hdr.AEP_ddb, 0, pDcb->DCB_cmn.DCB_dmd_flags, 
			pAep->AEP_d_c_hdr.AEP_lgn))
		{
			pgpDebugMsg("PGPdisk: IspInsertCalldown failed in AepDcbConfig");
			return AEP_FAILURE;
		}
	}

	return AEP_SUCCESS; 
}

// AepIopTimeout is called whenever a timeout is issued on an IOP. If it is
// one of ours, we just reset the timer (assuming that it's just waiting in
// our queue).

PGPUInt16 
CPGPdiskDrv::AepIopTimeout(PAEP_iop_timeout_occurred pAep)
{
	PGPUInt8	drive;
	PIOP		pIop;
	PIOR		pIor;
	PGPdisk		*pPGD	= NULL;

	pIop = (PIOP) pAep->AEP_i_t_o_iop;
	pgpAssertAddrValid(pIop, IOP);

	pIor = &pIop->IOP_ior;
	pgpAssertAddrValid(pIor, IOR);

	// First see if this request is coming FROM a PGPdisk.
	if (pIor->IOR_private_client == kPGPdiskIopMagic)
		DebugOut("PGPdisk: I/O from PGPdisk to a host drive timed-out.");

	// Now see if this request is going TO a PGPdisk.
	drive = pIop->IOP_ior.IOR_vol_designtr;

	if (drive < kMaxDrives)
	{
		pPGD = mPGPdisks.FindPGPdisk(drive);
	}

	// If so, reset the timer.
	if (pPGD)
	{
		DebugOut("PGPdisk: Resetting timed-out I/O to a PGPdisk.");

		pIop->IOP_timer = pIop->IOP_timer_orig;
		return AEP_SUCCESS;
	}
	else
	{
		return AEP_FAILURE;
	}
}

// AepOneSecondCallback performs certain operations that must occur each
// second at a safe time.

VOID 
__stdcall 
CPGPdiskDrv::AepOneSecondCallback(
	VMHANDLE		hVM, 
	THREADHANDLE	hThread, 
	PVOID			Refdata, 
	PCLIENT_STRUCT	pRegs)
{
	static PGPUInt32	lastToggleTime	= 0;
	static PGPUInt32	lastVerifyTime	= 0;

	// Increase the inactivity timer.
	Driver->mSecondsInactive++;
	
	// If we are in 'unmount all mode', keep trying to unmount all until we
	// succeed.

	if (Driver->mUnmountAllMode)
	{
		DualErr derr;

		derr = Driver->UnmountAllPGPdisks(FALSE);
		Driver->mUnmountAllMode = !derr.IsntError();
	}

	// Flip contexts on all PGPdisks if this is the time to do so.
	if (lastToggleTime++ > kContextToggleSeconds)
	{
		lastToggleTime = 0;
		Driver->mPGPdisks.FlipAllContexts();
	}

	// Verify cipher contexts on all PGPdisks if this is the time to do so.
	if (lastVerifyTime++ > kContextVerifySeconds)
	{
		lastVerifyTime = 0;
		Driver->mPGPdisks.ValidateAllCipherContexts();
	}

	// If it is time to auto-unmount, we do so. We schedule the unmounts to
	// occur during an appy-time callback.

	if ((Driver->mAutoUnmount) && 
		(Driver->mSecondsInactive > Driver->mUnmountTimeout*60))
	{
		Driver->mSecondsInactive = 0;
		Driver->UnmountAllPGPdisks();
	}
}

// AepOneSecond is called every one second. We schedule a callback to perform
// necessary tasks at a safe time.

PGPUInt16                                                                                    
CPGPdiskDrv::AepOneSecond()
{
	static RestrictedEvent_THUNK callbackThunk;

	Call_Restricted_Event(0, NULL, PEF_WAIT_NOT_CRIT, NULL, 
		AepOneSecondCallback, 0, &callbackThunk);

	return AEP_SUCCESS;
}

// PGPDISK_Aer is the asynchronous event handler for the driver. It is called
// to handle AEP packets sent by the system that represent various events and
// occurences, most importantly the sequence of events that result in
// initialization of the DCBs needed for volume mounting.

VOID 
__cdecl 
CPGPdiskDrv::PGPDISK_Aer(PAEP pAep)
{
	pgpAssertAddrValid(pAep, AEP);

	if ((pAep->AEP_func != AEP_HALF_SEC) && 
		(pAep->AEP_func != AEP_1_SEC))
	{
		DebugOut("PGPdisk: %s", Driver->GetAEPFunctionName(pAep->AEP_func));
	}

	switch (pAep->AEP_func)
	{
	case AEP_INITIALIZE:			// recieved during IOS registration
		pAep->AEP_result = Driver->AepInitialize((PAEP_bi_init) pAep);
		break;

	case AEP_CONFIG_DCB:			// recieved for each DCB we want allocated
		pAep->AEP_result = Driver->AepConfigDcb((PAEP_dcb_config) pAep);
		break;

	case AEP_DEVICE_INQUIRY:		// rcvd to determine how many DCBs we want
		pAep->AEP_result = Driver->AepDeviceInquiry((PAEP_inquiry_device) 
			pAep);
		break;

	case AEP_IOP_TIMEOUT:			// rcvd to notify driver of IOP timeout
		pAep->AEP_result = Driver->AepIopTimeout((PAEP_iop_timeout_occurred) 
			pAep);
		break;

	case AEP_1_SEC:					// notify us every half second
		pAep->AEP_result = Driver->AepOneSecond();

	default:						// default by returning success
		pAep->AEP_result = AEP_SUCCESS;
		break;
	}
}


////////////////////////////////
// I/O request handling routines
////////////////////////////////

// Due to an incredibly obscure interaction involving MSVC an optimization and
// an IOS bug (it passes IOP address on the stack, expected that value to be
// unaltered when the function returns - within local parameter storage no
// less!), IOS request handlers have to be called using a stub function.

VOID 
__cdecl 
CPGPdiskDrv::PGPDISK_RequestHandlerStub(PIOP pIop)
{
	PGPDISK_RequestHandler(pIop);
}

// PGPDISK_RequestHandler passes the IOP to our IopProcessor for - you
// guessed it - processing.

void  
CPGPdiskDrv::PGPDISK_RequestHandler(PIOP pIop)
{
	Driver->mIopProcessor.ProcessIop(pIop);
}
