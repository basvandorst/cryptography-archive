/*____________________________________________________________________________
	DebugTraps.c: includes all general purpose debugging code.
____________________________________________________________________________*/
#pragma once

#include "MacDebugTraps.h"

#ifndef USE_MAC_DEBUG_PATCHES
	#define USE_MAC_DEBUG_PATCHES		USE_MAC_DEBUG_TRAPS
#endif

#if USE_MAC_DEBUG_PATCHES

	void	DebugPatches_PatchDisposeTraps( void );
	Boolean	DebugPatches_DisposeTrapsPatched(  void );

#else

	#define DebugPatches_PatchDisposeTraps()
	#define DebugPatches_DisposeTrapsPatched()	( FALSE )

#endif