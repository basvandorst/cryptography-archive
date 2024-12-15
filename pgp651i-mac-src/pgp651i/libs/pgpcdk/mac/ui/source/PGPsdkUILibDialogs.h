/*____________________________________________________________________________	Copyright (C) 1997 Network Associates Inc. and affiliated companies.	All rights reserved.		$Id: PGPsdkUILibDialogs.h,v 1.14 1999/03/10 02:55:55 heller Exp $____________________________________________________________________________*/#pragma once#include <Dialogs.h>#include <PP_Types.h>#include "MacBasics.h"#include "pgpPubTypes.h"#include "pgpGroups.h"#include "pflPrefs.h"#include "WarningAlertTypes.h"const ResID		kPGPLibDialogsStringListResID	= 4747;enum{	kPrivateKeyPopupFormatStrIndex	= 1,	kRSAAlgorithmStringIndex,	kDSAAlgorithmStringIndex,	kUnknownAlgorithmStringIndex,	kNoSigningKeysFoundStrIndex,		kConfirmMatchingKeyStrIndex,	kConvEncryptionPassphraseNotConfirmedStrIndex,		kDSSDHKeySizeFormatStrIndex,	kNumGroupKeysFormatStrIndex,	kNumMissingGroupKeysFormatStrIndex,		kKeyADKNotFoundErrorStrIndex,	kKeyADKNotFoundWarningStrIndex,	kGroupADKNotFoundErrorStrIndex,	kGroupADKNotFoundWarningStrIndex,		kUnknownRecipientStrIndex,	kXUnknownRecipientsStrIndex,		kRemoveADKViolatesPolicyStrIndex,	kIncorrectPassphraseStrIndex,		kKeyServerConnectingStrIndex,	kKeyServerSearchingStrIndex,	kKeyServerReceivingStrIndex,	kKeyServerProcessingStrIndex,	kKeyServerSendingStrIndex,	kKeyServerDeletingSendingStrIndex,	kKeyServerDisablingSendingStrIndex,	kKeyServerClosingSendingStrIndex,		kNoDecryptionKeysFoundStrIndex,	kUnknownKeyIDStrIndex};#define kPGPUIGenericOKCancelAlert		(ResIDT)4755#define kPGPUIGenericOKAlert			(ResIDT)4756MessageT		PGPUIWarningAlert( ResIDT dialogResID);MessageT		PGPUIWarningAlert(					ResIDT dialogResID,					ConstStr255Param message, ConstStr255Param str1 = NULL,					ConstStr255Param str2=NULL, ConstStr255Param str3 = NULL);	MessageT		PGPUIWarningAlert(					ResIDT dialogResID,					ResID stringListResID, short stringListIndex,					ConstStr255Param str1 = NULL,					ConstStr255Param str2 = NULL,					ConstStr255Param str3 = NULL);