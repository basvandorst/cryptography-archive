//////////////////////////////////////////////////////////////////////////////
// CommonStrings.h
//
// Contains declarations for CommonStrings.cpp.
//////////////////////////////////////////////////////////////////////////////

// $Id: CommonStrings.h,v 1.1.2.10.2.4 1998/10/25 23:31:35 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CommonStrings_h	// [
#define Included_CommonStrings_h


////////////
// Constants
////////////

const PGPUInt32 kPGPdiskMaxCommonStrings = 1024;

static LPCSTR kPGPdiskUnknownString = "Unknown string.";


//////////
// Externs
//////////

extern LPCSTR kPGPdiskCommonStrings[kPGPdiskMaxCommonStrings];


////////
// Types
////////

enum kPGPdiskCommonStrings
{
	kPGPdiskCmdLineHelpString, 
	kPGPdiskBadCmdLineString, 
	kPGPdiskBecauseString, 
	kPGPdiskNoMemForErrorString, 
	kPGPdiskRemoveAltsSuccessString, 
	kPGPdiskNoPGPdisksToUnmountString, 
	kPGPdiskLicenseToText, 
	kPGPdiskDemoTimeoutWarning, 
	kPGPdiskDemoBadCommandWarning, 
	kPGPdiskURL, 
	kPGPdiskBuyNowURL, 
	kPGPdiskCPDTAddPassMsg, 
	kPGPdiskCPDTChangePassMsg, 
	kPGPdiskSDTCreateDialogTitle, 
	kPGPdiskSDTSaveFilter, 
	kPGPdiskSPDTChangePassMsg, 
	kPGPdiskSPDTMasterPassMsg, 
	kPGPdiskSPDTMountPassMsg, 
	kPGPdiskSPDTPublicKeyPassMsg, 
	kPGPdiskSPDTRemovePassMsg, 
	kPGPdiskPrefsSheetTitleString, 
	kPGPdiskDiskWizSheetTitleString, 
	kPGPdiskCreationCreateMsgString, 
	kPGPdiskCreationDoneMsgString, 
	kPGPdiskFreeSize1MsgString, 
	kPGPdiskFreeSize2MsgString, 
	kPGPdiskFreeSize3MsgString, 
	kPGPdiskOpenCmdString, 
	kPGPdiskGlobalConvertCmdString, 
	kPGPdiskCreateCmdString, 
	kPGPdiskMountCmdString, 
	kPGPdiskUnmountCmdString, 
	kPGPdiskUnmountAllCmdString, 
	kPGPdiskAddPassCmdString, 
	kPGPdiskChangePassCmdString, 
	kPGPdiskRemovePassCmdString, 
	kPGPdiskRemoveAltsCmdString, 
	kPGPdiskPubKeysCmdString, 
	kPGPdiskWipeAllCmdString, 
	kPGPdiskWipeIndCmdString, 
	kPGPdiskAboutCmdString, 
	kPGPdiskHelpCmdString, 
	kPGPdiskPrefsCmdString, 
	kPGPdiskShellExtOpenMenuString, 
	kPGPdiskShellExtMountMenuString, 
	kPGPdiskShellExtUnmountMenuString, 
	kPGPdiskShellExtAddPassMenuString, 
	kPGPdiskShellExtChangePassMenuString, 
	kPGPdiskShellExtRemovePassMenuString, 
	kPGPdiskShellExtRemoveAltsMenuString, 
	kPGPdiskShellExtPubKeysMenuString, 
	kPGPdiskShellExtHelpMenuString, 
	kPGPdiskShellExtDescribeOpenString, 
	kPGPdiskShellExtDescribeMountString, 
	kPGPdiskShellExtDescribeUnmountString, 
	kPGPdiskShellExtDescribeAddPassString, 
	kPGPdiskShellExtDescribeChangePassString, 
	kPGPdiskShellExtDescribeRemovePassString, 
	kPGPdiskShellExtDescribeRemoveAltsString, 
	kPGPdiskShellExtDescribePubKeysString, 
	kPGPdiskShellExtDescribeHelpString, 
	kPGPdiskBetaTimeOutWarning, 
	kPGPdiskVerifyRemoveAlternates, 
	kPGPdiskNotifyMountMaybeCorrupt, 
	kPGPdiskConfirmFormat, 
	kPGPdiskAskToSavePrefs, 
	kPGPdiskPGPdiskIsNotEncrypted, 
	kPGPdiskConfirmCancelFormat, 
	kPGPdiskResConfirmPGPdiskResQuit, 
	kPGPdiskConfirmCancelCreation, 
	kPGPdiskForceROOnLoopbackString, 
	kPGPdiskErrorWasString, 
	kPGPdiskConfirmWipeAllPasses, 
	kPGPdiskConfirmWipeIndPasses, 
	kPGPdiskBadCASTAskConvertPNM, 
	kPGPdiskBadCASTAskContinuePNM, 
	kPGPdiskBadCASTAskConvertPM, 
	kPGPdiskBadCASTAskContinuePM, 
	kPGPdiskBadCASTReadOnly, 
	kPGPdiskConvertWizSheetTitleString, 
	kPGPdiskConversionConvertMsgString, 
	kPGPdiskConversionDoneMsgString, 
	kPGPdiskConfirmCancelConversion, 
	kPGPdiskGlobalCWizSheetTitleString, 
	kPGPdiskConfirmCancelSearching, 
	kPGPdiskSearchingSearchMsgString, 
	kPGPdiskSearchingDoneMsgString, 
	kPGPdiskConversionWipedMsgString
};

#endif // Included_CommonStrings_h
