/*____________________________________________________________________________	Copyright (C) 1997 Network Associates Inc. and affiliated companies.	All rights reserved.		$Id: CSearchPanel.cp,v 1.24 1999/03/10 02:39:37 heller Exp $____________________________________________________________________________*/#include "CSearchPanel.h"#include "CGADurationEditField.h"#include "CPGPKeys.h"#include "CWarningAlert.h"#include "pgpPFLErrors.h"#include "MacStrings.h"#include "pgpKeys.h"#include "pgpUtilities.h"#include <LEditText.h>#include <LPopupButton.h>const Int16	kNumCategories		= 7;const Int16 kNumOperators		= 8;enum	{			kUserIDCategory		= 0,			kKeyIDCategory,			kKeyCategory,			kKeyTypeCategory,			kKeySizeCategory,			kCreationDateCategory,			kExpirationDateCategory		};enum	{			kContainsOperator = 0,			kNotContainsOperator,			kIsOperator,			kIsNotOperator,			kIsAtLeastOperator,			kIsAtMostOperator,			kIsSignedByOperator,			kNotSignedByOperator		};const Int16 kCategoryOperators[kNumCategories][kNumOperators] =		{			//kUserIDCategory			kContainsOperator, kNotContainsOperator,			kIsOperator, kIsNotOperator,			kIsSignedByOperator, kNotSignedByOperator, -1, -1,			//kKeyCategory			kIsOperator, kIsNotOperator, -1, -1, -1, -1, -1, -1,			//kKeyIDCategory			kIsOperator, kIsNotOperator, -1, -1, -1, -1, -1, -1,			//kKeyTypeCategory			kIsOperator, -1, -1, -1, -1, -1, -1, -1,			//kKeySizeCategory			kIsOperator, kIsAtLeastOperator, kIsAtMostOperator,			-1, -1, -1, -1, -1,			//kCreationDateCategory			kIsOperator, kIsAtLeastOperator, kIsAtMostOperator,			-1, -1, -1, -1, -1,			//kExpirationDateCategory			kIsOperator, kIsAtLeastOperator, kIsAtMostOperator,			-1, -1, -1, -1, -1,		};				Int16 CSearchPanel::sLastCategory;CSearchPanel::CSearchPanel(LStream *inStream)	:	LView(inStream){}CSearchPanel::~CSearchPanel(){}	voidCSearchPanel::FinishCreateSelf(){	UInt32	timeNow;		LView::FinishCreateSelf();	mNumCategories = kNumCategories;	mCategoryPopup = (LPopupButton *)FindPaneByID(kCategoryPopup);	mOperatorPopup = (LPopupButton *)FindPaneByID(kOperatorPopup);	mEditBox = (LEditText *)FindPaneByID(kEditBox);	mLittleArrows = (LLittleArrows *)FindPaneByID(kLittleArrows);	mDurationEdit = (CGADurationEditField *)FindPaneByID(kDurationEdit);		mAlgorithmPopup = (LPopupButton *)FindPaneByID(kAlgorithmPopup);	mAlgorithmPopup->AddListener(this);	mKeyPopup = (LPopupButton *)FindPaneByID(kKeyPopup);	mKeyPopup->AddListener(this);	mDurationEdit->SetDurationType(kDateDurationType);	GetDateTime(&timeNow);	mDurationEdit->SetDurationValue(timeNow);	mCategoryPopup->AddListener(this);	mLastArrowValue = mLittleArrows->GetValue();	mLittleArrows->AddListener(this);	AdjustOperatorMenu();	AdjustDataFields();	mEditBox->SwitchTarget(mEditBox);}	voidCSearchPanel::AdjustCategory(Boolean first){	if(first)		sLastCategory = 0;	else	{		sLastCategory = (sLastCategory + 1) % mNumCategories;		mCategoryPopup->SetValue(sLastCategory + 1);		AdjustOperatorMenu();		AdjustDataFields();	}}	voidCSearchPanel::AdjustDataFields(){	switch(mCategoryPopup->GetValue() - 1)	{		case kUserIDCategory:		case kKeySizeCategory:		case kKeyIDCategory:			mLittleArrows->Hide();			mDurationEdit->Hide();			mAlgorithmPopup->Hide();			mKeyPopup->Hide();			mEditBox->Show();			break;		case kKeyCategory:			mLittleArrows->Hide();			mDurationEdit->Hide();			mAlgorithmPopup->Hide();			mEditBox->Hide();			mKeyPopup->Show();			break;		case kKeyTypeCategory:			mLittleArrows->Hide();			mDurationEdit->Hide();			mEditBox->Hide();			mKeyPopup->Hide();			mAlgorithmPopup->Show();			break;		case kCreationDateCategory:		case kExpirationDateCategory:			mEditBox->Hide();			mAlgorithmPopup->Hide();			mKeyPopup->Hide();			mLittleArrows->Show();			mDurationEdit->Show();			break;	}}	PGPErrorCSearchPanel::MakeFilter(PGPFilterRef *outFilter){	PGPError			err;	PGPFilterRef		filter;	char				cstring[256];	Str255				pstring;	Int16				op,						cat;	Int32				value32;	UInt32				valueU32;	PGPMatchCriterion	match;	Boolean				boolValue,						negate = FALSE;	PGPPublicKeyAlgorithm	algorithm;		*outFilter = filter = kInvalidPGPFilterRef;	err = kPGPError_UnknownError;	cat = mCategoryPopup->GetValue() - 1;	op = mOperatorPopup->GetValue() - 1;	mEditBox->GetDescriptor(pstring);	PToCString(pstring, cstring);	valueU32 = mDurationEdit->GetDurationValue();	switch(kCategoryOperators[cat][op])	{		case kIsOperator:			match = kPGPMatchEqual;			break;		case kIsAtLeastOperator:			match = kPGPMatchGreaterOrEqual;			break;		case kIsAtMostOperator:			match = kPGPMatchLessOrEqual;			break;	}	switch(cat)	{		case kUserIDCategory:			if(cstring[0] == '\0')				break;			switch(kCategoryOperators[cat][op])			{				case kNotContainsOperator:					negate = TRUE;				case kContainsOperator:					err = PGPNewUserIDStringFilter(							gPGPContext, cstring,							kPGPMatchSubString, &filter);					pgpAssertNoErr(err);					if(negate && IsntPGPError(err))					{						err = PGPNegateFilter(filter, outFilter);						pgpAssertNoErr(err);					}					else						*outFilter = filter;					break;				case kIsNotOperator:					negate = TRUE;				case kIsOperator:					err = PGPNewUserIDStringFilter(							gPGPContext, cstring,							kPGPMatchEqual, &filter);					pgpAssertNoErr(err);					if(negate && IsntPGPError(err))					{						err = PGPNegateFilter(filter, outFilter);						pgpAssertNoErr(err);					}					else						*outFilter = filter;					break;				case kNotSignedByOperator:					negate = TRUE;				case kIsSignedByOperator:					err = PGPNewUserIDStringFilter(gPGPContext,								cstring, kPGPMatchSubString, &filter);					pgpAssertNoErr(err);					if(IsntPGPError(err) && PGPFilterRefIsValid(filter))					{						PGPKeySetRef	filterSet;												err = PGPFilterKeySet(									CPGPKeys::TheApp()->GetKeySet(),									filter, &filterSet);						pgpAssertNoErr(err);						PGPFreeFilter(filter);						filter = kInvalidPGPFilterRef;						if(IsntPGPError(err))						{							err = KeyIDFilterFromKeySet(filterSet, &filter);							PGPFreeKeySet(filterSet);							if(!PGPFilterRefIsValid(filter))							{								CWarningAlert::Display(kWANoteAlertType,										kWAOKStyle, kSearchPanelStringListID,										kNoSignerKeyExtantErrorStringID);							}						}					}					if(negate && IsntPGPError(err) && PGPFilterRefIsValid(filter))					{						err = PGPNegateFilter(filter, outFilter);						pgpAssertNoErr(err);					}					else						*outFilter = filter;					break;			}			break;		case kKeySizeCategory:			if(pstring[0] == 0)				break;			::StringToNum(pstring, &value32);			err = PGPNewKeyEncryptKeySizeFilter(					gPGPContext, value32, match, outFilter);			pgpAssertNoErr(err);			break;		case kKeyIDCategory:			{				PGPKeyID keyID;								err = PGPGetKeyIDFromString( cstring, &keyID);				if(IsPGPError(err))				{					CWarningAlert::Display(kWANoteAlertType, kWAOKStyle,								kSearchPanelStringListID,								kBadKeyIDErrorStringID);				}				else				{					err = PGPNewKeyIDFilter(gPGPContext, &keyID, &filter);					pgpAssertNoErr(err);					if((kCategoryOperators[cat][op] == kIsNotOperator) &&						IsntPGPError(err) && PGPFilterRefIsValid(filter))					{						err = PGPNegateFilter(filter, outFilter);						pgpAssertNoErr(err);					}					else						*outFilter = filter;				}			}			break;		case kKeyCategory:			switch(kCategoryOperators[cat][op])			{				case kIsOperator:					boolValue = TRUE;					break;				case kIsNotOperator:					boolValue = FALSE;					break;			}			switch(mKeyPopup->GetValue())			{				case 1:					err = PGPNewKeyRevokedFilter(							gPGPContext, boolValue, outFilter);					pgpAssertNoErr(err);					break;				case 2:					err = PGPNewKeyDisabledFilter(							gPGPContext, boolValue, outFilter);					pgpAssertNoErr(err);					break;			}			break;		case kKeyTypeCategory:			switch(mAlgorithmPopup->GetValue())			{				case 1:					algorithm = kPGPPublicKeyAlgorithm_ElGamal;					break;				case 2:					algorithm = kPGPPublicKeyAlgorithm_RSA;					break;			}			err = PGPNewKeyEncryptAlgorithmFilter(					gPGPContext, algorithm, outFilter);			pgpAssertNoErr(err);			break;		case kCreationDateCategory:			if(match == kPGPMatchEqual)			{				PGPFilterRef	mergeFilter;				DateTimeRec		endOfDay;				UInt32			endOfDaySeconds;								err = PGPNewKeyCreationTimeFilter(						gPGPContext, PGPTimeFromMacTime(valueU32),						kPGPMatchGreaterOrEqual, &mergeFilter);				pgpAssertNoErr(err);								SecondsToDate(valueU32, &endOfDay);				endOfDay.hour		= 23;				endOfDay.minute		= 59;				endOfDay.second		= 59;				DateToSeconds(&endOfDay, &endOfDaySeconds);								err = PGPNewKeyCreationTimeFilter(						gPGPContext, PGPTimeFromMacTime(endOfDaySeconds),						kPGPMatchLessOrEqual, &filter);				pgpAssertNoErr(err);								err = PGPIntersectFilters(filter, mergeFilter, outFilter);				pgpAssertNoErr(err);			}			else			{				err = PGPNewKeyCreationTimeFilter(						gPGPContext, PGPTimeFromMacTime(valueU32),						match, outFilter);				pgpAssertNoErr(err);			}			break;		case kExpirationDateCategory:			if(match == kPGPMatchEqual)			{				PGPFilterRef	mergeFilter;				DateTimeRec		endOfDay;				UInt32			endOfDaySeconds;								err = PGPNewKeyExpirationTimeFilter(						gPGPContext, PGPTimeFromMacTime(valueU32),						kPGPMatchGreaterOrEqual, &mergeFilter);				pgpAssertNoErr(err);								SecondsToDate(valueU32, &endOfDay);				endOfDay.hour		= 23;				endOfDay.minute		= 59;				endOfDay.second		= 59;				DateToSeconds(&endOfDay, &endOfDaySeconds);								err = PGPNewKeyExpirationTimeFilter(						gPGPContext, PGPTimeFromMacTime(endOfDaySeconds),						kPGPMatchLessOrEqual, &filter);				pgpAssertNoErr(err);								err = PGPIntersectFilters(filter, mergeFilter, outFilter);				pgpAssertNoErr(err);			}			else			{				err = PGPNewKeyExpirationTimeFilter(						gPGPContext, PGPTimeFromMacTime(valueU32),						match, outFilter);				pgpAssertNoErr(err);			}			break;		default:			pgpAssert(0);	}	return err;}	PGPErrorCSearchPanel::KeyIDFilterFromKeySet(	PGPKeySetRef	keySet,	PGPFilterRef	*filter){	PGPError		error	= kPGPError_NoErr;	PGPKeyListRef	list	= kInvalidPGPKeyListRef;	PGPKeyIterRef	iter	= kInvalidPGPKeyIterRef;	PGPKeyRef		key		= kInvalidPGPKeyRef;	PGPKeyID		keyID;	PGPFilterRef	aFilter = kInvalidPGPFilterRef;	PGPFilterRef	idFilter= kInvalidPGPFilterRef;	error = PGPOrderKeySet(keySet, kPGPAnyOrdering, &list);	if( IsntPGPError(error))	{		error = PGPNewKeyIter(list, &iter);		if(IsntPGPError(error))		{			PGPError iterError = kPGPError_NoErr;			do			{				iterError = PGPKeyIterNext(iter, &key);				if(IsntPGPError(iterError))				{					error = PGPGetKeyIDFromKey(key, &keyID);					if(IsntPGPError(error))					{						error = PGPNewSigKeyIDFilter(gPGPContext, 													&keyID, &aFilter);						if(idFilter == kInvalidPGPFilterRef)							idFilter = aFilter;						else						{							PGPFilterRef combinedFilter = NULL;							error = PGPUnionFilters(idFilter, aFilter, 													&combinedFilter);							idFilter = combinedFilter;						}					}				}			} while(IsntPGPError(iterError) && IsntPGPError(error));			PGPFreeKeyIter(iter);		}		PGPFreeKeyList(list);	}	if(IsntPGPError(error))		*filter = idFilter;	return error;}	voidCSearchPanel::AdjustOperatorMenu(){	Int16		itemIndex;	Str255		itemString;	Int16		cat;	Int32		numItems;		cat = mCategoryPopup->GetValue() - 1;	numItems = mOperatorPopup->GetMaxValue();	while( numItems > 0 )		mOperatorPopup->DeleteMenuItem( numItems-- );	for(itemIndex = 0 ; itemIndex < kNumOperators ; itemIndex++)	{		if(kCategoryOperators[cat][itemIndex] != -1)		{			GetIndString(	itemString, kOperatorStringListID,							kCategoryOperators[cat][itemIndex] + 1);			mOperatorPopup->AppendMenu(itemString, true);		}	}	mOperatorPopup->SetValue(1);	mOperatorPopup->Refresh();}	voidCSearchPanel::ListenToMessage(MessageT inMessage, void *ioParam){	Int32 newValue;		switch(inMessage)	{		case kCategoryPopup:			AdjustOperatorMenu();			AdjustDataFields();			break;		case kLittleArrows:			newValue = *(Int32 *)ioParam;			if(newValue < mLastArrowValue)				mDurationEdit->AdjustValue(kDownAdjust);			else				mDurationEdit->AdjustValue(kUpAdjust);			mLastArrowValue = newValue;			break;	}}