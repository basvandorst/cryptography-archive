/*____________________________________________________________________________	Copyright (C) 1997 Network Associates Inc. and affiliated companies.	All rights reserved.	$Id: CPassphraseEdit.h,v 1.11 1999/03/10 02:35:18 heller Exp $____________________________________________________________________________*/#pragma once#include <LEditText.h>#define	kMaxSpacesPerCharacter		2#define kMaxPassphraseLength		255class CPassphraseEdit : public LEditText{public:	enum { class_ID = 'PPef' };						CPassphraseEdit(LStream *inStream);	virtual				~CPassphraseEdit();	virtual	void		ClearPassphrase(void);	virtual	PGPUInt32	EstimatePassphraseQuality(void);	virtual void		FindCommandStatus(CommandT inCommand,							Boolean& outEnabled,							Boolean& outUsesMark, Char16& outMark,							Str255 outName);	virtual StringPtr	GetDescriptor(Str255 outDescriptor) const;	virtual void		GetPassphrase(char passphrase[256]);	virtual	PGPUInt32	GetPassphraseLength(void);	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam);	virtual void		SelectAll(void);	virtual void		SetDescriptor(ConstStr255Param inDescriptor);	void				SetHideTyping(Boolean hide);	static EKeyStatus	sPassphraseKeyFilter(TEHandle	inMacTEH,									   Char16	inKeyCode,									   Char16	&ioCharCode,									   UInt16	inModifiers);	protected:	virtual void		AdjustCursorSelf(Point inPortPt,							const EventRecord &inMacEvent);	virtual void		ClickSelf(const SMouseDownEvent &inMouseDown);	virtual Boolean		HandleKeyPress(const EventRecord& inKeyEvent);	virtual Boolean		TooManyCharacters(Int32 inCharsToAdd);private:	Boolean mPassphraseHidden;	char	mPassphrase[kMaxPassphraseLength];	char	mSpacePassphrase[kMaxPassphraseLength * kMaxSpacesPerCharacter];	UInt32	mPassphraseLength;	UInt32	mSpacePassphraseLength;		static UInt32	mNumActiveFields;	static UInt8	mSpaceCountList[kMaxPassphraseLength];};