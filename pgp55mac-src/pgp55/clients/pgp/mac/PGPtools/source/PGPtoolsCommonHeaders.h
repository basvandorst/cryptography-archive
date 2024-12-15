/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#pragma once

#if PGP_DEBUG
	#if __POWERPC__
		#include "PGPtoolsDebugHeadersPPC"
	#elif __CFM68K__
		#include "PGPtoolsDebugHeadersCFM68K"
	#else
		#include "PGPtoolsDebugHeaders68K"
	#endif
#else
	#if __POWERPC__
		#include "PGPtoolsHeadersPPC"
	#elif __CFM68K__
		#include "PGPtoolsHeadersCFM68K"
	#else
		#include "PGPtoolsHeaders68K"
	#endif
#endif

#define PGP_MACINTOSH				1
#define USE_PGP_OPERATOR_NEW		1
#define UNFINISHED_CODE_ALLOWED		1

#define USE_WHACKFREESPACE			PGP_DEBUG

#define USE_MAC_DEBUG_TRAPS			PGP_DEBUG

#define USE_PGP_LEAKS				PGP_DEBUG
#define USE_MAC_DEBUG_LEAKS			PGP_DEBUG
#define PGP_DEBUG_FIND_LEAKS		PGP_DEBUG


#include "PGPBuildFlags.h"
#include "pgpConfig.h"
#include "pgpLeaks.h"
#include "MacDebug.h"
