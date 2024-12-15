//////////////////////////////////////////////////////////////////////////////
// IopProcessor.cpp
//
// Implementation of class IopProcessor.
//////////////////////////////////////////////////////////////////////////////

// $Id: IopProcessor.cpp,v 1.1.2.17.2.1 1998/09/01 00:31:51 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include <vtoolscp.h>

#include "Required.h"

#include "CPGPdiskDrv.h"
#include "Globals.h"
#include "IopProcessor.h"
#include "PGPdisk.h"

#include LOCKED_CODE_SEGMENT
#include LOCKED_DATA_SEGMENT


/////////////////////////////////////////////
// Class IopProcessor public member functions
/////////////////////////////////////////////

// The IopProcessor constructor.

IopProcessor::IopProcessor()
{
	mUseAsyncIO = kDefaultUseAsyncIO;

	for (PGPUInt32 i=0; i<kMaxDrives; i++)
	{
		mIopReqs[i].isBusy = FALSE;
	}
}

// IsDriveBusy returns TRUE if the PGPdisk represented by the given drive
// number is busy processing I/O.

PGPBoolean 
IopProcessor::IsDriveBusy(PGPUInt8 drive)
{
	pgpAssert(IsLegalDriveNumber(drive));

	return mIopReqs[drive].isBusy;
}

// SetDriveBusy marks the given drive as busy or not busy.

void 
IopProcessor::SetDriveBusy(PGPUInt8 drive, PGPBoolean isBusy)
{
	pgpAssert(IsLegalDriveNumber(drive));

	mIopReqs[drive].isBusy = isBusy;
}

// IsIOAsync returns TRUE if I/O is currently being processed asynchronously,
// FALSE otherwise.

PGPBoolean 
IopProcessor::IsIOAsync()
{
	return mUseAsyncIO;
}

// SetIOAsync specifies that I/O be processed either async or sync.

void 
IopProcessor::SetIOAsync(PGPBoolean isIOAsync)
{
	mUseAsyncIO = isIOAsync;
}

// ProcessIop schedules later processing, at a safe time, of the given IOP
// request. Validate the IOP and fail silently if validation fails.

void 
IopProcessor::ProcessIop(PIOP pIop)
{
	PDCB		pDcb;
	PGPBoolean	failThisRequest	= FALSE;
	PGPUInt8	drive;
	PGPUInt16	failStatus;
	PIOR		pIor;

	pgpAssertAddrValid(pIop, IOP);

	pIor = &pIop->IOP_ior;
	pgpAssertAddrValid(pIor, IOR);

	DebugOut("PGPdisk: Saw an %s\n", 
		Driver->GetIORFunctionName(pIor->IOR_func));

	// Store a pointer to this object in the IOP itself.
	pIor->IOR_private_port = (PGPUInt32) this;

	switch (pIor->IOR_func)
	{
	case IOR_GEN_IOCTL:
		// NEVER SCHEDULE IOCTL REQUESTS. ALWAYS PROCESS IN SAME CONTEXT IN
		// WHICH THEY ARE RECEIVED.

		IorGenIoctlDispatcher(pIop);
		break;

	case IOR_READ:
	case IOR_WRITE:
		// Queue the request.
		pDcb = (PDCB) pIop->IOP_physical_dcb;
		pgpAssertAddrValid(pDcb, DCB);

		// Validate the drive letter.
		drive = pDcb->DCB_cmn.DCB_drive_lttr_equiv;

		if (!IsLegalDriveNumber(drive) || 
			IsNull(Driver->mPGPdisks.FindPGPdisk(drive)))
		{
			failThisRequest = TRUE;
			failStatus = IORS_NO_DEVICE;
		}

		if (!failThisRequest)
		{
			IlbEnqueueIop(pIop, pDcb);

			// We must determine whether or not to schedule dispatching.
			// If I/O is in process, we don't schedule another callback - 
			// this will be done later on when that I/O is finished.
			// Otherwise we schedule a callback right now.

			if (!IsDriveBusy(drive))
			{
				SetDriveBusy(drive, TRUE);
				ScheduleIorReadWriteDispatch(drive);
			}
		}
		break;

	default:
		failThisRequest = TRUE;
		failStatus = IORS_INVALID_COMMAND;
		break;
	}

	if (failThisRequest)
	{
		pIor->IOR_status = failStatus;
		--pIop->IOP_callback_ptr;	
		pIop->IOP_callback_ptr->IOP_CB_address(pIop);
	}
}


