/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CPGPemailerView.cp,v 1.20.8.1 1997/12/05 22:13:50 mhw Exp $
____________________________________________________________________________*/

#include <stdio.h>

#include "MacProcesses.h"
#include "MacDesktop.h"
#include "MacStrings.h"
#include "MacErrors.h"
#include "PGPSharedParse.h"
#include "SignatureStatusMessage.h"
#include "pgpMem.h"
#include "WarningAlert.h"
#include "PGPEncode.h"
#include "PGPKeys.h"
#include "PGPUtilities.h"

#include "PGPSharedEncryptDecrypt.h"

#include "CPGPEncoderDecoder.h"
#include "CWaitNextEventDispatcher.h"

#include "CString.h"
#include "CPGPemailerView.h"



// Constants
const PaneIDT		pane_EncryptButton			=	1002;
const PaneIDT		pane_KeysButton				=	1003;
const PaneIDT		pane_SignButton				=	1004;
const PaneIDT		pane_EncryptAndSignButton	=	1005;
const PaneIDT		pane_DecryptButton			=	2002;

const ResIDT		STRx_Strings				=	10000;
const SInt16		kSignatureBadID				=	1;
const SInt16		kSignatureNotVerifiedID		=	2;
const SInt16		kBadPassphraseID			=	3;
const SInt16		kAttachmentsWarningID		=	4;
const SInt16		kZeroRecipientsID			=	5;
const SInt16		kNothingToEncodeID			=	6;
const SInt16		kBadSelectionID				=	7;
const SInt16		kKeysNotFoundID				=	8;
const SInt16		kBeginPGPSignedSectionID	=	9;
const SInt16		kEndPGPSignedSectionID		=	10;
const SInt16		kAddKeysID					=	11;

const ResIDT		STRx_Errors					=	10001;
const SInt16		kErrorStringID				=	1;


const OSType		kPGPkeysCreator				=	'pgpK';
const OSType		kPGPkeysType				=	'APPL';

const DescType		cMessageWindow				=	'cMsW';
const DescType		cRecipient					=	'rcpt';

const DescType		pDisplayedMessage			=	'pMsg';
const DescType		pContent					=	'ctnt';
const DescType		pAddress					=	'addr';

	CPGPemailerView *							
CPGPemailerView::CreateCPGPemailerViewStream(
	LStream *inStream)
{
	return (new CPGPemailerView(inStream));
}



CPGPemailerView::CPGPemailerView(
	LStream* inStream)				// Stream to construct it from
		: LGrafPortView(inStream), mCursorRef(nil)
{
	PGPThrowIfOSErr_(Get1AnimatedCursor(acur_BeachBall, &mCursorRef));
}



CPGPemailerView::~CPGPemailerView()
{
	if (mCursorRef != nil) {
		DisposeAnimatedCursor(mCursorRef);
	}
}


	void
CPGPemailerView::ClickSelf(
	const SMouseDownEvent& inMouseDown)	// The mousedown event
{
	// Switch target to the view
	SwitchTarget(this);
}




	Boolean								// Was it handled?
CPGPemailerView::HandleKeyPress(
	const EventRecord	&inKeyEvent)	// The key event
{
	// Let the superclass handle it
	return LCommander::HandleKeyPress(inKeyEvent);
}



	void
CPGPemailerView::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	Char16		&outMark,
	Str255		outName)
{
	// Do nothing to prevent LCommander from setting enabled to false
}




	Boolean						// Was the command handled?
CPGPemailerView::ObeyCommand(
	CommandT	inCommand,		// The command to handle
	void		*ioParam)		// Extra data
{
	Boolean	cmdHandled = true;

	switch (inCommand) {
		default:
			cmdHandled = LCommander::ObeyCommand(inCommand, ioParam);
			break;
	}
	
	return cmdHandled;
}



	void
CPGPemailerView::DrawSelf()
{
	// Do nothing to avoid having LGrafPortView erase the window
}



	void
CPGPemailerView::FinishCreateSelf()
{
}


	void
CPGPemailerView::ListenToMessage(
	MessageT inMessage,			// Message sent
	void* ioParam)				// Extra data
{
	switch (inMessage) {
		case pane_EncryptButton:
		{
			EncodeMessage(encodeOptions_Encrypt);
		}
		break;
		
		
		case pane_SignButton:
		{
			EncodeMessage(encodeOptions_Sign);
		}
		break;
		
		
		case pane_EncryptAndSignButton:
		{
			EncodeMessage(encodeOptions_Sign
				| encodeOptions_Encrypt);
		}
		break;
		
		
		case pane_DecryptButton:
		{
			DecodeMessage();
		}
		break;
		
		
		case pane_KeysButton:
		{
			OSStatus	err;
			
			err = BringAppToFront(	kPGPkeysType,
									kPGPkeysCreator,
									kFindAppOnAllVolumes,
									true,
									nil,
									nil);
			if (err != noErr) {
				WarningAlert(	kWAStopAlertType,
								kWAOKStyle,
								STRx_Strings,
								kKeysNotFoundID);
			}
		}
		break;
	}
}


	void
