/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: PGPCMPlugin.cp,v 1.8.8.1 1997/12/05 22:13:45 mhw Exp $
____________________________________________________________________________*/


#include <AERegistry.h>
#include <CodeFragments.h>
#include "ContextualMenuPlugins.h"

#include "MacProcesses.h"
#include "MacFiles.h"
#include "MacDesktop.h"

#include "UPGPException.h"
#include "CString.h"
#include "StSaveCurResFile.h"
#include "PGPCMPlugin.h"

// Constants
const ResIDT		STRx_MenuItems				=	1000;
const SInt16		kEncryptID					=	1;
const SInt16		kSignID						=	2;
const SInt16		kEncryptAndSignID			=	3;
const SInt16		kDecryptVerifyID			=	4;
const SInt16		kWipeID						=	5;
const SInt16		kPGPID						=	6;

const OSType		kPGPtoolsType				=	'APPL';
const OSType		kPGPtoolsCreator			=	'pgpM';

const AEEventClass	kAEPGPMenuClass				=	'Menu';
const AEEventID		kAEMenuOpenApp				=	'OApp';
const AEEventID		kAESignID					=	'Sign';
const AEEventID		kAEEncryptID				=	'Encr';
const AEEventID		kAEEncryptAndSignID			=	'EnSi';
const AEEventID		kAEDecryptVerifyID			=	'DeVe';
const AEEventID		kAEWipeID					=	'Wipe';

// Menu items
const SInt16		kEncrypt					=	1;
const SInt16		kSign						=	2;
const SInt16		kEncryptAndSign				=	3;
const SInt16		kDecryptVerify				=	4;
const SInt16		kWipe						=	5;

const DescType		cFolder						=	'cfol';
const DescType		pTrash						=	'trsh';

static FSSpec		sFileSpec;

// Function declarations
extern pascal OSErr __initialize(CFragInitBlockPtr);
pascal OSErr ContextTypeCMPluginInitialize(CFragInitBlockPtr init);
OSStatus AddCommandToAEDescList(ConstStr255Param inCommandString,
	SInt32 inCommandID, AEDescList* ioCommandList);
OSErr BringToolsToFront();
void SendToolsEvent(SInt16	inSelection);


	pascal OSErr
ContextTypeCMPluginInitialize(CFragInitBlockPtr init)
{
	OSErr	theError = __initialize(init);
	if (theError == noErr){
		PGPCMPlugin* thePlugin = new PGPCMPlugin;
		if (thePlugin != NULL) {
			delete thePlugin;
		} else {
			theError = memFullErr;
		}
		
		UAppleEventsMgr::Initialize();
	}

	return theError;
}




	OSStatus 
PGPCMPlugin::Initialize(
	Environment*,
	FSSpec*			inFileSpec)
{
	// Set debug info
	SetDebugThrow_(debugAction_Nothing);
	SetDebugSignal_(debugAction_Nothing);

	::BlockMoveData(inFileSpec, &sFileSpec, sizeof(FSSpec));

    return noErr;
}



	OSStatus 
