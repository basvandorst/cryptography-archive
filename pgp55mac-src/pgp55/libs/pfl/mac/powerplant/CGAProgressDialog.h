/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CGAProgressDialog.h,v 1.2 1997/06/30 10:37:26 wprice Exp $
____________________________________________________________________________*/
#pragma once

#include <LGADialogBox.h>

const short		kProgressDialogResID = 31600;

class CProgressBar;

class CGAProgressDialog : public LGADialogBox
{
public:

	enum { class_ID = 'PrgD' };
	
					CGAProgressDialog();
	virtual			~CGAProgressDialog();
					CGAProgressDialog(LStream *inStream);
	
	virtual	void	AddCompletedOperations(UInt32 completedOps);
	virtual	void	SetButtonTitle(ConstStr255Param title);
	virtual	void	SetCaption(ConstStr255Param caption);
	virtual	void	SetPercentComplete(UInt32 percentComplete);
	virtual	void	SetTotalOperations(UInt32 totalOps);
	virtual	void	SetCompletedOperations(UInt32 completedOps);
	
protected:

	virtual void	FinishCreateSelf(void);

private:

	CProgressBar	*mProgressBarObj;
	UInt32			mTotalOps;
	UInt32			mCompletedOps;
};
