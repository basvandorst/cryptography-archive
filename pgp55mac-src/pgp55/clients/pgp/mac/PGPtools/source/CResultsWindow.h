/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#pragma once

#include "PGPtools.h"
#include "PGPSharedEncryptDecrypt.h"

#include "CPGPtoolsWindow.h"

class CResultsTable;

class CResultsWindow : public CPGPtoolsWindow
{
public:
	enum 				{ class_ID = 'RWin' };

						CResultsWindow(LStream *inStream);
	virtual				~CResultsWindow(void);

	Boolean				HaveSignatures( void )
							{ return( mHaveSignatures ); }
	
	virtual void		AttemptClose(void);
	virtual void		DoClose(void);
	virtual void		DrawSelf(void);	
	virtual void		FindCommandStatus(CommandT inCommand,
									Boolean &outEnabled,
									Boolean &outUsesMark, Char16 &outMark,
									Str255 outName);
	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam);
	virtual void		ResizeFrameBy(Int16 inWidthDelta, Int16 inHeightDelta,
									Boolean inRefresh);

	static void			IncrementVerifiedCount(void);
	static void			NewSummary(void);
	static void			NewResult(ConstStr255Param itemName,
									PGPtoolsSignatureData *signatureData);
	static void			CalcColumnBounds(const Rect *cellRect,
							Rect *iconRect, Rect *nameRect, Rect *statusRect,
							Rect *validityRect, Rect *signingDateRect);
									
protected:

	virtual	void		FinishCreateSelf(void);
	
private:

	CResultsTable		*mResultsTable;
	FontInfo			mTitlesFontInfo;
	Boolean				mHaveSignatures;
	Boolean				mPendingNewSummary;
	
	void				AddResult(ConstStr255Param itemName,
									PGPtoolsSignatureData *signatureData);
	void				IncrementCount(void);
	void				DrawColumnTitles(void);
	void				GetColumnTitlesRect(Rect *titlesRect);

};