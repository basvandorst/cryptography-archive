//////////////////////////////////////////////////////////////////////////////
// Wrappers.h
//
// Wrappers for many IOS and ILB functions used by the driver.
//////////////////////////////////////////////////////////////////////////////

// $Id: Wrappers.h,v 1.1.2.3.2.1 1998/10/05 18:28:24 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_Wrappers_h	// [
#define Included_Wrappers_h

#include <ddb.h>


////////////
// Constants
////////////

#define	VXDINLINE	static __inline
#define	NAKED		__declspec(naked)


/////////
// Macros
/////////

#define FIELDOFFSET(type, field) ((DWORD)(&((type *)0)->field))


//////////
// Externs
//////////

extern ILB PGPDISK_Ilb;			// the driver's Ilb


////////
// Types
////////

typedef IOP *PIOP;


///////////////////////////////////
// Wrappers around ILB system calls
///////////////////////////////////

// IlbDequeueIop pops the next IOP off of the IOP queue for the given DCB and
// returns it.

VXDINLINE 
PIOP 
IlbDequeueIop(PDCB pDcb)
{
	PIOP pIop;

	__asm
	{
		push pDcb
		call [PGPDISK_Ilb.ILB_dequeue_iop]
		add esp, 4
		mov pIop, eax
	}

	return pIop;
}

// IlbEnqueueIop places the given IOP into the IOS IOP queue for later retrieval
// and processing. Each DCB has a different queue, of course.

VXDINLINE 
VOID 
IlbEnqueueIop(PIOP pIop, PDCB pDcb)
{
	__asm
	{
		push pDcb
		push pIop
		call [PGPDISK_Ilb.ILB_enqueue_iop]
		add esp, 8
	}
}

// IlbInternalRequest is like IOS_SendCommand except it is meant for use by
// IOS layer drivers.

VXDINLINE 
VOID 
IlbInternalRequest(PIOP pIop, PDCB pDcb, void *calldown)
{
	__asm
	{
		mov edi, pIop
		mov ebx, pDcb
		mov edx, calldown
	}

	(* (VOID (*)()) PGPDISK_Ilb.ILB_internal_request)();
}

// IlbIntIoCriteria prepares the given IOR before it should be given to the
// IOS internal request routing for processing.

VXDINLINE 
PGPBoolean 
IlbIntIoCriteria(PIOP pIop)
{
	PGPUInt32 result;

	__asm
	{
		push pIop
		call [PGPDISK_Ilb.ILB_int_io_criteria_rtn]
		add esp, 4
		mov result, eax
	}

	return (result == 0);
}

// IlbIoCriteria prepares the given IOR before it should be given to the IOS
// for processing.

VXDINLINE 
PGPBoolean 
IlbIoCriteria(PIOR pIor)
{
	PGPUInt32 result;

	__asm
	{
		push pIor
		call [PGPDISK_Ilb.ILB_io_criteria_rtn]
		add esp, 4
		mov result, eax
	}

	return (result == 0);
}

// IlbService calls the driver's ILB service routine with the specified ISP
// packet, which defines a request for a certain service from the system.

VXDINLINE 
PGPUInt32 
IlbService(PISP pIsp)
{
	PGPUInt32 result;

	__asm
	{
		push pIsp
		call [PGPDISK_Ilb.ILB_service_rtn]
		add esp, 4
		mov result, eax
	}

	return result;
}

VXDINLINE 
VOID 
IlbWait10thSec()
{
	__asm call [PGPDISK_Ilb.ILB_Wait_10th_Sec]
}


///////////////////////////////////
// Wrappers around IRS system calls
///////////////////////////////////

// IrsGetDriveInfo returns information about the specified drive.

VXDINLINE 
DWORD 
IrsGetDriveInfo(PGPUInt8 drive)
{
	IRS_drv_get irsDG;
	
	irsDG.IRS_func		= IRS_GET_DRVINFO;
	irsDG.IRS_result	= 0;
	irsDG.IRS_DrvLetter	= drive + 'A';
	irsDG.IRS_DrvFlags	= NULL;

	IOS_Requestor_Service((PIRS) &irsDG);
	
	if (irsDG.IRS_result != 0)
		return 0xFFFFFFFF;
	else
		return irsDG.IRS_DrvFlags;
}


