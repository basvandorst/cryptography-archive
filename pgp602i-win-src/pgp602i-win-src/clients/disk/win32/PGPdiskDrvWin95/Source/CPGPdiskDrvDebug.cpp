//////////////////////////////////////////////////////////////////////////////
// CPGPdiskDrvDebug.cpp
//
// Contains debugging helper routines.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPGPdiskDrvDebug.cpp,v 1.1.2.19 1998/07/06 08:58:06 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include <vtoolscp.h>

#include "Required.h"
#include "CPGPdiskDrv.h"
#include "CPGPdiskDrvDebug.h"

#include LOCKED_CODE_SEGMENT
#include LOCKED_DATA_SEGMENT


////////////
// Constants
////////////

#define PGPDISK_DEBUGMEMORYLEAKS 0

const PGPUInt32 kGuardDWord = 0xF926A4E9;


#if PGP_DEBUG	// [

///////////////////////////////////
// Debug memory allocation routines
///////////////////////////////////

#if PGPDISK_DEBUGMEMORYLEAKS
static PGPUInt32 totalBytesAllocated = 0;
#endif	// PGPDISK_DEBUGMEMORYLEAKS

// __default_nh is the default error handler for new.

int _cdecl 
__default_nh(size_t)
{
	pgpDebugMsg("Couldn't allocate memory in operator new");

	return 0;
};

_PNH __newhandler = __default_nh;

// _set_new_handler sets the operator new error handler.

_PNH _cdecl 
_set_new_handler(_PNH pHandler)
{
	_PNH old = __newhandler;
	__newhandler = pHandler;

	return old;
}

// DebugNewHelper is the implementation of our debug new function.

void * _cdecl 
DebugNewHelper(size_t n)
{
	PGPUInt8	*p;
	PGPUInt32	dataSize, totalSize;

	dataSize	= n;
	totalSize	= dataSize + 3*sizeof(PGPUInt32);

	do
	{	
		p = (PGPUInt8 *) _HeapAllocate(totalSize, 0);
	}
	while ((!p) && __newhandler(totalSize));

	if (IsntNull(p))
	{
		PGPUInt8 *returnMe, *temp;

		pgpDebugWhackMemory(p, totalSize);		// upside the head

		temp = (PGPUInt8 *) p;
		returnMe = temp + 2*sizeof(PGPUInt32);

		((PGPUInt32 *) temp)[0]	= dataSize;
		((PGPUInt32 *) temp)[1]	= kGuardDWord;

		temp += 2*sizeof(PGPUInt32) + dataSize;

		((PGPUInt32 *) temp)[0]	= kGuardDWord;

		#if PGPDISK_DEBUGMEMORYLEAKS

		totalBytesAllocated += n;
		DebugOut("PGPdisk New: Alloced %d bytes, have %d total allocated\n", 
			n, totalBytesAllocated);

		#endif	// PGPDISK_DEBUGMEMORYLEAKS

		return returnMe;
	}
	else
	{
		return NULL;
	}
}

// DebugDeleteHelper is the implementation of our debug delete function.

void _cdecl 
DebugDeleteHelper(void *p)
{
	static char			msg[kMaxStringSize];	// don't waste stack space!
	static PGPUInt32	numDelete	= 0;

	if (IsntNull(p))
	{
		PGPBoolean	firstGuardWasValid, secondGuardWasValid;
		PGPUInt8	*freeMe, *temp;
		PGPUInt32	dataSize, firstGuard, secondGuard, totalSize;
		
		numDelete++;

		temp = freeMe = (PGPUInt8 *) p - 2*sizeof(PGPUInt32);

		dataSize	= ((PGPUInt32 *) temp)[0];
		totalSize	= dataSize + 3*sizeof(PGPUInt32);

		firstGuard = ((PGPUInt32 *) temp)[1];
		firstGuardWasValid = (firstGuard == kGuardDWord);

		if (!firstGuardWasValid)
		{
			sprintf(msg, "Beginning guard dword trashed on memory block at "
				"pointer address %x delete number %d", p, numDelete);

			pgpDebugMsg(msg);
		}
		else
		{
			temp += 2*sizeof(PGPUInt32) + dataSize;
			secondGuard = ((PGPUInt32 *) temp)[0];

			secondGuardWasValid = (secondGuard == kGuardDWord);

			if (!secondGuardWasValid)
			{
				sprintf(msg, "Ending guard dword trashed on memory block with "
					"size %d and pointer address %x delete number %d", 
					dataSize, p, numDelete);

				pgpDebugMsg(msg);
			}
		}

		// Don't whack unless we are sure of the correct size.
		if (firstGuardWasValid)
			pgpDebugWhackMemory(freeMe, totalSize);		// upside the head

		_HeapFree((void *) freeMe, 0);

		#if PGPDISK_DEBUGMEMORYLEAKS

		totalBytesAllocated -= dataSize;
		DebugOut("PGPdisk Delete: Freed %d bytes, have %d total allocated\n", 
			dataSize, totalBytesAllocated);

		#endif	// PGPDISK_DEBUGMEMORYLEAKS
	}
}

