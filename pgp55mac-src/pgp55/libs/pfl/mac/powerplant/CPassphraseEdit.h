/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CPassphraseEdit.h,v 1.5 1997/09/18 01:34:32 lloyd Exp $
____________________________________________________________________________*/
#pragma once

#include <LGAEditField.h>

#define	kMaxSpacesPerCharacter		2
#define kMaxPassphraseLength		255

class CPassphraseEdit	:	public LGAEditField
{
public:
	enum { class_ID = 'PPef' };
						CPassphraseEdit(LStream *inStream);
	virtual				~CPassphraseEdit();

	virtual	void		ClearPassphrase(void);
	virtual void		FindCommandStatus(CommandT inCommand,
							Boolean& outEnabled,
							Boolean& outUsesMark, Char16& outMark,
							Str255 outName);
	virtual StringPtr	GetDescriptor(Str255 outDescriptor) const;
	virtual	UInt32		GetPassphraseLength(void);
	virtual void		SelectAll(void);
	virtual void		SetDescriptor(ConstStr255Param inDescriptor);
	void				SetHideTyping(Boolean hide);
	virtual void		UserChangedText(void);

	static EKeyStatus		PassphraseKeyFilter(const EventRecord& inKeyEvent,
								Int16 inKeyPosition = 0);
	
protected:
	virtual void		AdjustCursorSelf(Point inPortPt,
							const EventRecord &inMacEvent);
	virtual void		ClickSelf(const SMouseDownEvent &inMouseDown);
	virtual Boolean		HandleKeyPress(const EventRecord& inKeyEvent);
	virtual Boolean		TooManyCharacters(Int32 inCharsToAdd);

private:

	Boolean mPassphraseHidden;
	uchar	mPassphrase[kMaxPassphraseLength];
	uchar	mSpacePassphrase[kMaxPassphraseLength * kMaxSpacesPerCharacter];
	UInt32	mPassphraseLength;
	UInt32	mSpacePassphraseLength;
	
	static UInt32	mNumActiveFields;
	static UInt8	mSpaceCountList[kMaxPassphraseLength];
};
