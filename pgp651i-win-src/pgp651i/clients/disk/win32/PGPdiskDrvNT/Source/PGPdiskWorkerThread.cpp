//////////////////////////////////////////////////////////////////////////////
// PGPdiskWorkerThread.cpp
//
// Implementation of class PGPdiskWorkerThread.
//////////////////////////////////////////////////////////////////////////////

// $Id: PGPdiskWorkerThread.cpp,v 1.5 1999/03/31 23:51:08 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#define	__w64
#include <vdw.h>

#include "Required.h"
#include "UtilityFunctions.h"

#include "Globals.h"
#include "KernelModeUtils.h"
#include "PGPdisk.h"
#include "PGPdiskWorkerThread.h"


////////////////////////////////////////////////////
// Class PGPdiskWorkerThread public member functions
////////////////////////////////////////////////////

// The PGPdiskWorkerThread default constructor.

PGPdiskWorkerThread::PGPdiskWorkerThread() 
	: KSystemThread(), mIoSem(0, kInfiniteKernelSemLimit)
{
	mExitThread = FALSE;
	mThreadRunning = FALSE;

	mInitErr = mRequestQueue.mInitErr;

	if (mInitErr.IsntError())
	{
		mInitErr = mIoSem.mInitErr;
	}
}

// The PGPdiskWorkerThread destructor.

PGPdiskWorkerThread::~PGPdiskWorkerThread()
{
	// Kill the thread.
	if (mThreadRunning)
		KillThread();
}

// IsThreadRunning returns TRUE if the thread is currently running, FALSE
// otherwise.

PGPBoolean 
PGPdiskWorkerThread::IsThreadRunning()
{
	return mThreadRunning;
}

// GetRequestQueue returns a pointer to the request queue for the thread.

PGPdiskRequestQueue * 
PGPdiskWorkerThread::GetRequestQueue()
{
	return &mRequestQueue;
}

// StartThread starts the thread.

DualErr 
PGPdiskWorkerThread::StartThread()
{
	DualErr		derr;
	NTSTATUS	status;

	mExitThread = FALSE;

	status = Start(LinkTo(ProcessIo), this);

	if (!NT_SUCCESS(status))
		derr = DualErr(kPGDMinorError_CreateSystemThreadFailed, status);

	mThreadRunning = derr.IsntError();

	return derr;
}

// KillThread kills the thread.

void 
PGPdiskWorkerThread::KillThread()
{
	pgpAssert(IsThreadRunning());

	// Ask thread to terminate.
	mExitThread = TRUE;
	mIoSem.Signal();

	// Wait for thread termination.
	Wait();

	mThreadRunning = FALSE;
}

// QueueIrpForProcessing queues an IRP for processing.

DualErr 
PGPdiskWorkerThread::QueueIrpForProcessing(PIRP pIrp)
{
	DualErr derr;
	KIrp I(pIrp);

	pgpAssert(IsThreadRunning());

	// Fire off the request.
	if (derr.IsntError())
	{
		derr = mRequestQueue.QueueIrpForProcessing(pIrp);
	}

	// Signal the thread that there is work waiting.
	if (derr.IsntError())
	{
		mIoSem.Signal();
	}

	return derr;
}

// ScheduleAsyncCallback schedules a callback to occur in the context of the
// thread, and return immediately.

DualErr 
PGPdiskWorkerThread::ScheduleAsyncCallback(
	PEZCALLBACK	callback, 
	PGPUInt32	refData)
{
	DualErr derr;

	pgpAssertAddrValid(callback, VoidAlign);
	pgpAssert(IsThreadRunning());

	// Fire off the request.
	if (derr.IsntError())
	{
		derr = mRequestQueue.QueueAsyncCallback(callback, refData);
	}

	// Signal the thread that there is work waiting.
	if (derr.IsntError())
	{
		mIoSem.Signal();
	}

	return derr;
}

// ScheduleSyncCallback schedules a callback to occur in the context of the
// thread, and doesn't return until the callback is complete.