// A debug version of new with guard bytes.

void * 
_cdecl 
operator new(size_t n)
{
	return DebugNewHelper(n);
}

// A debug version of operation new[] with guard bytes.

void * 
_cdecl 
operator new[](size_t n)
{
	return DebugNewHelper(n);
}

// A debug version of operation delete with guard bytes.

void 
_cdecl 
operator delete(void *p)
{
	DebugDeleteHelper(p);
}

// A debug version of operation delete[] with guard bytes.

void 
_cdecl 
operator delete[](void *p)
{
	DebugDeleteHelper(p);
}

#endif	// ] PGP_DEBUG

//////////////////////////////////////////////
// Functions for associating values with names
//////////////////////////////////////////////

// GetName parses a nametable and determines which string is associated with
// which constant, and returns it.

LPCSTR 
CPGPdiskDrv::GetName(NameAssoc nameTable[], PGPUInt32 n, PGPUInt32 func)
{
	PGPUInt32 i;

	for (i = 0; i < n; i++)
	{
		if (func == nameTable[i].func)
			return nameTable[i].name;
	}

	return "<Unknown>";
}

// GetADPacketName returns a string with the name of the AD packet passed in
// 'func'.

LPCSTR 
CPGPdiskDrv::GetADPacketName(PGPUInt32 func)
{
	static NameAssoc nameTable[] =
	{
		{kAD_Mount,					"kAD_Mount"},
		{kAD_Unmount,				"kAD_Unmount"},
		{kAD_QueryVersion,			"kAD_QueryVersion"},
		{kAD_QueryMounted,			"kAD_QueryMounted"},
		{kAD_QueryOpenFiles,		"kAD_QueryOpenFiles"},
		{kAD_ChangePrefs,			"kAD_ChangePrefs"},
		{kAD_LockUnlockMem,			"kAD_LockUnlockMem"}, 
		{kAD_GetPGPdiskInfo,		"kAD_GetPGPdiskInfo"}, 
		{kAD_LockUnlockVol,			"kAD_LockUnlockVol"}, 
		{kAD_ReadWriteVol,			"kAD_ReadWriteVol"}, 
		{kAD_QueryVolInfo,			"kAD_QueryVolInfo"}, 
		{kAD_NotifyUserLogoff,		"kAD_NotifyUserLogoff"}

	};

	return GetName(nameTable, (sizeof(nameTable)/sizeof(NameAssoc)), func);
}

// GetAEPFunctionName returns a string with the name of the AEP function
// passed in 'func'.