//////////////////////////////////////////////
// Class IopProcessor private member functions
//////////////////////////////////////////////

// ScheduleIorReadWriteDispatch asks the system to schedule our IOR_READ and
// IOR_WRITE dispatching function for execution at a safer time.

void 
IopProcessor::ScheduleIorReadWriteDispatch(PGPUInt8 drive)
{
	static RestrictedEvent_THUNK iopThunk;

	pgpAssert(IsLegalDriveNumber(drive));

	Call_Restricted_Event(0, NULL, PEF_ALWAYS_SCHED, (PVOID) drive, 
		IorReadWriteDispatcher, 0, &iopThunk);
}

// IorGenIoctlDispatcher is called in order to dispatch IOR_GEN_IOCTL
// requests.

void 
IopProcessor::IorGenIoctlDispatcher(PIOP pIop)
{
	PIOR pIor;

	pgpAssertAddrValid(pIop, IOP);

	pIor = &pIop->IOP_ior;
	pgpAssertAddrValid(pIor, IOR);

	DebugOut("PGPdisk: %s initiated\n", 
		Driver->GetIORFunctionName(pIor->IOR_func));

	// Switch on the IOR function.
	switch (pIor->IOR_func)
	{
	case IOR_GEN_IOCTL:			// an ioctl request
		Driver->mIopProcessor.ProcessIorGenIoctl(pIop);
		break;

	default:					// otherwise fail
		pgpAssert(FALSE);
		break;
	}
}

// IorReadWriteDispatcher is the event callback that is scheduled by the
// I/O request routine in order to dispatch IOR_READ and IOR_WRITE requests.

VOID 
__stdcall 
IopProcessor::IorReadWriteDispatcher(
	VMHANDLE		hVM, 
	THREADHANDLE	hThread, 
	PVOID			Refdata, 
	PCLIENT_STRUCT	pRegs)
{
	PDCB		pDcb;
	PGPBoolean	foundIop	= FALSE;
	PGPdisk		*pPGD;
	PGPUInt8	drive;
	PIOP		pIop;

	drive = (PGPUInt8) Refdata;
	pgpAssert(IsLegalDriveNumber(drive));

	pPGD = Driver->mPGPdisks.FindPGPdisk(drive);
	pgpAssertAddrValid(pPGD, PGPdisk);

	pDcb = pPGD->GetDcb();
	pgpAssertAddrValid(pDcb, DCB);

	// Any outstanding I/O for this PGPdisk?
	if (pIop = IlbDequeueIop(pDcb))
	{
		PIOR pIor;

		pgpAssertAddrValid(pIop, IOP);

		pIor = &pIop->IOP_ior;
		pgpAssertAddrValid(pIor, IOR);

		DebugOut("PGPdisk: %s initiated\n", 
			Driver->GetIORFunctionName(pIor->IOR_func));

		// Switch on the IOR function.
		switch (pIor->IOR_func)
		{
		case IOR_READ:									// a read request
		case IOR_WRITE:									// a write request
			Driver->mIopProcessor.ProcessIorReadWrite(pIop, 
				Driver->mIopProcessor.mUseAsyncIO);
			break;

		default:										// otherwise fail
			pgpAssert(FALSE);
			break;
		}
	}
	else
	{
		// Allow I/O to the PGPdisk.
		Driver->mIopProcessor.SetDriveBusy(drive, FALSE);
	}
}

// ProcessIorGenIoctl processes an IOR_GEN_IOCTL request synchronously. We
// need to process these so formatting will work on PGPdisks.

