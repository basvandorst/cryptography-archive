//////////////////////////////////////////////////////////////////////////////
// CPGPdiskDrvErrors.cpp
//
// Error handling functions.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPGPdiskDrvErrors.cpp,v 1.5 1999/02/13 04:24:34 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include <vtoolscp.h>

#include "Required.h"
#include "SimpleQueue.h"
#include "StringAssociation.h"

#include "CPGPdiskDrv.h"
#include "CPGPdiskDrvErrors.h"
#include "Globals.h"


///////////////////////////
// Error handling functions
///////////////////////////

// ReportErrorEndCallback is called when the user has closed the error
// reporting message box.

VOID 
__stdcall 
CPGPdiskDrv::ReportErrorEndCallback(DWORD ResponseCode, PVOID Refdata)
{
	ErrorCell	*pErrPacket;
	LPSTR		errString;

	pErrPacket = (ErrorCell *) Refdata;
	pgpAssertAddrValid(pErrPacket, ErrorCell);

	// If an I/O error, unmount the PGPdisk if user specified 'Yes', else
	// re-enable I/O to the PGPdisk.

	switch (pErrPacket->perr)
	{
	case kPGDMajorError_PGPdiskReadError:
	case kPGDMajorError_PGPdiskWriteError:
		{
		PGPdisk		*pPGD;
		PGPUInt8	drive	= pErrPacket->drive;

		pgpAssert(IsLegalDriveNumber(drive));

		if (ResponseCode == IDYES)
		{
			Driver->UnmountPGPdisk(drive, TRUE);
		}
		else
		{
			pPGD = Driver->mPGPdisks.FindPGPdisk(drive);

			if (IsntNull(pPGD))
				pPGD->EnableIO(TRUE);
		}
		}
		break;
	}

	// Free the error string.
	errString = pErrPacket->errString;
	pgpAssertStrValid(errString);

	delete[] errString;

	// Mark the packet as unused.
	pErrPacket->isInUse = FALSE;

	// If more errors in queue, then schedule another callback.
	if (!Driver->mErrorQueue.IsEmpty())
		Driver->ScheduleErrorCallback();
	else
		Driver->mIsErrorCallbackBusy = FALSE;
}

// ReportErrorStartCallback begins the process of reporting an error at a
// safe time.

VOID 
__stdcall 
CPGPdiskDrv::ReportErrorStartCallback(
	VMHANDLE		hVM, 
	THREADHANDLE	hThread, 
	PVOID			Refdata, 
	PCLIENT_STRUCT	pRegs)
{
	DualErr						derr;
	ErrorCell					*pErrPacket;
	LPSTR						errString;
	static SHELLMessage_THUNK	callbackThunk;

	// Pop an error off the queue.
	if (!Driver->mErrorQueue.Pop((PGPUInt32 *) &pErrPacket))
	{
		pgpAssert(FALSE);
		return;
	}

	pgpAssertAddrValid(pErrPacket, ErrorCell);

	// Allocate scratch space for the error string.
	if (IsNull(errString = new char[kMaxStringSize]))
		derr = DualErr(kPGDMinorError_OutOfMemory);

	// Format and display the error message.
	if (derr.IsntError())
	{
		FormatErrorString(pErrPacket->perr, pErrPacket->derr, 
			pErrPacket->drive, errString, kMaxStringSize);

		pErrPacket->errString = errString;

		switch (pErrPacket->perr)
		{
		case kPGDMajorError_PGPdiskReadError:
		case kPGDMajorError_PGPdiskWriteError:
			SHELL_Message(Get_Sys_VM_Handle(), MB_ICONEXCLAMATION | MB_YESNO, 
				errString, "PGPdisk Error", ReportErrorEndCallback, 
				pErrPacket, &callbackThunk);
			break;

		default:
			SHELL_Message(Get_Sys_VM_Handle(), MB_ICONEXCLAMATION | MB_OK, 
				errString, "PGPdisk Error", ReportErrorEndCallback, 
				pErrPacket, &callbackThunk);
			break;
		}
	}
}

// ScheduleErrorCallback schedules a callback that pops and reports errors on
// the error queue.

void 
CPGPdiskDrv::ScheduleErrorCallback()
{
	static RestrictedEvent_THUNK callbackThunk;

	Call_Restricted_Event(0, NULL, 
		PEF_WAIT_NOT_NESTED_EXEC | PEF_WAIT_NOT_CRIT, NULL, 
		ReportErrorStartCallback, 0, &callbackThunk);
}

// ReportError reports certain errors to the user.

void 
CPGPdiskDrv::ReportError(PGDMajorError perr, DualErr derr, PGPUInt8 drive)
{
	PGPBoolean	foundErrPacket		= FALSE;
	PGPUInt32	i;

	// Find a not-in-use error packet.
	for (i = 0; i < kNumErrPackets; i++)
	{
		if (!mErrPackets[i].isInUse)
		{
			foundErrPacket = TRUE;
			mErrPackets[i].isInUse = TRUE;
			break;
		}
	}

	if (!foundErrPacket)
		return;

	// Fill in the error info.
	mErrPackets[i].perr		= perr;
	mErrPackets[i].derr		= derr;
	mErrPackets[i].drive	= drive;

	// Stick it in the queue.
	mErrorQueue.Push((PGPUInt32) &mErrPackets[i]);

	// If callback is not busy, then schedule it.
	if (!mIsErrorCallbackBusy)
	{
		mIsErrorCallbackBusy = TRUE;
		ScheduleErrorCallback();
	}
}
