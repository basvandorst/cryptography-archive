/*____________________________________________________________________________
	PGPUILibDebugHeaders.prefix
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.

	$Id: PGPUILibCommonHeaders.h,v 1.9.6.1 1997/11/20 20:41:44 heller Exp $
____________________________________________________________________________*/

#pragma once

/* we have to do our own, to get rid of bool option in standard ones */
#if PGP_DEBUG

	#if __POWERPC__	
		#include "PGPUILibDebugHeadersPPC"
	#elif __CFM68K__
		#include "PGPUILibDebugHeadersCFM68K"
	#endif

#else

	#if __POWERPC__	
		#include "PGPUILibHeadersPPC"
	#elif __CFM68K__
		#include "PGPUILibHeadersCFM68K"
	#endif

#endif

/* Common flags: */
#define UNFINISHED_CODE_ALLOWED		1

#define USE_WHACKFREESPACE			PGP_DEBUG

#define USE_MAC_DEBUG_TRAPS			PGP_DEBUG
#define USE_MAC_DEBUG_LEAKS			PGP_DEBUG

#define USE_PGP_LEAKS				PGP_DEBUG
#define PGP_DEBUG_FIND_LEAKS		PGP_DEBUG

#define USE_PGP_OPERATOR_NEW		1

#define PGP_MACINTOSH				1

#define MSL_USE_PRECOMPILED_HEADERS	0

/* PGPUILib flags: */

#define KEYSERVER_SUPPORT			GENERATINGPOWERPC

/* Common headers: */

#include "ansi_prefix.mac.h"

#include "pgpConfig.h"
#include "PGPBuildFlags.h"
#include "MacDebug.h"

