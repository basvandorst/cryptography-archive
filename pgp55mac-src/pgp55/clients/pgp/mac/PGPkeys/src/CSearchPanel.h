/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CSearchPanel.h,v 1.8.4.1 1997/11/06 12:01:46 wprice Exp $
____________________________________________________________________________*/
#pragma once

#include "pgpPubTypes.h"
#include <LGALittleArrows.h>

class CPopupList;
class CGADurationEditField;

class CSearchPanel	:	public LView,
						public LListener,
						public LBroadcaster
{
public:
	enum { class_ID = 'sPan' };
								CSearchPanel(LStream *inStream);
								~CSearchPanel();
	void						FinishCreateSelf();
	void						ListenToMessage(MessageT	inMessage,
												void		*ioParam);
	PGPError					MakeFilter(PGPFilterRef *outFilter);
	
	void						AdjustCategory(Boolean	first);
private:
	void						AdjustOperatorMenu();
	void						AdjustDataFields();
	PGPError					KeyIDFilterFromKeySet(
											PGPKeySetRef	keySet,
											PGPFilterRef	*filter);
	
	CPopupList					*mCategoryPopup,
								*mOperatorPopup,
								*mAlgorithmPopup,
								*mKeyPopup;
	LGAEditField				*mEditBox;
	LGALittleArrows				*mLittleArrows;
	CGADurationEditField		*mDurationEdit;
	Int16						mNumCategories;
	
	Int32						mLastArrowValue;
	
	static Int16				sLastCategory;
	
	enum	{
				kCategoryPopup			= 'pCat',
				kOperatorPopup			= 'pOpe',
				kAlgorithmPopup			= 'pAlg',
				kLittleArrows			= 'lArr',
				kEditBox				= 'eTxt',
				kDurationEdit			= 'eDur',
				kKeyPopup				= 'pKey'
			};
	enum	{
				kCategoryStringListID 	= 1014,
				kUserIDCatStringID		= 1,
				kKeyIDCatStringID,
				kKeyCatStringID,
				kKeyTypeCatStringID,
				kKeySizeCatStringID,
				kCreationCatStringID,
				kExpirationCatStringID,
				kDHKeyTypeStringID,
				kRSAKeyTypeStringID,
				kRevokedStringID,
				kDisabledStringID,
				kBadKeyIDErrorStringID,
				kNoSignerKeyExtantErrorStringID,
				kOperatorStringListID	= 1016
			};
};

