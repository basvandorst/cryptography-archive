/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	$Id: pgpDialogs.h,v 1.30.10.1 1998/11/12 03:23:00 heller Exp $
____________________________________________________________________________*/

#ifndef Included_pgpDialogs_h	/* [ */
#define Included_pgpDialogs_h

#include "pgpOptionListPriv.h"
#include "pgpUserInterface.h"

struct PGPRecipientSpec;

class CPGPDialogOptions
{
public:

	PGPContextRef			mContext;
	PGPOptionListRef		mClientOptions;
	char					*mWindowTitle;
	const PGPKeyServerSpec	*mServerList;
	PGPUInt32				mServerCount;
	PGPBoolean				mSearchBeforeDisplay;
	PGPBoolean				mTextUI;
	PGPKeySetRef			*mNewKeys;			/* Server search results */
	PGPtlsContextRef		mTLSContext;
	const char *			mPrompt;
	
#if PGP_WIN32
	HWND                mHwndParent;
#endif

						CPGPDialogOptions(void);
	virtual				~CPGPDialogOptions(void);
	
	virtual PGPError	GatherOptions(PGPContextRef context,
								PGPOptionListRef optionList);
};

class CPGPRecipientDialogOptions : public CPGPDialogOptions
{
public:

	PGPOptionListRef		mDialogOptions;
	PGPUInt32				mNumDefaultRecipients;
	const PGPRecipientSpec	*mDefaultRecipients;
	PGPBoolean				mDisplayMarginalValidity;
	PGPBoolean				mIgnoreMarginalValidity;
	PGPGroupSetRef			mGroupSet;
	PGPKeySetRef			mClientKeySet;
	PGPBoolean				mAlwaysDisplay;
	PGPBoolean				mAlwaysDisplayWithARRs;
	PGPKeySetRef			*mRecipientKeysPtr;		/* Output */
	PGPUInt32				*mRecipientCount;		/* Output */
	PGPRecipientSpec		**mRecipientList;		/* Output */
	
	PGPAdditionalRecipientRequestEnforcement	mEnforcement;
	
						CPGPRecipientDialogOptions(void);
	virtual				~CPGPRecipientDialogOptions(void);

	virtual PGPError	GatherOptions(PGPContextRef context,
								PGPOptionListRef optionList);
};

class CPGPRandomDataDialogOptions : public CPGPDialogOptions
{
public:

	PGPUInt32			mNeededEntropyBits;
	
						CPGPRandomDataDialogOptions(void);
	virtual				~CPGPRandomDataDialogOptions(void);
};

class CPGPKeyServerDialogOptions : public CPGPDialogOptions
{
public:

						CPGPKeyServerDialogOptions(void);
	virtual				~CPGPKeyServerDialogOptions(void);
};

class CPGPSearchKeyServerDialogOptions : public CPGPKeyServerDialogOptions
{
public:

	PGPBoolean			mSearchAllServers;
	PGPFilterRef		mFilter;
	char				mKeyDescription[256];
	
						CPGPSearchKeyServerDialogOptions(void);
	virtual				~CPGPSearchKeyServerDialogOptions(void);

	virtual PGPError	GatherOptions(PGPContextRef context,
								PGPOptionListRef optionList);
								
private:

	PGPError			NewKeyIDListSearchFilter(PGPContextRef context,
								const PGPKeyID *keyIDList,
								PGPUInt32 keyIDCount,
								PGPFilterRef *filter);
	PGPError			NewKeySetSearchFilter(PGPContextRef context,
								PGPKeySetRef keySet, PGPFilterRef *filter);
};

class CPGPSendToKeyServerDialogOptions : public CPGPKeyServerDialogOptions
{
public:

	PGPKeySetRef		mKeysToSend;
	PGPKeySetRef		*mFailedKeys;
	
						CPGPSendToKeyServerDialogOptions(void);
	virtual				~CPGPSendToKeyServerDialogOptions(void);
};


class CPGPPassphraseDialogOptions : public CPGPDialogOptions
{
public:

	char **				mPassphrasePtr;
	PGPOptionListRef	mDialogOptions;
	PGPUInt32			mMinPassphraseLength;
	PGPUInt32			mMinPassphraseQuality;

						CPGPPassphraseDialogOptions(void);
	virtual				~CPGPPassphraseDialogOptions(void);
	
	virtual PGPError	GatherOptions(PGPContextRef context,
								PGPOptionListRef optionList);
};

