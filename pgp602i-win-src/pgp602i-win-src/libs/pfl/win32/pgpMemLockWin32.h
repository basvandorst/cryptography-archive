/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	pgpMemLockWin32.h - header for Win32 PGPmemlock drivers
	

	$Id: pgpMemLockWin32.h,v 1.1.16.1 1998/11/12 03:20:15 heller Exp $
____________________________________________________________________________*/

#if ! PGP_WIN32
#error this file should only be used for PGP_WIN32
#endif

/* driver-specific error codes */
#define kPGPMemLockError_LockError		0x0001
#define kPGPMemLockError_UnlockError	0x0002
#define kPGPMemLockError_ListError		0x0003
#define kPGPMemLockError_AllocError		0x0004

/* virtual memory paging in Win32 uses 4K byte pages (2^12) */
#define WIN32PAGESIZE	12	

/* Define the various device type values.  Note that values used by Microsoft
   Corporation are in the range 0-32767, and 32768-65535 are reserved for use
   by customers. */

#define FILE_DEVICE_PGPMEMLOCK  0x00008000

/* Macro definition for defining IOCTL and FSCTL function control codes.  Note
   that function codes 0-2047 are reserved for Microsoft Corporation, and
   2048-4095 are reserved for customers. */

#define PGPMEMLOCK_IOCTL_INDEX  0x800

/* Define our own private IOCTL */

#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)
#define METHOD_BUFFERED                 0
#define FILE_ANY_ACCESS                 0

#define IOCTL_PGPMEMLOCK_LOCK_MEMORY	CTL_CODE(FILE_DEVICE_PGPMEMLOCK, \
                                                 PGPMEMLOCK_IOCTL_INDEX,  \
                                                 METHOD_BUFFERED,     \
                                                 FILE_ANY_ACCESS)

#define IOCTL_PGPMEMLOCK_UNLOCK_MEMORY	CTL_CODE(FILE_DEVICE_PGPMEMLOCK,  \
                                                 PGPMEMLOCK_IOCTL_INDEX+1,\
                                                 METHOD_BUFFERED,     \
                                                 FILE_ANY_ACCESS)

/* Our user mode app will pass an initialized structure like this
   down to the VxD or kernel mode driver */

#pragma pack(8)
typedef struct {
	PVOID	pMem;
	ULONG	ulNumBytes;
	ULONG	ulError;
} PGPMEMLOCKSTRUCT, *PPGPMEMLOCKSTRUCT;
#pragma pack()
