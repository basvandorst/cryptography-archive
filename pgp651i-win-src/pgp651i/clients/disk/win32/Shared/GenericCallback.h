//////////////////////////////////////////////////////////////////////////////
// GenericCallback.h
//
// The PGPdisk GenericCallback implementation.
//////////////////////////////////////////////////////////////////////////////

// $Id: GenericCallback.h,v 1.4 1998/12/14 19:00:58 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_GenericCallback_h	// [
#define Included_GenericCallback_h

#include "DualErr.h"


////////
// Types
////////

struct GenericCallbackInfo;

// PEZCALLBACK defines a pointer to a simple callback function.

typedef void (* PEZCALLBACK)(PGPUInt32);

// PGENCBFUNC defines a pointer to a GenericCallbackInfo callback function.

typedef void (* PGENCBFUNC)(GenericCallbackInfo *);

// GenericCallbackInfo is for use by certain PGPdisk classes that use
// callbacks.

typedef struct GenericCallbackInfo
{
	PGENCBFUNC	callback;		// address of callback
	PGPUInt32	refData[8];		// reference data
	DualErr		derr;			// error code

} GenericCallbackInfo, *PGenericCallbackInfo;

#endif // ] Included_GenericCallback_h
