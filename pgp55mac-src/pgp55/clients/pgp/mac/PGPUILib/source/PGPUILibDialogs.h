/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	$Id: PGPUILibDialogs.h,v 1.25 1997/10/17 17:40:52 heller Exp $
____________________________________________________________________________*/

#pragma once

#include <Dialogs.h>

#include <PP_Types.h>

#include "MacBasics.h"
#include "pgpPubTypes.h"
#include "pgpGroups.h"
#include "pflPrefs.h"
#include "WarningAlertTypes.h"

const ResID		kPGPLibDialogsStringListResID	= 4747;

enum
{
	kPrivateKeyPopupFormatStrIndex	= 1,
	kRSAAlgorithmStringIndex,
	kDSAAlgorithmStringIndex,
	kUnknownAlgorithmStringIndex,
	kNoSigningKeysFoundStrIndex,
	
	kInvalidKeysStrIndex,
	kKeysNotFoundStrIndex,
	kMultipleKeysFoundStrIndex,
	
	kConfirmMatchingKeyStrIndex,
	
	kKeysNotSignedByCorporateKeyStrIndex,
	kConvEncryptionPassphraseNotConfirmedStrIndex,
	
	kDSSDHKeySizeFormatStrIndex,
	kNumGroupKeysFormatStrIndex,
	kNumMissingGroupKeysFormatStrIndex,
	
	kErrorSearchingServerStrIndex,
	kNoDefaultPrivateKeyFoundStrIndex,
	
	kKeyADKNotFoundErrorStrIndex,
	kKeyADKNotFoundWarningStrIndex,
	kGroupADKNotFoundErrorStrIndex,
	kGroupADKNotFoundWarningStrIndex,
	
	kCorporateADKDisabledOrMissingStrIndex,
	kCorporateSigningKeyMissingStrIndex,
	
	kUnknownKeyStrIndex,
	kXUnknownKeysStrIndex
};


void			InitializePowerPlant(void);
void			BevelBorder(const Rect &frame);

PGPError		PGPGetKeyFromServerInternal(
							PGPContextRef 	context,
							PGPPrefRef 		clientPrefsRef,
							PGPKeyID const * 	keyID,
							PGPKeyRef 		key,
							const char *	domain,
							PGPUInt32 		dialogDelayTicks,
							PGPKeySetRef 	*resultSet);
							
PGPError		PGPGetGroupFromServerInternal(
							PGPContextRef 	context,
							PGPPrefRef		clientPrefsRef,
							PGPGroupSetRef	groupSet,
							PGPGroupID		groupID,
							const char		*domain,
							PGPUInt32		dialogDelayTicks,
							PGPKeySetRef	*resultSet);

#define kPGPUIGenericOKCancelAlert		(ResIDT)4755
#define kPGPUIGenericOKAlert			(ResIDT)4756

MessageT		PGPUIWarningAlert( ResIDT dialogResID);

MessageT		PGPUIWarningAlert(
					ResIDT dialogResID,
					ConstStr255Param message, ConstStr255Param str1 = NULL,
					ConstStr255Param str2=NULL, ConstStr255Param str3 = NULL);
	
MessageT		PGPUIWarningAlert(
					ResIDT dialogResID,
					ResID stringListResID, short stringListIndex,
					ConstStr255Param str1 = NULL,
					ConstStr255Param str2 = NULL,
					ConstStr255Param str3 = NULL);
