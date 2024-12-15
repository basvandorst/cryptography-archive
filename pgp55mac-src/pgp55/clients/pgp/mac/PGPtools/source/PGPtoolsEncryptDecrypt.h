/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#pragma once

#include "MacCursors.h"

#include "PGPSharedEncryptDecrypt.h"

const OSType	kPGPMacFileTypeMacPGPEncryptedData = 'Cryp';

typedef struct AnimatedCursorAnalyzeCallbackInfo
{
	AnimatedCursorRef	animatedCursor;
	Boolean				userAborted;

} AnimatedCursorAnalyzeCallbackInfo;

void		GetDefaultEncryptSignName(ConstStringPtr srcName,
						Boolean textOutput, Boolean detachedSignature,
						StringPtr destName );
void		GetDefaultEncryptSignFSSpec(const FSSpec *sourceSpec,
						Boolean textOutput, Boolean detachedSignature,
						FSSpec *destSpec);
Boolean		GetDefaultDecryptVerifyFSSpec(const FSSpec *sourceSpec,
						FSSpec *destSpec);
Boolean		GetDefaultDecryptVerifyName(ConstStringPtr srcName,
						StringPtr destname );
Boolean		GetDetachedSigFileSourceSpec(const FSSpec *sigSpec,
						FSSpec *sourceSpec);
PGPError	AnimatedCursorAnalyzeCallbackProc(void *arg, size_t soFar,
						size_t total);