CPGPemailerView::AdjustCursorSelf(
	Point				inPortPt,	// Where the cursor is
	const EventRecord&	inMacEvent)	// Current event
{
	// Do nothing to avoid having LGrafPortView set the cursor to an arrow
}



	void
CPGPemailerView::ClickInContent(
const EventRecord &	inMacEvent)
{
	if (IsActive()) {
		LGrafPortView::ClickInContent(inMacEvent);
	}
}



	void
CPGPemailerView::EncodeMessage(
	EEncodeOptions	inOptions)
{
	PGPKeySetRef	defaultKeySet = kPGPInvalidRef;

	try {
		OSErr			err = noErr;
		StHandleBlock	theContent(0, true, true);
		Boolean			setSelection;
		SInt32			numDefaultRecipients;
		StHandleBlock	defaultRecipientsH(0);
		Boolean			useCache;
		PGPError		pgpErr;
		void *			outputBuffer = nil;
				
		mEncoding = true;
		
		// Attempt to get selection if it is empty get entire content
		GetFrontWindowSelection(theContent);
		if (::GetHandleSize(theContent) != 0) {
			setSelection = true;
		} else {
			setSelection = false;
			GetFrontWindowContent(theContent);
		}

		// If there is no content, just return
		Size	theMessageSize = ::GetHandleSize(theContent);

		if (theMessageSize == 0) {
			WarningAlert(	kWAStopAlertType,
							kWAOKStyle,
							STRx_Strings,
							kNothingToEncodeID);
			return;
		}

		// Check to see if the data is already encrypted if so, decrypt it
		// instead
		pgpErr = PGPOpenDefaultKeyRings(	mGlobalsP->pgpContext,
											0,
											&defaultKeySet);
		PGPThrowIfPGPErr_(pgpErr);
		mEncrypted = false;
		::HLock(theContent);
		PGPDecode(	mGlobalsP->pgpContext,
					PGPOInputBuffer(	mGlobalsP->pgpContext,
										*theContent,
										theMessageSize),
					PGPODiscardOutput(	mGlobalsP->pgpContext,
										TRUE),
					PGPOEventHandler(	mGlobalsP->pgpContext,
										PGPEventHandler,
										this),
					PGPOSendNullEvents(	mGlobalsP->pgpContext,
										500),
					PGPOKeySetRef(	mGlobalsP->pgpContext,
									defaultKeySet),
					PGPOLastOption(mGlobalsP->pgpContext));
		::HUnlock(theContent);
		if (mEncrypted) {
			DecodeMessage(false);
		} else {
			// Grab the default recipients
			GetFrontWindowRecipients(	&numDefaultRecipients,
										defaultRecipientsH);
			StHandleLocker	theLock(defaultRecipientsH);
			
			// Do we use the cache and show recipients
			if (CWaitNextEventDispatcher::GetEventParamsPtr()->theEvent->modifiers
			& optionKey) {
				useCache = false;
			} else {
				useCache = true;
			}

			// Do the encryption
			inOptions |= encodeOptions_TreatInputAsText;
			if (inOptions & encodeOptions_Encrypt) {
				inOptions |= encodeOptions_TextOutput;
			} else {
				inOptions |= encodeOptions_ClearSign;
			}
			if (CPGPEncoderDecoder::EncodeHandle(mGlobalsP->pgpContext,
			theContent, inOptions, numDefaultRecipients,
			(PGPKeySpec *) *defaultRecipientsH, useCache)) {
				// Warn about attachments
				if (CountFrontWindowAttachments() > 0) {
					WarningAlert(	kWACautionAlertType,
									kWAOKStyle,
									STRx_Strings,
									kAttachmentsWarningID);
				}
				
				// Set the window content
				if (setSelection) {
					try {
						SetFrontWindowSelection(theContent);
					}
					catch (...) {
						WarningAlert(	kWAStopAlertType,
										kWAOKStyle,
										STRx_Strings,
										kBadSelectionID);
					}			
				} else {
					SetFrontWindowContent(theContent);
				}
			}
		}
	}
	
	catch (CComboError & comboError) {
		if (comboError.HavePGPError()) {
			ShowError(comboError.pgpErr);
		} else {
			ShowError(MacErrorToPGPError(comboError.err));
		}
	}

	catch(...) {
		ShowError(kPGPError_UnknownError);
	}
	
	if (defaultKeySet != kPGPInvalidRef) {
		PGPFreeKeySet(defaultKeySet);
	}
}



	void
