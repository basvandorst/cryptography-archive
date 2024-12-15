/*____________________________________________________________________________
	TranslatorStrings.h
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: TranslatorStrings.h,v 1.5 1997/09/18 23:30:26 lloyd Exp $
____________________________________________________________________________*/

 


#define kTranslatorStringsRID	128	// STR#
enum
{
	kSelectiveImportPromptStrIndex = 1,
	kSelectiveImportAfterSendPromptStrIndex
};



#define kTranslatorErrorStringsResID	129	// STR#
enum
{
	kKeysAddedSuccessfullyStrIndex = 1,
	kKeyAddedSuccessfullyStrIndex,
	kErrorWhileAddingKeyStrIndex,
	kMessageIsNotSignedStrIndex,
	kMessageIsNotEncryptedStrIndex,
	kCantAddKeyAddItInPGPkeysInsteadStrIndex,
	kNoKeysPresentStrIndex,
	kPleaseEnterSigningPassphraseStrIndex,
	kPleaseEnterDecryptionPassphraseStrIndex,
	kPassphraseInvalidPleaseReenterError,
	kGenericErrorTemplateString,
	kNoDefaultKeyStrIndex,
	kOutOfMemoryStrIndex,
	kNoRSAStrIndex,
	
	kKeyringsNotFoundStrIndex,
	kAdminPrefsNotFoundStrIndex
};
