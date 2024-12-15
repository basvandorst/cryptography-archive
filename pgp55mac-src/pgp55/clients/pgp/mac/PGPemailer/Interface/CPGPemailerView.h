/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CPGPemailerView.h,v 1.6.10.1 1997/12/05 22:13:51 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include <LGrafPortView.h>
#include <LListener.h>

#include "PGPemailerGlobals.h"
#include "MacCursors.h"
#include "CPGPEncoderDecoder.h"

class CPGPemailerView : public LGrafPortView, public LListener {
public:
	enum { class_ID = 'PeVw' };
	static CPGPemailerView *	CreateCPGPemailerViewStream(
										LStream *inStream);

								CPGPemailerView(LStream* inStream);
	virtual						~CPGPemailerView();
	
	void						SetGlobals(SGlobalsPtr inGlobalsP)
									{ mGlobalsP = inGlobalsP; }
	
	virtual void				ClickInContent(const EventRecord &inMacEvent);
	virtual void				ClickSelf(const SMouseDownEvent &inMouseDown);
	virtual void				FindCommandStatus(CommandT inCommand,
									Boolean &outEnabled,
									Boolean &outUsesMark, Char16 &outMark,
									Str255 outName);
	virtual Boolean				ObeyCommand(CommandT inCommand,
									void* ioParam = nil);
	virtual Boolean				HandleKeyPress(const EventRecord &inKeyEvent);
	virtual void				ListenToMessage(MessageT inMessage,
									void* ioParam);	
	virtual void				AdjustCursorSelf(Point inPortPt,
									const EventRecord &inMacEvent);

protected:
	SGlobalsPtr					mGlobalsP;
	AnimatedCursorRef			mCursorRef;
	Boolean						mEncrypted;
	Boolean						mEncoding;

	virtual void				FinishCreateSelf();
	virtual void				DrawSelf();
	
	void						EncodeMessage(EEncodeOptions inOptions);
	void						DecodeMessage(Boolean inAppendStatus = true);
	
	void						GetFrontWindowContent(Handle outContentH);
	void						SetFrontWindowContent(Handle inContentH);
	void						GetFrontWindowSelection(Handle outContentH);
	void						SetFrontWindowSelection(Handle inContentH);
	void						GetFrontWindowRecipients(
									SInt32 * outNumRecipients,
									Handle outRecipientsH);
	SInt32						CountFrontWindowAttachments();
	void						GetFrontWindowAttachments(
									LArray & outAttachments);
	
	void						ShowError(PGPError inErr);
	
	static PGPError 			ShowProgress(void * inCursorRef, size_t soFar,
									size_t total)
									{ AnimateCursor(
											(AnimatedCursorRef) inCursorRef);
									return noErr; }
									
	static OSStatus				SharedShowProgress(UInt32 bytesProcessed,
									UInt32 totalBytesToProcess,
									void * inCursorRef)
										{ AnimateCursor(
											(AnimatedCursorRef) inCursorRef);
										return noErr; }
										
	static PGPError				PGPEventHandler(PGPContextRef context,
										PGPEvent *event,
										PGPUserValue userValue);
};
