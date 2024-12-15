/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#pragma once

#include "pgpPubTypes.h"

enum
{
	kPGPtoolsPreferencesVersion	= 1
};

typedef struct PGPtoolsPreferences
{
	ushort	version;

	Point	mainWindowLocation;
	Rect	resultsWindowBounds;
	Boolean	smallMainWindow;
	
} PGPtoolsPreferences;

#ifdef __cplusplus
extern "C" {
#endif

PGPError		LoadPreferences(PGPContextRef context);
PGPError		SavePreferences( void );

#ifdef __cplusplus
}
#endif

extern PGPtoolsPreferences	gPreferences;
