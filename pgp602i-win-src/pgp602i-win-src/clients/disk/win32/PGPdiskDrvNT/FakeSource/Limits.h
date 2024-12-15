//////////////////////////////////////////////////////////////////////////////
// Limits.h
//
// A fake source file designed to pacify the PFL.
//////////////////////////////////////////////////////////////////////////////

// $Id: Limits.h,v 1.1.2.5 1998/05/22 08:35:28 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_Limits_h	// [
#define Included_Limits_h

#include <vdw.h>
//#include "CRunTimeExtras.h"

// Stuff to get the PFL to work.

#undef	alignof
#define	alignof(type) (1)

// We need to 'roll our own' _CrtDbgReport.
int _CrtDbgReport(int arg1, int arg2, int arg3, int arg4, const char *format, 
		const char* message);

// We need to 'roll our own' _CrtDbgBreak.
int	_CrtDbgBreak();

#endif	// ] Included_Limits_h