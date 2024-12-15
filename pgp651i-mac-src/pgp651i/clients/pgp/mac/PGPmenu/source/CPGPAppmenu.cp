/*____________________________________________________________________________	Copyright (C) 1997 Network Associates Inc. and affiliated companies.	All rights reserved.				$Id: CPGPAppmenu.cp,v 1.7.4.2.2.1.2.1 1999/07/09 00:02:39 heller Exp $____________________________________________________________________________*/#include <Scrap.h>#include <LFile.h>#include "pgpFeatures.h"#include "pgpMacCustomContextAlloc.h"#include "pgpSDKPrefs.h"#include "pgpMem.h"#include "StSaveCurResFile.h"#include "StSaveHeapZone.h"#include "StPGPRefs.h"#include "CPGPAppmenu.h"namespace {	const PGPInt8	event_EncodeDecode		=	0;	const PGPInt8	event_SuspendKey		=	1;	const PGPInt8	event_ResumeKey			=	2;	const PGPInt8	event_PutScrap			=	3;	const PGPInt8	event_Cleanup			=	4;	const PGPInt8	kNumEvents				=	5;		const PGPInt8	state_Idle				=	0;	const PGPInt8	state_ZeroScrap			=	1;	const PGPInt8	state_PutScrap			=	2;	const PGPInt8	state_SelectAllPutScrap	=	3;	const PGPInt8	state_Processing		=	4;	const PGPInt8	state_Cleanup			=	5;	const PGPInt8	kNumStates				=	6;		const PGPInt8	kStateTable[kNumStates][kNumEvents] = 	{	/*			ED	SK	RK	PS	C	*/	{	/* I    */	1,	-2,	-2,	-1,	5	},	{	/* ZS   */	-2,	-1,	2,	-1,	5	},	{	/* PS   */	-2,	-1,	3,	4,	5	},	{	/* SAPS */	-2, -1, 5,	4,	5	},	{	/* P    */	-2,	-1,	-1, -1,	5	},	{	/* C    */	-1,	-1,	0,	-1,	-1	}	};	const SInt32	cmdKey_Suspend			=	1;	const SInt32	cmdKey_Resume			=	2;	const SInt32	cmdKey_Copy				=	0x0863;	const SInt32	cmdKey_SelectAll		=	0x0061;	const SInt32	cmdKey_Paste			=	0x0976;	const KeyMap	kCopyKeyMap				=	{	0x00010000,													0x00008000,													0x00000000,													0x00000000};	const KeyMap	kSelectAllKeyMap		=	{	0x01000000,													0x00008000,													0x00000000,													0x00000000};	const KeyMap	kPasteKeyMap			=	{	0x00020000,													0x00008000,													0x00000000,													0x00000000};}CPGPAppmenu::CPGPAppmenu(	FSSpec *			inFSSpec,	SPGPmenuMESPData *	inMESPData)		: CPGPmenu(inFSSpec), mContext(kInvalidPGPContextRef),			mMemoryMgr(kInvalidPGPMemoryMgrRef), mTLSContext(kInvalidPGPtlsContextRef),			mClientEncodeDecodeData(kInvalidPGPClientEncodeDecodeDataRef),			mUsesOutputDialog(inMESPData->usesOutputDialog), mState(0), mSaveRestoreScrap(0),			mPatchEventAvail(true), mPatchGetKeys(true), mPatchPutScrap(true), mData(0){}CPGPAppmenu::~CPGPAppmenu(){	if (mData != 0) {		PGPFreeData(mData);	}	delete mSaveRestoreScrap;	if (PGPClientEncodeDecodeDataRefIsValid(mClientEncodeDecodeData)) {		PGPCleanupClientEncodeDecode(mClientEncodeDecodeData);	}	if (PGPtlsContextRefIsValid(mTLSContext)) {		PGPFreeTLSContext(mTLSContext);	}	if (PGPContextRefIsValid(mContext)) {		PGPFreeContext(mContext);	}}	voidCPGPAppmenu::Initialize(){	PGPError			pgpErr;	PGPFlags			featureFlags;	pgpErr = pgpNewContextCustomMacAllocators(&mContext);	PGPThrowIfPGPError_(pgpErr);	mMemoryMgr = PGPGetContextMemoryMgr(mContext);	pgpErr = PGPsdkLoadDefaultPrefs(mContext);	PGPThrowIfPGPError_(pgpErr);	pgpErr = PGPNewTLSContext(mContext, &mTLSContext);	PGPThrowIfPGPError_(pgpErr);		pgpErr = PGPGetFeatureFlags(kPGPFeatures_GeneralSelector, &featureFlags);	PGPThrowIfPGPError_(pgpErr);	mEncryptEnabled = PGPFeatureExists(featureFlags, kPGPFeatureMask_CanEncrypt);	mSignEnabled = PGPFeatureExists(featureFlags, kPGPFeatureMask_CanSign);	mDecryptEnabled = PGPFeatureExists(featureFlags, kPGPFeatureMask_CanDecrypt);	mVerifyEnabled = PGPFeatureExists(featureFlags, kPGPFeatureMask_CanVerify);	#if PGP_BUSINESS_SECURITY	pgpErr = PGPCheckAutoUpdateKeysFromServer(mMemoryMgr, true, 0, 0);	PGPThrowIfPGPError_(pgpErr);#endif	CPGPmenu::Initialize();	}	voidCPGPAppmenu::Patch(){	if (mPatchEventAvail) {		new CPGPAppmenuEventAvailPatch;	}	if (mPatchGetKeys) {		new CPGPAppmenuGetKeysPatch;	}	if (mPatchPutScrap) {		new CPGPAppmenuPutScrapPatch;	}	CPGPmenu::Patch();}	voidCPGPAppmenu::Sign(){	DoOperation(op_Sign);}	voidCPGPAppmenu::Encrypt(){	DoOperation(op_Encrypt);}	voidCPGPAppmenu::EncryptAndSign(){	DoOperation(op_EncryptAndSign);}	voidCPGPAppmenu::DecryptVerify(){	DoOperation(op_DecryptVerify);}	voidCPGPAppmenu::ClearPassphraseCaches(){	PGPError	pgpErr;		pgpErr = PGPClearClientEncodeDecodeCaches(mClientEncodeDecodeData, true, true);	PGPThrowIfPGPError_(pgpErr);	CPGPmenu::ClearPassphraseCaches();}	voidCPGPAppmenu::UpdatePrefs(){	PGPError	pgpErr;		CPGPmenu::UpdatePrefs();	if (PGPClientEncodeDecodeDataRefIsValid(mClientEncodeDecodeData)) {		PGPCleanupClientEncodeDecode(mClientEncodeDecodeData);		mClientEncodeDecodeData = kInvalidPGPClientEncodeDecodeDataRef;	}	pgpErr = PGPInitClientEncodeDecode(mContext, mTLSContext, &mClientEncodeDecodeData);	PGPThrowIfPGPError_(pgpErr);}	voidCPGPAppmenu::AppSign(	const char *	inText,	PGPSize			inTextSize,	char **			outText,	PGPSize * 		outTextSize){	PGPError		pgpErr;		pgpErr = PGPClientEncode(mClientEncodeDecodeData, true, kPGPUISettingsTextOutput,				kPGPGetPassphraseOptionsHideFileOptions, true, 				true, true, false, 0, 0, 0, 0, false,				reinterpret_cast<const PGPByte *>(inText), inTextSize,				reinterpret_cast<PGPByte **>(outText), outTextSize);	PGPThrowIfPGPError_(pgpErr);}	voidCPGPAppmenu::AppEncrypt(	PGPUInt32			inNumRecipients,	PGPRecipientSpec *	inRecipients,	const char *		inText,	PGPSize				inTextSize,	char **				outText,	PGPSize * 			outTextSize){	PGPError		pgpErr;		pgpErr = PGPClientEncode(mClientEncodeDecodeData, false, 0, 0, false, false, true,				true, kPGPUISettingsTextOutput,				kPGPRecipientOptionsHideFileOptions | 					kPGPRecipientOptionsShowFYEO | ((mForceRecipientsDialog) ?					kPGPRecipientOptionsAlwaysShowDialog : 0),				inNumRecipients, inRecipients, true,				reinterpret_cast<const PGPByte *>(inText), inTextSize,				reinterpret_cast<PGPByte **>(outText), outTextSize);	PGPThrowIfPGPError_(pgpErr);}	voidCPGPAppmenu::AppEncryptAndSign(	PGPUInt32			inNumRecipients,	PGPRecipientSpec *	inRecipients,	const char *		inText,	PGPSize				inTextSize,	char **				outText,	PGPSize * 			outTextSize){	PGPError		pgpErr;		pgpErr = PGPClientEncode(mClientEncodeDecodeData, true, kPGPUISettingsTextOutput,				kPGPGetPassphraseOptionsHideFileOptions, true, 				false, true, true, kPGPUISettingsTextOutput,				kPGPRecipientOptionsHideFileOptions | 					kPGPRecipientOptionsShowFYEO | ((mForceRecipientsDialog) ?					kPGPRecipientOptionsAlwaysShowDialog : 0),				inNumRecipients, inRecipients, true,				reinterpret_cast<const PGPByte *>(inText), inTextSize,				reinterpret_cast<PGPByte **>(outText), outTextSize);	PGPThrowIfPGPError_(pgpErr);}	voidCPGPAppmenu::AppDecryptVerify(	const char *	inText,	PGPSize			inTextSize,	char **			outText,	PGPSize * 		outTextSize){	PGPError		pgpErr;		pgpErr = PGPClientDecode(mClientEncodeDecodeData, true,				reinterpret_cast<const PGPByte *>(inText), inTextSize,				reinterpret_cast<PGPByte **>(outText), outTextSize);	PGPThrowIfPGPError_(pgpErr);}	BooleanCPGPAppmenu::CPGPAppmenuEventAvailPatch::NewEventAvail(	EventMask		eventMask,	EventRecord *	theEvent){	Boolean	result = OldEventAvail(eventMask, theEvent);	try {		dynamic_cast<CPGPAppmenu &>(*CPGPmenu::Getmenu()).EventAvail(theEvent);	}		catch (CPGPException & error) {		CPGPmenu::ShowError(error);	}		catch (...) {		CPGPmenu::ShowError(CPGPException(kPGPError_UnknownError));	}		return result;}	voidCPGPAppmenu::CPGPAppmenuGetKeysPatch::NewGetKeys(	KeyMap	theKeys){	OldGetKeys(theKeys);	try {		// We need to do this in order to prevent Macsbug from crashing		if (CPGPmenu::sMenu != 0) {			dynamic_cast<CPGPAppmenu &>(*CPGPmenu::Getmenu()).GetKeys(theKeys);		}	}		catch (CPGPException & error) {		CPGPmenu::ShowError(error);	}		catch (...) {		CPGPmenu::ShowError(CPGPException(kPGPError_UnknownError));	}}	longCPGPAppmenu::CPGPAppmenuPutScrapPatch::NewPutScrap(	long	length,	ResType	theType,	void *	source){	long		result;	PGPBoolean	callThrough = true;		try {		CPGPAppmenu *		pgpmenu = dynamic_cast<CPGPAppmenu *>(CPGPmenu::Getmenu());		StSaveHeapZone		savedZone(::SystemZone());		StSaveCurResFile	curResFile;		LFile				ourFile(*pgpmenu->GetPGPmenuSpec());		ourFile.OpenResourceFork(fsRdWrPerm);		try {			if (pgpmenu->PutScrap(length, theType, source)) {				result = noErr;				callThrough = false;			}		}				catch (CPGPException & error) {			CPGPmenu::ShowError(error);		}				catch (...) {			CPGPmenu::ShowError(CPGPException(kPGPError_UnknownError));		}	}		catch (...) {	}	if (callThrough) {		result = OldPutScrap(	length,								theType,								source);	}		return result;}	voidCPGPAppmenu::DoOperation(	EOperation	inOperation){	SwitchState(event_EncodeDecode);	mOperation = inOperation;	PostCommandKey(cmdKey_Suspend);	PostCommandKey(cmdKey_Resume);}	PGPInt8CPGPAppmenu::SwitchState(	PGPInt8	inEvent){	PGPInt8	result = kStateTable[mState][inEvent];		if (result >= 0) {		mState = result;	} else if (result == -2) {		Cleanup();		PGPThrowPGPError_(kPGPError_UnknownError);	}		return mState;}	voidCPGPAppmenu::HandleKeyDown(){	SEventParamsPtr	eventParams = CPGPmenuWaitNextEventPatch::GetEventParamsPtr();		if ((mState != state_Idle) && (eventParams->theEvent->modifiers & cmdKey)) {		switch (eventParams->theEvent->message) {			case cmdKey_Suspend:			{				eventParams->theEvent->what = osEvt;				eventParams->theEvent->message = (suspendResumeMessage << 24)					| convertClipboardFlag;			}			break;									case cmdKey_Resume:			{				eventParams->theEvent->what = osEvt;				eventParams->theEvent->message = (suspendResumeMessage << 24)					| resumeFlag | convertClipboardFlag;				switch (SwitchState(event_ResumeKey)) {					case state_PutScrap:					{						mSaveRestoreScrap = new CSaveRestoreScrap;						::ZeroScrap();						PostCommandKey(cmdKey_Copy);						PostCommandKey(cmdKey_Suspend);						PostCommandKey(cmdKey_Resume);					}					break;															case state_SelectAllPutScrap:					{						PostCommandKey(cmdKey_SelectAll);						PostCommandKey(cmdKey_Copy);						PostCommandKey(cmdKey_Suspend);						PostCommandKey(cmdKey_Resume);					}					break;															case state_Processing:					{						try {							StPGPDataRef	input;							StPGPDataRef	output;							PGPSize			outputSize;							PGPError		pgpErr;							OSStatus		err;											input = mData;							mData = 0;							switch (mOperation) {								case op_Sign:								{									AppSign(input, mDataSize, reinterpret_cast<char **>(&output), &outputSize);								}								break;																								case op_Encrypt:								{									AppEncrypt(0, 0, input, mDataSize, reinterpret_cast<char **>(&output),										&outputSize);								}								break;																								case op_EncryptAndSign:								{									AppEncryptAndSign(0, 0, input, mDataSize, reinterpret_cast<char **>(&output),										&outputSize);								}								break;																								case op_DecryptVerify:								{									AppDecryptVerify(input, mDataSize, reinterpret_cast<char **>(&output),										&outputSize);								}								break;							}							::ZeroScrap();							err = ::PutScrap(outputSize, 'TEXT',										static_cast<PGPByte *>(output));							PGPThrowIfOSError_(err);							if ((mOperation == op_DecryptVerify) && mUsesOutputDialog) {								PGPBoolean	copyToClipboard;																pgpErr = PGPOutputDialog(mContext, output, &copyToClipboard);								PGPThrowIfPGPError_(pgpErr);								if (copyToClipboard) {									mSaveRestoreScrap->SetRestore(false);								}							} else {								PostCommandKey(cmdKey_Paste);							}							Cleanup();						}												catch (...) {							Cleanup();							throw;						}					}					break;															case state_Cleanup:					{						Cleanup();					}					break;															case state_Idle:					{						delete mSaveRestoreScrap;						mSaveRestoreScrap = 0;						if (mData != 0) {							PGPFreeData(mData);							mData = 0;						}					}					break;				}			}			break;		}		if (eventParams->theEvent->what == keyDown) {			mCurrentKey = eventParams->theEvent->message;		} else {			mCurrentKey = 0;		}	} else {		CPGPmenu::HandleKeyDown();	}}	voidCPGPAppmenu::EventAvail(	EventRecord *	inEvent){	if ((mState != state_Idle) && (inEvent->what == keyDown)	&& (inEvent->modifiers & cmdKey)) {		if (inEvent->message == cmdKey_Suspend) {			inEvent->what = osEvt;			inEvent->message = (suspendResumeMessage << 24) | convertClipboardFlag;		} else if (inEvent->message == cmdKey_Resume) {			inEvent->what = osEvt;			inEvent->message = (suspendResumeMessage << 24)				| resumeFlag | convertClipboardFlag;		}	}}	voidCPGPAppmenu::GetKeys(	KeyMap	outKeys){	if (mState != state_Idle) {		switch (mCurrentKey) {			case cmdKey_Copy:			{				pgpCopyMemory(kCopyKeyMap, outKeys, sizeof(kCopyKeyMap));			}			break;									case cmdKey_SelectAll:			{				pgpCopyMemory(kSelectAllKeyMap, outKeys, sizeof(kSelectAllKeyMap));			}			break;									case cmdKey_Paste:			{				pgpCopyMemory(kPasteKeyMap, outKeys, sizeof(kPasteKeyMap));			}			break;		}	}}	PGPBooleanCPGPAppmenu::PutScrap(	PGPInt32	inLength,	ResType		inType,	void *		inSource){	PGPBoolean	result;		if ((mState != state_Idle) && (mState != state_Processing)) {		result = true;		if (inType == 'TEXT') {			try {				SwitchState(event_PutScrap);				mData = static_cast<PGPByte *>(PGPNewData(mMemoryMgr, inLength,												kPGPMemoryMgrFlags_None));				PGPThrowIfMemFail_(mData);				pgpCopyMemory(inSource, mData, inLength);				mDataSize = inLength;			}						catch (...) {				Cleanup();				throw;			}		}	} else {		result = false;	}		return result;}	voidCPGPAppmenu::PostCommandKey(	SInt32	inKey){	OSStatus	err;	EvQElPtr	qEvent;		try {		err = ::PPostEvent(keyDown, inKey, &qEvent);		PGPThrowIfOSError_(err);		qEvent->evtQModifiers |= cmdKey;	}		catch (...) {		Cleanup();		throw;	}}	voidCPGPAppmenu::Cleanup(){	SwitchState(event_Cleanup);	PostCommandKey(cmdKey_Suspend);	PostCommandKey(cmdKey_Resume);		}