void 
IopProcessor::ProcessIorGenIoctl(PIOP pIop)
{
	DualErr		derr;
	PGPUInt8	drive;
	PGPUInt16	iorsResult;
	PGPUInt32	ioctlResult;
	PIOR		pIor;
	PGPdisk		*pPGD;

	pgpAssertAddrValid(pIop, IOP);

	pIor = &pIop->IOP_ior;
	pgpAssertAddrValid(pIor, IOR);
	
	drive = pIor->IOR_ioctl_drive - 1;

	// Check for invalid drive.
	if ((drive == 0) || !IsLegalDriveNumber(drive) || 
		IsNull(Driver->mPGPdisks.FindPGPdisk(drive)))
	{
		ioctlResult = ERROR_INVALID_DRIVE;
		iorsResult = IORS_INVALID_COMMAND;
	}
	else
	{
		pPGD = Driver->mPGPdisks.FindPGPdisk(drive);
		pgpAssertAddrValid(pPGD, PGPdisk);

		DebugOut("PGPdisk: Ioctl - drive=%c function=%x minor %x\n", 
			DriveNumToLet(drive), pIor->IOR_ioctl_function, 
			pIor->IOR_ioctl_control_param & 0xff);

		// Now process the IOCTL request. We only process some 440D functions
		// and the 4411 function.

		switch (pIor->IOR_ioctl_function)
		{

		case 0x440D:
			switch (GetLowByte((PGPUInt16) pIor->IOR_ioctl_control_param))
			{

			case 0x40:			// Set Device Parameters
			{
				// We don't actually have to 'do' anything here. The system
				// will set these parameters when it writes the boot block,
				// which is where we get them from anyways.

				ioctlResult = ERROR_SUCCESS;
				iorsResult = IORS_SUCCESS;
				break;
			}

			case 0x41:			// Write Track on Logical Drive
			{
				PGPUInt8	*buf;
				PGPUInt16	offset, segment;
				PGPUInt32	nBlocks;
				PGPUInt64	pos;
				RwBlock		*pRWB;
		
				pRWB = (RwBlock *) pIor->IOR_ioctl_buffer_ptr;

				// Calculate the starting sector.
				pos	= pRWB->rwCylinder * kSizeCyl;
				pos	+= pRWB->rwHead * kSizeHead;
				pos	+= pRWB->rwFirstSector;

				// Calculate the number of blocks to read.
				nBlocks	= pRWB->rwSectors;

				// Calculate the buffer address.
				segment = GetHighWord(pRWB->rwBuffer);
				offset = GetLowWord(pRWB->rwBuffer);

				buf = (PGPUInt8 *) Get_Cur_VM_Handle()->CB_High_Linear +
					(segment << 4) + offset;

				// Perform the write.
				derr = pPGD->Read(buf, pos*kDefaultBlockSize, 
					nBlocks*kDefaultBlockSize);

				ioctlResult = (derr.IsntError() ? ERROR_SUCCESS :
					ERROR_INVALID_FUNCTION);
				iorsResult = IORS_SUCCESS;
				break;
			}

			case 0x42:			// Format Track on Logical Drive
			{
				FvBlock *pFVB;
			
				// Don't actually do anything, since PGPdisks aren't physical
				// drives that need any special treatment of sectors before
				// being written to.

				pFVB = (FvBlock *) pIor->IOR_ioctl_buffer_ptr;

				if (pFVB->fvSpecFunc == 1)
				{
					pFVB->fvSpecFunc = 0;
				}

				ioctlResult = ERROR_SUCCESS;
				iorsResult = IORS_SUCCESS;
				break;
			}

			case 0x60:			// Get Device Parameters
			{
				switch (
					GetHighByte((PGPUInt16) pIor->IOR_ioctl_control_param))
				{
				case 0x08:
					derr = pPGD->GetDevParams16(
						(DevParams16 *) pIor->IOR_ioctl_buffer_ptr);
					break;

				case 0x48:
					derr = pPGD->GetDevParams32(
						(DevParams32 *) pIor->IOR_ioctl_buffer_ptr);
					break;
				}

				ioctlResult = (derr.IsntError() ? ERROR_SUCCESS :
					ERROR_INVALID_FUNCTION);
				iorsResult = IORS_SUCCESS;
				break;
			}

			case 0x61:			// Read Track on Logical Drive
			{
				PGPUInt8	*buf;
				PGPUInt16	offset, segment;
				PGPUInt32	nBlocks;
				PGPUInt64	pos;
				RwBlock		*pRWB;
		
				pRWB = (RwBlock *) pIor->IOR_ioctl_buffer_ptr;

				// Calculate the starting sector.
				pos	= pRWB->rwCylinder * kSizeCyl;
				pos	+= pRWB->rwHead * kSizeHead;
				pos	+= pRWB->rwFirstSector;

				// Calculate the number of blocks to read.
				nBlocks	= pRWB->rwSectors;

				// Calculate the buffer address.
				segment = GetHighWord(pRWB->rwBuffer);
				offset = GetLowWord(pRWB->rwBuffer);

				buf = (PGPUInt8 *) Get_Cur_VM_Handle()->CB_High_Linear +
					(segment << 4) + offset;

				// Perform the read.
				derr = pPGD->Read(buf, pos*kDefaultBlockSize, 
					nBlocks*kDefaultBlockSize);

				ioctlResult = (derr.IsntError() ? ERROR_SUCCESS :
					ERROR_INVALID_FUNCTION);
				iorsResult = IORS_SUCCESS;
				break;
			}

			case 0x66:			// Get Media ID
			{	
				derr = pPGD->GetMediaId((MID *) pIor->IOR_ioctl_buffer_ptr);

				ioctlResult = (derr.IsntError() ? ERROR_SUCCESS :
					ERROR_INVALID_FUNCTION);
				iorsResult = IORS_SUCCESS;
				break;
			}

			default:
				ioctlResult = ERROR_INVALID_FUNCTION;
				iorsResult = IORS_INVALID_COMMAND;
				break;
			}
			break;

		case 0x4411:		// Query if we handle specified 440D code
			switch (GetLowByte((PGPUInt16) pIor->IOR_ioctl_control_param))
			{
			case 0x40:
			case 0x41:
			case 0x42:
			case 0x60:
			case 0x61:
			case 0x66:
				ioctlResult = ERROR_SUCCESS;
				iorsResult = IORS_SUCCESS;
				break;

			default:
				ioctlResult = ERROR_INVALID_FUNCTION;
				iorsResult = IORS_SUCCESS;
				break;
			}
			break;

		default:
			ioctlResult = ERROR_INVALID_FUNCTION;
			iorsResult = IORS_INVALID_COMMAND;
			break;
		}
	}

	pIor->IOR_ioctl_return = ioctlResult;
	ScheduleAsyncCallback(pIop, iorsResult);
}

