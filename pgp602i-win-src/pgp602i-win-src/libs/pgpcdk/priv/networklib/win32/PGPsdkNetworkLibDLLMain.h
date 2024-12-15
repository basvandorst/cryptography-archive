/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	
	
	$Id: PGPsdkNetworkLibDLLMain.h,v 1.2.10.1 1998/11/12 03:22:56 heller Exp $
____________________________________________________________________________*/

#ifndef Included_PGPsdkNetworkLibDLLMain_h
#define Included_PGPsdkNetworkLibDLLMain_h

#include "pgpBase.h"

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

extern CRITICAL_SECTION		gKeyServerInitMutex;
extern CRITICAL_SECTION		gSocketsInitMutex;

#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS

#endif
