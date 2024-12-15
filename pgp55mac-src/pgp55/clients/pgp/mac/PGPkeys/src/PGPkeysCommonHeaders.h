/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: PGPkeysCommonHeaders.h,v 1.8 1997/09/18 02:27:23 heller Exp $
____________________________________________________________________________*/
#pragma once

#if PGP_DEBUG
	#if __POWERPC__
		#include "PGPkeysDebugHeadersPPC"
	#elif __CFM68K__
		#include "PGPkeysDebugHeadersCFM68K"
	#else
		#include "PGPkeysDebugHeaders68K"
	#endif
#else
	#if __POWERPC__
		#include "PGPkeysHeadersPPC"
	#elif __CFM68K__
		#include "PGPkeysHeadersCFM68K"
	#else
		#include "PGPkeysHeaders68K"
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

#include "pgpBuildFlags.h"
#include "pgpConfig.h"
#include "pgpLeaks.h"
#include "MacDebug.h"

