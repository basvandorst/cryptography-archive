/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	$Id: PGPRecipientGrafPort.h,v 1.28.4.2 1997/11/20 20:41:39 heller Exp $
____________________________________________________________________________*/

#pragma once

#include <UTables.h>

#include "CPGPLibGrafPortView.h"
#include "pflPrefs.h"
#include "pgpUserInterface.h"
#include "pgpKeys.h"
#include "CComboError.h"
#include "pgpGroups.h"

const ResIDT kPGPRecipientDialogID = 4747;

class CUserIDTable;
class CPrivateKeysPopup;
class CPassphraseEdit;
class LGACheckbox;
class LGAPopup;
class LGAIconSuiteControl;
class LGAPushButton;

// The user value for a key reflects where it came from and
// whether or not it is in a display list.

enum
{
	kKeyStateNew = 0,
	kKeyStateAddedFromClient,
	kKeyStateAddedFromKeySearch
};

class CPGPRecipientGrafPort	:	public CPGPLibGrafPortView
{
public:

	enum { class_ID = 'CPgp' };
						CPGPRecipientGrafPort(LStream *inStream);
	virtual				~CPGPRecipientGrafPort();

	void				DrawSelf(void);
	virtual void		FinishCreateSelf(void);
	PGPError			GetParams(	PGPRecipientSettings *userSettings,
									PGPKeySetRef *actualRecipients,
									PGPBoolean *haveNewKeys);
	Boolean				HandleKeyPress( const EventRecord&	inKeyEvent);
	virtual void		ListenToMessage(MessageT inMessage, void *ioParam);
	CComboError			SetParams( PGPContextRef context,
									PGPPrefRef clientPrefsRef,
									PGPPrefRef adminPrefsRef,
									PGPKeySetRef allKeys,
									PGPUInt32 numDefaultRecipients,
									const PGPKeySpec *defaultRecipients,
									PGPRecipientOptions dialogOptions,	
									PGPRecipientSettings defaultSettings,
									PGPGroupSetRef	groupSet,
									PGPBoolean *allRecipientsValid);
	
private:

	LGAPopup				*mMacBinaryPopup;
	LGACheckbox				*mTextOutputCheckbox;
	LGACheckbox				*mWipeOriginalCheckbox;
	LGACheckbox				*mConvEncryptCheckbox;
	LGAPushButton			*mUpdateFromServerButton;
	
	CUserIDTable			*mRecipientTable;
	CUserIDTable			*mUserIDTable;
	PGPUInt32				mNumDefaultRecipients;
	const PGPKeySpec		*mDefaultRecipients;
	PGPKeySetRef			mAllKeys;
	PGPBoolean				mHaveFileOptions;
	PGPBoolean				mUserIDListsEnabled;
	PGPBoolean				mHaveNewKeys;
	PGPBoolean				mHaveKeyServerSupport;
	PGPPrefRef				mClientPrefsRef;
	PGPPrefRef				mAdminPrefsRef;
	PGPContextRef			mContext;
	PGPBoolean				mEnforceRemoteADKClass;
	PGPGroupSetRef			mGroupSet;
	
	PGPError				BuildRecipientList(void);

	PGPError				GetAllKeysInRecipientList( PGPKeySetRef *outSet );
	
	PGPError				AddSearchedKeysToUserIDTable(void);
	void					AdjustButtons(void);
	void					DisableUserIDLists(void);
	void					EnableUserIDLists(void);
	Boolean					FindAndMarkUser(const char *userID,
									Boolean *foundMultiple);
	PGPError				FindMissingRecipients(void);
	void					LockRecipient(UInt32 row);
	PGPError				MoveDefaultUserIDs(PGPBoolean *allRecipientsValid);

	Boolean					MarkADKeysForUserToRecipientMove(PGPKeyRef key );
	void					MarkADKeysInGroupForRecipientToUserMove(
								PGPGroupSetRef set, PGPGroupID id );
	Boolean					MarkADKeysInGroupForUserToRecipientMove(
								PGPGroupSetRef set, PGPGroupID id );
	UInt32					SendSelectedToRecipients(void);
	UInt32					SendMarkedToRecipients(void);

	UInt32					SendSelectedToUserIDs(void);
	void					MarkADKeysForRecipientToUserMove(PGPKeyRef key );
	UInt32					SendMarkedToUserIDs(void);

	void					UpdateMissingRecipients(Boolean *foundAmbiguous);

#if PGP_BUSINESS_SECURITY
	PGPBoolean				mEnforceOutgoingADK;
	PGPKeyRef				mCorporateKeyRef;
	
	PGPKeyRef				mOutgoingADKRef;
	
	PGPUInt32				GetNumKeysNotSignedByCorporateKey(CUserIDTable *whichList,
								PGPKeyRef corporateKeyRef, Boolean markedOnly);
	PGPError				MoveOutgoingAdditionalKey(void);
#endif

};