DualErr 
PGPdiskWorkerThread::ScheduleSyncCallback(
	PEZCALLBACK	callback, 
	PGPUInt32	refData)
{
	DualErr	derr;
	KEVENT	event;

	pgpAssertAddrValid(callback, VoidAlign);
	pgpAssert(IsThreadRunning());

	if (derr.IsntError())
	{
		// Initialize the event.
		KeInitializeEvent(&event, NotificationEvent, FALSE);

		// Fire off the request.
		derr = mRequestQueue.QueueSyncCallback(callback, refData, &event);
	}

	if (derr.IsntError())
	{
		// Signal the thread that there is work waiting.
		mIoSem.Signal();

		// Wait for completion.
		KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
	}

	return derr;
}


/////////////////////////////////////////////////////
// Class PGPdiskWorkerThread private member functions
/////////////////////////////////////////////////////

// OpenAndCloseToPreventHang quickly opens and closes the specified PGPdisk
// to prevent a hang under NT. Why this works is a mystery.

void 
PGPdiskWorkerThread::OpenAndCloseToPreventHang(PGPdisk *pPGD)
{
	File diskFile;

	diskFile.Open(pPGD->GetPath(), kOF_MustExist | kOF_ReadOnly);

	if (diskFile.Opened())
		diskFile.Close();
}

// GetPGPdiskFromIrp extracts the address of the PGPdisk object that the
// given IRP is directed to.

PGPdisk * 
PGPdiskWorkerThread::GetPGPdiskFromIrp(KIrp I)
{
	PDEVICE_OBJECT	pDevObj;
	PGPdisk			*pPGD;

	pDevObj = I.DeviceObject();
	pgpAssertAddrValid(pDevObj, DEVICE_OBJECT);

	pPGD = (PGPdisk *) pDevObj->DeviceExtension;
	pgpAssertAddrValid(pPGD, PGPdisk);

	return pPGD;
}

// ProcessIrpMjRead processes an IRP_MJ_READ request.

NTSTATUS 
PGPdiskWorkerThread::ProcessIrpMjRead(KIrp I)
{
	DualErr		derr;
	NTSTATUS	status	= STATUS_SUCCESS;
	PGPdisk		*pPGD;
	PGPUInt8	*buf;

	pgpAssert(I.MajorFunction() == IRP_MJ_READ);

	// Locate the corresponding PGPdisk object.
	pPGD = GetPGPdiskFromIrp(I);

	// Extract the buffer.
	buf = (PGPUInt8 *) KMemory(I.Mdl()).MapToSystemSpace();
	pgpAssertAddrValid(buf, PGPUInt8);

	// Perform the read.
	derr = pPGD->Read(buf, I.ReadOffset().QuadPart, I.ReadSize());

	if (derr.IsError())
		status = STATUS_DEVICE_DATA_ERROR;

	if (derr.IsntError())
	{
		I.Information() = I.ReadSize();
	}
	else
	{
		I.Information() = 0;
	}

	if (derr.IsError())
	{
		Interface->ReportError(kPGDMajorError_PGPdiskReadError, derr, 
			pPGD->GetDrive());
	}

	return status;
}

// ProcessIrpMjWrite processes an IRP_MJ_WRITE request.

NTSTATUS 
PGPdiskWorkerThread::ProcessIrpMjWrite(KIrp I)
{
	DualErr		derr;
	NTSTATUS	status	= STATUS_SUCCESS;
	PGPdisk		*pPGD;
	PGPUInt8	*buf;

	pgpAssert(I.MajorFunction() == IRP_MJ_WRITE);

	// Locate the corresponding PGPdisk object.
	pPGD = GetPGPdiskFromIrp(I);

	if (pPGD->IsReadOnly())
		return STATUS_MEDIA_WRITE_PROTECTED;

	// Prevent hang.
	OpenAndCloseToPreventHang(pPGD);

	// Extract the buffer.
	buf = (PGPUInt8 *) KMemory(I.Mdl()).MapToSystemSpace();
	pgpAssertAddrValid(buf, PGPUInt8);

	// Perform the write.
	derr = pPGD->Write(buf, I.WriteOffset().QuadPart, I.WriteSize());

	if (derr.IsError())
		status = STATUS_DEVICE_DATA_ERROR;
	
	if (derr.IsntError())
	{
		I.Information() = I.WriteSize();
	}
	else
	{
		I.Information() = 0;
	}

	if (derr.IsError())
	{
		Interface->ReportError(kPGDMajorError_PGPdiskWriteError, derr, 
			pPGD->GetDrive());
	}

	return status;
}

