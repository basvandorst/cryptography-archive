//////////////////////////////////////////////////////////////////////////////
// FSCTLCodes.h
//
// Definitions for FSCTL codes not available in kernel-mode headers.
//////////////////////////////////////////////////////////////////////////////

// $Id: FSCTLCodes.h,v 1.1.2.1 1998/05/23 01:43:28 nryan Exp $

// Portions Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_FSCTLCodes_h	// [
#define Included_FSCTLCodes_h


////////////
// Constants
////////////

// Ripped from winioctl.h.

const PGPUInt32 FSCTL_LOCK_VOLUME = 
	CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 6, METHOD_BUFFERED, FILE_ANY_ACCESS);

const PGPUInt32 FSCTL_UNLOCK_VOLUME = 
	CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 7, METHOD_BUFFERED, FILE_ANY_ACCESS);

const PGPUInt32 FSCTL_DISMOUNT_VOLUME = 
	CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 8, METHOD_BUFFERED, FILE_ANY_ACCESS);

const PGPUInt32 FSCTL_MOUNT_DBLS_VOLUME	= 
	CTL_CODE(FILE_DEVICE_FILE_SYSTEM,13, METHOD_BUFFERED, FILE_ANY_ACCESS);

const PGPUInt32 FSCTL_GET_COMPRESSION = 
	CTL_CODE(FILE_DEVICE_FILE_SYSTEM,15, METHOD_BUFFERED, FILE_ANY_ACCESS);

const PGPUInt32 FSCTL_SET_COMPRESSION = 
	CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 16, METHOD_BUFFERED, 
	FILE_READ_DATA | FILE_WRITE_DATA);

#endif	// ] Included_FSCTLCodes_h
