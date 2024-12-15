//////////////////////////////////////////////////////////////////////////////
// Packets.h
//
// Contains definitions for the app/driver packet communication language.
//////////////////////////////////////////////////////////////////////////////

// $Id: Packets.h,v 1.5 1998/12/14 19:01:16 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_Packets_h	// [
#define Included_Packets_h

#if !defined(PGPDISK_NTDRIVER)
#include <winioctl.h>
#endif // !PGPDISK_NTDRIVER
#include "DualErr.h"

// Align to 1.
#pragma pack(push, 1)

////////
// Types
////////

class CipherContext;

// A PGPdiskInfo structure holds some simple information about a mounted
// PGPdisk. A linked list of these structures is passed to the application
// from the driver during registration.

typedef struct PGPdiskInfo
{
	char path[kMaxStringSize];	// path to the PGPdisk

	PGPUInt8	drive;		// drive # of a mounted PGPdisk
	PGPUInt64	sessionId;	// unique session Id

} PGPdiskInfo;

// Lock op enumeration.

enum LockOp {kLO_LockReadWrite, kLO_UnlockReadWrite, kLO_LockFormat, 
	kLO_UnlockFormat};


/////////////////////////////////////////////////////////////
// Application to driver (DeviceIoControl) packet definitions
/////////////////////////////////////////////////////////////

// The PGPdisk app is allowed to pass two values to the driver - a control
// code and a pointer.

const PGPUInt32 IOCTL_PGPDISK_SENDPACKET = CTL_CODE(FILE_DEVICE_DISK, 0x800, 
	METHOD_BUFFERED, FILE_ANY_ACCESS) | 0x40000000;

const PGPUInt32 kPGPdiskADPacketMagic = 0x820F7353;

// Functions codes for packets.

const PGPUInt16	kAD_Mount				= 0x0001;
const PGPUInt16	kAD_Unmount				= 0x0002;
const PGPUInt16	kAD_QueryVersion		= 0x0003;
const PGPUInt16	kAD_QueryMounted		= 0x0004;
const PGPUInt16	kAD_QueryOpenFiles		= 0x0005;
const PGPUInt16	kAD_ChangePrefs			= 0x0006;
const PGPUInt16	kAD_LockUnlockMem		= 0x0007;
const PGPUInt16	kAD_GetPGPdiskInfo		= 0x0008;
const PGPUInt16	kAD_LockUnlockVol		= 0x0009;
const PGPUInt16	kAD_ReadWriteVol		= 0x000A;
const PGPUInt16	kAD_QueryVolInfo		= 0x000B;
const PGPUInt16	kAD_NotifyUserLogoff	= 0x000C;

// Standard packet header.

typedef struct ADPacketHeader
{
	PGPUInt32	magic;		// must be kPGPdiskADPacketMagic
	PGPUInt16	code;		// packet code
	DualErr		*pDerr;		// error (DualErr *)

} ADPacketHeader, *PADPacketHeader;

// Struct AD_Mount.

typedef struct AD_Mount
{
	ADPacketHeader header;				// standard header

	LPCSTR			path;				// path to the PGPdisk
	PGPUInt32		sizePath;			// size of path

	PGPUInt8		drive;				// preferred drive letter
	PGPBoolean		readOnly;			// mount as read-only?

	CipherContext	*pContext;			// the cipher context
	PGPUInt8		*pDrive;			// drive mounted on (PGPUInt8 *)

} AD_Mount, *PAD_Mount;

// Struct AD_Unmount.

typedef struct AD_Unmount
{
	ADPacketHeader header;			// standard header

	PGPUInt8	drive;				// drive to unmount
	PGPBoolean	isThisEmergency;	// emergency unmount?

} AD_Unmount, *PAD_Unmount;

// Struct AD_QueryVersion.

typedef struct AD_QueryVersion
{
	ADPacketHeader header;			// standard header

	PGPUInt32	appVersion;			// application version
	PGPUInt32	*pDriverVersion;	// driver version (PGPUInt32 *)

} AD_QueryVersion, *PAD_QueryVersion;

// Struct AD_QueryMounted.

typedef struct AD_QueryMounted
{
	ADPacketHeader header;		// standard header

	PGPBoolean	trueIfUsePath;	// TRUE to use path, FALSE to use drive

	LPCSTR		path;			// file to enquire about
	PGPUInt32	sizePath;		// size of path
	PGPUInt8	drive;			// drive to enquire about

	PGPBoolean	*pIsPGPdisk;	// is a mounted PGPdisk? (PGPBoolean *)

} AD_QueryMounted, *PAD_QueryMounted;

// Struct AD_QueryOpenFiles.

typedef struct AD_QueryOpenFiles
{
	ADPacketHeader header;			// standard header

	PGPUInt8	drive;				// drive to enumerate open files on
	PGPBoolean	*pHasOpenFiles;		// pointer to answer (PGPBoolean *)

} AD_QueryOpenFiles, *PAD_QueryOpenFiles;

// Struct AD_ChangePrefs.

typedef struct AD_ChangePrefs
{
	ADPacketHeader header;			// standard header
	
	PGPBoolean	autoUnmount;		// is auto-unmount enabled?
	PGPUInt32	unmountTimeout;		// auto unmount timeout in minutes

} AD_ChangePrefs, *PAD_ChangePrefs;

// Struct AD_LockUnlockMem.

typedef struct AD_LockUnlockMem
{
	ADPacketHeader header;		// standard header
	
	void		*pMem;			// the linear memory address
	PGPUInt32	nBytes;			// size of memory block
	PGPBoolean	trueForLock;	// TRUE to lock, FALSE to unlock

} AD_LockUnlockMem, *PAD_LockUnlockMem;

// Struct AD_GetPGPdiskInfo.

typedef struct AD_GetPGPdiskInfo
{
	ADPacketHeader header;		// standard header

	PGPUInt32	arrayElems;		// elems in below array
	PGPdiskInfo *pPDIArray;		// array of kMaxDrives # of structures.

} AD_GetPGPdiskInfo, *PAD_GetPGPdiskInfo;

// Struct AD_LockUnlockVol.

typedef struct AD_LockUnlockVol
{
	ADPacketHeader header;		// standard header

	PGPUInt8	drive;			// drive number
	LockOp		lockOp;			// lock op to perform

} AD_LockUnlockVol, *PAD_LockUnlockVol;

// Struct AD_ReadWriteVol.

typedef struct AD_ReadWriteVol
{
	ADPacketHeader header;		// standard header

	PGPBoolean	trueIfRead;		// TRUE for read, FALSE for write

	PGPUInt8	drive;			// drive number
	PGPUInt64	pos;			// starting block
	PGPUInt32	nBlocks;		// number of blocks

	PGPUInt8	*buf;			// buffer
	PGPUInt32	bufSize;		// size of buffer

} AD_ReadWriteVol, *PAD_ReadWriteVol;

// Struct AD_QueryVolInfo.

typedef struct AD_QueryVolInfo
{
	ADPacketHeader header;		// standard header

	PGPUInt8	drive;			// drive number
	PGPUInt16	*pBlockSize;	// return blocksize here (PGPUInt16 *)
	PGPUInt64	*pTotalBlocks;	// return total blocks here (PGPUInt64 *)

} AD_QueryVolInfo, *PAD_QueryVolInfo;

// Struct AD_NotifyUserLogoff.

typedef struct AD_NotifyUserLogoff
{
	ADPacketHeader header;		// standard header

} AD_NotifyUserLogoff, *PAD_NotifyUserLogoff;

// Restore alignment.
#pragma pack(pop)

#endif	// ] Included_Packets_h
