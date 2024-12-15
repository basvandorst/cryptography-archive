//////////////////////////////////////////////////////////////////////////////
// WindowsVersion.h
//
// Small inline utility functions to help with determing the Windows version.
//////////////////////////////////////////////////////////////////////////////

// $Id: WindowsVersion.h,v 1.1.2.8.2.1 1998/08/21 05:57:01 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_WindowsVersion_h	// [
#define Included_WindowsVersion_h


/////////////////////
// Exported Functions
/////////////////////

PGPBoolean	IsWin95CompatibleMachine();
PGPBoolean	IsWin95OSR2CompatibleMachine();
PGPBoolean	IsWin98CompatibleMachine();
PGPBoolean	IsWinNT4CompatibleMachine();
PGPBoolean	IsWinNT5CompatibleMachine();

#endif	// ] Included_WindowsVersion_h
