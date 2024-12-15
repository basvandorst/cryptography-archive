/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CSignDialog.h,v 1.3 1997/07/22 08:54:09 wprice Exp $
____________________________________________________________________________*/
#pragma once

#include "CKeyTable.h"

const MessageT	kSignSuccessful		=	'SSuc';
const ResIDT	kSignDialogResID	=	152;

class CSignDialog		:					public LGADialogBox,
											public LListener,
											public LBroadcaster,
											public LCommander
{
public:
	enum { class_ID = 'SigD' };
					CSignDialog();
					CSignDialog(LStream *inStream);
					~CSignDialog();
	void			ListenToMessage(MessageT inMessage, void *ioParam);
	virtual void	FindCommandStatus(CommandT inCommand,
							Boolean &outEnabled, Boolean &outUsesMark,
							Char16 &outMark, Str255 outName);
	void			FinishCreateSelf();
	void			DrawSelf();
	
	void			SetUserIDs(	PGPKeyRef		signer,
								KeyTableRef		*userIDs,
								Int32			numUserIDs);
private:

	LGACheckbox		*mExportCheckbox;
	LGAPushButton	*mAdvancedButton,
					*mSignButton,
					*mCancelButton;
	LGAPrimaryGroup	*mSignTypesGroup;
	LGARadioButton	*mSignN0Radio,
					*mSignE0Radio,
					*mSignE1Radio,
					*mSignN2Radio;
	Boolean			mAdvancedMode;
	LActiveScroller	*mScroller;
	PGPKeyRef		mSigner;
	KeyTableRef		*mUserIDs;
	Int32			mNumUserIDs;

	enum	{	
				kSignButton				= 'sign',
				kCancelButton			= 'canc',
				kAdvancedButton			= 'adva',
				kExportCheckbox			= 'xExp',
				kSignTypeN0Radio		= 'rNor',
				kSignTypeE0Radio		= 'rEVA',
				kSignTypeE1Radio		= 'rTru',
				kSignTypeN2Radio		= 'rMet',
				kSignTypesGroup			= 'sGrp',
				kUserIDScroller			= 'uSCR',
				kUserIDList				= 'lUID'
			};
};

