///////////////////////////////////////////////////////////////////////////////
// PGPdiskResidentDefines.h
//
// Public defines for the PGPdiskResident project.
///////////////////////////////////////////////////////////////////////////////

// $Id: PGPdiskResidentDefines.h,v 1.3 1998/12/14 19:01:13 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_PGPdiskResidentDefines_h	// [
#define Included_PGPdiskResidentDefines_h


////////////
// Constants
////////////

const PGPUInt16 WM_PGPDISKRES_NEWPREFS = WM_USER + 197;

static LPCSTR	kPGPdiskResAppName			= "PGPdiskResident";
static LPCSTR	kPGPdiskResWindowTitle		= "PGPdiskResHiddenWindow";
static LPCSTR	kPGPdiskResSharedMemName	= "PGPdiskResidentSharedInfo";


////////
// Types
////////

// PGPdiskResInfo holds info about the PGPdiskResident application.

typedef struct PGPdiskResInfo
{
	HWND hiddenWindowHwnd;

} PGPdiskResInfo;

#endif	// ] Included_PGPdiskResidentDefines_h
