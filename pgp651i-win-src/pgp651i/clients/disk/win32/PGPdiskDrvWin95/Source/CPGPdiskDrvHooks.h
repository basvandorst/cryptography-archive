//////////////////////////////////////////////////////////////////////////////
// CPGPdiskDrvHooks.h
//
// Declarations for CPGPdiskDrvHooks.cpp.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPGPdiskDrvHooks.h,v 1.4 1998/12/14 19:00:06 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CPGPdiskDrvHooks_h	// [
#define Included_CPGPdiskDrvHooks_h


////////////
// Constants
////////////

// From Winuser.h.

#define WM_DEVICECHANGE 0x0219

// From "Dbt.h".

#define DBT_VOLLOCKQUERYLOCK 0x8041


////////
// Types
////////

// From "Dbt.h".

typedef struct _DEV_BROADCAST_HDR
{
	DWORD	dbch_size;
	DWORD	dbch_devicetype;
	DWORD	dbch_reserved;

} DEV_BROADCAST_HDR, *PDEV_BROADCAST_HDR;

typedef struct VolLockBroadcast
{
	struct	_DEV_BROADCAST_HDR vlb_dbh;
	DWORD	vlb_owner;		// thread on which lock request is being issued
	BYTE	vlb_perms;		// lock permission flags defined below
	BYTE	vlb_lockType;	// type of lock
	BYTE	vlb_drive;		// drive on which lock is issued
	BYTE	vlb_flags;		// miscellaneous flags

} VolLockBroadcast, *pVolLockBroadcast;

#endif	// ] Included_CPGPdiskDrvHooks_h
