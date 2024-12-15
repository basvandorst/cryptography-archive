/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	$Id: pgpSDKNetworkLibPriv.h,v 1.3 1999/03/10 02:55:49 heller Exp $
____________________________________________________________________________*/

#ifndef Included_PGPsdkNetworkLibPriv_h
#define Included_PGPsdkNetworkLibPriv_h

#include "pgpConfig.h"
#include "pgpBase.h"

#if PGP_WIN32
#include <windows.h>
#endif

PGP_BEGIN_C_DECLARATIONS

#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

#if PGP_WIN32
extern CRITICAL_SECTION		gKeyServerInitMutex;
extern CRITICAL_SECTION		gSocketsInitMutex;
#endif

PGPError	pgpForceSDKNetworkLibCleanup(void);

#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif

PGP_END_C_DECLARATIONS

#endif	/* Included_PGPsdkNetworkLibPriv_h */
