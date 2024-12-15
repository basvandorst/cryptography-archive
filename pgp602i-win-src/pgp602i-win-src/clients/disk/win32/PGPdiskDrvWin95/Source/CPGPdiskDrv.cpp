//////////////////////////////////////////////////////////////////////////////
// CPGPdiskDrv.cpp
//
// Holds the entrance and exit routines for the driver.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPGPdiskDrv.cpp,v 1.1.2.24.2.1 1998/09/01 00:31:43 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#define	DEVICE_MAIN
#include <vtoolscp.h>

#include "Required.h"
#include "PGPdiskVersion.h"
#include "UtilityFunctions.h"

#include "CPGPdiskDrv.h"
#include "Globals.h"
#undef DEVICE_MAIN

// Declare_Port_Driver is a VToolsD macro that sets up our DRP (driver
// registration packet) for use in registering with the IOS.

Declare_Port_Driver(PGPDISK, DRP_MISC_PD, PGPDISK_NAME, PGPDISK_REV, 
	PGPDISK_FEATURE, PGPDISK_IFR, DRP_BT_ESDI, 0)


//////////////////////////////////
// Global variables for the driver
//////////////////////////////////

CPGPdiskDrv *Driver;				// pointer to the driver object


//////////////////////////////////////////////////////////////
// Initialization and cleanup routines for class PGPdiskDevice
//////////////////////////////////////////////////////////////

// ClearVariables initializes many variables to default values.

void 
CPGPdiskDrv::ClearVariables()
{
	PGPUInt32 i;

	Driver = this;

	mTheDDB = NULL;

	mSecondsInactive = 0;
	mPGPdiskAppVersion	= 0;

	for (i=0; i<kMaxDrives; i++)
		mIsEjectHooked[i] = FALSE;

	mBroadcastHookHandle = FALSE;
	mUnmountAllMode = FALSE;

	mHookedVKD = mHookedVMD = mHookedSystemBroadcast = FALSE;

	mLockInProgress = FALSE;
	mDriveBeingLocked = kInvalidDrive;

	mAutoUnmount = kDefaultAutoUnmount;
	mUnmountTimeout = kDefaultUnmountTimeout;

	for (i=0; i<kNumErrPackets; i++)
	{
		mErrPackets[i].isInUse		= FALSE;
		mErrPackets[i].perr			= kPGDMajorError_NoErr;
		mErrPackets[i].drive		= kInvalidDrive;
		mErrPackets[i].errString	= NULL;
	}

	mIsErrorCallbackBusy = FALSE;
}

// OnSysDynamicDeviceInit is called when the driver is being loaded. We
// register with the IOS subsystem and remain resident if everything went OK.

BOOL 
CPGPdiskDrv::OnSysDynamicDeviceInit()
{
	DebugOut("PGPdisk Port Driver v%s loaded.", kVersionTextString);
	DebugOut("Copyright (C) 1998 Network Associates, Inc.\n");

	// Init variables.
	ClearVariables();

	// Register with the IOS.
	PGPDISK_Drp.DRP_feature_code |= DRP_FC_1_SEC;
	::IOS_Register(&PGPDISK_Drp);

	if (PGPDISK_Drp.DRP_reg_result != DRP_REMAIN_RESIDENT)
		mInitErr = DualErr(kPGDMinorError_IosRegistrationFailed);

	return TRUE;
}

// OnInitComplete is called after all devices have initialized. We perform
// additional initialization.

BOOL 
CPGPdiskDrv::OnInitComplete(VMHANDLE hVM, PCHAR CommandTail)
{
	// Install our system hooks.
	SetupSystemHooks();

	return TRUE;
}

// OnSysDynamicDeviceExit is called when the driver is about to be unloaded,
// which only occurs on system shutdown since we are a port driver.

BOOL 
CPGPdiskDrv::OnSysDynamicDeviceExit()
{
	DebugOut("PGPdisk Driver v%s unloading.\n", kVersionTextString);

	// Cleanup.
	RemoveSystemHooks();

	return TRUE;
}

// OnSystemExit is called right before the system shuts down. Unmount all
// PGPdisks at this point.

VOID 
CPGPdiskDrv::OnSystemExit(VMHANDLE hVM)
{
	UnmountAllPGPdisks();
}
