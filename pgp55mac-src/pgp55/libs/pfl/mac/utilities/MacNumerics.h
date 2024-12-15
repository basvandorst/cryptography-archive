/*____________________________________________________________________________
	Copyright (C) 1996,1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	$Id: MacNumerics.h,v 1.7 1997/09/11 05:46:12 lloyd Exp $
____________________________________________________________________________*/

#pragma once

#include "pgpBase.h"
#include "pgpTypes.h"

typedef SInt16	CRC16;
typedef SInt32	CRC32;

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif



void	BCDToDecimal(UInt32 bcdValue, UInt32 *decimalValue);

CRC16	ComputeCRC16(const void *theBytes, UInt32 numBytes);
CRC16	ContinueCRC16(const void *theBytes, UInt32 numBytes, CRC16 startCRC);

CRC32	ComputeCRC32(const void *theBytes, UInt32 numBytes);
CRC32	ContinueCRC32(const void *theBytes, UInt32 numBytes, CRC32 startCRC);



#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS