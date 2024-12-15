/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#pragma once

#include "MacMemory.h"

#include "pgpKeys.h"
#include "pgpPubTypes.h"

#pragma warn_hidevirtual off
template class TArray<FSSpec>;
#pragma warn_hidevirtual reset

#ifdef __cplusplus
extern "C" {
#endif

#include "pgpTools.h"

OSStatus		GetFileOrClipboard(
						PGPtoolsOperation	operation,
						ConstStr255Param openButtonTitle,
						FSSpec *fsSpec, Boolean *useClipboard);
void			GetErrorString(CToolsError err, StringPtr errorStr);
Boolean			ShouldReportError(CToolsError err);
OSStatus		PromptForSignatureSourceFile(const FSSpec *sigFileSpec,
						FSSpec *sourceFileSpec);
Boolean			HaveDirWriteAccess(short vRefNum, long dirID);
OSStatus		RemoveChildAndDuplicateSpecsFromList(
						TArray<FSSpec>	*specList);

typedef struct
{
	Boolean		isEncrypted;
	Boolean		isSigned;
	Boolean		isDetachedSig;
	Boolean		isPGPFile;
	Boolean		maybePGPFile;
} FileKindInfo;


OSStatus	GetPGPFileKindInfo( const FSSpec *spec, FileKindInfo *info );


#ifdef __cplusplus
}
#endif