///////////////////////////////////
// Wrappers around ISP system calls
///////////////////////////////////

// IspAssociateDcb associates the given DCB with the given drive number. This
// effectively mounts the DCB as the given drive number on the system.

VXDINLINE 
PGPBoolean 
IspAssociateDcb(PDCB pDcb, PGPUInt8 drive, PGPUInt8 flags)
{
	ISP_dcb_associate ispDA;
	
	ispDA.ISP_d_a_hdr.ISP_func	= ISP_ASSOCIATE_DCB;
	ispDA.ISP_d_a_dcb			= (PDCB_COMMON) pDcb;
	ispDA.ISP_d_a_drive			= drive;
	ispDA.ISP_d_a_flags			= flags;
	
	IlbService((PISP) &ispDA);
	return (ispDA.ISP_d_a_hdr.ISP_result == 0);
}

// IspCreateDdb creates the driver's DDB structure during IOS initialization.

VXDINLINE 
PDDB 
IspCreateDdb(PGPUInt16 size, PGPUInt8 flags)
{
	ISP_ddb_create ispDC;

	ispDC.ISP_ddb_hdr.ISP_func	= ISP_CREATE_DDB;
	ispDC.ISP_ddb_size			= sizeof(IOSDDB);
	ispDC.ISP_ddb_flags			= flags;
	
	IlbService((PISP) &ispDC);
	return (PDDB) ispDC.ISP_ddb_ptr;
}

// IspCreateIop allocates a new IOP/IOR structure.

VXDINLINE 
PIOP 
IspCreateIop(
	PGPUInt16	size, 
	PGPUInt32	delta, 
	PGPUInt8	flags, 
	PIOP		pIop = NULL)
{
	ISP_IOP_alloc ispIA;

	if (pIop)
		pgpAssert(flags & ISP_M_FL_EXTERNAL_IOP == ISP_M_FL_EXTERNAL_IOP);
	
	ispIA.ISP_i_c_hdr.ISP_func	= ISP_CREATE_IOP;
	ispIA.ISP_IOP_size			= size;
	ispIA.ISP_delta_to_ior		= delta;
	ispIA.ISP_IOP_ptr			= (PGPUInt32) pIop;	// for external allocation
	ispIA.ISP_i_c_flags			= flags;

	IlbService((PISP) &ispIA);

	if (ispIA.ISP_i_c_hdr.ISP_result == 0)
		return (PIOP) (ispIA.ISP_IOP_ptr + delta - FIELDOFFSET(IOP, IOP_ior));
	else
		return NULL;
}

// IspDeallocMem deallocates memory allocated by the IOS.

VXDINLINE 
PGPBoolean 
IspDeallocMem(void *pMem)
{
	ISP_mem_dealloc ispMD;
	
	ispMD.ISP_mem_da_hdr.ISP_func = ISP_DEALLOC_MEM;
	ispMD.ISP_mem_ptr_da = (ULONG) pMem;

	IlbService((PISP) &ispMD);
	return (ispMD.ISP_mem_da_hdr.ISP_result == 0);
}

// IspDisassociateDcb disassociates the given drive letter from the mounted
// DCB that describes the volume it is associated with.

VXDINLINE 
PGPBoolean 
IspDisassociateDcb(PGPUInt8 drive)
{
	ISP_DISASSOC_DCB ispDD;

	ispDD.ISP_dis_dcb_hdr.ISP_func = ISP_DISASSOCIATE_DCB;
	ispDD.ISP_dis_dcb_drive = drive;

	IlbService((PISP) &ispDD);
	return (ispDD.ISP_dis_dcb_hdr.ISP_result == 0);
}

// IspDriveLetterPick asks the system to pick and return a free drive letter
// that can be used for mounting.

VXDINLINE 
PGPUInt8 
IspDriveLetterPick(PDCB pDcb, PGPUInt8 flags)
{
	ISP_pick_drive_letter ispPDL;
	
	ispPDL.ISP_p_d_l_hdr.ISP_func	= ISP_DRIVE_LETTER_PICK;
	ispPDL.ISP_p_d_l_pdcb			= pDcb;
	ispPDL.ISP_p_d_l_flags			= flags;

	IlbService((PISP) &ispPDL);

	if (ispPDL.ISP_p_d_l_hdr.ISP_result == 0)
		return ispPDL.ISP_p_d_l_letter[0];
	else
		return kInvalidDrive;
}

