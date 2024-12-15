/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	This is the algorithm
	(P) means that the step is only done if on private scraps:

	Zero the scrap
	Suspend (P)
	Save previous scrap
	ZeroScrap
	Resume (P)

	Copy
	Copy
	Suspend (P)

	If PutScrap					If not PutScrap
	Goto finish					 Resume (P)
								 Select All
								 Copy
								 Suspend (P)

								If PutScrap			If not PutScrap
								 Goto finish		 Restore previous scrap
													 Resume (P)

	Finish
	Encode/Decode scrap

	If Output Dialog				If Paste
	Restore previous scrap		 Resume (P)
	Resume (P)					 Paste
	Show Dialog					 Suspend (P)
								 Restore previous scrap
								 Resume (P)
	
	
	$Id: CCopyPasteHack.cp,v 1.16.8.1 1997/12/05 22:14:24 mhw Exp $
____________________________________________________________________________*/


#include "WarningAlert.h"
#include "SignatureStatusMessage.h"
#include "MacStrings.h"
#include "MacErrors.h"

#include "StSaveHeapZone.h"

#include "CPGPmenu.h"
#include "COutputDialog.h"

#include "CCopyPasteHack.h"

// Statics
CCopyPasteHack *	CCopyPasteHack::sCopyPasteHack = nil;



// Constants
const SInt32		kSuspendKey			=	1;
const SInt32		kResumeKey			=	2;
const SInt32		kCopyKey			=	0x0863;
const SInt32		kSelectAllKey		=	0x0061;
const SInt32		kPasteKey			=	0x0976;

const KeyMap		kCopyKeyMap			=	{	0x00010000,
												0x00008000,
												0x00000000,
												0x00000000};
const KeyMap		kSelectAllKeyMap	=	{	0x01000000,
												0x00008000,
												0x00000000,
												0x00000000};
const KeyMap		kPasteKeyMap		=	{	0x00020000,
												0x00008000,
												0x00000000,
												0x00000000};



CCopyPasteHack::CCopyPasteHack(
	SGlobals &		inGlobals,
	EEncodeDecode	inEncodeDecode,
	EEncodeOptions	inOptions)
		: mGlobals(inGlobals), mEncodeDecode(inEncodeDecode), mTextH(nil),
		  mOptions(inOptions), mChecked(true), mVerified(true),
		  mSuspended(false), mSaveRestoreScrap(nil),
		  mImportKeys(kPGPInvalidRef)
{
	if (sCopyPasteHack != nil) {
		delete sCopyPasteHack;
	}
	
	if (mGlobals.privateScrap) {
		PostKeyDown(kSuspendKey, cmdKey);
		mState = state_SuspendZeroScrap;
	} else {
		mState = state_ZeroScrap;
	}
	sCopyPasteHack = this;
}



CCopyPasteHack::~CCopyPasteHack()
{
	PGPKeySetRef	defaultKeySet = kPGPInvalidRef;
	PGPKeySetRef	selectedSet = kPGPInvalidRef;

	try {
		sCopyPasteHack = nil;
		
		if (mEncodeDecode == encodeDecode_Decode) {
			// Import the keys
			if (mImportKeys != kPGPInvalidRef) {
				PGPError	pgpErr;
				PGPUInt32	count;
				
				pgpErr = PGPCountKeys(mImportKeys, &count);
				PGPThrowIfPGPErr_(pgpErr);
				if (count > 0) {
					pgpErr = PGPOpenDefaultKeyRings(mGlobals.pgpContext,
									0,
									&defaultKeySet);
					PGPThrowIfPGPErr_(pgpErr);
					pgpErr = PGPSelectiveImportDialog(
									mGlobals.pgpContext,
									CString(STRx_Strings, kAddKeysID),
									mImportKeys,
									defaultKeySet,
									&selectedSet);
					
					if (pgpErr != kPGPError_UserAbort) {
						PGPThrowIfPGPErr_(pgpErr);
						pgpErr = PGPSharedAddKeysToDefaultKeyring(
									selectedSet);
						PGPThrowIfPGPErr_(pgpErr);
					}
				}
			}
				
			// Show alert if flag is set
			if ((! mChecked) || (! mVerified)) {
				WarningAlert(	kWAStopAlertType,
								kWAOKStyle,
								STRx_Strings,
								(mChecked) ? kSignatureBadID
									: kSignatureNotVerifiedID);
			}
			
			// Show Output dialog if necessary
			if ((mGlobals.useOutputDialog) && (mTextH != nil)) {
				COutputDialog *	theOutputDialog = (COutputDialog *)
					CModalDialogGrafPortView::CreateCModalDialogGrafPortView(
							kOutputDialogResID,
							nil);
																	
				theOutputDialog->SetText(mTextH);
				theOutputDialog->Show();
				theOutputDialog->Run();
				if (theOutputDialog->GetCopyToClipboard() 
				&& (mSaveRestoreScrap != nil)) {
					mSaveRestoreScrap->SetRestore(false);
				}
				delete theOutputDialog;
			}
		}
		
	}
	
	catch (CComboError & comboError) {
		if (comboError.HavePGPError()) {
			CPGPmenuPlug::ShowError(comboError.pgpErr);
		} else {
			CPGPmenuPlug::ShowError(MacErrorToPGPError(comboError.err));
		}
	}

	catch(...) {
		CPGPmenuPlug::ShowError(kPGPError_UnknownError);
	}
	
	delete mSaveRestoreScrap;
	if (mTextH != nil) {
		::DisposeHandle(mTextH);
	}
	if (selectedSet != kPGPInvalidRef) {
		PGPFreeKeySet(selectedSet);
	}
	if (mImportKeys != kPGPInvalidRef) {
		PGPFreeKeySet(mImportKeys);
	}
	if (defaultKeySet != kPGPInvalidRef) {
		PGPFreeKeySet(defaultKeySet);
	}
}



	void