CPGPemailerView::DecodeMessage(
	Boolean	inAppendStatus)
{
	PGPKeySetRef	defaultKeySet = kPGPInvalidRef;
	PGPKeySetRef	importSet = kPGPInvalidRef;
	PGPKeySetRef	selectedSet = kPGPInvalidRef;
	
	try {
		OSErr					err = noErr;
		StHandleBlock			theContent(0, true, true);
		SDecodeSectionHandle	theSectionsH;
		Boolean					useCache;
		PGPError				pgpErr;
	
		// Get the content
		GetFrontWindowContent(theContent);
		
		// Do we use the cache?
		if (CWaitNextEventDispatcher::GetEventParamsPtr()->theEvent->modifiers
		& optionKey) {
			useCache = false;
		} else {
			useCache = true;
		}

		// Open the default keyrings
		pgpErr = PGPOpenDefaultKeyRings(	mGlobalsP->pgpContext,
											0,
											&defaultKeySet);
		PGPThrowIfPGPErr_(pgpErr);
			
		// Decrypt the content
		if (CPGPEncoderDecoder::DecodeHandle(mGlobalsP->pgpContext,
		theContent, defaultKeySet, theSectionsH, useCache)) {
			CString	sigString;
			CString	beginSignedSection(	STRx_Strings,
										kBeginPGPSignedSectionID);
			CString	endSignedSection(STRx_Strings, kEndPGPSignedSectionID);
			Boolean	checked = true;
			Boolean	verified = true;
			
			::SetHandleSize(theContent, 0);
			
			// Create the import set
			pgpErr = PGPNewKeySet(mGlobalsP->pgpContext, &importSet);
			PGPThrowIfPGPErr_(pgpErr);
			
			// Walk through the sections reassembling them
			for (UInt32 i = 0,
			x = ::GetHandleSize((Handle) theSectionsH)/sizeof(SDecodeSection);
			i < x; i++) {
				// If it is a signed section, add sig info
				if ((*theSectionsH)[i].sigDataValid) {
					SignatureStatusInfo	theStatus;
					char	keyIDString[ kPGPMaxKeyIDStringSize ];
					
					// Set an alert flag if we were unable to verify a section
					if (! (*theSectionsH)[i].sigData.checked) {
						checked = false;
					}
					if (! (*theSectionsH)[i].sigData.verified) {
						verified = false;
					}
					
					// Add status message
					::PtrAndHand(	beginSignedSection.GetConstCString(),
									theContent,
									beginSignedSection.GetLength());
					::HLock((Handle) theSectionsH);
					theStatus.sigData = (*theSectionsH)[i].sigData;
					pgpErr = PGPGetKeyIDString(	&theStatus.sigData.signingKeyID,
												kPGPKeyIDString_Abbreviated,
												keyIDString);
					PGPThrowIfPGPErr_(pgpErr);
					CopyCString(keyIDString, theStatus.keyIDString);
					GetSignatureStatusMessage(	&theStatus,
												sigString);
					::HUnlock((Handle) theSectionsH);
					AppendPString("\p\r", sigString);
					::PtrAndHand(	sigString.GetConstCString(),
									theContent,
									sigString.GetLength());
				}
				
				// If it is a key section add it to our import list
				if ((*theSectionsH)[i].addKeysSet != kPGPInvalidRef) {
					pgpErr = PGPAddKeys(	(*theSectionsH)[i].addKeysSet,
											importSet);
					PGPThrowIfPGPErr_(pgpErr);
					pgpErr = PGPCommitKeyRingChanges(importSet);
					PGPThrowIfPGPErr_(pgpErr);
				}
					
				// Append the section
				if ((*theSectionsH)[i].buf != nil) {
					::PtrAndHand(	(*theSectionsH)[i].buf,
									theContent,
									(*theSectionsH)[i].size);
				}
				
				// Close out signed section
				if ((*theSectionsH)[i].sigDataValid) {
					::PtrAndHand(	endSignedSection.GetConstCString(),
									theContent,
									endSignedSection.GetLength());
				}
				// Free the data associated with the section
				::HLock((Handle) theSectionsH);
				CPGPEncoderDecoder::FreeDecodeSection(&(*theSectionsH)[i]);
				::HUnlock((Handle) theSectionsH);
			}
			
			// Free up the sections handle
			::DisposeHandle((Handle) theSectionsH);
			
			// Import the keys
			PGPUInt32	count;
			
			pgpErr = PGPCountKeys(importSet, &count);
			PGPThrowIfPGPErr_(pgpErr);
			if (count > 0) {				
				pgpErr = PGPSelectiveImportDialog(
							mGlobalsP->pgpContext,							
							CString(	STRx_Strings,
										kAddKeysID),							
							importSet,							
							defaultKeySet,
							&selectedSet);
				if (pgpErr != kPGPError_UserAbort) {
					PGPThrowIfPGPErr_(pgpErr);
					pgpErr = PGPSharedAddKeysToDefaultKeyring(
								selectedSet);
					PGPThrowIfPGPErr_(pgpErr);
				}
			}

					
			// Set the content
			SetFrontWindowContent(theContent);

			// Show alert if flag is set
			if ((! checked) || (! verified)) {
				WarningAlert(	kWAStopAlertType,
								kWAOKStyle,
								STRx_Strings,
								(checked) ? kSignatureBadID :
											kSignatureNotVerifiedID);
			}				
		}
	}
	
	catch (CComboError & comboError) {
		if (comboError.HavePGPError()) {
			ShowError(comboError.pgpErr);
		} else {
			ShowError(MacErrorToPGPError(comboError.err));
		}
	}

	catch(...) {
		ShowError(kPGPError_UnknownError);
	}
	
	if (selectedSet != kPGPInvalidRef) {
		PGPFreeKeySet(selectedSet);
	}
	if (importSet != kPGPInvalidRef) {
		PGPFreeKeySet(importSet);
	}
	if (defaultKeySet != kPGPInvalidRef) {
		PGPFreeKeySet(defaultKeySet);
	}
}



	void
