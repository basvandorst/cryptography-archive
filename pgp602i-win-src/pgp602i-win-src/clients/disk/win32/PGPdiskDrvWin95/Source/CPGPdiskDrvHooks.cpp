//////////////////////////////////////////////////////////////////////////////
// CPGPdiskDrvHooks.cpp
//
// Functions for hooking system services.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPGPdiskDrvHooks.cpp,v 1.3.2.27.2.1 1998/10/21 07:12:31 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include <vtoolscp.h>

#include "Required.h"
#include "UtilityFunctions.h"

#include "CPGPdiskDrvHooks.h"
#include "Globals.h"

#include LOCKED_CODE_SEGMENT
#include LOCKED_DATA_SEGMENT


////////////
// Constants
////////////

const PGPUInt8	kButtonStatusMask	= 0x3C;		// all but button bits mask
const PGPUInt8	kSharedModeMask		= 0x0F;		// file open share bits mask


///////////////////////////
// Hooking helper functions
///////////////////////////

// BroadcastMessageHook checks for unsafe volume locking requests.

BOOL 
__cdecl 
CPGPdiskDrv::BroadcastMessageHook(
	DWORD	uMsg, 
	DWORD	wParam, 
	DWORD	lParam, 
	DWORD	dwRef)
{
	DualErr		derr;
	PGPBoolean	denyThisMessage	= FALSE;

	switch (GetLowWord(uMsg))
	{
	case WM_DEVICECHANGE:
		if (wParam == DBT_VOLLOCKQUERYLOCK)
		{
			PGPUInt8			drive;
			VolLockBroadcast	*pVLB;

			pVLB = (VolLockBroadcast *) lParam;
			pgpAssertAddrValid(pVLB, VolLockBroadcast);

			drive = pVLB->vlb_drive;

			// Deny lock request on host drives of PGPdisks as long as the
			// request did not come from PGPdisk itself.

			if (Driver->mPGPdisks.IsLocalDriveAPGPdiskHost(drive))
			{
				if (Driver->mLockInProgress && 
					(Driver->mDriveBeingLocked == drive))
				{
					Driver->mLockInProgress = FALSE;
				}
				else
				{
					denyThisMessage = TRUE;
				}
			}
		}
		break;
	}

	return !denyThisMessage;
}

// KeyboardHook hooks all input from the keyboard.

void 
__stdcall 
CPGPdiskDrv::KeyboardHook(PDSFRAME pRegs)
{
	// Reset the inactivity timer.
	Driver->mSecondsInactive = 0;

	// Clear the carry flag, meaning don't destroy any input.
	pRegs->RFLAGS &= ~1;
	Call_Previous_Hook_Proc(pRegs, &Driver->mKeyboard_Thunk);
}

// MouseHook hooks all input from the mouse.

void 
__stdcall 
CPGPdiskDrv::MouseHook(PDSFRAME regs)
{
	PGPUInt8 buttonStatus;

	buttonStatus = GetLowByte(GetLowWord(regs->REAX));

	// Reset the inactivity timer.
	if ((buttonStatus & kButtonStatusMask) != 0)
		Driver->mSecondsInactive = 0;

	Call_Previous_Hook_Proc(regs, &Driver->mMouse_Thunk);
}

// Due to an incredibly obscure interaction involving MSVC an optimization and
// an IOS bug (it passes IOP address on the stack, expected that value to be
// unaltered when the function returns - within local parameter storage no
// less!), IOS request handlers have to be called using a stub function.

VOID 
__cdecl 
CPGPdiskDrv::PGPDISK_EjectHandlerStub(PIOP pIop)
{
	PGPDISK_EjectHandler(pIop);
}

// PGPDISK_EjectHandler is an IOP request handler that is installed for every
// drive that hosts a PGPdisk. It makes sure that the system can't eject a
// removable volume that has a mounted PGPdisk on it. However, the user can
// still eject such volumes manually, which is totally outside of Win95's
// control.

