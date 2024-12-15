/*____________________________________________________________________________
	pgpEvent.h
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	This file contains the types and prototypes for functions which manipulate
	PGPEvent data structures

	$Id: pgpEventPriv.h,v 1.15 1997/10/08 01:47:58 lloyd Exp $
____________________________________________________________________________*/

#ifndef Included_pgpEvent_h	/* [ */
#define Included_pgpEvent_h

#include "pgpPubTypes.h"
#include "pgpEncode.h"


PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

PGPError 		pgpEventNull(PGPContextRef context,
							PGPOptionListRef *newOptionList,
							PGPEventHandlerProcPtr func,
							PGPUserValue userValue,
							PGPFileOffset soFar, PGPFileOffset total);

PGPError 		pgpEventInitial(PGPContextRef context,
							PGPOptionListRef *newOptionList,
							PGPEventHandlerProcPtr func,
							PGPUserValue userValue);

PGPError 		pgpEventFinal(PGPContextRef context,
							PGPOptionListRef *newOptionList,
							PGPEventHandlerProcPtr func,
							PGPUserValue userValue);

PGPError 		pgpEventError(PGPContextRef context,
							PGPOptionListRef *newOptionList,
							PGPEventHandlerProcPtr func,
							PGPUserValue userValue,
							PGPError error, void *errorArg);

PGPError 		pgpEventWarning(PGPContextRef context,
							PGPOptionListRef *newOptionList,
							PGPEventHandlerProcPtr func,
							PGPUserValue userValue,
							PGPError warning, void *warningArg);

PGPError 		pgpEventEntropy(PGPContextRef context,
							PGPOptionListRef *newOptionList,
							PGPEventHandlerProcPtr func,
							PGPUserValue userValue,
							PGPUInt32 entropyBitsNeeded);

PGPError 		pgpEventPassphrase(PGPContextRef context,
							PGPOptionListRef *newOptionList,
							PGPEventHandlerProcPtr func,
							PGPUserValue userValue,
							PGPBoolean fConventional, PGPKeySetRef keyset);

PGPError 		pgpEventAnalyze(PGPContextRef context,
							PGPOptionListRef *newOptionList,
							PGPEventHandlerProcPtr func,
							PGPUserValue userValue,
							PGPAnalyzeType type);

PGPError 		pgpEventRecipients(PGPContextRef context,
							PGPOptionListRef *newOptionList,
							PGPEventHandlerProcPtr func,
							PGPUserValue userValue,
							PGPKeySetRef recipientSet,
							PGPKeyID *keyIDArray,
							PGPUInt32 passPhraseCount,
							PGPUInt32 keyCount);

PGPError 		pgpEventKeyFound(PGPContextRef context,
							PGPOptionListRef *newOptionList,
							PGPEventHandlerProcPtr func,
							PGPUserValue userValue,
							PGPKeySetRef keySet);

PGPError 		pgpEventOutput(PGPContextRef context,
							PGPOptionListRef *newOptionList,
							PGPEventHandlerProcPtr func,
							PGPUserValue userValue,
							PGPUInt32 messageType,
							char const *suggestedName,
							PGPBoolean FYEO);

PGPError 		pgpEventSignature(PGPContextRef context,
							PGPOptionListRef *newOptionList,
							PGPEventHandlerProcPtr func,
							PGPUserValue userValue,
							PGPKeyID const * signingKeyID,
							PGPKeyRef signingKey, PGPBoolean checked,
							PGPBoolean verified, PGPBoolean keyDisabled,
							PGPBoolean keyRevoked, PGPBoolean keyExpired,
							PGPBoolean keyValidityThreshold,
							PGPValidity keyValidity,
							PGPTime creationTime);

PGPError 		pgpEventBeginLex(PGPContextRef context,
							PGPOptionListRef *newOptionList,
							PGPEventHandlerProcPtr func,
							PGPUserValue userValue,
							PGPUInt32 sectionNumber,
							PGPSize sectionOffset);

PGPError 		pgpEventEndLex(PGPContextRef context,
							PGPOptionListRef *newOptionList,
							PGPEventHandlerProcPtr func,
							PGPUserValue userValue,
							PGPUInt32 sectionNumber);

PGPError 		pgpEventDetachedSignature(PGPContextRef context,
							PGPOptionListRef *newOptionList,
							PGPEventHandlerProcPtr func,
							PGPUserValue userValue);

PGPError 		pgpEventKeyGen(PGPContextRef context,
							PGPOptionListRef *newOptionList,
							PGPEventHandlerProcPtr func,
							PGPUserValue userValue,
							PGPUInt32 state);

PGPError 		pgpEventKeyServer(PGPContextRef context,
							PGPOptionListRef *newOptionList,
							PGPEventHandlerProcPtr func,
							PGPUserValue userValue,
							PGPUInt32 state,
							PGPUInt32 soFar,
							PGPUInt32 total);

PGPError 		pgpEventKeyServerSign(PGPContextRef context,
							PGPOptionListRef *newOptionList,
							PGPEventHandlerProcPtr func,
							PGPUserValue userValue,
							PGPUInt32 state);

#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpEvent_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