// IspGetDcb will retrieve the logical DCB associated with a given drive #,
// which must represent a mounted volume.

VXDINLINE 
PDCB 
IspGetDcb(PGPUInt8 drive)
{
	ISP_dcb_get ispDG;

	ispDG.ISP_g_d_hdr.ISP_func = ISP_GET_DCB;
	ispDG.ISP_g_d_drive = drive;

	IlbService((PISP) &ispDG);

	if (ispDG.ISP_g_d_hdr.ISP_result == 0)
		return (PDCB) ispDG.ISP_g_d_dcb;
	else
		return NULL;
}

// IspInsertCalldown inserts the driver into the I/O calldown table for the
// given DCB.

VXDINLINE 
PGPBoolean 
IspInsertCalldown(
	PDCB		pDcb, 
	void		*req, 
	PDDB		ddb, 
	PGPUInt16	expan, 
	PGPUInt32	flags, 
	PGPUInt8	lgn)
{
	ISP_calldown_insert ispCI;

	pgpFillMemory(&ispCI, sizeof(ispCI), NULL);

	ispCI.ISP_i_cd_hdr.ISP_func	= ISP_INSERT_CALLDOWN;
	ispCI.ISP_i_cd_dcb			= (PDCB_COMMON) pDcb;
	ispCI.ISP_i_cd_req			= req;		
	ispCI.ISP_i_cd_ddb			= ddb;		
	ispCI.ISP_i_cd_expan_len	= expan;
	ispCI.ISP_i_cd_flags		= flags;	
	ispCI.ISP_i_cd_lgn			= lgn;		

	IlbService((PISP) &ispCI);
	return (!ispCI.ISP_i_cd_hdr.ISP_result);
}

// IspDeallocMem allocates memory from the IOS.

VXDINLINE 
void * 
IspAllocMem(PGPUInt16 memsize, PGPUInt8 flags)
{
	ISP_mem_alloc ispMA;

	ispMA.ISP_mem_a_hdr.ISP_func	= ISP_ALLOC_MEM;
	ispMA.ISP_mem_size				= memsize;
	ispMA.ISP_mem_type				= NULL;
	ispMA.ISP_mem_flags				= flags;

	IlbService((PISP) &ispMA);
	return (void *) (ispMA.ISP_mem_ptr);
}


//////////////////////////////
// Small IOS utility functions
//////////////////////////////

/// ExtractBlockSize returns the block size of the volume describe by the DCB.

inline 
PGPUInt16 
ExtractBlockSize(PDCB pDcb)
{
	return (1 << pDcb->DCB_cmn.DCB_apparent_blk_shift);
}

// BusID returns the bus ID of the DCB.

inline 
PGPUInt8 
ExtractBusID(PDCB pDcb)
{
	return pDcb->DCB_unit_on_ctl;
}

// Drive returns the drive number associated with a DCB.

inline 
PGPUInt8 
ExtractDrive(PDCB pDcb)
{
	return pDcb->DCB_cmn.DCB_drive_lttr_equiv;
}

// InBytes returns TRUE if the IOR's transfer length data field is in bytes,
// FALSE if it's in sectors.

inline 
PGPBoolean 
IsInBytes(PIOR pIor)
{
	return (pIor->IOR_flags & IORF_CHAR_COMMAND ? TRUE : FALSE);
}

// IsPhysical returns TRUE if the DCB describes a physical drive, FALSE
// otherwise.

inline 
PGPBoolean 
IsPhysical(PDCB pDcb)
{
	return (pDcb->DCB_cmn.DCB_device_flags & DCB_DEV_PHYSICAL ? TRUE : FALSE);
}

// ScatterGather returns TRUE if the IOR uses scatter/gather blocks in its
// request.

inline 
PGPBoolean 
IsScatterGather(PIOR pIor)
{
	return (pIor->IOR_flags & IORF_SCATTER_GATHER ? TRUE : FALSE);
}

#endif	// ] Included_Wrappers_h