CCopyPasteHack::PreWaitNextEvent(
	SEventParams &	inParams)
{
	mCurrentKey = 0;
	
	switch (mState) {
		case state_ZeroScrap:
		{
			mSaveRestoreScrap = new CSaveRestoreScrap;
			::ZeroScrap();
			if (mGlobals.privateScrap) {
				PostKeyDown(kResumeKey, cmdKey);
				mState = state_ResumeZeroScrap;
			} else {
				PostKeyDown(kCopyKey, cmdKey);
				mState = state_CopyKey;
			}
		}
		break;
		
		
		case state_PutScrap:
		{
			// No put scrap, so try select all first
			if (mGlobals.privateScrap) {
				PostKeyDown(kResumeKey, cmdKey);
				mState = state_ResumePutScrap;
			} else {
				PostKeyDown(kSelectAllKey, cmdKey);
				PostKeyDown(kCopyKey, cmdKey);
				mState = state_AllCopyKey;
			}
		}
		break;
		
		
		case state_Finish:
		{
			StHandleLocker	theLock(mTextH);
			
			// Set the scrap
			::ZeroScrap();
			::PutScrap(::GetHandleSize(mTextH), 'TEXT', *mTextH);
			
			if ((mEncodeDecode == encodeDecode_Encode)
			|| (! mGlobals.useOutputDialog)) {
				if (mGlobals.privateScrap) {
					PostKeyDown(kResumeKey, cmdKey);
					mState = state_ResumeFinish;
				} else {
					PostKeyDown(kPasteKey, cmdKey);
					mState = state_Paste;
				}
			} else {
				mState = state_Cleanup;
			}
		}
		break;
		
		
		case state_AllPutScrap: // Never got data, so cleanup
		case state_Cleanup:
		{
			if (mSuspended) {
				PostKeyDown(kResumeKey, cmdKey);
				mState = state_ResumeCleanup;
			} else if (mGlobals.privateScrap) {
				PostKeyDown(kSuspendKey, cmdKey);
				PostKeyDown(kResumeKey, cmdKey);
				mState = state_SuspendCleanup;
			} else {
				mState = state_Done;
			}
		}
		break;
		
		
		case state_Done:
		{
			delete this;
		}
		break;
	}
}



	void