LPCSTR 
CPGPdiskDrv::GetAEPFunctionName(PGPUInt32 func)
{
	static NameAssoc nameTable[] =
	{
		{AEP_INITIALIZE,			"AEP_INITIALIZE"},
		{AEP_SYSTEM_CRIT_SHUTDOWN,	"AEP_SYSTEM_CRIT_SHUTDOWN"},
		{AEP_BOOT_COMPLETE,			"AEP_BOOT_COMPLETE"},
		{AEP_CONFIG_DCB,			"AEP_CONFIG_DCB"},
		{AEP_UNCONFIG_DCB,			"AEP_UNCONFIG_DCB"},
		{AEP_IOP_TIMEOUT,			"AEP_IOP_TIMEOUT"},
		{AEP_DEVICE_INQUIRY,		"AEP_DEVICE_INQUIRY"},
		{AEP_HALF_SEC,				"AEP_HALF_SEC"},
		{AEP_1_SEC,					"AEP_1_SEC"},
		{AEP_2_SECS,				"AEP_2_SECS"},
		{AEP_4_SECS,				"AEP_4_SECS"},
		{AEP_DBG_DOT_CMD,			"AEP_DBG_DOT_CMD"},
		{AEP_ASSOCIATE_DCB,			"AEP_ASSOCIATE_DCB"},
		{AEP_REAL_MODE_HANDOFF,		"AEP_REAL_MODE_HANDOFF"},
		{AEP_SYSTEM_SHUTDOWN,		"AEP_SYSTEM_SHUTDOWN"},
		{AEP_UNINITIALIZE,			"AEP_UNINITIALIZE"},
		{AEP_DCB_LOCK,				"AEP_DCB_LOCK"},
		{AEP_MOUNT_NOTIFY,			"AEP_MOUNT_NOTIFY"},
		{AEP_CREATE_VRP,			"AEP_CREATE_VRP"},
		{AEP_DESTROY_VRP,			"AEP_DESTROY_VRP"},
		{AEP_REFRESH_DRIVE,			"AEP_REFRESH_DRIVE"},
		{AEP_PEND_UNCONFIG_DCB,		"AEP_PEND_UNCONFIG_DCB"},
		{AEP_1E_VEC_UPDATE,			"AEP_1E_VEC_UPDATE"},
		{AEP_CHANGE_RPM,			"AEP_CHANGE_RPM"}

	};

	return GetName(nameTable, (sizeof(nameTable) / sizeof(NameAssoc)), func);
}

// GetIORFunctionName returns a string with the name of the IOR function 
// passed in 'func'.

LPCSTR 
CPGPdiskDrv::GetIORFunctionName(PGPUInt32 func)
{
	static NameAssoc nameTable[] =
	{
		{IOR_READ,						"IOR_READ"},
		{IOR_WRITE,						"IOR_WRITE"},
		{IOR_VERIFY,					"IOR_VERIFY"},
		{IOR_CANCEL,					"IOR_CANCEL"},
		{IOR_WRITEV,					"IOR_WRITEV"},
		{IOR_MEDIA_CHECK,				"IOR_MEDIA_CHECK"},
		{IOR_MEDIA_CHECK_RESET,			"IOR_MEDIA_CHECK_RESET"},
		{IOR_LOAD_MEDIA,				"{IOR_IOR_LOAD_MEDIA"},
		{IOR_EJECT_MEDIA,				"IOR_EJECT_MEDIA"},
		{IOR_LOCK_MEDIA,				"IOR_LOCK_MEDIA"},
		{IOR_UNLOCK_MEDIA,				"IOR_UNLOCK_MEDIA"},
		{IOR_REQUEST_SENSE,				"IOR_REQUEST_SENSE"},
		{IOR_COMPUTE_GEOM,				"IOR_COMPUTE_GEOM"},
		{IOR_GEN_IOCTL,					"IOR_GEN_IOCTL"},
		{IOR_FORMAT,					"IOR_FORMAT"},
		{IOR_SCSI_PASS_THROUGH,			"IOR_SCSI_PASS_THROUGH"},
		{IOR_CLEAR_QUEUE,				"IOR_CLEAR_QUEUE"},
		{IOR_DOS_RESET,					"IOR_DOS_RESET"},
		{IOR_SCSI_REQUEST,				"IOR_SCSI_REQUEST"},
		{IOR_SET_WRITE_STATUS,			"IOR_SET_WRITE_STATUS"},
		{IOR_RESTART_QUEUE,				"IOR_RESTART_QUEUE"},
		{IOR_ABORT_QUEUE,				"IOR_ABORT_QUEUE"},
		{IOR_SPIN_DOWN,					"IOR_SPIN_DOWN"},
		{IOR_SPIN_UP,					"IOR_SPIN_UP"},
		{IOR_FLUSH_DRIVE,				"IOR_FLUSH_DRIVE"},
		{IOR_FLUSH_DRIVE_AND_DISCARD,	"IOR_FLUSH_DRIVE_AND_DISCARD"},
		{IOR_FSD_EXT,					"IOR_FSD_EXT"}

	};

	return GetName(nameTable, (sizeof(nameTable) / sizeof(NameAssoc)), func);
}
