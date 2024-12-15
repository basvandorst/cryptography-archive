//////////////////////////////////////////////////////////////////////////////
// Required.h
//
// Includes for every source file in the project.
//////////////////////////////////////////////////////////////////////////////

// $Id: Required.h,v 1.5.6.1 1999/06/09 17:51:37 build Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_Required_h	// [
#define Included_Required_h

///////////////
// PFL includes
///////////////

#include "PGPdiskPfl.h"


////////////////////////
// Condition compilation
////////////////////////

#define PGPDISK_BETA_VERSION 0
#define PGPDISK_DEMO_VERSION 0

#if PGPDISK_BETA_VERSION && PGPDISK_DEMO_VERSION
#error Defined only one of PGPDISK_BETA_VERSION and PGPDISK_DEMO_VERSION.
#endif	// PGPDISK_BETA_VERSION && PGPDISK_DEMO_VERSION

#define PGPDISK_WIPE_FUNCS 1


///////////////////
// Further includes
///////////////////

#include "SystemConstants.h"
#include "PGPdiskDefaults.h"

#endif	// ] Included_Required_h
