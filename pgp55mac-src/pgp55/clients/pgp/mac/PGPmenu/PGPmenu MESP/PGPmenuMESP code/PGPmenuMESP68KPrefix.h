/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: PGPmenuMESP68KPrefix.h,v 1.3.8.1 1997/12/05 22:14:13 mhw Exp $
____________________________________________________________________________*/

#include <MacHeaders.h>

#define PGP_DEBUG					0

#define USE_PGP_OPERATOR_NEW		1
#define UNFINISHED_CODE_ALLOWED		1

#define USE_WHACKFREESPACE			PGP_DEBUG

#define USE_MAC_DEBUG_TRAPS			PGP_DEBUG

#define USE_PGP_LEAKS				PGP_DEBUG
#define USE_MAC_DEBUG_LEAKS			PGP_DEBUG
#define PGP_DEBUG_FIND_LEAKS		PGP_DEBUG

#include "MacBasics.h"
#include "PGPBuildFlags.h"
#include "pgpConfig.h"
#include "pgpLeaks.h"
#include "MacDebug.h"

#define CFM_MESPTarget_	(kM68kISA | kCFM68kRTA)