// ProcessIorReadWrite processes an IOR_READ or an IOR_WRITE request
// asynchronously.

void 
IopProcessor::ProcessIorReadWrite(PIOP pIop, PGPBoolean useAsync)
{
	DualErr		derr;
	PDCB		pDcb;
	PGPBoolean	doneWithRequest;
	PGPdisk		*pPGD;
	PGPUInt8	drive, *buf;
	PGPUInt16	blockSize, failSilentStatus;
	PIOR		pIor;

	pgpAssertAddrValid(pIop, IOP);

	pIor = &pIop->IOP_ior;
	pgpAssertAddrValid(pIor, IOR);
	
	pDcb = (PDCB) pIop->IOP_physical_dcb;
	pgpAssertAddrValid(pDcb, DCB);

	drive = pDcb->DCB_cmn.DCB_drive_lttr_equiv;
	pgpAssert(IsLegalDriveNumber(drive));
	
	pPGD = Driver->mPGPdisks.FindPGPdisk(drive);
	pgpAssertAddrValid(pPGD, PGPdisk);

	blockSize = ExtractBlockSize(pDcb);

	pgpAssert(IsDriveBusy(drive));

	// Reset the inactivity timer.
	Driver->mSecondsInactive = 0;

	// Verify that IO is enabled for this PGPdisk.
	if (!pPGD->IsIOEnabled())
	{
		derr = kPGDMinorError_FailSilently;
		failSilentStatus = IORS_NOT_READY;
	}

	// If this is a write, verify that the volume isn't write-protected.
	if (derr.IsntError())
	{
		if (pIor->IOR_func == IOR_WRITE)
		{
			if (pPGD->IsReadOnly())
			{
				derr = kPGDMinorError_FailSilently;
				failSilentStatus = IORS_WRITE_PROTECT;
			}
		}
	}

	// Prepare the request for execution.
	if (derr.IsntError())
	{
		PGPUInt32	i, nBytes;
		PGPUInt64	pos;

		mIopReqs[drive].useAsync	= useAsync;
		mIopReqs[drive].pIop		= pIop;
		mIopReqs[drive].pIor		= pIor;

		mIopReqs[drive].downInfo.callback	= IorReadWriteCallback;
		mIopReqs[drive].downInfo.refData[0]	= (PGPUInt32) this;
		mIopReqs[drive].downInfo.refData[1]	= drive;

		// Mark all the mini-request stuctures as unused.
		for (i=0; i<kIopProcMaxMinis; i++)
		{
			mIopReqs[drive].miniReqs[i].isInUse = FALSE;
		}

		// Calculate the byte offset on the volume where the request begins.
		pos = pIor->IOR_start_addr[0] * blockSize;

		// Initialize the list of miniReqs that describe the I/O transfers that
		// make up this request.

		if (!IsScatterGather(pIor))		// Just one 'piece'
		{
			nBytes = pIor->IOR_xfer_count;

			// Translate to bytes
			if (!IsInBytes(pIor))
			{
				nBytes *= blockSize;
			}
			
			buf = (PGPUInt8 *) pIor->IOR_buffer_ptr;
			pgpAssertAddrValid(buf, PGPUInt8);

			// Just one minireq at position 0 for this request.
			mIopReqs[drive].miniReqs[0].isInUse	= TRUE;
			mIopReqs[drive].miniReqs[0].buf		= buf;
			mIopReqs[drive].miniReqs[0].pos		= pos;
			mIopReqs[drive].miniReqs[0].nBytes	= nBytes;
		}
		else							// More than one 'piece'
		{
			PGPUInt32	curMiniReq;
			PSGD		pSgd;

			// We are dealing with scatter/gather requests. The pointer to
			// the first S/G structure will be in IOR_buffer_ptr. Each SGD
			// describes a number of bytes to transfer and provides a buffer.

			pSgd = (PSGD) pIor->IOR_buffer_ptr;
			pgpAssertAddrValid(pSgd, SGD);

			curMiniReq = 0;

			// Initialize a separate mini-request structure for each SGD.
			while (derr.IsntError && (nBytes = pSgd->bd.SG_buff_size))
			{
				if (curMiniReq >= kIopProcMaxMinis)
				{
					derr = DualErr(kPGDMinorError_MiniRequestOverflow);
				}

				if (derr.IsntError())
				{
					// Translate to bytes.
					if (!IsInBytes(pIor))
					{
						nBytes *= blockSize;
					}

					buf = (PGPUInt8 *) pSgd->bd.SG_buff_ptr;
					pgpAssertAddrValid(buf, PGPUInt8);

					mIopReqs[drive].miniReqs[curMiniReq].isInUse	= TRUE;
					mIopReqs[drive].miniReqs[curMiniReq].buf		= buf;
					mIopReqs[drive].miniReqs[curMiniReq].pos		= pos;
					mIopReqs[drive].miniReqs[curMiniReq].nBytes		= nBytes;

					curMiniReq++;

					pos += nBytes;		// where the next read will begin
					pSgd++;				// move on to the next S/G structure
				}
			}
		}
	}

	// Now begin executing the request.
	if (derr.IsntError())
	{
		// ExecuteRequest won't fail on async.
		if (useAsync)
			ExecuteRequest(drive, &doneWithRequest);
		else
			derr = ExecuteRequest(drive, &doneWithRequest);
	}

	// If we are sync, then the request has been completed. If not, then
	// doneWithRequest shouldn't be TRUE assuming we were passed valid params.

	if (derr.IsntError())
	{
		if (doneWithRequest)
		{
			pgpAssert(!useAsync);
			ScheduleAsyncCallback(pIop, IORS_SUCCESS);
		}
	}

	// Call back errors now.
	if (derr.IsError())
	{
		switch (derr.mMinorError)
		{
		case kPGDMinorError_IopRequestOverflow:
		case kPGDMinorError_MiniRequestOverflow:
			ScheduleAsyncCallback(pIop, IORS_BUSY);
			break;

		case kPGDMinorError_FailSilently:
			ScheduleAsyncCallback(pIop, failSilentStatus);
			break;

		default:
			if (IsntNull(pPGD))
			{
				pPGD->EnableIO(FALSE);
				ScheduleAsyncCallback(pIop, IORS_MEDIA_ERROR);

				// For I/O errors, user will be asked for options.
				if (pIor->IOR_func == IOR_READ)
				{
					Driver->ReportError(kPGDMajorError_PGPdiskReadError, 
						derr, pPGD->GetDrive());
				}
				else
				{
					Driver->ReportError(kPGDMajorError_PGPdiskWriteError, 
						derr, pPGD->GetDrive());
				}
			}
			else
			{
				ScheduleAsyncCallback(pIop, IORS_NO_DEVICE);
			}
			break;
		}
	}
}

