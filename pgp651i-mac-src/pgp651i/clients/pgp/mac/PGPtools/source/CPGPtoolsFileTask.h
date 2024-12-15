/*____________________________________________________________________________	Copyright (C) 1997-1998 Network Associates Inc. and affiliated companies.	All rights reserved.	$Id: CPGPtoolsFileTask.h,v 1.31 1999/03/10 02:38:00 heller Exp $____________________________________________________________________________*/#pragma once#include "CPGPtoolsCryptoTask.h"class LArray;class CPGPtoolsProcessingList;struct ProcessingInfo;class CPGPtoolsFileTask : public CPGPtoolsCryptoTask{public:						CPGPtoolsFileTask(PGPtoolsOperation operation,							const FSSpec *fileSpec);	virtual				~CPGPtoolsFileTask(void);								virtual	OSStatus	CalcProgressBytes(PGPContextRef context,							ByteCount *bytes);	OSStatus			CanUseFYEO(Boolean * outCanUseFYEO);	virtual	CToolsError	DecryptVerify(PGPContextRef context,								PGPtlsContextRef tlsContext,								CDecodeParams *params);	virtual	CToolsError	EncryptSign(PGPContextRef context,								PGPtlsContextRef tlsContext,								CEncodeParams *params);	void				GetSourceSpec(FSSpec *spec) { *spec = mSourceSpec; }	protected:	FSSpec				mSourceSpec;		// Original target file or folder	Boolean				mSourceIsFolder;	Boolean				mProcessInPlace;	FSSpec				mWorkingFileSpec;	// Current child target (if any)	FSSpec				mDetachedSigInputSpec;		CPGPtoolsProcessingList	*mProcessingList;								virtual	CToolsError	EncryptSignFileInternal(							CEncodeParams 			*params,							const ProcessingInfo	*srcInfo,							const FSSpec 			*destSpec);								virtual	CToolsError	EncryptSignFolder( CEncodeParams *params);									virtual Boolean		ShouldEncryptSignFile( Boolean	encrypting,							const FSSpec *spec );															virtual CToolsError	DecryptKeyShareFile(CDecodeParams *params,							const ProcessingInfo * srcSpecInfo,							const FSSpec * destSpec);	virtual	CToolsError	DecryptVerifyFileInternal(							CDecodeParams *			params,							const ProcessingInfo *	srcSpecInfo,							const FSSpec *			destSpec);		virtual	CToolsError	DecryptVerifyFolder(CDecodeParams *params);	virtual CToolsError	DecryptVerifyEncryptedFile(CDecodeParams *params,							const ProcessingInfo * srcSpecInfo,							const FSSpec * destSpec);	virtual PGPError	DecodeDetachedSignatureEvent(							PGPEvent 				*event,							CDecodeEventHandlerData	*params);								virtual PGPError	DecodeEventHandler(							PGPEvent *event, CDecodeEventHandlerData *params);									virtual	PGPError	DecodeOutputEvent(							PGPEvent *event,							CDecodeEventHandlerData *params);								virtual	void		GetDecryptPassphrasePromptString(StringPtr prompt);		virtual	void		GetTaskItemName(StringPtr name);									virtual	void		BuildErrorMessage(CToolsError err,							short errorStrIndex, StringPtr msg);								OSStatus			GetOutputFolderSpec( Boolean encryptSign,							const FSSpec *srcSpec,							FSSpec * destSpec );								OSStatus			GetDecryptSpec(Boolean verifying,								const FSSpec *sourceSpec,								FSSpec *destSpec);};