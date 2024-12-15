/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: TranslatorIDs.h,v 1.3.8.1 1998/11/12 03:11:50 heller Exp $
____________________________________________________________________________*/
#ifndef Included_TranslatorIDs_h	/* [ */
#define Included_TranslatorIDs_h

typedef enum TranslatorID
{
	kInvalidTranslatorID = 0,
	
	// note: ids must start at 1 and be sequential
	kFirstTranslatorID = 1,

	kDecryptTranslatorID = kFirstTranslatorID,
	kVerifyTranslatorID,
	kEncryptTranslatorID ,
	kSignTranslatorID,
	kEncryptAndSignTranslatorID,
	
	kFirstManualTranslatorID,
	
	kManualEncryptTranslatorID = kFirstManualTranslatorID,
	kManualSignTranslatorID,
	kManualEncryptSignTranslatorID, 
	kManualDecryptVerifyTranslatorID,

	kLastTranslatorIDPlusOne,
	
	kPGPNumTranslators	= kLastTranslatorIDPlusOne - kFirstTranslatorID
} TranslatorID;


typedef enum SpecialID
{
	kInvalidSpecialID = 0,
	
	// note: ids must start at 1 and be sequential
	kFirstSpecialID = 1,

	kSpecialLaunchKeysID = kFirstSpecialID,

	kLastSpecialIDPlusOne,
	
	kPGPNumSpecials	= kLastSpecialIDPlusOne - kFirstSpecialID
} TranslatorID;


#endif /* ] Included_TranslatorIDs_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/