PGPCMPlugin::ExamineContext(
	Environment*,
	AEDesc *inContextDescriptor,
	SInt32 inTimeOutInTicks,
	AEDescList* ioCommands,
	Boolean* outNeedMoreTime)
{
	OSStatus		err;

	try {
		*outNeedMoreTime = false;
		if (inContextDescriptor != nil) {
			StAEDescriptor	theSubMenu;
			StAEDescriptor	theSubMenuList;
			StAEDescriptor	list;
			SInt32			numItems;
			AEKeyword		keyWord;
			Boolean			addMenuItems = true;
			Boolean			isTrash = false;
			
			// Walk the list of items making sure that they are folders or
			// files
			err = ::AECoerceDesc(inContextDescriptor, typeAEList, list);
			PGPThrowIfOSErr_(err);
			
			err = ::AECountItems(list, &numItems);
			PGPThrowIfOSErr_(err);
			
			for (SInt32 i = 1; i <= numItems; i++) {
				StAEDescriptor	item;
				StAEDescriptor	itemRecord;
				StAEDescriptor	itemType;
				
				// We coerce the spec into a record to get its type
				err = ::AEGetNthDesc(	list,
										i,
										typeObjectSpecifier,
										&keyWord,
										item);
				PGPThrowIfOSErr_(err);
				err = ::AECoerceDesc(	item,
										typeAERecord,
										itemRecord);
				PGPThrowIfOSErr_(err);
				err = ::AEGetKeyDesc(	itemRecord,
										keyAEDesiredClass,
										typeType,
										itemType);
				PGPThrowIfOSErr_(err);
				
				// Bail if it is not a file or folder or trash
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
						addMenuItems = false;
						break;
					}
					err = ::AEGetKeyDesc(	itemRecord,
											keyAEKeyData,
											typeType,
											propertyID);
					PGPThrowIfOSErr_(err);
					if (*((DescType *) *propertyID.mDesc.dataHandle)
					!= pTrash) {
						addMenuItems = false;
						break;
					} else {
						isTrash = true;
					}
				} else {
					if ((*((DescType *) *itemType.mDesc.dataHandle) != cFile)
					&& (*((DescType *) *itemType.mDesc.dataHandle)
					!= cFolder)) {
						addMenuItems = false;
						break;
					}
				}
			}
				
			SetDebugThrow_(debugAction_LowLevelDebugger);
			SetDebugSignal_(debugAction_LowLevelDebugger);
			if (addMenuItems) {
				// Save current resource info and then get our menu string
				StSaveCurResFile	savedResFile(sFileSpec);
				
				// Get our submenu list
				err = ::AECreateList(	nil,
										0,
										false,
										theSubMenuList);
				PGPThrowIfOSErr_(err);
				if (! isTrash) {
					AddCommandToAEDescList(	CString(	STRx_MenuItems,
														kEncryptID),
											kEncrypt,
											theSubMenuList);
					AddCommandToAEDescList(	CString(	STRx_MenuItems,
														kSignID),
											kSign,
											theSubMenuList);
					AddCommandToAEDescList(	CString(	STRx_MenuItems,
														kEncryptAndSignID),
											kEncryptAndSign,
											theSubMenuList);
					AddCommandToAEDescList(	CString(	STRx_MenuItems,
														kDecryptVerifyID),
											kDecryptVerify,
											theSubMenuList);
				}
				AddCommandToAEDescList(	CString(	STRx_MenuItems,
													kWipeID),
										kWipe,
										theSubMenuList);
			
				// Add PGP with submenu to ioCommands
				CString				menuItem(STRx_MenuItems, kPGPID);

				err = ::AECreateList(	nil,
										0,
										true,
										theSubMenu);
				PGPThrowIfOSErr_(err);
				err = ::AEPutKeyPtr(	theSubMenu,
										keyAEName,
										typeChar,
										(char *) menuItem,
										menuItem.GetLength());
				PGPThrowIfOSErr_(err);
				err = ::AEPutKeyDesc(	theSubMenu,
										keyContextualMenuSubmenu,
										theSubMenuList);
				err = ::AEPutDesc(	ioCommands,
									0,
									theSubMenu);
				PGPThrowIfOSErr_(err);
			}
		}
	}
	
	catch (...) {
	}
	
	return noErr;
}



	OSStatus
PGPCMPlugin::HandleSelection(
	Environment*,
	AEDesc *inContextDescriptor,
	SInt32 inCommandID)
{
	try {
		SendToolsEvent(inCommandID);
	}
	
	catch (...) {
	}
	
    return noErr;
}



	OSStatus
PGPCMPlugin::PostMenuCleanup(
	Environment*)
{
    return noErr;
}


	OSStatus
AddCommandToAEDescList(
	ConstStr255Param inCommandString,
	SInt32 inCommandID,
	AEDescList* ioCommandList)
{
	OSStatus err = noErr;
	
	AERecord theCommandRecord = { typeNull, nil };
	
	// create an apple event record for our command
	err = ::AECreateList(	nil,
							0,
							true,
							&theCommandRecord);
	if (err == noErr) {
		// stick the command text into the aerecord
		err = ::AEPutKeyPtr(	&theCommandRecord,
								keyAEName,
								typeChar,
								&inCommandString[1],
								inCommandString[0]);
		if (err == noErr) {
			// stick the command ID into the AERecord
			err = ::AEPutKeyPtr(	&theCommandRecord,
									keyContextualMenuCommandID,
									typeLongInteger,
									&inCommandID,
									sizeof(inCommandID));
			if (err == noErr) {
				// stick this record into the list of commands that
				// we are passing back to CMM
				err = ::AEPutDesc(	ioCommandList,
									0,
									&theCommandRecord);
			}
		}
	}
		
	// clean up after ourself; dispose of the AERecord
	::AEDisposeDesc(&theCommandRecord);

    return err;
    
}



	void
SendToolsEvent(
	SInt16	inSelection)
{
	StAEDescriptor	theEvent;
	AEEventID		theEventID;
	OSErr			err;
	
	switch (inSelection) {
		case kSign:
		{
			theEventID = kAESignID;
		}
		break;
		
		
		case kEncrypt:
		{
			theEventID = kAEEncryptID;
		}
		break;
		
		
		case kEncryptAndSign:
		{
			theEventID = kAEEncryptAndSignID;
		}
		break;
		
		
		case kDecryptVerify:
		{
			theEventID = kAEDecryptVerifyID;
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
