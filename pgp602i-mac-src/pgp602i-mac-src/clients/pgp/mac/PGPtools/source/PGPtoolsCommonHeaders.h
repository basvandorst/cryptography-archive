/*____________________________________________________________________________	Copyright (C) 1997-1998 Network Associates, Inc. and its affiliates.	All rights reserved.	$Id: PGPtoolsCommonHeaders.h,v 1.12.8.1 1998/11/12 03:09:29 heller Exp $____________________________________________________________________________*/#pragma once#if __POWERPC__	#include "MacHeadersPPC++"#elif __CFM68K__	#include "MacHeadersCFM68K++"#else	#error Cannot compile in this environment#endif#include <Sound.h>	// Needed for PP file LTextEditView.cp#define PGP_MACINTOSH				1#define USE_PGP_OPERATOR_NEW		1#define UNFINISHED_CODE_ALLOWED		1#define USE_MAC_DEBUG_TRAPS					PGP_DEBUG#define USE_MAC_DEBUG_PATCHES				PGP_DEBUG#define MAC_DEBUG_PATCHES_WHACK_FREE_SPACE	PGP_DEBUG#define USE_PGP_LEAKS				PGP_DEBUG#define USE_MAC_DEBUG_LEAKS			PGP_DEBUG#define PGP_DEBUG_FIND_LEAKS		PGP_DEBUG#include "PGPBuildFlags.h"#include "pgpConfig.h"#include "pgpLeaks.h"#include "MacDebug.h"