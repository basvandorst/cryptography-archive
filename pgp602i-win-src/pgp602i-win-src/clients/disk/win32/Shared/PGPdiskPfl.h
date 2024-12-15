//////////////////////////////////////////////////////////////////////////////
// PGPdiskPfl.h
//
// Declarations for PGPdiskPfl.cpp.
//////////////////////////////////////////////////////////////////////////////

// $Id: PGPdiskPfl.h,v 1.1.2.13 1998/08/04 02:10:21 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_PGPdiskPfl_h	// [
#define Included_PGPdiskPfl_h


////////////////
// Fixes for MFC
////////////////

#if defined(PGPDISK_MFC)			// MFC

// No fixes needed for MFC.


/////////////////////////////
// Fixes for the Win95 driver
/////////////////////////////

#elif defined(PGPDISK_95DRIVER)		// Win95 Driver

// We need to 'roll our own' _CrtDbgReport.
extern "C" int _CrtDbgReport(int arg1, int arg2, int arg3, int arg4, 
				const char *format, const char* message);

// We need to 'roll our own' _CrtDbgBreak.
extern "C" int	_CrtDbgBreak();


/////////////////////////////
// Fixes for the WinNT driver
/////////////////////////////

#elif defined(PGPDISK_NTDRIVER)

#include "OperatorNewFixups.h"

// We need to 'roll our own' _CrtDbgReport.
int	_CrtDbgReport(int arg1, int arg2, int arg3, int arg4, 
		const char *format, const char* message);

// We need to 'roll our own' _CrtDbgBreak.
int	_CrtDbgBreak();

#else
#error Define PGPDISK_MFC, PGPDISK_95DRIVER, or PGPDISK_NTDRIVER.
#endif	// PGPDISK_MFC


///////////////
// Common fixes
///////////////

#include "pgpBase.h"
#include "pgpTypes.h"
#include "pgpDebug.h"
#include "pgpLeaks.h"
#include "pgpMem.h"

// VC doesn't allow structure definitions within parentheses. 'Alignof' dies.
#undef	alignof
#define	alignof(type) (1)

#endif	// ] Included_PGPdiskPfl_h
