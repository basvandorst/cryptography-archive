//////////////////////////////////////////////////////////////////////////////
// PGPdiskRegistry.h
//
// Declarations for PGPdiskRegistry.cpp.
//////////////////////////////////////////////////////////////////////////////

// $Id: PGPdiskRegistry.h,v 1.5 1998/12/15 01:25:05 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_PGPdiskRegistry_h	// [
#define Included_PGPdiskRegistry_h

#include "DualErr.h"


/////////////////////
// Exported Functions
/////////////////////

DualErr		PGPdiskEnumSubkeys(HKEY root, LPCSTR section, 
				PGPUInt32 index, LPSTR keyName, 
				unsigned long valueSize, PGPBoolean *isIndexValid);
DualErr		PGPdiskEnumValues(HKEY root, LPCSTR section, 
				PGPUInt32 index, LPSTR valueName, 
				unsigned long valueSize, unsigned long *valueType, 
				PGPBoolean *isIndexValid);

DualErr		PGPdiskGetRawKey(HKEY root, LPCSTR section, 
				LPCSTR keyName, PGPUInt8 *buf, unsigned long bufSize);

DualErr		PGPdiskCreateAndSetRawKey(HKEY root, LPCSTR section, 
				LPCSTR keyName, const PGPUInt8 *value, 
				PGPUInt32 valueSize, PGPUInt32 dataType);

DualErr		PGPdiskDeleteSubkey(HKEY root, LPCSTR section, 
				LPCSTR keyName);
DualErr		PGPdiskDeleteValue(HKEY root, LPCSTR section, 
				LPCSTR valueName);

#endif	// ] Included_PGPdiskRegistry_h