VOID 
CPGPdiskDrv::PGPDISK_EjectHandler(PIOP pIop)
{
	PGPBoolean	DenyIOP	= FALSE;
	PGPUInt8	drive;
	PIOR		pIor;

	pgpAssertAddrValid(pIop, IOP);

	pIor = &pIop->IOP_ior;
	pgpAssertAddrValid(pIor, IOR);

	// Is this an eject request directed at a PGPdisk host?
	drive = pIor->IOR_vol_designtr;

	if (pIor->IOR_func == IOR_EJECT_MEDIA)			// is this an eject?
	{
		if (Driver->mPGPdisks.IsLocalDriveAPGPdiskHost(drive))
			DenyIOP = TRUE;
	}

	// Are we denying the IOP?
	if (DenyIOP)
	{
		DebugOut("PGPdisk: Denying eject request for PGPdisk host %d", drive);

		pIor->IOR_status = IORS_VOL_IN_USE;
		--pIop->IOP_callback_ptr;	
		pIop->IOP_callback_ptr->IOP_CB_address(pIop);
	}
	else
	{
		pIop->IOP_calldown_ptr = pIop->IOP_calldown_ptr->DCB_cd_next;
		((PFNIOP) pIop->IOP_calldown_ptr->DCB_cd_io_address)(pIop);
	}
}


//////////////////////////////////////////
// Hook installation and cleanup functions
//////////////////////////////////////////

// HookEjectionFilter hooks an ejection filter for the specified drive.

DualErr 
CPGPdiskDrv::HookEjectionFilter(PGPUInt8 drive)
{
	DualErr	derr;
	PDCB	hostDcb	= NULL;

	pgpAssert(IsLegalDriveNumber(drive));

	// If we haven't already hooked this drive and if it is NOT a PGPdisk,
	// then hook it. (PGPdisks have no eject buttons).

	if (!mIsEjectHooked[drive] && !mPGPdisks.FindPGPdisk(drive))
	{
		if (!(hostDcb = IspGetDcb(drive)))
			derr = DualErr(kPGDMinorError_IspGetDcbFailed);

		if (derr.IsntError())
		{
			if (!IspInsertCalldown(hostDcb, PGPDISK_EjectHandlerStub, 
				mTheDDB, 0, hostDcb->DCB_cmn.DCB_dmd_flags, DRP_FSD_EXT_1))
			{
				derr = DualErr(kPGDMinorError_IspInsertCalldownFailed);
			}
		}

		if (derr.IsntError())
		{
			mIsEjectHooked[drive] = TRUE;
		}
	}

	return derr;
}

// SetupSystemHooksCallback installs the default system hooks required by the
// driver.

VOID 
__stdcall 
CPGPdiskDrv::SetupSystemHooksCallback(
	VMHANDLE		hVM, 
	THREADHANDLE	hThread, 
	PVOID			Refdata, 
	PCLIENT_STRUCT	pRegs)
{
	// We need a mouse hook to monitor mouse activity.
	Driver->mHookedVKD = IsntNull(
		::Hook_Device_Service(__VKD_Filter_Keyboard_Input, KeyboardHook, 
		&Driver->mKeyboard_Thunk));

	// We need a keyboard hook to monitor keyboard activity.
	Driver->mHookedVMD = IsntNull(
		::Hook_Device_Service(__VMD_Manipulate_Pointer_Message, MouseHook, 
		&Driver->mMouse_Thunk));

	// Install our broadcast hook so we can prevent certain volume locks.
	Driver->mBroadcastHookHandle = 
		::SHELL_HookSystemBroadcast(BroadcastMessageHook, NULL, 0);

	Driver->mHookedSystemBroadcast = IsntNull(Driver->mBroadcastHookHandle);
}

// SetupSystemHooks schedules SetupSystemHooksCallback for execution at a
// safe time.

void 
CPGPdiskDrv::SetupSystemHooks()
{
	static RestrictedEvent_THUNK callbackThunk;

	::Call_Restricted_Event(0, NULL, PEF_ALWAYS_SCHED | PEF_WAIT_NOT_CRIT, 
		NULL, SetupSystemHooksCallback, 0, &callbackThunk);
}

// RemoveSystemHooks unhooks as many of our hooks as is possible.

void 
CPGPdiskDrv::RemoveSystemHooks()
{
	if (mHookedSystemBroadcast)
	{
		::SHELL_UnhookSystemBroadcast(mBroadcastHookHandle);
		mHookedSystemBroadcast = FALSE;
	}

	if (Driver->mHookedVKD)
	{
		Unhook_Device_Service(__VKD_Filter_Keyboard_Input, KeyboardHook, 
			&mKeyboard_Thunk);
	}

	if (Driver->mHookedVMD)
	{
		Unhook_Device_Service(__VMD_Manipulate_Pointer_Message, MouseHook, 
			&mMouse_Thunk);
	}
}
