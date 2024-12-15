//////////////////////////////////////////////////////////////////////////////
// DualErr.cpp
//
// Holds static data members of class DualErr.
//////////////////////////////////////////////////////////////////////////////

// $Id: DualErr.cpp,v 1.7 1999/03/31 23:51:09 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#if defined(PGPDISK_MFC)

#include "StdAfx.h"

#elif defined(PGPDISK_95DRIVER)

#include <vtoolscp.h>

#elif defined(PGPDISK_NTDRIVER)

#define	__w64
#include <vdw.h>

#else
#error Define PGPDISK_MFC, PGPDISK_95DRIVER, or PGPDISK_NTDRIVER.
#endif	// PGPDISK_MFC

#include "Required.h"
#include "DualErr.h"


////////////////////////////
// Globals for class DualErr
////////////////////////////

DualErr DualErr::NoError = DualErr();
