/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CCopyPasteHack.h,v 1.8.10.1 1997/12/05 22:14:25 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include "PGPSharedEncryptDecrypt.h"
#include "CPGPEncoderDecoder.h"

#include "CWaitNextEventDispatcher.h"
#include "CSaveRestoreScrap.h"
#include "PGPmenuGlobals.h"



enum EEncodeDecode {
	encodeDecode_Encode	=	0,
	encodeDecode_Decode	=	1
};


class CCopyPasteHack {
public:
							CCopyPasteHack(SGlobals & inGlobals,
								EEncodeDecode inEncodeDecode,
								EEncodeOptions inOptions = 0);
							~CCopyPasteHack();

	void					PreWaitNextEvent(SEventParams & inParams);
	void					HandleKeyDown(SEventParams & inParams);
	Boolean					PutScrap(SInt32 inLength, ResType inType,
								void * inSource);
	void					GetKeys(KeyMap outKeys);
			
	static CCopyPasteHack *	GetCopyPasteHack() { return sCopyPasteHack; }
			
protected:
	enum EState {
		state_SuspendZeroScrap,
		state_ZeroScrap,
		state_ResumeZeroScrap,
		state_CopyKey,
		state_SuspendPutScrap,
		state_PutScrap,
		state_ResumePutScrap,
		state_AllCopyKey,
		state_SuspendAllPutScrap,
		state_AllPutScrap,
		state_Processing,
		state_Paste,
		state_Finish,
		state_ResumeFinish,
		state_SuspendCleanup,
		state_Cleanup,
		state_ResumeCleanup,
		state_RestoreScrapSuspend,
		state_Done
	};

	static CCopyPasteHack *	sCopyPasteHack;
	EEncodeDecode			mEncodeDecode;
	EState					mState;
	Handle					mTextH;
	EEncodeOptions			mOptions;
	Boolean					mChecked;
	Boolean					mVerified;
	Boolean					mSuspended;
	SInt32					mCurrentKey;
	CSaveRestoreScrap *		mSaveRestoreScrap;
	SGlobals &				mGlobals;
	PGPKeySetRef			mImportKeys;
	
	void					PostKeyDown(SInt32 inKey, SInt16 inModifiers);
};
