//////////////////////////////////////////////////////////////////////////////
// GlobalPGPContext.cpp
//
// Functions for creating, deleting, and returning the global PGPContextRef.
//////////////////////////////////////////////////////////////////////////////

// $Id: GlobalPGPContext.cpp,v 1.2 1999/03/09 22:36:30 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#if defined(PGPDISK_MFC)

#include "StdAfx.h"
#include <new.h>

#elif defined(PGPDISK_95DRIVER)

#include <vtoolscp.h>
#include "PlacementNew.h"
#elif defined(PGPDISK_NTDRIVER)

#include <vdw.h>

#else
#error Define PGPDISK_MFC, PGPDISK_95DRIVER, or PGPDISK_NTDRIVER.
#endif	// PGPDISK_MFC

#include "Required.h"

#include "pgpErrors.h"
#include "pgpPublicKey.h"
#include "pgpUtilities.h"

#include "GlobalPGPContext.h"


//////////
// Globals
//////////

PGPContextRef	GlobalContext;


///////////////////////////////
// Public key utility functions
///////////////////////////////

DualErr 
CreateGlobalPGPContext()
{
	DualErr	derr;

	if (!PGPContextRefIsValid(GlobalContext))
		derr =  PGPNewContext(kPGPsdkAPIVersion, &GlobalContext);

	return derr;
}

void  
FreeGlobalPGPContext()
{
	if (PGPContextRefIsValid(GlobalContext))
	{
		PGPFreeContext(GlobalContext);
		GlobalContext = NULL;
	}
}

PGPContextRef 
GetGlobalPGPContext()
{
	return GlobalContext;
}