class CPGPConfirmationPassphraseDialogOptions :
			public CPGPPassphraseDialogOptions
{
public:

	PGPBoolean			mShowPassphraseQuality;

						CPGPConfirmationPassphraseDialogOptions(void);
	virtual				~CPGPConfirmationPassphraseDialogOptions(void);
	
	virtual PGPError	GatherOptions(PGPContextRef context,
								PGPOptionListRef optionList);
};

class CPGPKeyPassphraseDialogOptions : public CPGPPassphraseDialogOptions
{
public:

	PGPBoolean			mVerifyPassphrase;
	PGPKeyRef			mDefaultKey;

						CPGPKeyPassphraseDialogOptions(void);
	virtual				~CPGPKeyPassphraseDialogOptions(void);
	
	virtual PGPError	GatherOptions(PGPContextRef context,
								PGPOptionListRef optionList);
};

class CPGPKeySetPassphraseDialogOptions :
			public CPGPKeyPassphraseDialogOptions
{
public:

	PGPBoolean			mFindMatchingKey;
	PGPKeySetRef		mKeySet;
	PGPKeyRef *			mPassphraseKeyPtr;

						CPGPKeySetPassphraseDialogOptions(void);
	virtual				~CPGPKeySetPassphraseDialogOptions(void);
	
	virtual PGPError	GatherOptions(PGPContextRef context,
								PGPOptionListRef optionList);
};

class CPGPSigningPassphraseDialogOptions :
			public CPGPKeySetPassphraseDialogOptions
{
public:

						CPGPSigningPassphraseDialogOptions(void);
	virtual				~CPGPSigningPassphraseDialogOptions(void);
};

class CPGPDecryptionPassphraseDialogOptions :
			public CPGPKeySetPassphraseDialogOptions
{
public:

	const PGPKeyID		*mKeyIDList;
	PGPUInt32			mKeyIDCount;
	PGPKeyID			*mMissingKeyIDList;
	PGPUInt32			mMissingKeyIDCount;
	
						CPGPDecryptionPassphraseDialogOptions(void);
	virtual				~CPGPDecryptionPassphraseDialogOptions(void);
	
	PGPError			GatherMissingKeys(void);
	PGPError			SearchForMissingKeys(void *hwndParent,
								PGPBoolean *foundNewKeys);
	
private:

	PGPError			RemoveFoundKeysFromSet(PGPKeySetRef keySet);
};

class CPGPOptionsDialogOptions : public CPGPDialogOptions
{
public:

						CPGPOptionsDialogOptions(void);
	virtual				~CPGPOptionsDialogOptions(void);
};


PGP_BEGIN_C_DECLARATIONS

/* Platform specific handler functions */

PGPError	pgpRecipientDialogPlatform(PGPContextRef context,
					CPGPRecipientDialogOptions *options);
PGPError	pgpPassphraseDialogPlatform(PGPContextRef context,
					CPGPPassphraseDialogOptions *options);
PGPError	pgpKeyPassphraseDialogPlatform(PGPContextRef context,
					CPGPKeyPassphraseDialogOptions *options);
PGPError	pgpSigningPassphraseDialogPlatform(PGPContextRef context,
					CPGPSigningPassphraseDialogOptions *options);
PGPError	pgpDecryptionPassphraseDialogPlatform(PGPContextRef context,
					CPGPDecryptionPassphraseDialogOptions *options);
PGPError	pgpConfirmationPassphraseDialogPlatform(PGPContextRef context,
					CPGPConfirmationPassphraseDialogOptions *options);
PGPError	pgpOptionsDialogPlatform(PGPContextRef context,
					CPGPOptionsDialogOptions *options);
PGPError	pgpCollectRandomDataDialogPlatform(PGPContextRef context,
					CPGPRandomDataDialogOptions *options);
PGPError	pgpSearchKeyServerDialogPlatform(PGPContextRef context,
					CPGPSearchKeyServerDialogOptions *options);
PGPError	pgpSendToKeyServerDialogPlatform(PGPContextRef context,
					CPGPSendToKeyServerDialogOptions *options);
PGPError	pgpGetMissingRecipientKeyIDStringPlatform(PGPContextRef context,
					const PGPKeyID	*keyID, char keyIDString[256]);
					
/* Utilitiy functions */

PGPError	pgpCheckNetworklibAvailability(void);
PGPKeyRef   GetKeyForPassphrase(PGPKeySetRef keySet, const char *passphrase,
					PGPBoolean signing);


PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpDialogs_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