CCopyPasteHack::HandleKeyDown(
	SEventParams &	inParams)
{
	if (inParams.theEvent->modifiers & cmdKey) {
		switch (mState) {
			case state_SuspendZeroScrap:
			case state_SuspendPutScrap:
			case state_SuspendAllPutScrap:
			case state_SuspendCleanup:
			{
				if (inParams.theEvent->message == kSuspendKey) {
				
					// Change the event into a suspend event
					inParams.theEvent->what = osEvt;
					inParams.theEvent->message = (suspendResumeMessage << 24)
													| convertClipboardFlag;
					mSuspended = true;
					switch (mState) {
						case state_SuspendZeroScrap:
						{
							mState = state_ZeroScrap;
						}
						break;
						
						
						case state_SuspendPutScrap:
						{
							mState = state_PutScrap;
						}
						break;
						
						
						case state_SuspendAllPutScrap:
						{
							mState = state_AllPutScrap;
						}
						break;
						
						
						case state_SuspendCleanup:
						{
							mState = state_ResumeCleanup;
						}
						break;
					}
				}
			}
			break;


			case state_ResumeZeroScrap:
			case state_ResumePutScrap:
			case state_ResumeCleanup:
			case state_ResumeFinish:
			{
				if (inParams.theEvent->message == kResumeKey) {

					// Change the event into a resume event
					inParams.theEvent->what = osEvt;
					inParams.theEvent->message = (suspendResumeMessage << 24)
										| resumeFlag | convertClipboardFlag;
					mSuspended = false;
					switch (mState) {
						case state_ResumeZeroScrap:
						{
							PostKeyDown(kCopyKey, cmdKey);
							mState = state_CopyKey;
						}
						break;
						
						
						case state_ResumePutScrap:
						{
							PostKeyDown(kSelectAllKey, cmdKey);
							PostKeyDown(kCopyKey, cmdKey);
							mState = state_AllCopyKey;
						}
						break;
						
						
						case state_ResumeFinish:
						{
							PostKeyDown(kPasteKey, cmdKey);
							mState = state_Paste;
						}
						break;
						
						
						case state_ResumeCleanup:
						{
							mState = state_Done;
						}
						break;
							
					}
				}
			}
			break;
			
			
			case state_CopyKey:
			{
				if (inParams.theEvent->message == kCopyKey) {
					if (mGlobals.privateScrap) {
						PostKeyDown(kSuspendKey, cmdKey);
						mState = state_SuspendPutScrap;
					} else {
						mState = state_PutScrap;
					}
				}
			}
			break;


			case state_AllCopyKey:
			{
				if (inParams.theEvent->message == kCopyKey) {
					if (mGlobals.privateScrap) {
						PostKeyDown(kSuspendKey, cmdKey);
						mState = state_SuspendAllPutScrap;
					} else {
						mState = state_AllPutScrap;
					}
				}
			}
			break;
			
			
			case state_Paste:
			{
				if (inParams.theEvent->message == kPasteKey) {
					mState = state_Cleanup;
				}
			}
			break;
		}

		// Save off key if keyDown and cmdKey for use in generating a
		// keymap
		if (inParams.theEvent->what == keyDown) {
			mCurrentKey = inParams.theEvent->message;
		} else {
			mCurrentKey = 0;
		}
	}
}



	Boolean
