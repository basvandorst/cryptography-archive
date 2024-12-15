/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#pragma once

#include "pgpErrors.h"
#include "pgpEncode.h"
#include "pgpKeys.h"

#include "pgpUserInterface.h"

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif


typedef PGPError (*PGPSharedForEackKeyDoProcPtr)( PGPKeyIterRef keyIterator,
							PGPKeyRef theKey, void *userData );


typedef struct PGPSharedKeyProperties
{
	PGPBoolean	isSecret;
	PGPBoolean	isAxiomatic;
	PGPBoolean	isRevoked;
	PGPBoolean	isDisabled;
	PGPBoolean	isCorrupt;
	PGPBoolean	isExpired;
	PGPBoolean	canSign;		// Special ViaCrypt property
	PGPBoolean	canEncrypt;		// Special ViaCrypt property

	PGPValidity	userValidity;	// Valid for a particular user ID only
	
} PGPSharedKeyProperties;



PGPError	CountKeysInDefaultKeyring(PGPContextRef context,
						UInt32 *numPublicKeys, UInt32 *numPrivateKeys);
	
PGPError	PGPSharedForEachKeyDo( PGPContextRef context,
						PGPKeyOrdering keyOrdering,
						PGPSharedForEackKeyDoProcPtr callbackProc,
						void *callbackData);
	
PGPError	PGPSharedGetKeyProperties(PGPKeyRef theKey, const char *userID,
						PGPSharedKeyProperties *properties);
						
PGPError	PGPSharedAddKeysToDefaultKeyring(PGPKeySetRef keysToAdd);
	
PGPError	PGPSharedGetIndKeyFromKeySet(PGPKeySetRef keySet,
						PGPKeyOrdering keyOrdering, PGPUInt32 searchIndex,
						PGPKeyRef *theKey);

	
#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS