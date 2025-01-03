/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CmenuMenuSelectPatch.cp,v 1.9.10.1 1997/12/05 22:14:18 mhw Exp $
____________________________________________________________________________*/

#include "WarningAlert.h"
#include "MacDesktop.h"
#include "MacProcesses.h"
#include "MacFiles.h"
#include "SignatureStatusMessage.h"
#include "MacStrings.h"
#include "MacErrors.h"

#include "StSaveCurResFile.h"
#include "StSaveHeapZone.h"
#include "StSuspendAESpecialHandler.h"
#include "CPGPEncoderDecoder.h"
#include "CCopyPasteHack.h"

#include "CPGPmenu.h"
#include "CString.h"
#include "CmenuWaitNextEventPatch.h"
#include "CmenuMenuSelectPatch.h"

const OSType		kPGPkeysType				=	'APPL';
const OSType		kPGPkeysCreator				=	'pgpK';
const OSType		kPGPtoolsType				=	'APPL';
const OSType		kPGPtoolsCreator			=	'pgpM';

const AEEventClass	kAEPGPMenuClass				=	'Menu';
const AEEventID		kAEMenuOpenApp				=	'OApp';
const AEEventID		kAESignID					=	'Sign';
const AEEventID		kAEEncryptID				=	'Encr';
const AEEventID		kAEEncryptAndSignID			=	'EnSi';
const AEEventID		kAEDecryptVerifyID			=	'DeVe';
const AEEventID		kAEOptionSignID				=	'OSig';
const AEEventID		kAEOptionEncryptID			=	'OEnc';
const AEEventID		kAEOptionEncryptAndSignID	=	'OEnS';
const AEEventID		kAEOptionDecryptVerifyID	=	'ODeV';
const AEEventID		kAEWipeID					=	'Wipe';

const DescType		cFolder						=	'cfol';
const DescType		pTrash						=	'trsh';




CmenuMenuSelectPatch::CmenuMenuSelectPatch(
	SGlobals &	inGlobals)
		: mGlobals(inGlobals), mWorkingCount(0)
{
}



CmenuMenuSelectPatch::~CmenuMenuSelectPatch()
{
}



	long
CmenuMenuSelectPatch::NewMenuSelect(
	Point	startPt)
{
	SInt32				result = 0;

	try {
#if PGP_FINDERMENU
		if (mWorkingCount == 0) {
			StSaveHeapZone		savedZone(::SystemZone());
			StSaveCurResFile	saveResFile;
			LFile				thePlugFile(mGlobals.plugSpec);
					
			thePlugFile.OpenResourceFork(fsRdPerm);
			AdjustMenuItems();
			thePlugFile.CloseResourceFork();
		}
#endif

#if BETA || PGP_DEMO
		if (mGlobals.disableEncryptAndSign) {
			::DisableItem(mGlobals.menuH, kSign);
			::DisableItem(mGlobals.menuH, kEncrypt);
			::DisableItem(mGlobals.menuH, kEncryptAndSign);
		}
#endif
	}
	
	catch (...) {
	}
	
	result = OldMenuSelect(startPt);
	
	if (HiWord(result) == MENU_PGPmenu) {
		// Semaphore to prevent finder from crashing
		mWorkingCount++;
		try {
			StSaveHeapZone		savedZone(::SystemZone());
			StSaveCurResFile	saveResFile;
			LFile				thePlugFile(mGlobals.plugSpec);

			thePlugFile.OpenResourceFork(fsRdPerm);
			result = PGPmenuFilter(result);
			thePlugFile.CloseResourceFork();
		}
		
		catch (...) {
		}
		
		mWorkingCount--;
	}
	return result;
}



	SInt32
CmenuMenuSelectPatch::PGPmenuFilter(
	SInt32	inSelection)
{
	try {
		switch (LoWord(inSelection)) {
#if PGP_FINDERMENU
			case kSign:
			case kEncrypt:
			case kEncryptAndSign:
			case kDecryptVerify:
			case kWipe:
			{
				SendToolsEvent(LoWord(inSelection));
			}
			break;
#else
			case kSign:
			case kEncrypt:
			case kEncryptAndSign:
			{
				EEncodeOptions	options;
				
				switch (LoWord(inSelection)) {
					case kSign:
					{
						options = encodeOptions_Sign;
					}
					break;
					
					
					case kEncrypt:
					{
						options = encodeOptions_Encrypt;
					}
					break;
					
					
					case kEncryptAndSign:
					{
						options = encodeOptions_Sign
								| encodeOptions_Encrypt;
					}
					break;
				}
				Encode(options);
			}
			break;
			
			
			// Check to see if it is the Finder and then decode a file or
			// text appropriately
			case kDecryptVerify:
			{
				Decode();
			}
			break;
#endif				
			
			// Bring keys to the front
			case kPGPkeys:
			{
				if (BringAppToFront(kPGPkeysType, kPGPkeysCreator,
				kFindAppOnAllVolumes, true, nil, nil) != noErr) {
					WarningAlert(	kWAStopAlertType,
									kWAOKStyle,
									STRx_Strings,
									kKeysNotFoundID);
				}
			}
			break;
		}
	}
	
	catch (CComboError & comboError) {
#if ! PGP_FINDERMENU
		delete CCopyPasteHack::GetCopyPasteHack();
		if (! comboError.IsCancelError()) {
			if (comboError.HavePGPError()) {
				CPGPmenuPlug::ShowError(comboError.pgpErr);
			} else {
				CPGPmenuPlug::ShowError(MacErrorToPGPError(comboError.err));
			}
		}
#endif
	}

	catch(...) {
#if ! PGP_FINDERMENU
		delete CCopyPasteHack::GetCopyPasteHack();
		CPGPmenuPlug::ShowError(kPGPError_UnknownError);
#endif
	}
	
	return 0;
}



	void
CmenuMenuSelectPatch::SendToolsEvent(
	SInt16	inSelection)
{
	StAEDescriptor	theEvent;
	AEEventID		theEventID;
	OSErr			err;
	
	switch (inSelection) {
		case kSign:
		{
			if (mOptionDown) {
				theEventID = kAEOptionSignID;
			} else {
				theEventID = kAESignID;
			}
		}
		break;
		
		
		case kEncrypt:
		{
			if (mOptionDown) {
				theEventID = kAEOptionEncryptID;
			} else {
				theEventID = kAEEncryptID;
			}
		}
		break;
		
		
		case kEncryptAndSign:
		{
			if (mOptionDown) {
				theEventID = kAEOptionEncryptAndSignID;
			} else {
				theEventID = kAEEncryptAndSignID;
			}
		}
		break;
		
		
		case kDecryptVerify:
		{
			if (mOptionDown) {
				theEventID = kAEOptionDecryptVerifyID;
			} else {
				theEventID = kAEDecryptVerifyID;
			}
		}
		break;
		
		
		case kWipe:
		{
			theEventID = kAEWipeID;
		}
		break;
		
		
		default:
		{
			return;
		}
		break;
	}
	


	// Bring tools to the front
	AppParameters	params;
	
	params.theMsgEvent.what = kHighLevelEvent;
	params.theMsgEvent.message = kAEPGPMenuClass;
	params.theMsgEvent.where = *(Point *) & kAEMenuOpenApp;
	params.theMsgEvent.when = params.theMsgEvent.modifiers = 0;
	params.eventRefCon = 0;
	params.messageLength = 0;
	err = BringAppToFront(	kPGPtoolsType,
							kPGPtoolsCreator,
							kFindAppOnAllVolumes,
							true,
							&params,
							nil);
	PGPThrowIfOSErr_(err);

	// Send the event
	ProcessSerialNumber	toolsPSN;
	StAEDescriptor		theAddress;

	PGPThrowIfNot_(FindProcess(	kPGPtoolsType,
								kPGPtoolsCreator,
								&toolsPSN,
								nil,
								nil,
								nil));
	err = ::AECreateDesc(	typeProcessSerialNumber,
							(Ptr) &toolsPSN,
							sizeof(toolsPSN),
							theAddress);
	PGPThrowIfOSErr_(err);
	err = ::AECreateAppleEvent(	kAEPGPMenuClass,
								theEventID,
								theAddress,
								kAutoGenerateReturnID,
								kAnyTransactionID,
								theEvent);
	PGPThrowIfOSErr_(err);
	
	UAppleEventsMgr::SendAppleEvent(theEvent);
}



	void
CmenuMenuSelectPatch::Encode(
	EEncodeOptions	inOptions)
{
	inOptions |= encodeOptions_TreatInputAsText;
	if (inOptions & encodeOptions_Encrypt) {
		inOptions |= encodeOptions_TextOutput;
	} else {
		inOptions |= encodeOptions_ClearSign;
	}

	new CCopyPasteHack(mGlobals, encodeDecode_Encode, inOptions);
}



	void
CmenuMenuSelectPatch::Decode()
{
	new CCopyPasteHack(mGlobals, encodeDecode_Decode);
}
	


/*	SInt32
CmenuMenuSelectPatch::CountFinderSelection(
	DescType	inObjectType)
{
	StSuspendAESpecialHandler	theSuspendedHandler(keyPreDispatch);
	AppleEvent					theCountSelectionEvent = {typeNull, nil};
	AppleEvent					theReply = {typeNull, nil};
	SInt32						result = 0;
				
	try {
		OSErr		err = noErr;

		// Create the AppleEvent
		UAppleEventsMgr::MakeAppleEvent(	kAECoreSuite,
											kAECountElements,
											theCountSelectionEvent);
											
		// Create the specifier
		StAEDescriptor	nullDescriptor;
		StAEDescriptor	selectionProperty((DescType) pUserSelection);
		StAEDescriptor	selectionSpec;
		
		err = ::CreateObjSpecifier(	cProperty,
									nullDescriptor,
									formPropertyID,
									selectionProperty,
									false,
									selectionSpec);
		PGPThrowIfOSErr_(err);

		// Add the direct object to the appleevent
		err = ::AEPutParamDesc(	&theCountSelectionEvent,
								keyDirectObject,
								selectionSpec);
		PGPThrowIfOSErr_(err);
		
		StAEDescriptor	objectType((DescType) inObjectType);
		
		err = ::AEPutParamDesc(	&theCountSelectionEvent,
								keyAEObjectClass,
								objectType);
		PGPThrowIfOSErr_(err);
		
		UAppleEventsMgr::SendAppleEventWithReply(	theCountSelectionEvent,
													theReply,
													false);						
		::AEDisposeDesc(&theCountSelectionEvent);
		theCountSelectionEvent.descriptorType = typeNull;
		
		// Check for errors
		StAEDescriptor	errorCode(theReply, keyErrorNumber);
		
		if (errorCode.mDesc.descriptorType != typeNull) {
			UExtractFromAEDesc::TheInt16(errorCode, err);
			
			PGPThrowIfOSErr_(err);
		}
		
		StAEDescriptor	theCount(theReply, keyDirectObject);
		UExtractFromAEDesc::TheInt32(theCount, result);
		
		// Check for missing parms and dispose of the reply
		UAppleEventsMgr::CheckForMissedParams(theReply);
		::AEDisposeDesc(&theReply);
	}
	
	catch (...) {
		if (theCountSelectionEvent.descriptorType != typeNull) {
			::AEDisposeDesc(&theCountSelectionEvent);
		}
		if (theReply.descriptorType != typeNull) {
			::AEDisposeDesc(&theReply);
		}
	}
	
	return result;
}
*/



	void
CmenuMenuSelectPatch::AdjustMenuItems()
{
	if (CmenuWaitNextEventPatch::GetEventParamsPtr()->theEvent->modifiers
	& optionKey) {
		mOptionDown = true;
/*		::SetMenuItemText(	mGlobals.menuH,
							kSign,
							CString(STRx_Strings, kSignAsID));
		::SetMenuItemText(	mGlobals.menuH,
							kEncrypt,
							CString(STRx_Strings, kEncryptAsID));
		::SetMenuItemText(	mGlobals.menuH,
							kEncryptAndSign,
							CString(STRx_Strings, kEncryptAndSignAsID));
		::SetMenuItemText(	mGlobals.menuH,
							kDecryptVerify,
							CString(STRx_Strings, kDecryptVerifyAsID));
*/	} else {
		mOptionDown = false;
		::SetMenuItemText(	mGlobals.menuH,
							kSign,
							CString(STRx_Strings, kSignID));
		::SetMenuItemText(	mGlobals.menuH,
							kEncrypt,
							CString(STRx_Strings, kEncryptID));
		::SetMenuItemText(	mGlobals.menuH,
							kEncryptAndSign,
							CString(STRx_Strings, kEncryptAndSignID));
		::SetMenuItemText(	mGlobals.menuH,
							kDecryptVerify,
							CString(STRx_Strings, kDecryptVerifyID));
	}
	
	Boolean	trashSelected;
	Boolean	enableItems = CheckFinderSelection(&trashSelected);
	
	if ((enableItems) && (! trashSelected)) {
		::EnableItem(mGlobals.menuH, kSign);
		::EnableItem(mGlobals.menuH, kEncrypt);
		::EnableItem(mGlobals.menuH, kEncryptAndSign);
		::EnableItem(mGlobals.menuH, kDecryptVerify);
	} else {
		::DisableItem(mGlobals.menuH, kSign);
		::DisableItem(mGlobals.menuH, kEncrypt);
		::DisableItem(mGlobals.menuH, kEncryptAndSign);
		::DisableItem(mGlobals.menuH, kDecryptVerify);
	}
		
	if (enableItems) {
		::EnableItem(mGlobals.menuH, kWipe);
	} else {
		::DisableItem(mGlobals.menuH, kWipe);
	}
}



Boolean
CmenuMenuSelectPatch::CheckFinderSelection(
	Boolean *	outTrashSelected)
{
	StSuspendAESpecialHandler	theSuspendedHandler(keyPreDispatch);
	Boolean						result = true;
	
	*outTrashSelected = false;
	
	try {
		StAEDescriptor		theReply;
		OSErr				err = noErr;

		// Send the appleevent
		{
			StAEDescriptor		theGetSelectionEvent;
			StAEDescriptor		theAddress;

			// Create the AppleEvent
			UAppleEventsMgr::MakeAppleEvent(	kAECoreSuite,
												kAEGetData,
												theGetSelectionEvent);

			// Create the specifier
			StAEDescriptor	nullDescriptor;
			StAEDescriptor	selectionProperty( (DescType) pUserSelection );
			StAEDescriptor	selectionSpec;
			
			err = ::CreateObjSpecifier(	cProperty,
										nullDescriptor,
										formPropertyID,
										selectionProperty,
										false,
										selectionSpec);
			PGPThrowIfOSErr_(err);

/*			UInt32			allOrdinal = kAEAll;
			StAEDescriptor	position(	typeAbsoluteOrdinal,
										&allOrdinal,
										sizeof(allOrdinal));
			StAEDescriptor	foldersSpec;

			err = ::CreateObjSpecifier(	cFolder,
										selectionSpec,
										formAbsolutePosition,
										position,
										false,
										foldersSpec);
			PGPThrowIfOSErr_(err);
*/
			// Add the direct object to the appleevent
			err = ::AEPutParamDesc(	theGetSelectionEvent,
									keyDirectObject,
//									foldersSpec);
									selectionSpec);
			PGPThrowIfOSErr_(err);
			
			StAEDescriptor	asObjectSpecifierType(
									(DescType) typeObjectSpecifier);
			
			err = ::AEPutParamDesc(	theGetSelectionEvent,
									keyAERequestedType,
									asObjectSpecifierType);
			PGPThrowIfOSErr_(err);
			
			UAppleEventsMgr::SendAppleEventWithReply(	theGetSelectionEvent,
														theReply,
														false);
		}				
		
		// Check for errors
		StAEDescriptor	errorCode(theReply, keyErrorNumber);
		
		if (errorCode.mDesc.descriptorType != typeNull) {
			UExtractFromAEDesc::TheInt16(errorCode, err);
			
			PGPThrowIfOSErr_(err);
		}
		
		// Walk the list making sure that only files and folders are selected
		AEKeyword		keyWord;
		SInt32			numItems = 0;
		StAEDescriptor	theSelection(theReply, keyDirectObject);
		StAEDescriptor	theSelectionList;
		
		if ((theSelection.mDesc.descriptorType == typeObjectSpecifier)
		|| (theSelection.mDesc.descriptorType == typeAEList)) {
			// Walk the list of items making sure that they are folders or
			// files
			err = ::AECoerceDesc(theSelection, typeAEList, theSelectionList);
			PGPThrowIfOSErr_(err);
			
			err = ::AECountItems(theSelectionList, &numItems);
			PGPThrowIfOSErr_(err);
			PGPThrowIf_(numItems == 0);
			for (SInt32 i = 1; i <= numItems; i++) {
				StAEDescriptor	item;
				StAEDescriptor	itemRecord;
				StAEDescriptor	itemType;
				
				// We coerce the spec into a record to get its type
				err = ::AEGetNthDesc(	theSelectionList,
										i,
										typeObjectSpecifier,
										&keyWord,
										item);
				PGPThrowIfOSErr_(err);
				err = ::AECoerceDesc(item, typeAERecord, itemRecord);
				PGPThrowIfOSErr_(err);
				err = ::AEGetKeyDesc(	itemRecord,
										keyAEDesiredClass,
										typeType,
										itemType);
				PGPThrowIfOSErr_(err);
				
				// Determine whether the item is valid or not
				if (*((DescType *) *itemType.mDesc.dataHandle) == cProperty) {
					StAEDescriptor	form;
					StAEDescriptor	propertyID;
					
					err = ::AEGetKeyDesc(	itemRecord,
											keyAEKeyForm,
											typeType,
											form);
					PGPThrowIfOSErr_(err);
					if (*((DescType *) *form.mDesc.dataHandle)
					!= formPropertyID) {
						result = false;
						break;
					}
					err = ::AEGetKeyDesc(	itemRecord,
											keyAEKeyData,
											typeType,
											propertyID);
					PGPThrowIfOSErr_(err);
					if (*((DescType *) *propertyID.mDesc.dataHandle)
					!= pTrash) {
						result = false;
						break;
					} else {
						*outTrashSelected = true;
					}
				} else {
					if ((*((DescType *) *itemType.mDesc.dataHandle) != cFile)
					&& (*((DescType *) *itemType.mDesc.dataHandle)
					!= cFolder)) {
						result = false;
						break;
					}
				}
			}
		} else {
			result = false;
		}
		
		// Check for missing parms and dispose of the reply
		UAppleEventsMgr::CheckForMissedParams(theReply);
	}
	
	catch (...) {
		result = false;
	}
	
	return result;
}
