/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	TranslatorIDs.h,v 1.2 1997/10/22 23:05:58 elrod Exp
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
	
	kManualEncryptSignTranslatorID = kFirstManualTranslatorID,
	kManualDecryptVerifyTranslatorID,
	kManualAddKeyTranslatorID, 
	kLastTranslatorIDPlusOne,
	
	kPGPNumTranslators	= kLastTranslatorIDPlusOne - kFirstTranslatorID
} TranslatorID;



#endif /* ] Included_TranslatorIDs_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/