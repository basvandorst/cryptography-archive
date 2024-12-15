/*____________________________________________________________________________
	TranslatorIDs.h
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: TranslatorIDs.h,v 1.3 1997/08/08 20:54:18 lloyd Exp $
____________________________________________________________________________*/

#pragma once


typedef enum TranslatorID
{
	kInvalidTranslatorID = 0,
	
	// note: ids must start at 1 and be sequential
	kFirstTranslatorID = 1,
	
	kEncryptTranslatorID = kFirstTranslatorID,
	kSignTranslatorID,
	kEncryptAndSignTranslatorID,
	
	kDecryptTranslatorID,
	kVerifyTranslatorID,
	
	kManualSignTranslatorID,
	kManualEncryptTranslatorID,
	kManualEncryptSignTranslatorID,
	kManualDecryptVerifyTranslatorID,
	kManualAddKeyTranslatorID,
	
#if PGPMIME_KEYS
	kAddKeysTranslatorID,
#endif
	
	kLastTranslatorIDPlusOne,
	
	kPGPNumTranslators	= kLastTranslatorIDPlusOne - kFirstTranslatorID
} TranslatorID;


const SInt16	kEncryptIconSuiteID			=	500;
const SInt16	kSignIconSuiteID			=	501;
const SInt16	kAddKeysIconSuiteID			=	502;