// ProcessIrpMjCleanup processes an IRP_MJ_CLEANUP request.

NTSTATUS 
PGPdiskWorkerThread::ProcessIrpMjCleanup(KIrp I)
{
	DualErr		derr;
	NTSTATUS	status	= STATUS_SUCCESS;
	PGPdisk		*pPGD;

	pgpAssert(I.MajorFunction() == IRP_MJ_CLEANUP);

	// Locate the corresponding PGPdisk object.
	pPGD = GetPGPdiskFromIrp(I);

	// Cancel all IRPs for the PGPdisk.
	mRequestQueue.CancelIRPs(pPGD->GetDrive());
	status = STATUS_SUCCESS;

	if (derr.IsError())
	{
		Interface->ReportError(kPGDMajorError_PGPdiskIrpCleanupError, derr, 
			pPGD->GetDrive());
	}

	I.Information() = 0;

	return status;
}

// ProcessIrpMjDeviceControl processes an IRP_MJ_DEVICE_CONTROL request.

NTSTATUS 
PGPdiskWorkerThread::ProcessIrpMjDeviceControl(KIrp I)
{
	DualErr					derr;
	NTSTATUS				status	= STATUS_SUCCESS;
	PDISK_GEOMETRY			pGeom;
	PPARTITION_INFORMATION	pPartInfo;
	PGPdisk					*pPGD;

	pgpAssert(I.MajorFunction() == IRP_MJ_DEVICE_CONTROL);

	// Locate the corresponding PGPdisk object.
	pPGD = GetPGPdiskFromIrp(I);

	DebugOut("PGPdisk: Processing an %s\n", 
		GetIOCTLFunctionName(I.IoctlCode()));

	// Perform the IOCTL.
	if (derr.IsntError())
	{
		switch (I.IoctlCode())
		{
		case IOCTL_DISK_CHECK_VERIFY:
		case IOCTL_DISK_SET_PARTITION_INFO: 
		case IOCTL_DISK_VERIFY:

			I.Information() = 0;
			status = STATUS_SUCCESS;
			break;

		case IOCTL_DISK_IS_WRITABLE:

			I.Information() = 0;

			if (pPGD->IsReadOnly())
				status = STATUS_MEDIA_WRITE_PROTECTED;
			else
				status = STATUS_SUCCESS;
			break;
		
		case IOCTL_DISK_GET_DRIVE_GEOMETRY:
		case IOCTL_DISK_GET_MEDIA_TYPES:

			// Return drive geometry.
			if (I.IoctlOutputBufferSize() < sizeof(DISK_GEOMETRY))
			{
				status = STATUS_INVALID_PARAMETER;
				derr = DualErr(kPGDMinorError_InvalidParameter);
			}

			if (derr.IsntError())
			{
				pGeom = (PDISK_GEOMETRY) I.IoctlBuffer();
				pPGD->GetGeometry(pGeom);
			}

			I.Information() = sizeof(DISK_GEOMETRY);
			status = STATUS_SUCCESS;
			break;

		case IOCTL_DISK_GET_PARTITION_INFO: 

			// Return information about the partition.
			if (I.IoctlOutputBufferSize() < sizeof(PARTITION_INFORMATION))
			{
				status = STATUS_INVALID_PARAMETER;
				derr = DualErr(kPGDMinorError_InvalidParameter);
			}

			if (derr.IsntError())
			{
				pPartInfo = (PPARTITION_INFORMATION) I.IoctlBuffer();
				pPGD->GetPartitionInfo(pPartInfo);
			}

			I.Information() = sizeof(PARTITION_INFORMATION);
			status = STATUS_SUCCESS;
			break;

		default:

			I.Information() = 0;
			status = STATUS_INVALID_DEVICE_REQUEST;
			break;
		}
	}

	return status;
}

// ProcessIrp processes a kPRQOp_ProcessIrp request.