// ExecuteRequest executes the next 'piece' of the request being processed on
// the given drive. If there nothing left to execute, 'doneWithRequest' is
// set to TRUE, otherwise FALSE.

DualErr 
IopProcessor::ExecuteRequest(PGPUInt8 drive, PGPBoolean *doneWithRequest)
{
	DualErr		derr;
	PGPBoolean	foundMiniReq	= FALSE;
	PGPBoolean	useAsync		= FALSE;
	PGPUInt32	i;
	PDCB		pDcb;
	PGPdisk		*pPGD;
	PIOP		pIop;
	PIOR		pIor;

	pgpAssertAddrValid(doneWithRequest, PGPBoolean);
	(* doneWithRequest) = FALSE;

	pgpAssert(IsLegalDriveNumber(drive));
	pgpAssert(IsDriveBusy(drive));

	pIop = mIopReqs[drive].pIop;
	pgpAssertAddrValid(pIop, IOP);

	pIor = mIopReqs[drive].pIor;
	pgpAssertAddrValid(pIor, IOR);

	pDcb = (PDCB) pIop->IOP_physical_dcb;
	pgpAssertAddrValid(pDcb, DCB);

	pPGD = Driver->mPGPdisks.FindPGPdisk(drive);
	pgpAssertAddrValid(pPGD, PGPdisk);

	useAsync = mIopReqs[drive].useAsync;

	// Look for the next piece we need to process, if any are left.
	for (i=0; i<kIopProcMaxMinis; i++)
	{
		if (mIopReqs[drive].miniReqs[i].isInUse)
		{
			PGPUInt8	*buf;
			PGPUInt32	nBytes;
			PGPUInt64	pos;

			foundMiniReq = TRUE;

			buf		= mIopReqs[drive].miniReqs[i].buf;
			pos		= mIopReqs[drive].miniReqs[i].pos;
			nBytes	= mIopReqs[drive].miniReqs[i].nBytes;
			
			pgpAssertAddrValid(buf, PGPUInt8);

			// Call it down. Don't ask for an error value on async calls since
			// async errors will be returned using the callback, not here.

			switch (pIor->IOR_func)
			{
			case IOR_READ:
				if (useAsync)
					pPGD->Read(buf, pos, nBytes, &mIopReqs[drive].downInfo);
				else
					derr = pPGD->Read(buf, pos, nBytes);
				break;

			case IOR_WRITE:
				if (useAsync)
					pPGD->Write(buf, pos, nBytes, &mIopReqs[drive].downInfo);
				else
					derr = pPGD->Write(buf, pos, nBytes);
				break;
			}

			// If we're async we only process one piece at a time.
			if (useAsync || derr.IsError())
				break;
		}
	}

	if (useAsync)
		(* doneWithRequest) = !foundMiniReq;
	else
		(* doneWithRequest) = TRUE;

	return derr;
}

