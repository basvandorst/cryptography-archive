//////////////////////////////////////////////////////////////////////////////
// PGPdiskRequestQueue.cpp
//
// Definition of class PGPdiskRequestQueue.
//////////////////////////////////////////////////////////////////////////////

// $Id: PGPdiskRequestQueue.cpp,v 1.4 1999/03/31 23:51:08 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#define	__w64
#include <vdw.h>

#include "Required.h"

#include "Globals.h"
#include "PGPdiskRequestQueue.h"
#include "PGPdiskWorkerThread.h"
#include "PGPdisk.h"


////////////
// Constants
////////////

const PGPUInt16 kPGPdiskDefaultPRQDepth = 32;


////////////////////////////////////////////////////
// Class PGPdiskRequestQueue public member functions
////////////////////////////////////////////////////

// The PGPdiskRequestQueue default constructor.

PGPdiskRequestQueue::PGPdiskRequestQueue() 
	: mPRQAllocator(kPGPdiskDefaultPRQDepth)
{
	mInitErr = mPRQAllocator.mInitErr;
}

// The PGPdiskRequestQueue destructor.

PGPdiskRequestQueue::~PGPdiskRequestQueue()
{
}

// Count returns the number of request in the queue.

PGPUInt32 
PGPdiskRequestQueue::Count()
{
	return mPRQList.Count();
}

// QueueIrpForProcessing queues an IRP for processing.

DualErr 
PGPdiskRequestQueue::QueueIrpForProcessing(PIRP pIrp)
{
	DualErr		derr;
	KIrp		I(pIrp);
	PRQRequest	*pReq;

	pgpAssertAddrValid(pIrp, IRP);

	// Get memory for request structure.
	derr = mPRQAllocator.Allocate(&pReq);

	// Initialize the request and place it in the queue.
	if (derr.IsntError())
	{
		pReq->op	= kPRQOp_ProcessIrp;
		pReq->pIrp	= pIrp;

		I.MarkPending();
		I.SetCancelRoutine(Cancel);

		Push(pReq);
	}

	return derr;
}

// QueueAsyncCallback queues an async callback request.

DualErr 
PGPdiskRequestQueue::QueueAsyncCallback(
	PEZCALLBACK	callback, 
	PGPUInt32	refData)
{
	DualErr		derr;
	PRQRequest	*pReq;

	pgpAssertAddrValid(callback, VoidAlign);

	// Get memory for request structure.
	derr = mPRQAllocator.Allocate(&pReq);

	// Initialize the request and place it in the queue.
	if (derr.IsntError())
	{
		pReq->op		= kPRQOp_AsyncCallback;
		pReq->callback	= callback;
		pReq->refData	= refData;

		Push(pReq);
	}

	return derr;
}

// QueueSyncCallback queues a ssync callback request.

DualErr 
PGPdiskRequestQueue::QueueSyncCallback(
	PEZCALLBACK	callback, 
	PGPUInt32	refData, 
	PKEVENT		pEvent)
{
	DualErr		derr;
	PRQRequest	*pReq;

	pgpAssertAddrValid(callback, VoidAlign);
	pgpAssertAddrValid(pEvent, KEVENT);

	// Get memory for request structure.
	derr = mPRQAllocator.Allocate(&pReq);

	// Initialize the request and place it in the queue.
	if (derr.IsntError())
	{
		pReq->op			= kPRQOp_SyncCallback;
		pReq->callback		= callback;
		pReq->refData		= refData;
		pReq->pSyncEvent	= pEvent;

		Push(pReq);
	}

	return derr;
}

// ReturnPRQ is called to return a processed PRQRequest to the memory pool.

void 
PGPdiskRequestQueue::ReturnPRQ(PRQRequest *pReq)
{
	pgpAssertAddrValid(pReq, PRQRequest);

	mPRQAllocator.Free(pReq);
}

// CancelIRPs cancels all outstanding IRPs in the queue for a given drive.