void 
PGPdiskWorkerThread::ProcessIrp(
	PRQRequest	*pPRQ, 
	PGPBoolean	*irpNeedsCompletion)
{
	KIrp		I;
	NTSTATUS	status			= STATUS_SUCCESS;
	PGPBoolean	completedIrp	= FALSE;
	PGPUInt8	majorFunc;

	pgpAssertAddrValid(pPRQ, PRQRequest);
	pgpAssertAddrValid(pPRQ->pIrp, PIRP);
	pgpAssertAddrValid(irpNeedsCompletion, PGPBoolean);

	( * irpNeedsCompletion) = TRUE;

	I = KIrp(pPRQ->pIrp);
	majorFunc = I.MajorFunction();

	DebugOut("PGPdisk: Processing an %s\n", 
		GetIRPMajorFunctionName(majorFunc));

	// Process the IRP according to function.
	switch (majorFunc)
	{

	case IRP_MJ_CLEANUP:
		status = ProcessIrpMjCleanup(pPRQ->pIrp);
		break;

	case IRP_MJ_DEVICE_CONTROL:
		status = ProcessIrpMjDeviceControl(pPRQ->pIrp);
		break;

	case IRP_MJ_READ:
 		status = ProcessIrpMjRead(pPRQ->pIrp);
		break;

	case IRP_MJ_WRITE:
		status = ProcessIrpMjWrite(pPRQ->pIrp);
		break;

	default:
		// All others should have been handled already in the dispatch
		// filter, even IRPs we don't know about.

		pgpAssert(FALSE);

		I.Information() = 0;
		status = STATUS_NOT_IMPLEMENTED;
		break;
	}

	I.Complete(status);
	( * irpNeedsCompletion) = FALSE;

	DebugOut("PGPdisk: Completed an %s\n", 
		GetIRPMajorFunctionName(majorFunc));
}

// ProcessAsyncCallback processes a kPRQOp_AsyncCallback request.

void 
PGPdiskWorkerThread::ProcessAsyncCallback(PRQRequest *pPRQ)
{
	DualErr derr;

	pgpAssertAddrValid(pPRQ, PRQRequest);
	pgpAssertAddrValid(pPRQ->callback, VoidAlign);

	// Perform the callback.
	pPRQ->callback(pPRQ->refData);
}

// ProcessSyncCallback processes a kPRQOp_SyncCallback request.

void 
PGPdiskWorkerThread::ProcessSyncCallback(PRQRequest *pPRQ)
{
	DualErr derr;

	pgpAssertAddrValid(pPRQ, PRQRequest);

	pgpAssertAddrValid(pPRQ->callback, VoidAlign);
	pgpAssertAddrValid(pPRQ->pSyncEvent, KEVENT);

	// Perform the callback.
	pPRQ->callback(pPRQ->refData);

	// Tell the caller we're done.
	KeSetEvent(pPRQ->pSyncEvent, 0, FALSE);
}

// ProcessIo is the main loop of the worker thread.

void 
PGPdiskWorkerThread::ProcessIo()
{
	KeSetPriorityThread(KeGetCurrentThread(), LOW_REALTIME_PRIORITY);

	while (TRUE)
	{
		KIrp		I;
		PGPBoolean	ifIrpItNeedsCompletion	= TRUE;
		PRQRequest	*pPRQ;

		__try 
		{
			// Wait for input.
			mIoSem.Wait();

			// Are we unloading?
			if (mExitThread)
				break;

			// Get next request.
			pPRQ = mRequestQueue.Pop();

			if (IsNull(pPRQ))
				continue;

			pgpAssertAddrValid(pPRQ, PRQRequest);

			// Dispatch the request.
			switch (pPRQ->op)
			{
			case kPRQOp_ProcessIrp:
				ProcessIrp(pPRQ, &ifIrpItNeedsCompletion);
				break;

			case kPRQOp_AsyncCallback:
				ProcessAsyncCallback(pPRQ);
				break;

			case kPRQOp_SyncCallback:
				ProcessSyncCallback(pPRQ);
				break;

			default:
				pgpAssert(FALSE);
				break;
			}

			// Return the PRQ to the memory pool.
			mRequestQueue.ReturnPRQ(pPRQ);
		} 
		__except(EXCEPTION_EXECUTE_HANDLER) 
		{
			Interface->ReportError(kPGDMajorError_KModeExceptionOccured);

			// We don't exit with uncompleted IRPs.
			if (ifIrpItNeedsCompletion && (pPRQ->op == kPRQOp_ProcessIrp))
			{
				I = KIrp(pPRQ->pIrp);
				I.Complete(STATUS_DEVICE_DATA_ERROR);
			}
		}
	}

	// Terminated.
	Terminate(STATUS_SUCCESS);
}
