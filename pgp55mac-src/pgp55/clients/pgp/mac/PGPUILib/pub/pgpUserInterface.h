/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	This file contains the prototypes for functions which use UI to interact
	with the user.

	$Id: pgpUserInterface.h,v 1.31 1997/10/10 18:48:23 heller Exp $
____________________________________________________________________________*/

#ifndef Included_pgpUserInterface_h	/* [ */
#define Included_pgpUserInterface_h

#include "pgpPubTypes.h"

#include "pgpGroups.h"
#include "pgpKeyServerPrefs.h"
#include "pgpMacClientErrors.h"

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

typedef struct PGPKeySpec
{
	char	userIDStr[256];		/* Null terminated string */

} PGPKeySpec;

enum
{
	/* relevant signing and encryption */
	kPGPUISettingsTextOutput		= (1UL << 0 ),
	kPGPUISettingsNoMacBinary		= (1UL << 1),
	kPGPUISettingsSmartMacBinary	= (1UL << 2),
	kPGPUISettingsForceMacBinary	= (1UL << 3),
	
	/* encryption only */
	kPGPRecipientSettingsWipeOriginal		= (1UL << 8),
	kPGPRecipientSettingsConvEncrypt		= (1UL << 9),
	
	/* signing only */
	kPGPGetPassphraseSettingsDetachedSig	= (1UL << 16)
};

/* Default values for settings. MacBinary popup default comes */
/* from client prefs file. */

#define kPGPRecipientSettingsDefault		0
#define kPGPGetPassphraseSettingsDefault	0


typedef PGPFlags	PGPUISettings;
typedef PGPFlags	PGPRecipientSettings;
typedef PGPFlags	PGPGetPassphraseSettings;



/* PGPRecipientOptions */
typedef PGPFlags	PGPRecipientOptions;
enum
{
	kPGPRecipientOptionsHideFileOptions		= (1L << 0),
	kPGPRecipientOptionsAlwaysShowDialog	= (1L << 1),

	kPGPRecipientOptionsDefault = 0
};


/* PGPRecipientOptions */
typedef PGPFlags	PGPGetPassphraseOptions;
enum
{
	kPGPGetPassphraseOptionsHideFileOptions	= (1L << 0),

	kPGPGetPassphraseOptionsDefault = 0
};



//	numDefaultRecipients containes the number of recipients already present
//	in the recipients list. Specify 0 for no default recipients.
//	defaultRecipients is the list of numDefaultRecipients recipients. Specify
//	nil for no default recipients.
//	dialogOptions contains programatic options for controlling dialog behavior.
//	defaultSettings contains the initial states of the dialog checkboxes.
//	userSettings contains the final states of the dialog checkboxes.
//	actualRecipients specifies the PGP encrypted
//  recipients chosen by the user. Use PGPFreeRecipientList() to delete the
//  recipient list. 

	PGPError
PGPRecipientDialog(
	PGPContextRef			context,					
	PGPKeySetRef			allKeys,					
	PGPUInt32				numDefaultRecipients,		
	const PGPKeySpec		*defaultRecipients,			
	PGPRecipientOptions		dialogOptions,				
	PGPRecipientSettings	defaultSettings,			
	PGPRecipientSettings	*userSettings,				
	PGPKeySetRef			*actualRecipients,			
	PGPBoolean				*didShowDialog,
	PGPKeySetRef			*newKeys);			
							 
			

	PGPError
PGPGetConventionalEncryptionPassphraseDialog(
	PGPContextRef	context,							
	const char		*prompt,							
	char			passphrase[256]);					

	PGPError
PGPGetConventionalDecryptionPassphraseDialog(
	PGPContextRef	context,							
	const char		*prompt,							
	char			passphrase[256]);					

	PGPError
PGPGetDecryptionPassphraseDialog(
	PGPContextRef	context,							
	const char		*prompt,
	PGPKeySetRef	recipientSet,			
	PGPUInt32		numMissingRecipients,
	PGPKeyRef		preferredKey,						
	char			passphrase[256],
	PGPKeyRef *		outKey );					


/* caller must call PGPFreeKey() on actualSigningKey */
	PGPError
PGPGetSigningPassphraseDialog(
	PGPContextRef				context,				
	PGPKeySetRef				allKeys,				
	const char 					*prompt,				
	PGPGetPassphraseOptions		dialogOptions,			
	PGPGetPassphraseSettings	defaultSettings,		
	PGPKeyRef					defaultSigningKey,		
	char						passphrase[256],
	PGPGetPassphraseSettings	*userSettings,			
	PGPKeyRef 					*actualSigningKey);		
	
	PGPError
PGPSelectiveImportDialog(
	PGPContextRef		context,							
	const char			*prompt,							
	PGPKeySetRef		fromSet,							
	PGPKeySetRef		validitySet,
	PGPKeySetRef		*selectedSet);	/* Caller is responsible for	*/
										/* freeing selectedSet			*/

	PGPError
PGPGetKeyFromServer(
	PGPContextRef		context,
	PGPKeyID const *	keyID,
	PGPKeyRef			key,		/* if not-NULL, used for domain info */
	const char			*domain,	/* user ID or domain info */
	PGPUInt32			dialogDelayTicks,
	PGPKeySetRef		*resultSet);

	PGPError
PGPGetGroupFromServer(
	PGPContextRef		context,
	PGPGroupSetRef		groupSet,
	PGPGroupID			groupID,
	const char			*domain,	/* if not-NULL, used for domain info */
	PGPUInt32			dialogDelayTicks,
	PGPKeySetRef		*resultSet);
	
	PGPError
PGPSendKeyToServer(
	PGPContextRef		context,
	PGPKeyRef			key,
	PGPUInt32			dialogDelayTicks,
	PGPKeyServerEntry	*targetServer = NULL);	/* if NULL, follows domain */


PGPUInt16 	PGPCalcPassphraseQuality(const char *passphrase);
OSStatus	PGPGetUILibFSSpec(FSSpec *fileSpec);
PGPError	PGPKeyCanEncrypt(PGPKeyRef theKey, PGPBoolean *keyCanEncrypt);
PGPError	PGPKeyCanSign(PGPKeyRef theKey, PGPBoolean *keyCanSign);
PGPError	PGPKeyCanDecrypt(PGPKeyRef theKey, PGPBoolean *keyCanDecrypt);
	
#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpUserInterface_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
