/*____________________________________________________________________________
	Copyright (C) 1996,1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	$Id: MacTraps.h,v 1.5 1997/09/18 01:34:57 lloyd Exp $
____________________________________________________________________________*/

#pragma once

#include <OSUtils.h>

#include "pgpBase.h"

PGP_BEGIN_C_DECLARATIONS

short		NumToolboxTraps(void);
TrapType	GetTrapType(short theTrap);
Boolean		TrapAvailable(short theTrap);
Boolean		PFLPatchTrap(short trapNum,
				UniversalProcPtr newAddr, UniversalProcPtr *oldAddr);

PGP_END_C_DECLARATIONS

