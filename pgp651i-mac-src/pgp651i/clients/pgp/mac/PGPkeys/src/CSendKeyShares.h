/*____________________________________________________________________________	Copyright (C) 1997 Network Associates Inc. and affiliated companies.	All rights reserved.	$Id: CSendKeyShares.h,v 1.9.6.1 1999/06/09 23:32:24 heller Exp $____________________________________________________________________________*/#pragma once#include <LGADialog.h>#include <LListener.h>#include <LCommander.h>#include <LPeriodical.h>#include <LStaticText.h>#include <LPushButton.h>#include "pgpEncode.h"#include "pgpSKEP.h"#include "MacSecureMemory.h"class LSimpleThread;class CSendKeyShares		:				public LGADialog,											public LCommander,											public LListener,											public LPeriodical{public:	enum { class_ID = 'SKss' };					CSendKeyShares();					CSendKeyShares(LStream *inStream);					~CSendKeyShares();	void			ListenToMessage(MessageT inMessage, void *ioParam);	void			FinishCreateSelf();	void			SpendTime(									const EventRecord		&inMacEvent);	virtual void	FindCommandStatus(CommandT inCommand,							Boolean &outEnabled, Boolean &outUsesMark,							Char16 &outMark, Str255 outName);		static void		SendKeyShares(	FSSpec *			inFSSpec );	void			SendSharesThread();private:	void			SetShareInfo(	PGPShareRef			shares,									char				*name,									PGPUInt32			numShares,									PGPKeyRef			authKey,									PGPKeySetRef		authKeySet,									char const			*passphrase );	static PGPError	DecryptSharesHandler(									PGPContextRef		context,									PGPEvent			*event,									void				*userValue);	static PGPError	SKEPHandler(	PGPskepRef			skepRef,									PGPskepEvent		*event,									PGPUserValue		userValue );	PGPError		SKEPHandler1(	PGPskepEvent		*event	);		PGPShareRef			mShares;	Str255				mName;	PGPUInt32			mNumShares;	PGPKeyRef			mAuthKey;	PGPKeySetRef		mAuthKeySet;	CSecureCString256	mAuthPhrase;	char				mRemoteAddress[256];	LPushButton			*mSendButton;	PGPBoolean			mSending;	PGPBoolean			mAbort;	PGPBoolean			mFinished;	LStaticText			*mStatusText,						*mAuthText;	LSimpleThread		*mSKEPThread;		enum	{					kCancelButton			= 'bCan',				kShareNameText			= 'cNam',				kNumSharesText			= 'cSha',				kSendSharesButton		= 'bSnd',				kRemoteAddressField		= 'eAdd',				kStatusText				= 'cSta',				kAuthenticatedText		= 'cAut'			};};