CCopyPasteHack::PutScrap(
	SInt32	inLength,
	ResType	inType,
	void *	inSource)
{
	Boolean	result = false;
	
	switch (mState) {
		case state_PutScrap:
		case state_SuspendPutScrap:
		case state_AllPutScrap:
		case state_SuspendAllPutScrap:
		{
			if (inType == 'TEXT') {
				try {
					StSaveHeapZone		savedZone(::SystemZone());
					OSErr				err;
					EState				savedState = mState;
					PGPError			pgpErr;
					
					// Convert data to handle and call the encoder or
					// decoder
					mTextH = ::TempNewHandle(inLength, &err);
					if (mTextH == nil) {
						mTextH = ::NewHandle(inLength);
					}
					PGPThrowIfMemFail_(mTextH);
					::BlockMoveData(	inSource,
										*mTextH,
										inLength);

					mState = state_Processing;
					if (mEncodeDecode == encodeDecode_Encode) {
						result = CPGPEncoderDecoder::EncodeHandle(
										mGlobals.pgpContext,
										mTextH,
										mOptions);
					} else {
						PGPKeySetRef			defaultKeySet
														= kPGPInvalidRef;
						CString					sigString;
						CString					beginSignedSection(
													STRx_Strings,
													kBeginPGPSignedSectionID);
						CString					endSignedSection(
													STRx_Strings,
													kEndPGPSignedSectionID);
						SDecodeSectionHandle	theSectionsH = nil;
						
						try {
							pgpErr = PGPOpenDefaultKeyRings(
											mGlobals.pgpContext,
											0,
											&defaultKeySet);
							PGPThrowIfPGPErr_(pgpErr);
							result = CPGPEncoderDecoder::DecodeHandle(
											mGlobals.pgpContext,
											mTextH,
											defaultKeySet,
											theSectionsH);
							if (result) {
								::SetHandleSize(mTextH, 0);
								
								pgpErr = PGPNewKeySet(	mGlobals.pgpContext,
														&mImportKeys);
								PGPThrowIfPGPErr_(pgpErr);
								
								// Walk through the sections reassembling
								// them
								for (UInt32 i = 0,
								x = ::GetHandleSize((Handle) theSectionsH)
								/sizeof(SDecodeSection);
								i < x; i++) {
									// If it is a signed section, add sig info
									if ((*theSectionsH)[i].sigDataValid) {
										SignatureStatusInfo	theStatus;
										char	keyIDString[ kPGPMaxKeyIDStringSize ];

										// Set an alert flag if we were unable
										// to verify a section
										if (!
										(*theSectionsH)[i].sigData.checked) {
											mChecked = false;
										}
										if (!
										(*theSectionsH)[i].sigData.verified) {
											mVerified = false;
										}
										
										// Add status message
										::PtrAndHand(
											beginSignedSection.
												GetConstCString(),
											mTextH,
											beginSignedSection.GetLength());
										::HLock((Handle) theSectionsH);
										theStatus.sigData =
											(*theSectionsH)[i].sigData;
										pgpErr = PGPGetKeyIDString(
												&theStatus.sigData.signingKeyID,
												kPGPKeyIDString_Abbreviated,
												keyIDString);
										PGPThrowIfPGPErr_(pgpErr);
										CopyCString(keyIDString,
													theStatus.keyIDString);
										GetSignatureStatusMessage(
											&theStatus,
											sigString);
										::HUnlock((Handle) theSectionsH);
										AppendPString("\p\r", sigString);
										::PtrAndHand(
											sigString.GetConstCString(),
											mTextH,
											sigString.GetLength());
									}
										
									// If it is a key section add it to our
									// import list
									if ((*theSectionsH)[i].addKeysSet
									!= kPGPInvalidRef) {
										pgpErr = PGPAddKeys(
													(*theSectionsH)[i].
														addKeysSet,
													mImportKeys);
										PGPThrowIfPGPErr_(pgpErr);
										pgpErr = PGPCommitKeyRingChanges(
														mImportKeys);
										PGPThrowIfPGPErr_(pgpErr);
									}
					
									// Append the section
									if ((*theSectionsH)[i].buf != nil) {
										::PtrAndHand(
											(*theSectionsH)[i].buf,
											mTextH,
											(*theSectionsH)[i].size);
									}
									
									// Close out signed section
									if ((*theSectionsH)[i].sigDataValid) {
										::PtrAndHand(
											endSignedSection.
												GetConstCString(),
											mTextH,
											endSignedSection.GetLength());
									}
									// Free the data associated with the
									// section
									::HLock((Handle) theSectionsH);
									CPGPEncoderDecoder::FreeDecodeSection(
														&(*theSectionsH)[i]);
									::HUnlock((Handle) theSectionsH);
								}
							}
						}
						
						catch (...) {
							result = false;
						}

						::DisposeHandle((Handle) theSectionsH);
						PGPFreeKeySet(defaultKeySet);
					}
					
					if ((result == true) && (::GetHandleSize(mTextH) > 0)) {
							mState = state_Finish;
					} else {
						if (mTextH != nil) {
							::DisposeHandle(mTextH);
							mTextH = nil;
						}
						mState = state_Cleanup;
					}
				}
				
				catch (CComboError & comboError) {
					if (comboError.HavePGPError()) {
						CPGPmenuPlug::ShowError(comboError.pgpErr);
					} else {
						CPGPmenuPlug::ShowError(
							MacErrorToPGPError(comboError.err));
					}
					mState = state_Cleanup;
				}

				catch(...) {
					CPGPmenuPlug::ShowError(kPGPError_UnknownError);
					mState = state_Cleanup;
				}
			}
			
		}
		break;
	}
	
	return result;
}



	void
CCopyPasteHack::PostKeyDown(
	SInt32	inKey,
	SInt16	inModifiers)
{
	OSErr		err;
	EvQElPtr	qEvent;
	
	// Post the event
	err = ::PPostEvent(	keyDown,
						inKey,
						&qEvent);
	PGPThrowIfOSErr_(err);
	qEvent->evtQModifiers |= inModifiers;
}



	void
CCopyPasteHack::GetKeys(
	KeyMap	outKeys)
{
	if ((mState == state_PutScrap)
	|| (mState == state_AllCopyKey)
	|| (mState == state_AllPutScrap)
	|| (mState == state_Done)
	|| (mState == state_RestoreScrapSuspend)) {
		switch (mCurrentKey) {
			case kCopyKey:
			{
				::BlockMoveData(	kCopyKeyMap,
									outKeys,
									sizeof(kCopyKeyMap));
			}
			break;
			
			
			case kSelectAllKey:
			{
				::BlockMoveData(	kSelectAllKeyMap,
									outKeys,
									sizeof(kCopyKeyMap));
			}
			break;
			
			
			case kPasteKey:
			{
				::BlockMoveData(	kPasteKeyMap,
									outKeys,
									sizeof(kCopyKeyMap));
			}
			break;
		}
	}
}	
