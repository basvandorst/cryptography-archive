/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	$Id: pgpSDKUILibPriv.h,v 1.5 1999/03/10 02:55:52 heller Exp $
____________________________________________________________________________*/

#ifndef Included_PGPsdkUILibPriv_h
#define Included_PGPsdkUILibPriv_h

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
// Linker deals with C and C++ variables differently, so
// we have to extern "C" the global to keep things sane -wjb
#ifdef __cplusplus
extern "C" HINSTANCE gPGPsdkUILibInst;
#else
extern HINSTANCE gPGPsdkUILibInst;
#endif
#endif

PGPError	pgpForceSDKUILibCleanup(void);

#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif

PGP_END_C_DECLARATIONS

#endif	/* Included_PGPsdkUILibPriv_h */