CPGPemailerView::GetFrontWindowContent(
	Handle	outContentH)
{
	OSErr		err = noErr;
	AppleEvent	theGetContentEvent = {typeNull, nil};
	
	// Create the AppleEvent
	UAppleEventsMgr::MakeAppleEvent(	kAECoreSuite,
										kAEGetData,
										theGetContentEvent);
										
	// Create the window specifier
	StAEDescriptor	nullDescriptor;
	StAEDescriptor	windowSpec;
	DescType		first = kAEFirst;
	StAEDescriptor	windowPositionDesc(	typeAbsoluteOrdinal,
										&first,
										sizeof(first));
	
	err = ::CreateObjSpecifier(	cWindow,
								nullDescriptor,
								formAbsolutePosition,
								windowPositionDesc,
								false,
								windowSpec);
	PGPThrowIfOSErr_(err);
	
		
	// Create message property
	StAEDescriptor	displayedMessageProperty(pDisplayedMessage);
	StAEDescriptor	messageSpec;
	
	err = ::CreateObjSpecifier(	cProperty,
								windowSpec,
								formPropertyID,
								displayedMessageProperty,
								false,
								messageSpec);
	PGPThrowIfOSErr_(err);
	
	// Create the content property
	StAEDescriptor	contentProperty(pContent);
	StAEDescriptor	contentSpec;
	
	err = ::CreateObjSpecifier(	cProperty,
								messageSpec,
								formPropertyID,
								contentProperty,
								false,
								contentSpec);
	PGPThrowIfOSErr_(err);


	// Add the spec to the appleevent and send it
	err = ::AEPutParamDesc(&theGetContentEvent, keyDirectObject, contentSpec);
	PGPThrowIfOSErr_(err);
	AppleEvent	theReply = {typeNull, nil};
	
	UAppleEventsMgr::SendAppleEventWithReply(	theGetContentEvent,
												theReply,
												false);						
	::AEDisposeDesc(&theGetContentEvent);
	
	// Retrieve the data
	StAEDescriptor	errorCode(theReply, keyErrorNumber);
	
	if (errorCode.mDesc.descriptorType != typeNull) {
		UExtractFromAEDesc::TheInt16(errorCode, err);
		
		PGPThrowIfOSErr_(err);
	}
	
	StAEDescriptor	theContent(theReply, keyDirectObject);

	PGPThrowIf_(theContent.mDesc.descriptorType == typeNull);
	
	Size	theDataSize = ::GetHandleSize(theContent.mDesc.dataHandle);

	::SetHandleSize(outContentH, theDataSize);
	::BlockMoveData(	*theContent.mDesc.dataHandle,
						*outContentH,
						theDataSize);
	
	// Check for missing parms and dispose of the reply
	UAppleEventsMgr::CheckForMissedParams(theReply);
	::AEDisposeDesc(&theReply);
}



	void
