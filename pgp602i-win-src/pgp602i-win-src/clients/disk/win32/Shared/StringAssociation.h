//////////////////////////////////////////////////////////////////////////////
// StringAssociation.h
//
// Contains declarations for StringAssociation.cpp.
//////////////////////////////////////////////////////////////////////////////

// $Id: StringAssociation.h,v 1.1.2.7 1998/08/04 02:10:34 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_StringAssociation_h	// [
#define Included_StringAssociation_h

#include "CommonStrings.h"
#include "DualErr.h"
#include "Errors.h"


/////////////////////
// Exported functions
/////////////////////

LPCSTR	GetCommonString(PGPUInt32 StringId, 
			LPCSTR defaultString = kPGPdiskUnknownString);

void	FormatErrorString(PGDMajorError perr, DualErr derr, PGPUInt8 drive, 
			LPSTR outString, PGPUInt32 sizeOutString);

#endif // Included_StringAssociation_h
