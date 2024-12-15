/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#pragma once

#include "CPGPtoolsCryptoTask.h"

class LArray;
class CPGPtoolsProcessingList;
struct ProcessingInfo;

class CPGPtoolsFileTask : public CPGPtoolsCryptoTask
{
public:
						CPGPtoolsFileTask(PGPtoolsOperation operation,
							const FSSpec *fileSpec);
	virtual				~CPGPtoolsFileTask(void);
							
	virtual	OSStatus	CalcProgressBytes(PGPContextRef context,
							ByteCount *bytes);
	virtual	CToolsError	DecryptVerify(PGPContextRef context,
								CDecodeParams *params);
	virtual	CToolsError	EncryptSign(PGPContextRef context,
								CEncodeParams *params);

protected:

	FSSpec				mSourceSpec;		// Original target file or folder
	Boolean				mSourceIsFolder;
	Boolean				mProcessInPlace;
	FSSpec				mWorkingFileSpec;	// Current child target (if any)
	FSSpec				mDetachedSigInputSpec;
	
	CPGPtoolsProcessingList	*mProcessingList;
							
	virtual	CToolsError	EncryptSignFileInternal(
							CEncodeParams 			*params,
							const ProcessingInfo	*srcInfo,
							const FSSpec 			*destSpec);
							
	virtual	CToolsError	EncryptSignFolder( CEncodeParams *params);
							
	
	virtual Boolean		ShouldEncryptSignFile( Boolean	encrypting,
							const FSSpec *spec );
							
							
	virtual Boolean		ShouldDecryptVerifyFile( const FSSpec *spec );
							
	virtual	CToolsError	DecryptVerifyFileInternal(
							CDecodeParams *			params,
							const ProcessingInfo *	srcSpecInfo,
							const FSSpec *			destSpec);
	
	virtual	CToolsError	DecryptVerifyFolder(CDecodeParams *params);
							
	virtual PGPError	DecodeDetachedSignatureEvent(
							PGPEvent 				*event,
							CDecodeEventHandlerData	*params);
							
	virtual PGPError	DecodeEventHandler(
							PGPEvent *event, CDecodeEventHandlerData *params);
								
	virtual	PGPError	DecodeOutputEvent(
							PGPEvent *event,
							CDecodeEventHandlerData *params);
							
	virtual	void		GetDecryptPassphrasePromptString(StringPtr prompt);
	
	virtual	void		GetTaskItemName(StringPtr name);
								
	virtual	void		BuildErrorMessage(CToolsError err,
							short errorStrIndex, StringPtr msg);
							
	OSStatus			GetOutputFolderSpec( Boolean encryptSign,
							const FSSpec *srcSpec,
							FSSpec * destSpec );
							
	OSStatus			GetDecryptSpec(Boolean verifying,
								const FSSpec *sourceSpec,
								FSSpec *destSpec);
};