// IorReadWriteCallback is called as a callback by the routines who executed
// our read and write requests. We extract the address of the PGPdisk object
// in question and pass the callback to it.

void 
IopProcessor::IorReadWriteCallback(GenericCallbackInfo *downInfo)
{
	IopProcessor	*pIP;
	PGPUInt8		drive;

	pgpAssertAddrValid(downInfo, GenericCallbackInfo);

	pIP = (IopProcessor *) downInfo->refData[0];
	pgpAssertAddrValid(pIP, IopProcessor);

	drive = (PGPUInt8) downInfo->refData[1];
	pgpAssert(IsLegalDriveNumber(drive));

	pIP->IorReadWriteCallbackAux(drive);
}

// IorReadWriteCallbackAux is called by the static callback function
// 'IorReadWriteCallback' so we don't have to type 'pIP' before every
// reference to an object member or method.

void 
IopProcessor::IorReadWriteCallbackAux(PGPUInt8 drive)
{
	DualErr		derr;
	PGPBoolean	doneWithRequest;
	PGPUInt32	i;

	pgpAssert(IsLegalDriveNumber(drive));
	pgpAssert(IsDriveBusy(drive));

	derr = mIopReqs[drive].downInfo.derr;

	if (derr.IsntError())
	{
		// Mark the mini-request structure we just completed as unused.
		for (i=0; i<kIopProcMaxMinis; i++)
		{
			if (mIopReqs[drive].miniReqs[i].isInUse)
			{
				mIopReqs[drive].miniReqs[i].isInUse = FALSE;
				break;
			}
		}
	}

	// Perform further processing on this request, if needed.
	if (derr.IsntError())
	{
		derr = ExecuteRequest(drive, &doneWithRequest);
	}

	// Callback if we're done or on error.
	if (doneWithRequest || derr.IsError())
	{	
		PIOP	pIop;
		PIOR	pIor;

		pIop = mIopReqs[drive].pIop;
		pgpAssertAddrValid(pIop, IOP);

		pIor = &pIop->IOP_ior;
		pgpAssertAddrValid(pIor, IOR);

		if (derr.IsntError())
		{
			ScheduleAsyncCallback(pIop, IORS_SUCCESS);
		}
		else
		{
			PGPdisk *pPGD;

			switch (derr.mMinorError)
			{
			case kPGDMinorError_IopRequestOverflow:
			case kPGDMinorError_MiniRequestOverflow:
				ScheduleAsyncCallback(pIop, IORS_BUSY);
				break;

			default:
				pPGD = Driver->mPGPdisks.FindPGPdisk(drive);
				pgpAssertAddrValid(pPGD, PGPdisk);

				// Disable I/O to the PGPdisk.
				pPGD->EnableIO(FALSE);
				ScheduleAsyncCallback(pIop, IORS_MEDIA_ERROR);

				// For I/O errors, user will be asked for options.
				if (pIor->IOR_func == IOR_READ)
				{
					Driver->ReportError(kPGDMajorError_PGPdiskReadError, 
						DualErr::NoError, pPGD->GetDrive());
				}
				else
				{
					Driver->ReportError(kPGDMajorError_PGPdiskWriteError, 
						DualErr::NoError, pPGD->GetDrive());
				}
				break;
			}
		}
	}
}

