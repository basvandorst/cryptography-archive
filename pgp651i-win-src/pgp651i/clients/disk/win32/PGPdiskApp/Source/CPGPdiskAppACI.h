//////////////////////////////////////////////////////////////////////////////
// CPGPdiskAppACI.h
//
// Declarations for CPGPdiskAppACI.cpp.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPGPdiskAppACI.h,v 1.4 1998/12/14 18:57:36 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CPGPdiskAppACI_h	// [
#define Included_CPGPdiskAppACI_h



////////////
// Constants
////////////

// Flags for AppCommandInfo.

const PGPUInt32	kACF_EmergencyUnmount	= 0x0001;	// emergency unmount
const PGPUInt32	kACF_MountWithNoDialog	= 0x0002;	// use given pass info
const PGPUInt32	kACF_ForceReadOnly		= 0x0004;	// force read only mount
const PGPUInt32	kACF_SilentWiping		= 0x0008;	// silent wiping
const PGPUInt32	kACF_NeedMaster			= 0x0010;	// mount with master


////////
// Types
////////

// Application operation identifiers.

enum AppOp	{kAppOp_GlobalConvert, kAppOp_Create, kAppOp_Mount, 
			 kAppOp_Unmount, kAppOp_UnmountAll, kAppOp_AddPassphrase, 
			 kAppOp_ChangePassphrase, kAppOp_RemovePassphrase, 
			 kAppOp_RemoveAlternates, kAppOp_AddRemovePublicKeys, 
			 kAppOp_ShowAboutBox, kAppOp_ShowHelp, kAppOp_ShowPrefs, 
			 kAppOp_WipePassesAllMounted, kAppOp_WipePassesThisDisk, 
			 kAppOp_InvalidRequest};

// An AppCommandInfo structure is used to encapsulate volume operation
// requests within the PGPdisk application.

typedef struct AppCommandInfo
{
	AppOp		op;							// operation to perform
	PGPUInt32	flags;						// flags
	PGPUInt8	drive;						// drive number
	char		path[kMaxStringSize];		// pathname of file/volume
	PGPUInt32	data[10];					// extra data

} AppCommandInfo, *PAppCommandInfo;

#endif	// ] Included_CPGPdiskAppACI_h
