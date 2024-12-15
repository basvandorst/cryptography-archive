/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CGAProgressDialog.cp,v 1.5 1997/09/18 01:34:31 lloyd Exp $
____________________________________________________________________________*/
#include <LCaption.h>
#include <LControl.h>
#include <UModalDialogs.h>

#include "MacMemory.h"
#include "MacStrings.h"

#include "CGAProgressDialog.h"
#include "CProgressBar.h"

const PaneIDT	kCancelButtonPaneID		= 'bCan';
const PaneIDT	kProgressCaptionPaneID	= 'Capt';
const PaneIDT	kProgressBarPaneID		= 'PBar';



CGAProgressDialog::CGAProgressDialog()
{
	mTotalOps 		= 0;
	mCompletedOps	= 0;
}

CGAProgressDialog::CGAProgressDialog(LStream *inStream)
	: LGADialogBox(inStream)
{
	mTotalOps 		= 0;
	mCompletedOps	= 0;
}

CGAProgressDialog::~CGAProgressDialog()
{
}


	void
CGAProgressDialog::FinishCreateSelf(void)
{
	LGADialogBox::FinishCreateSelf();
	
	mProgressBarObj = (CProgressBar *) FindPaneByID( kProgressBarPaneID );
	pgpAssertAddrValid( mProgressBarObj, CProgressBar ); 
}


//	Sets the text of the description above the progress bar.

	void
CGAProgressDialog::SetCaption(ConstStr255Param caption)
{
	LCaption	*captionObj;
	
	pgpAssertAddrValid( caption, uchar );
	
	captionObj = (LCaption *) FindPaneByID( kProgressCaptionPaneID );
	pgpAssertAddrValid( captionObj, LCaption );
	
	captionObj->SetDescriptor( caption );
}


//	Sets the title of the "Cancel" button.

	void
CGAProgressDialog::SetButtonTitle(ConstStr255Param title)
{
	LControl	*buttonObj;
	
	pgpAssertAddrValid( title, uchar );
	
	buttonObj = (LControl *) FindPaneByID( kCancelButtonPaneID );
	pgpAssertAddrValid( buttonObj, LControl );
	
	buttonObj->SetDescriptor( title );
}


	void
CGAProgressDialog::SetPercentComplete(UInt32 percentComplete)
{
	pgpAssertAddrValid( mProgressBarObj, CProgressBar );
	
	mProgressBarObj->SetPercentComplete( percentComplete );
}


	void
CGAProgressDialog::SetTotalOperations(UInt32 totalOps)
{
	mTotalOps = totalOps;
}


	void
CGAProgressDialog::SetCompletedOperations(UInt32 completedOps)
{
	UInt32	percentComplete;
	
	pgpAssert( mTotalOps != 0 );
	
	mCompletedOps = completedOps;
	if( mCompletedOps > mTotalOps )
		mCompletedOps = mTotalOps;
		
	percentComplete = (UInt32)
		(( (float) mCompletedOps * 100.0 ) / (float) mTotalOps);
	
	SetPercentComplete( percentComplete );
}

	void
CGAProgressDialog::AddCompletedOperations(UInt32 completedOps)
{
	SetCompletedOperations( mCompletedOps + completedOps );
}