// ScheduleAsyncCallback schedules a windows event that calls our function
// that will call the asynchronous request up.

void 
IopProcessor::ScheduleAsyncCallback(PIOP pIop, PGPUInt16 status)
{
	PIOR							pIor;
	static RestrictedEvent_THUNK	callbackThunk;

	pgpAssertAddrValid(pIop, IOP);

	pIor = &pIop->IOP_ior;
	pgpAssertAddrValid(pIor, IOR);

	pIor->IOR_status = status;

	Call_Restricted_Event(0, NULL, PEF_ALWAYS_SCHED, (PVOID) pIop, 
		AsyncExecuteCallback, 0, &callbackThunk);
}

// AsyncExecuteCallback was scheduled by 'ScheduleAsyncCallback' for the
// purpose of calling back up the asynchronous request we received.

VOID 
__stdcall 
IopProcessor::AsyncExecuteCallback(
	VMHANDLE		hVM, 
	THREADHANDLE	hThread, 
	PVOID			Refdata, 
	PCLIENT_STRUCT	pRegs)
{
	IopProcessor	*pIP;
	PGPUInt8		drive;
	PIOP			pIop;
	PIOR			pIor;

	pIop = (PIOP) Refdata;
	pgpAssertAddrValid(pIop, IOP);

	pIor = &pIop->IOP_ior;
	pgpAssertAddrValid(pIor, IOR);

	pIP = (IopProcessor *) pIor->IOR_private_port;
	pgpAssertAddrValid(pIP, IopProcessor);

	// If the IOR was a read or write request, we must check if there is more
	// I/O from that PGPdisk to process. We do this by scheduling the IOP
	// dispatch function as a callback.

	PDCB pDcb;

	switch (pIor->IOR_func)
	{
	case IOR_READ:
	case IOR_WRITE:
		pDcb = (PDCB) pIop->IOP_physical_dcb;
		pgpAssertAddrValid(pDcb, DCB);

		drive = pDcb->DCB_cmn.DCB_drive_lttr_equiv;
		pgpAssert(IsLegalDriveNumber(drive));

		--pIop->IOP_callback_ptr;	
		pIop->IOP_callback_ptr->IOP_CB_address(pIop);

		pIP->ScheduleIorReadWriteDispatch(drive);
		break;

	default:
		--pIop->IOP_callback_ptr;	
		pIop->IOP_callback_ptr->IOP_CB_address(pIop);
		break;
	}
}