CPGPemailerView::SetFrontWindowContent(
Handle	inContentH)
{
	OSErr		err = noErr;
	AppleEvent	theSetContentEvent = {typeNull, nil};
	
	// Create the AppleEvent
	UAppleEventsMgr::MakeAppleEvent(	kAECoreSuite,
										kAESetData,
										theSetContentEvent);
										
	// Create the window specifier
	StAEDescriptor	nullDescriptor;
	StAEDescriptor	windowSpec;
	DescType		first = kAEFirst;
	StAEDescriptor	windowPositionDesc(	typeAbsoluteOrdinal,
										&first,
										sizeof(first));
	
	err = ::CreateObjSpecifier(	cWindow,
								nullDescriptor,
								formAbsolutePosition,
								windowPositionDesc,
								false,
								windowSpec);
	PGPThrowIfOSErr_(err);
		
	// Create message property
	StAEDescriptor	displayedMessageProperty(pDisplayedMessage);
	StAEDescriptor	messageSpec;
	
	err = ::CreateObjSpecifier(	cProperty,
								windowSpec,
								formPropertyID,
								displayedMessageProperty,
								false,
								messageSpec);
	PGPThrowIfOSErr_(err);
	
	// Create the content property
	StAEDescriptor	contentProperty(pContent);
	StAEDescriptor	contentSpec;
	
	err = ::CreateObjSpecifier(	cProperty,
								messageSpec,
								formPropertyID,
								contentProperty,
								false,
								contentSpec);
	PGPThrowIfOSErr_(err);
	
	// Create the data spec
	StHandleLocker	theLock(inContentH);
	StAEDescriptor	dataSpec(	typeChar,
								*inContentH,
								::GetHandleSize(inContentH));


	// Add the specs to the appleevent and send it
	err = ::AEPutParamDesc(&theSetContentEvent, keyDirectObject, contentSpec);
	PGPThrowIfOSErr_(err);
	err = ::AEPutParamDesc(&theSetContentEvent, keyAEData, dataSpec);
	PGPThrowIfOSErr_(err);
	
	UAppleEventsMgr::SendAppleEvent(theSetContentEvent);						
	::AEDisposeDesc(&theSetContentEvent);
}



	void
CPGPemailerView::GetFrontWindowSelection(
	Handle	outContentH)
{
	OSErr		err = noErr;
	AppleEvent	theGetSelectionEvent = {typeNull, nil};
	
	// Create the AppleEvent
	UAppleEventsMgr::MakeAppleEvent(	kAECoreSuite,
										kAEGetData,
										theGetSelectionEvent);
										
	// Create the window specifier
	StAEDescriptor	nullDescriptor;
	StAEDescriptor	windowSpec;
	DescType		first = kAEFirst;
	StAEDescriptor	windowPositionDesc(	typeAbsoluteOrdinal,
										&first,
										sizeof(first));
	
	err = ::CreateObjSpecifier(	cWindow,
								nullDescriptor,
								formAbsolutePosition,
								windowPositionDesc,
								false,
								windowSpec);
	PGPThrowIfOSErr_(err);
	
		
	// Create message property
	StAEDescriptor	selectionProperty((DescType) pUserSelection);
	StAEDescriptor	selectionSpec;
	
	err = ::CreateObjSpecifier(	cProperty,
								windowSpec,
								formPropertyID,
								selectionProperty,
								false,
								selectionSpec);
	PGPThrowIfOSErr_(err);

	// Add the spec to the appleevent and send it
	err = ::AEPutParamDesc(	&theGetSelectionEvent,
							keyDirectObject,
							selectionSpec);
	PGPThrowIfOSErr_(err);
	
	AppleEvent	theReply = {typeNull, nil};
	
	UAppleEventsMgr::SendAppleEventWithReply(	theGetSelectionEvent,
												theReply,
												false);						
	::AEDisposeDesc(&theGetSelectionEvent);
	
	// Retrieve the data
	StAEDescriptor	errorCode(theReply, keyErrorNumber);
	
	if (errorCode.mDesc.descriptorType != typeNull) {
		UExtractFromAEDesc::TheInt16(errorCode, err);
		
		PGPThrowIfOSErr_(err);
	}
	
	StAEDescriptor	theSelection(theReply, keyDirectObject);

	PGPThrowIf_(theSelection.mDesc.descriptorType == typeNull);
	
	Size			theDataSize =
						::GetHandleSize(theSelection.mDesc.dataHandle);

	::SetHandleSize(outContentH, theDataSize);
	::BlockMoveData(	*theSelection.mDesc.dataHandle,
						*outContentH,
						theDataSize);
	
	// Check for missing parms and dispose of the reply
	UAppleEventsMgr::CheckForMissedParams(theReply);
	::AEDisposeDesc(&theReply);
}



	void
