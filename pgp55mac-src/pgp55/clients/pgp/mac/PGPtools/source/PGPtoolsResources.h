/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

const ResID		window_Main		= 2000;
const ResID		window_About	= 3000;
const ResID		window_Text		= 4000;
const ResID		window_Results	= 5000;

const ResID		kPGPtoolsMiscStringsResID		= 1000;
const ResID		kErrorStringListResID			= 1001;
const ResID		kResultsTableStringListResID	= 1002;
const ResID		kDialogStringsListResID			= 1003;

const ResID		kAnimatedCursorResID			= 128;

enum	// kPGPtoolsMiscStringsResID
{
	kSaveEncryptedFileAsPromptStrIndex	= 1,
	kSaveSignedFileAsPromptStrIndex,
	kSaveDecryptedFileAsPromptStrIndex,
	kSaveAsPromptStrIndex,

	kBinaryFileSuffixStrIndex,
	kDetachedBinarySigFileSuffixStrIndex,
	kASCIIFileSuffixStrIndex,

	kEncryptStrIndex,
	kSignStrIndex,
	kEncryptSignStrIndex,
	kDecryptStrIndex,
	kDecryptVerifyStrIndex,
	kWipeStrIndex,
	
	kSelectStrIndex,

	kShowResultsStrIndex,
	kHideResultsStrIndex,
	
	kClipboardStrIndex,
	
	kPGPWebSiteURLStrIndex,
	kAboutCreditsButtonTitleStrIndex,
	kAboutInfoButtonTitleStrIndex
};

enum	// kErrorStringListResID
{
	kLibraryInitErrorStrIndex	= 1,
	kNoKeyringsFoundStrIndex,
	kNoKeysFoundStrIndex,
	kNoPublicKeysFoundStrIndex,
	kNoPrivateKeysFoundStrIndex,
	kNoAdminPrefsFileFoundStrIndex,
	
	kAnErrorOccurredStrIndex,
	kOperationCouldNotBeCompletedStrIndex,
	
	kFileCannotBeEncryptedStrIndex,
	kFileCannotBeSignedStrIndex,
	kFileCannotBeDecryptedVerifiedStrIndex,

	kClipboardCannotBeEncryptedStrIndex,
	kClipboardCannotBeSignedStrIndex,
	kClipboardCannotBeDecryptedVerifiedStrIndex,

	kTextCannotBeEncryptedStrIndex,
	kTextCannotBeSignedStrIndex,
	kTextCannotBeDecryptedVerifiedStrIndex,

	kNamedTextCannotBeEncryptedStrIndex,
	kNamedTextCannotBeSignedStrIndex,
	kNamedTextCannotBeDecryptedVerifiedStrIndex,
	
	kCantLaunchPGPkeysBecauseStrIndex,
	kCantLaunchWebBrowserBecauseStrIndex,
	
	kErrorWhileWipingStrIndex
};

enum	// kResultsTableStringListResID
{
	kNameColumnTitleStrIndex = 1,
	kStatusColumnTitleStrIndex,
	kValidityColumnTitleStrIndex,
	kSigningDateColumnTitleStrIndex,
	
	kVerifiedOneItemStrIndex,
	kVerifiedNItemsStrIndex,
	
	kNoSignatureStrIndex,
	kVerifiedSignatureStrIndex,
	kVerifiedExpiredStrIndex,
	kVerifiedRevokedStrIndex,
	kVerifiedDisabledStrIndex,
	kVerifiedCorrputStrIndex,
	kCouldNotVerifyStrIndex,
	kCouldNotVerifyKeyIDStrIndex,
	kBadSignatureStrIndex,
	
	kSelectedTextStrIndex,
	kDataForkOnlyFormatStrIndex
};

enum	// kDialogStringsListResID
{
	kTextLargerThan32KOptionsStrIndex	= 1,
	
	kEncryptingProgressFormatStrIndex,
	kSigningProgressFormatStrIndex,
	kEncryptingSigningProgressFormatStrIndex,
	kDecryptingVerifyingProgressFormatStrIndex,
	kWipingProgressFormatStrIndex,
	
	kGetPassphraseForFilePromptStrIndex,
	kGetPassphraseForClipboardPromptStrIndex,
	kGetPassphraseForNamedTextPromptStrIndex,
	kGetPassphraseForSelectedTextPromptStrIndex,
	kIncorrectPassphrasePrefixStrIndex,
	kSigningPassphraseIncorrectTryAgainStrIndex,
	
	kLocateDetachedSigSourceFilePromptStrIndex,
	kPleaseEnterSigningPassphrasePromptStrIndex,
	kPleaseEnterConventionalPassphrasePromptStrIndex,
	
	kConfirmMacBinaryOffStrIndex,
	
	kConfirmWipingItemsAfterSigningStrIndex,
	kConfirmWipingItemsAfterEncryptingStrIndex,
	kConfirmWipingItemsAfterEncryptingSigningStrIndex,
	kConfirmWipingItemAfterSigningStrIndex,
	kConfirmWipingItemAfterEncryptingStrIndex,
	kConfirmWipingItemAfterEncryptingSigningStrIndex,
	kConfirmWipingItemsStrIndex,
	kConfirmWipingItemStrIndex,
	
	kConfirmNotEncryptingToSelf,	
	kSelectiveImportUpdatedServerPromptStrIndex,
	kSelectiveImportFoundKeysPromptStrIndex,
	
	kDummyLastDialogStringsStrIndex
};




