//////////////////////////////////////////////////////////////////////////////
// KernelModeUtils.h
//
// Declarations for KernelModeUtils.cpp.
//////////////////////////////////////////////////////////////////////////////

// $Id: KernelModeUtils.h,v 1.1.2.7 1998/07/06 08:57:49 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_KernelModeUtils_h	// [
#define Included_KernelModeUtils_h

#include "DualErr.h"


////////
// Types
////////

// NameAssoc is used by the debug routines to associate strings to constants.

typedef struct NameAssoc
{
	PGPUInt32	func;
	LPCSTR		name;

} NameAssoc;


/////////////////////
// Exported functions
/////////////////////

DualErr		UniToAnsi(KUstring *uniString, LPSTR *string);

DualErr		AssignToUni(KUstring *outUniString, 
				PUNICODE_STRING inUniString);
DualErr		AssignToUni(KUstring *outUniString, 
				LPCWSTR inUniString);
DualErr		AssignToUni(KUstring *outUniString, 
				LPCSTR inString);

DualErr		AppendToUni(KUstring *outUniString, 
				PUNICODE_STRING inUniString);
DualErr		AppendToUni(KUstring *outUniString, 
				LPCWSTR inUniString);
DualErr		AppendToUni(KUstring *outUniString, 
				LPCSTR inString);

DualErr		PrependToUni(KUstring *outUniString, 
				PUNICODE_STRING inUniString);
DualErr		PrependToUni(KUstring *outUniString, 
				LPCWSTR inUniString);
DualErr		PrependToUni(KUstring *outUniString, 
				LPCSTR inString);

LPCSTR		GetName(NameAssoc nameTable[], PGPUInt32 n, PGPUInt32 func);
LPCSTR		GetADPacketName(PGPUInt32 code);
LPCSTR		GetIOCTLFunctionName(PGPUInt32 ioctlCode);
LPCSTR		GetIRPMajorFunctionName(PGPUInt8 majorFunc);

DualErr		MakePathToDrive(PGPUInt8 drive, KUstring *outPath);
PGPBoolean	IsValidDeviceName(PUNICODE_STRING deviceName);

PGPBoolean	IsFileInUseByReader(LPCSTR path);
PGPBoolean	IsFileInUseByWriter(LPCSTR path);
PGPBoolean	IsFileInUse(LPCSTR path);


#endif	// ] Included_KernelModeUtils_h