CPGPemailerView::SetFrontWindowSelection(
	Handle inContentH)
{
	OSErr		err = noErr;
	AppleEvent	theSetSelectionEvent = {typeNull, nil};
	
	// Create the AppleEvent
	UAppleEventsMgr::MakeAppleEvent(	kAECoreSuite,
										kAESetData,
										theSetSelectionEvent);
										
	// Create the window specifier
	StAEDescriptor	nullDescriptor;
	StAEDescriptor	windowSpec;
	DescType		first = kAEFirst;
	StAEDescriptor	windowPositionDesc(	typeAbsoluteOrdinal,
										&first,
										sizeof(first));
	
	err = ::CreateObjSpecifier(	cWindow,
								nullDescriptor,
								formAbsolutePosition,
								windowPositionDesc,
								false,
								windowSpec);
	PGPThrowIfOSErr_(err);
	
		
	// Create message property
	StAEDescriptor	selectionProperty((DescType) pUserSelection);
	StAEDescriptor	selectionSpec;
	
	err = ::CreateObjSpecifier(	cProperty,
								windowSpec,
								formPropertyID,
								selectionProperty,
								false,
								selectionSpec);
	PGPThrowIfOSErr_(err);

	// Add the spec to the appleevent
	err = ::AEPutParamDesc(	&theSetSelectionEvent,
							keyDirectObject,
							selectionSpec);
	PGPThrowIfOSErr_(err);

	// Create the data spec
	StHandleLocker	theLock(inContentH);
	StAEDescriptor	dataSpec(	typeChar,
								*inContentH,
								::GetHandleSize(inContentH));

	// Add the spec to the appleevent and send it
	err = ::AEPutParamDesc(&theSetSelectionEvent, keyAEData, dataSpec);
	PGPThrowIfOSErr_(err);
	
	UAppleEventsMgr::SendAppleEvent(theSetSelectionEvent);						
	::AEDisposeDesc(&theSetSelectionEvent);
}



	void
CPGPemailerView::GetFrontWindowRecipients(
	SInt32 *	outNumRecipients,
	Handle		outRecipientsH)
{
	OSErr		err = noErr;
	AppleEvent	theGetRecipientsEvent = {typeNull, nil};
	
	// Create the AppleEvent
	UAppleEventsMgr::MakeAppleEvent(	kAECoreSuite,
										kAEGetData,
										theGetRecipientsEvent);
										
	// Create the window specifier
	StAEDescriptor	nullDescriptor;
	StAEDescriptor	windowSpec;
	DescType		first = kAEFirst;
	StAEDescriptor	windowPositionDesc(	typeAbsoluteOrdinal,
										&first,
										sizeof(first));
	
	err = ::CreateObjSpecifier(	cWindow,
								nullDescriptor,
								formAbsolutePosition,
								windowPositionDesc,
								false,
								windowSpec);
	PGPThrowIfOSErr_(err);

	// Create message property
	StAEDescriptor	displayedMessageProperty(pDisplayedMessage);
	StAEDescriptor	messageSpec;
	
	err = ::CreateObjSpecifier(	cProperty,
								windowSpec,
								formPropertyID,
								displayedMessageProperty,
								false,
								messageSpec);
	PGPThrowIfOSErr_(err);

	// Create recipient element
	StAEDescriptor	recipientSpec;
	DescType		all = kAEAll;
	StAEDescriptor	allDesc(typeAbsoluteOrdinal, &all, sizeof(all));
	
	err = ::CreateObjSpecifier(	cRecipient,
								messageSpec,
								formAbsolutePosition,
								allDesc,
								false,
								recipientSpec);
	PGPThrowIfOSErr_(err);
	
	// Create address property
	StAEDescriptor	addressSpec;
	StAEDescriptor	addressProperty(pAddress);
	
	err = ::CreateObjSpecifier(	cProperty,
								recipientSpec,
								formPropertyID,
								addressProperty,
								false,
								addressSpec);

	// Add the spec to the appleevent and send it
	err = ::AEPutParamDesc(	&theGetRecipientsEvent,
							keyDirectObject,
							addressSpec);
	PGPThrowIfOSErr_(err);
	
	AppleEvent	theReply = {typeNull, nil};
	
	UAppleEventsMgr::SendAppleEventWithReply(	theGetRecipientsEvent,
												theReply,
												false);						
	::AEDisposeDesc(&theGetRecipientsEvent);
	
	// Check for errors
	StAEDescriptor	errorCode(theReply, keyErrorNumber);
	
	if (errorCode.mDesc.descriptorType != typeNull) {
		UExtractFromAEDesc::TheInt16(errorCode, err);
		
		PGPThrowIfOSErr_(err);
	}
	
	// Turn the reply into a desc
	StAEDescriptor	theRecipientList(theReply, keyDirectObject);

	// Count the items
	err = ::AECountItems(theRecipientList, outNumRecipients);
	PGPThrowIfOSErr_(err);
	
	::SetHandleSize(outRecipientsH, *outNumRecipients * sizeof(PGPKeySpec));
	PGPThrowIfMemError_();
	
	// Get the address out of each one and add it to the recipients
	StHandleLocker	theLock(outRecipientsH);
	AEKeyword		keyWord;
	SInt32			dataLength;
	PGPKeySpec *	keySpec = (PGPKeySpec *) *outRecipientsH;
	
	for (SInt32 i = 1; i <= *outNumRecipients; i++) {
		StAEDescriptor	recipient;
		
		err = ::AEGetNthDesc(	theRecipientList,
								i,
								typeWildCard,
								&keyWord,
								recipient);
		PGPThrowIfOSErr_(err);
		
		StAEDescriptor	address;
		err = ::AEGetKeyDesc(	recipient,
								pAddress,
								typeWildCard,
								address);
		PGPThrowIfOSErr_(err);

		dataLength = ::GetHandleSize(address.mDesc.dataHandle);
		if (dataLength > 253) {
			dataLength = 253;
		}
		keySpec[i - 1].userIDStr[0] = '<';
		::BlockMoveData(	*(address.mDesc.dataHandle),
							&keySpec[i - 1].userIDStr[1],
							dataLength);
		keySpec[i - 1].userIDStr[dataLength + 1] = '>';
		keySpec[i - 1].userIDStr[dataLength + 2] = 0;
	}

	// Check for missing parms and dispose of the reply
	UAppleEventsMgr::CheckForMissedParams(theReply);
	::AEDisposeDesc(&theReply);
}



	SInt32
