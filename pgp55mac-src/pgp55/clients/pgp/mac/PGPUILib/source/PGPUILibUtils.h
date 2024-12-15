/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	$Id: PGPUILibUtils.h,v 1.9 1997/10/28 20:45:38 heller Exp $
____________________________________________________________________________*/

#pragma once

#include <Files.h>
#include <Memory.h>
#include <Quickdraw.h>

#include "pflPrefs.h"
#include "pgpBase.h"
#include "pgpPubTypes.h"
#include "CComboError.h"

typedef struct PGPUILibState
{
	short			libResFileRefNum;

	GrafPtr			savedPort;
	PGPBoolean		savedResLoad;
	THz				savedZone;
	short			savedResFile;
	
	PGPPrefRef		clientPrefsRef;
	PGPPrefRef		adminPrefsRef;

} PGPUILibState;

PGP_BEGIN_C_DECLARATIONS

OSStatus	SetLibraryFSSpec(const FSSpec *fileSpec);

CComboError	EnterPGPUILib(PGPContextRef context, PGPUILibState *state);
void		ExitPGPUILib(const PGPUILibState *state);
	
PGP_END_C_DECLARATIONS
