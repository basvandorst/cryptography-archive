//////////////////////////////////////////////////////////////////////////////
// PGPdiskPfl.cpp
//
// Code fixes to make the three PGPdisk projects work with a stock PFL.
//////////////////////////////////////////////////////////////////////////////

// $Id: PGPdiskPfl.cpp,v 1.7 1999/03/31 23:51:09 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

////////////////
// Fixes for MFC
////////////////

#if defined(PGPDISK_MFC)			// MFC

#include "StdAfx.h"
#include "Required.h"

// No fixes needed for MFC.

/////////////////////////////
// Fixes for the Win95 driver
/////////////////////////////

#elif defined(PGPDISK_95DRIVER)		// Win95 Driver

#include <vtoolscp.h>
#include "Required.h"
// _CrtDbgReport causes a blue-screen assert.

int	
_CrtDbgReport(
	int			arg1, 
	int			arg2, 
	int			arg3, 
	int			arg4, 
	const char	*format, 
	const char	*message)
{
	static char debugMsg[kMaxStringSize];	// don't waste stack space!

	sprintf(debugMsg, format, message);
	dprintf(debugMsg);

	SHELL_SYSMODAL_Message(Get_Sys_VM_Handle(), MB_SYSTEMMODAL, debugMsg, 
		"PGPdisk Message");
	
	return 1;
}

// _CrtDbgBreak causes an 'int 3'.

int 
_CrtDbgBreak()
{
	__asm int 3

	return 1;
}


/////////////////////////////
// Fixes for the WinNT driver
/////////////////////////////

#elif defined(PGPDISK_NTDRIVER)		// WinNT Driver

#define	__w64
#include <vdw.h>
#include <stdio.h>

#include "Required.h"

#include "KernelModeUtils.h"

// _CrtDbgReport outputs a debug string and breaks.

int	
_CrtDbgReport(
	int			arg1, 
	int			arg2, 
	int			arg3, 
	int			arg4, 
	const char	*format, 
	const char	*message)
{
	static char debugMsg[kMaxStringSize];	// don't waste stack space!

	sprintf(debugMsg, (char *) format, message);
	DbgPrint(debugMsg);

	// Pop-up dialog at passive level only.
	if (KeGetCurrentIrql() == PASSIVE_LEVEL)
	{
		KUstring uniErrString;

		AssignToUni(&uniErrString, debugMsg);

		IoRaiseInformationalHardError(IO_ERR_DRIVER_ERROR, uniErrString, 
			KeGetCurrentThread());
	}

	return 1;
}

// _CrtDbgBreak does nothing on NT.

int 
_CrtDbgBreak()
{
	return 1;
}

#else
#error Define PGPDISK_MFC, PGPDISK_95DRIVER, or PGPDISK_NTDRIVER.
#endif	// PGPDISK_MFC