void 
PGPdiskRequestQueue::CancelIRPs(PGPUInt8 drive)
{
	KIrp		I;
	PRQRequest	*pReq;
	PGPUInt32	i;

	pgpAssert(IsLegalDriveNumber(drive));

	mPRQList.PublicLock();

	pReq = mPRQList.Head();

	// For every request in the queue...
	for (i = 0; i < mPRQList.Count(); i++)
	{
		if (IsNull(pReq))
			break;

		// ... if it is an IRP request...
		if (pReq->op == kPRQOp_ProcessIrp)
		{
			PDEVICE_OBJECT	pDevObj;
			PGPdisk			*pPGD;
			PGPUInt8		irpDrive;

			// ... and matches the drive letter we are cancelling...
			I = KIrp(pReq->pIrp);

			pDevObj = I.DeviceObject();
			pgpAssertAddrValid(pDevObj, DEVICE_OBJECT);

			pPGD = (PGPdisk *) pDevObj->DeviceExtension;
			pgpAssertAddrValid(pPGD, PGPdisk);

			irpDrive = pPGD->GetDrive();
			pgpAssert(IsLegalDriveNumber(drive));

			// ... then cancel it and remove it from the queue.
			if (irpDrive == drive)
			{
				mPRQList.Remove(pReq);
				ReturnPRQ(pReq);

				I.SetCancelRoutine(NULL);
				I.Information() = 0;
				I.Complete(STATUS_CANCELLED);

				pReq = mPRQList.Head();
			}
			else
			{
				pReq = mPRQList.Next(pReq);
			}
		}
		else
		{
			pReq = mPRQList.Next(pReq);
		}
	}

	mPRQList.PublicUnlock();
}

// Pop pops a request from the bottom of the queue. TRUE if successful, FALSE
// otherwise.
 
PRQRequest * 
PGPdiskRequestQueue::Pop()
{
	PRQRequest *pReq;

	// If list is empty, bail.
	if (mPRQList.IsEmpty())
		return NULL;

	// Pop an item off the queue.
	mPRQList.PublicLock();
	pReq = mPRQList.RemoveTail();
	mPRQList.PublicUnlock();

	// Clear cancel routine for IRP requests.
	if (pReq->op == kPRQOp_ProcessIrp)
	{
		KIrp I = pReq->pIrp;

		I.SetCancelRoutine(NULL);	
	}

	return pReq;
}


/////////////////////////////////////////////////////
// Class PGPdiskRequestQueue private member functions
/////////////////////////////////////////////////////

// Push pushes a new request on top of the queue. TRUE if successful, FALSE
// otherwise.

void 
PGPdiskRequestQueue::Push(PRQRequest *pReq)
{
	pgpAssertAddrValid(pReq, PRQRequest);

	// Insert the request into the queue.
	mPRQList.PublicLock();
	mPRQList.InsertHead(pReq);
	mPRQList.PublicUnlock();
}

// CancelAux looks for and cancels an IRP in the queue.

void 
PGPdiskRequestQueue::CancelAux()
{
	KIrp		I;
	PRQRequest	*pReq;
	PGPBoolean	foundIrpToCancel	= FALSE;
	PGPUInt32	i;

	mPRQList.PublicLock();

	pReq = mPRQList.Head();

	// Find an IRP in need of a cancel.
	for (i = 0; i < mPRQList.Count(); i++)
	{
		if (IsNull(pReq))
			break;

		if (pReq->op == kPRQOp_ProcessIrp)
		{
			I = KIrp(pReq->pIrp);

			foundIrpToCancel = TRUE;
			break;
		}

		pReq = mPRQList.Next(pReq);
	}

	// If we found one, complete it and remove it from the queue.
	if (foundIrpToCancel)
	{
		mPRQList.Remove(pReq);
		mPRQList.PublicUnlock();

		ReturnPRQ(pReq);

		I.Information() = 0;
		I.Complete(STATUS_CANCELLED);
	}
	else
	{
		mPRQList.PublicUnlock();
	}
}

// Cancel is the static member function set as the cancel routine for all
// IRPs put into the queue.

VOID 
PGPdiskRequestQueue::Cancel(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	KIrp				I(pIrp);
	PGPdisk				*pPGD;
	PGPdiskRequestQueue	*pPRQ;

	CancelSpinLock::Release(I.CancelIrql());

	// Extract PGPdisk pointer.
	pPGD = (PGPdisk *) pDevObj->DeviceExtension;
	pgpAssertAddrValid(pPGD, PGPdisk);

	// Call the associated request queue cancel function.
	pPRQ = pPGD->GetThread()->GetRequestQueue();
	pgpAssertAddrValid(pPRQ, PGPdiskRequestQueue);

	pPRQ->CancelAux();
}
