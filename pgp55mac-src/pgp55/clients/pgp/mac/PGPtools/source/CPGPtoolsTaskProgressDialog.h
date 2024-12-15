/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#pragma once

#include "PGPtools.h"
#include "CGAProgressDialog.h"

class CPGPStDialogHandler;

class CPGPtoolsTaskProgressDialog : public CGAProgressDialog
{
public:

	enum { class_ID = 'TskP' };

	virtual				~CPGPtoolsTaskProgressDialog();
						CPGPtoolsTaskProgressDialog(LStream *inStream);

	UInt32				GetTotalCompletedBytes(void)
									{ return( mTotalCompletedBytes ); };
	void				SetTotalCompletedBytes(UInt32 totalBytes)
									{ mTotalCompletedBytes = totalBytes; };

	void				SetTaskCompletedBytes(UInt32 taskBytes)
									{ mTaskCompletedBytes = taskBytes; };

	CPGPStDialogHandler	*GetDialogHandler(void)
									{ return( mDialogHandler ); };
	void				SetDialogHandler(CPGPStDialogHandler *handler)
									{ mDialogHandler = handler; };

	void				SetProgressOperation(PGPtoolsOperation operation);

	void				SetNewProgressItem(ConstStr255Param itemName);
	virtual	void		SetCompletedOperations(UInt32 completedOps);
						
private:

	UInt32				mTaskCompletedBytes;
	// Does not include mTaskCompletedBytes:
	UInt32				mTotalCompletedBytes;
	CPGPStDialogHandler	*mDialogHandler;
	PGPtoolsOperation	mProgressOperation;
	Str255				mItemNameFormatStr;
	UInt32				mShowDelayStartTicks;
};