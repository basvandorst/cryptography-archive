/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CKeyInfoWindow.h,v 1.6 1997/09/13 07:06:29 wprice Exp $
____________________________________________________________________________*/
#pragma once

#include "pgpKeys.h"

class CFingerprintField;
class CKeyTable;

const short	kMaxKeyInfoWindows	=	16;	// Prevent memory problems

class CKeyInfoWindow		:		public LWindow,
									public LListener
{
public:
	enum { class_ID = 'KInf' };
								CKeyInfoWindow();
								CKeyInfoWindow(LStream *inStream);
								~CKeyInfoWindow();
	void						ListenToMessage(
									MessageT inMessage, void *ioParam);
	void						FinishCreateSelf();
	void						DrawSelf();
	
	virtual Boolean				ObeyCommand(
									CommandT inCommand, void* ioParam);	
	virtual void				FindCommandStatus(CommandT inCommand,
									Boolean &outEnabled, Boolean &outUsesMark,
									Char16 &outMark, Str255 outName);
									
	static Boolean				OpenKeyInfo(PGPKey *key, CKeyTable *table);
	static void					CloseAll();
	
	static	void				GetFingerprintString(
									Str255 pstr, PGPKeyRef key);
	
	PGPKeyRef					GetKey();
private:
	void						SetInfo(PGPKey *key, CKeyTable *table);
	void						SetValidityCaption(
									LCaption	*inCaption,
									Int16		inValidityLevel);
	void						SetTrustCaption(
									LCaption	*inCaption,
									Int16		inTrustLevel);
		
	PGPKeyRef					mKey;
	CKeyTable					*mParentTable;
	Boolean						mChanged,
								mSecret,
								mAxiomatic,
								mTrustChanged;
	
	LCaption					*mKeyIDCaption,
								*mTrustCaption,
								*mValidityCaption,
								*mCreationCaption,
								*mExpiresCaption,
								*mKeyTypeCaption,
								*mSizeCaption,
								*mCipherCaption;
	CFingerprintField			*mFingerprintCaption;
	LGACheckbox					*mEnabledCheckbox,
								*mAxiomaticCheckbox;
	LControl					*mTrustSlider;
	LControl					*mValidityBox;
	LGAPushButton				*mPassphraseButton;
	
	static CKeyInfoWindow		*sKeyInfoWindows[kMaxKeyInfoWindows];
	static short				sNumKeyInfoWindows;
		
	enum	{
				kKeyIDCaption			= 'cKID',
				kCreationCaption		= 'cCre',
				kValidityCaption		= 'cVal',
				kValidityBox			= 'bVal',
				kTrustCaption			= 'cTru',
				kTrustSlider			= 'sTru',
				kFingerprintCaption		= 'eFin',
				kEnabledCheckbox		= 'xEna',
				kPassphraseButton		= 'bPas',
				kKeyTypeCaption			= 'cTyp',
				kExpiresCaption			= 'cExp',
				kSizeCaption			= 'cSiz',
				kCipherCaption			= 'cCyp',
				kAxiomaticCheckbox		= 'xTru',
				kRecoveryBox			= 'cMsg',
				kAgentCaption			= 'cAnt',
				kAgentRCaption			= 'cAge'
			};
	enum	{
				kTrustStringListID		= 1005,
				kValidityStringListID	= 1009,
				kKeyInfoStringListID	= 1010
			};
	enum	{
				kNeverExpiresID			= 1,
				kRSATypeID,
				kDSATypeID,
				kTooManyWindowsID,
				kInvalidKeySetTrustID,
				kUnknownMRKID
			};
};

