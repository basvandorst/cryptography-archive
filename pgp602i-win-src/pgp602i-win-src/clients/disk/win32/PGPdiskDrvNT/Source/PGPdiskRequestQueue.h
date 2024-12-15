//////////////////////////////////////////////////////////////////////////////
// PGPdiskRequestQueue.h
//
// Declaration of class PGPdiskRequestQueue.
//////////////////////////////////////////////////////////////////////////////

// $Id: PGPdiskRequestQueue.h,v 1.1.2.12 1998/07/06 08:57:55 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_PGPdiskRequestQueue_h	// [
#define Included_PGPdiskRequestQueue_h

#include "DualErr.h"
#include "GenericCallback.h"

#include "CipherContext.h"

#include "CPGPdiskDriver.h"
#include "LockableList.h"
#include "LookasideList.h"


////////////
// Constants
////////////

const PGPUInt32	kPRQMaxRequests = 100;		// max requests in queue


////////
// Types
////////

class PGPdisk;

// PRQOp describes the operation embedded in a PRQRequest.

enum PRQOp {kPRQOp_ProcessIrp, kPRQOp_AsyncCallback, kPRQOp_SyncCallback, 
	kPRQOp_InvalidRequest};

// A PRQRequest request structure describes requests that can be stored in
// the queue.

typedef struct PRQRequest
{
	// Common members:

	LIST_ENTRY	m_ListEntry;
	PRQOp		op;

	// For kPRQOp_ProcessIrp:

	PIRP pIrp;

	// For kPRQOp_AsyncCallback and kPRQOp_SyncCallback.

	PEZCALLBACK callback;
	PGPUInt32 refData;

	PKEVENT pSyncEvent;

} PRQRequest;

typedef LookasideList<PRQRequest> PRQRequestAllocator;
typedef LockableList<PRQRequest> PRQRequestList;


////////////////////////////
// Class PGPdiskRequestQueue
////////////////////////////

class PGPdiskRequestQueue
{
public:
	DualErr mInitErr;

					PGPdiskRequestQueue();
					~PGPdiskRequestQueue();

	PGPUInt32		Count();

	DualErr			QueueIrpForProcessing(PIRP pIrp);
	DualErr			QueueAsyncCallback(PEZCALLBACK callback, 
						PGPUInt32 refData);
	DualErr			QueueSyncCallback(PEZCALLBACK callback, 
						PGPUInt32 refData, PKEVENT pEvent);

	void			CancelIRPs(PGPUInt8 drive);

	void			ReturnPRQ(PRQRequest *pReq);
	PRQRequest *	Pop();

private:
	PRQRequestAllocator	mPRQAllocator;		// request allocator
	PRQRequestList		mPRQList;			// linked list of requests

	void		Push(PRQRequest *pReq);

	void		CancelAux();
	static VOID	Cancel(PDEVICE_OBJECT pDevObj, PIRP pIrp);
};

#endif	// ] Included_PGPdiskRequestQueue_h