CPGPemailerView::CountFrontWindowAttachments()
{
	OSErr		err = noErr;
	AppleEvent	theCountAttachmentsEvent = {typeNull, nil};
	
	// Create the AppleEvent
	UAppleEventsMgr::MakeAppleEvent(	kAECoreSuite,
										kAECountElements,
										theCountAttachmentsEvent);
										
	// Create the window specifier
	StAEDescriptor	nullDescriptor;
	StAEDescriptor	windowSpec;
	DescType		first = kAEFirst;
	StAEDescriptor	windowPositionDesc(	typeAbsoluteOrdinal,
										&first,
										sizeof(first));
	
	err = ::CreateObjSpecifier(	cWindow,
								nullDescriptor,
								formAbsolutePosition,
								windowPositionDesc,
								false,
								windowSpec);
	PGPThrowIfOSErr_(err);

	// Create message property
	StAEDescriptor	displayedMessageProperty(pDisplayedMessage);
	StAEDescriptor	messageSpec;
	
	err = ::CreateObjSpecifier(	cProperty,
								windowSpec,
								formPropertyID,
								displayedMessageProperty,
								false,
								messageSpec);
	PGPThrowIfOSErr_(err);

	// Add the spec to the appleevent
	err = ::AEPutParamDesc(	&theCountAttachmentsEvent,
							keyDirectObject,
							messageSpec);
	PGPThrowIfOSErr_(err);
	
	// Add enclosure type to the appleevent
	StAEDescriptor	enclosureType((DescType) cFile);
	
	err = ::AEPutParamDesc(	&theCountAttachmentsEvent,
							keyAEObjectClass,
							enclosureType);
	PGPThrowIfOSErr_(err);
	
	AppleEvent	theReply = {typeNull, nil};
	
	UAppleEventsMgr::SendAppleEventWithReply(	theCountAttachmentsEvent,
												theReply,
												false);						
	::AEDisposeDesc(&theCountAttachmentsEvent);
	
	// Check for errors
	StAEDescriptor	errorCode(theReply, keyErrorNumber);
	
	if (errorCode.mDesc.descriptorType != typeNull) {
		UExtractFromAEDesc::TheInt16(errorCode, err);
		
		PGPThrowIfOSErr_(err);
	}
	
	// Extract the count
	StAEDescriptor	theCountDesc(theReply, keyDirectObject);
	SInt32	theCount;
	
	UExtractFromAEDesc::TheInt32(theCountDesc, theCount);

	// Check for missing parms and dispose of the reply
	UAppleEventsMgr::CheckForMissedParams(theReply);
	::AEDisposeDesc(&theReply);
	
	return theCount;
}



	void
