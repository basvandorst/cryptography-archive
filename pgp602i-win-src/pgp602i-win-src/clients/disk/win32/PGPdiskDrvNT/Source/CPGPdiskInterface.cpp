//////////////////////////////////////////////////////////////////////////////
// CPGPdiskInterface.cpp
//
// Definition of class CPGPdiskInterface.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPGPdiskInterface.cpp,v 1.1.2.28 1998/07/30 18:23:22 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include <vdw.h>

#include "Required.h"
#include "Packets.h"

#include "CPGPdiskDriver.h"
#include "CPGPdiskInterface.h"
#include "File.h"
#include "Globals.h"


////////////
// Constants
////////////

const PGPUInt32	kContextToggleSeconds	= 5;
const PGPUInt32	kContextVerifySeconds	= 5;


///////////////////////////////////////////////////////////////
// Class CPGPdiskInterface initialization and cleanup functions
///////////////////////////////////////////////////////////////

// The CPGPdiskInterface constructor creates a 'device object' named PGPdisk.
// The app communicates with the driver using this object.

CPGPdiskInterface::CPGPdiskInterface(CPGPdiskDriver *pDriver)
	: KDevice(L"PGPdisk", FILE_DEVICE_UNKNOWN, L"PGPdisk", 0, DO_DIRECT_IO)

{
	NTSTATUS status;

	pgpAssertAddrValid(pDriver, CPGPdiskDriver);

	mSecondsInactive = 0;

	mPGPdiskDriver = pDriver;

	mTimerStarted	= FALSE;
	mUnmountAllMode	= FALSE;

	mAutoUnmount	= kDefaultAutoUnmount;
	mUnmountTimeout	= kDefaultUnmountTimeout;

	mKeyboardFilter = NULL;
	mMouseFilter = NULL;

	mCreatedKeyboardFilter	= FALSE;
	mCreatedMouseFilter		= FALSE;

	if (!NT_SUCCESS(m_ConstructorStatus))
	{
		mInitErr = DualErr(kPGDMinorError_DeviceConstructFailed, 
			m_ConstructorStatus);
	}

	if (mInitErr.IsntError())
	{
		mInitErr = mUserMemManager.mInitErr;
	}

	// Start our utility thread.
	if (mInitErr.IsntError())
	{
		mInitErr = mUtilityThread.StartThread();
	}

	// Setup our system hooks.
	if (mInitErr.IsntError())
	{
		mInitErr = SetupSystemHooks();
	}

	// Start our one-second timer.
	if (mInitErr.IsntError())
	{
		status = IoInitializeTimer(m_pDeviceObject, EverySecondCallback, 
			NULL);

		if (!NT_SUCCESS(status))
		{
			mInitErr = DualErr(kPGDMinorError_IoInitializeTimerFailed, 
				status);
		}
	}

	if (mInitErr.IsntError())
	{
		IoRegisterShutdownNotification(m_pDeviceObject);

		IoStartTimer(m_pDeviceObject);
		mTimerStarted = TRUE;
	}
}

// The CPGPdiskInterface destructor.

CPGPdiskInterface::~CPGPdiskInterface()
{
	if (mTimerStarted)
		IoStopTimer(m_pDeviceObject);

	DeleteSystemHooks();
}


/////////////////////////////////////
// Miscellaneous dispatcher functions
/////////////////////////////////////

// CleanUp is called to cancel all pending IRPs targeted to the same file
// object as the given IRP.

NTSTATUS 
CPGPdiskInterface::CleanUp(KIrp I)
{
	return I.Complete(STATUS_SUCCESS);
}

// Close is called when a handle is closed to the driver.

NTSTATUS 
CPGPdiskInterface::Close(KIrp I)
{
	return I.Complete(STATUS_SUCCESS);
}

// Create is called when a handle is opened to the driver.

NTSTATUS 
CPGPdiskInterface::Create(KIrp I)
{
	return I.Complete(STATUS_SUCCESS);
}

// Read is not implemented by CPGPdiskInterface.

NTSTATUS 
CPGPdiskInterface::Read(KIrp I)
{
	I.Information() = 0;
	return I.Complete(STATUS_NOT_IMPLEMENTED);
}

// Write is not implemented by CPGPdiskInterface.

NTSTATUS 
CPGPdiskInterface::Write(KIrp I)
{
	I.Information() = 0;
	return I.Complete(STATUS_NOT_IMPLEMENTED);
}


/////////////////////////
// Miscellaneous routines
/////////////////////////

// EverySecondCallback schedules EverySecondCallbackAux for callback at a
// safe time.

VOID 
CPGPdiskInterface::EverySecondCallback(
	PDEVICE_OBJECT	DeviceObject, 
	PVOID			Context)
{
	// Call at a safe time in our utility thread.
	Interface->mUtilityThread.ScheduleAsyncCallback(EverySecondCallbackAux);
}

// EverySecondCallbackAux is called once per second so we can perform various
// time-related tasks.

void 
CPGPdiskInterface::EverySecondCallbackAux(PGPUInt32 refData)
{
	static PGPUInt32	lastToggleTime	= 0;
	static PGPUInt32	lastVerifyTime	= 0;

	// Increase the inactivity timer.
	Interface->mSecondsInactive++;

	// If we are in 'unmount all mode', keep trying to unmount all until we
	// succeed.

	if (Interface->mUnmountAllMode)
	{
		DualErr derr;

		derr = Interface->UnmountAllPGPdisks();
		Interface->mUnmountAllMode = !derr.IsntError();
	}

	// Flip contexts on all PGPdisks if this is the time to do so.
	if (lastToggleTime++ > kContextToggleSeconds)
	{
		lastToggleTime = 0;
		Interface->mPGPdisks.FlipAllContexts();
	}

	// Verify cipher contexts on all PGPdisks if this is the time to do so.
	if (lastVerifyTime++ > kContextVerifySeconds)
	{
		lastVerifyTime = 0;
		Interface->mPGPdisks.ValidateAllCipherContexts();
	}

	// If it is time to auto-unmount, we do so. We schedule the unmounts to
	// occur during an appy-time callback.

	if ((Interface->mAutoUnmount) && 
		(Interface->mSecondsInactive > Interface->mUnmountTimeout*60))
	{
		Interface->mSecondsInactive = 0;
		Interface->UnmountAllPGPdisks();
	}
}