CPGPemailerView::GetFrontWindowAttachments(
	LArray &	outAttachments)
{
	try {
		OSErr		err = noErr;
		AppleEvent	theGetAttachmentsEvent = {typeNull, nil};
		
		// Create the AppleEvent
		UAppleEventsMgr::MakeAppleEvent(	kAECoreSuite,
											kAEGetData,
											theGetAttachmentsEvent);
											
		// Create the window specifier
		StAEDescriptor	nullDescriptor;
		StAEDescriptor	windowSpec;
		DescType		first = kAEFirst;
		StAEDescriptor	windowPositionDesc(	typeAbsoluteOrdinal,
											&first,
											sizeof(first));
		
		err = ::CreateObjSpecifier(	cWindow,
									nullDescriptor,
									formAbsolutePosition,
									windowPositionDesc,
									false,
									windowSpec);
		PGPThrowIfOSErr_(err);

		// Create message property
		StAEDescriptor	displayedMessageProperty(pDisplayedMessage);
		StAEDescriptor	messageSpec;
		
		err = ::CreateObjSpecifier(	cProperty,
									windowSpec,
									formPropertyID,
									displayedMessageProperty,
									false,
									messageSpec);
		PGPThrowIfOSErr_(err);
		
		// Create file element
		StAEDescriptor	fileSpec;
		DescType		all = kAEAll;
		StAEDescriptor	allDesc(typeAbsoluteOrdinal, &all, sizeof(all));
		
		err = ::CreateObjSpecifier(	cFile,
									messageSpec,
									formAbsolutePosition,
									allDesc,
									false,
									fileSpec);
		PGPThrowIfOSErr_(err);
		
		// Create name property
		StAEDescriptor	nameProperty((DescType) pName);
		StAEDescriptor	nameSpec;

		err = ::CreateObjSpecifier(	cProperty,
									fileSpec,
									formPropertyID,
									nameProperty,
									false,
									nameSpec);
		PGPThrowIfOSErr_(err);

		// Add the spec to the appleevent
		err = ::AEPutParamDesc(	&theGetAttachmentsEvent,
								keyDirectObject,
								nameSpec);
		PGPThrowIfOSErr_(err);
		
		AppleEvent	theReply = {typeNull, nil};
		
		UAppleEventsMgr::SendAppleEventWithReply(	theGetAttachmentsEvent,
													theReply,
													false);						
		::AEDisposeDesc(&theGetAttachmentsEvent);
		
		// Check for errors
		StAEDescriptor	errorCode(theReply, keyErrorNumber);
		
		if (errorCode.mDesc.descriptorType != typeNull) {
			UExtractFromAEDesc::TheInt16(errorCode, err);
			
			PGPThrowIfOSErr_(err);
		}
		
		// Turn the reply into a desc
		StAEDescriptor	theAttachmentsList(theReply, keyDirectObject);
		SInt32			numAttachments;
		
		// Count the items
		err = ::AECountItems(theAttachmentsList, &numAttachments);
		PGPThrowIfOSErr_(err);	
		
		// Get each of the specs and check to see if it is a detached sig
		AEKeyword	keyWord;
		
		for (SInt32 i = 1; i <= numAttachments; i++) {
			StAEDescriptor	attachment;
			
			err = ::AEGetNthDesc(	theAttachmentsList,
									i,
									typeChar,
									&keyWord,
									attachment);
			PGPThrowIfOSErr_(err);
			
			StHandleLocker	theLock(attachment.mDesc.dataHandle);
			Size			fullPathSize =
								::GetHandleSize(attachment.mDesc.dataHandle);
			CString			fullPath(	*attachment.mDesc.dataHandle,
										fullPathSize,
										fullPathSize);
			FSSpec			spec;
			
			err = ::FSMakeFSSpec(0, 0, fullPath, &spec);
			PGPThrowIfOSErr_(err);
			
			outAttachments.InsertItemsAt(	1,
											LArray::index_Last,
											&spec);
		}

		// Check for missing parms and dispose of the reply
		UAppleEventsMgr::CheckForMissedParams(theReply);
		::AEDisposeDesc(&theReply);
	}
	
	catch (...) {
	}

}



	void
CPGPemailerView::ShowError(
	PGPError	inErr)
{
	CString		theErrorString;
	
	PGPGetErrorString(	inErr,
						theErrorString.GetBufferSize() + 1,
						theErrorString);
	WarningAlert(	kWAStopAlertType,
					kWAOKStyle,
					STRx_Errors,
					kErrorStringID,
					CString(inErr),
					theErrorString);
}



	PGPError
CPGPemailerView::PGPEventHandler(
	PGPContextRef	context,
	PGPEvent *		event,
	PGPUserValue	userValue)
{
	CPGPemailerView *	theView = (CPGPemailerView *) userValue;
	PGPError			result = kPGPError_NoErr;
	
	switch (event->type) {
		case kPGPEvent_AnalyzeEvent:
		{
			if (theView->mEncoding) {
				result = kPGPError_SkipSection;
				if (event->data.analyzeData.sectionType
				== kPGPAnalyze_Encrypted) {
					theView->mEncrypted = true;
				}
			}
		}
		break;
	}
	
	AnimateCursor(theView->mCursorRef);
	
	return result;
}
