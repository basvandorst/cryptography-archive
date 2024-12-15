/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#include "MacStrings.h"

#include "PGPtoolsResources.h"

#include "CPGPtoolsTaskProgressDialog.h"

CPGPtoolsTaskProgressDialog::CPGPtoolsTaskProgressDialog(LStream *inStream)
	: CGAProgressDialog(inStream)
{
	mTotalCompletedBytes 	= 0;
	mTaskCompletedBytes		= 0;
	mDialogHandler			= nil;
	mShowDelayStartTicks	= 0;
}

CPGPtoolsTaskProgressDialog::~CPGPtoolsTaskProgressDialog()
{
}

	void
CPGPtoolsTaskProgressDialog::SetProgressOperation(PGPtoolsOperation operation)
{
	ResID	windowTitleStrIndex = 0;
	ResID	formatStrIndex 		= 0;
	Str255	windowTitle;
	
	mProgressOperation = operation;
	
	switch( operation )
	{
		case kPGPtoolsEncryptOperation:
			windowTitleStrIndex = kEncryptStrIndex;
			formatStrIndex		= kEncryptingProgressFormatStrIndex;
			break;

		case kPGPtoolsSignOperation:
			windowTitleStrIndex = kSignStrIndex;
			formatStrIndex		= kSigningProgressFormatStrIndex;
			break;

		case kPGPtoolsEncryptSignOperation:
			windowTitleStrIndex = kEncryptSignStrIndex;
			formatStrIndex		= kEncryptingProgressFormatStrIndex;
			break;

		case kPGPtoolsDecryptVerifyOperation:
			windowTitleStrIndex = kDecryptVerifyStrIndex;
			formatStrIndex		= kDecryptingVerifyingProgressFormatStrIndex;
			break;

		case kPGPtoolsWipeOperation:
			windowTitleStrIndex = kWipeStrIndex;
			formatStrIndex		= kWipingProgressFormatStrIndex;
			break;

		default:
			pgpDebugMsg( "SetProgressOperation: Unknown operation" );
			break;
	}
	
	GetIndString( windowTitle, kPGPtoolsMiscStringsResID,
				windowTitleStrIndex );
	SetDescriptor( windowTitle );
	
	GetIndString( mItemNameFormatStr, kDialogStringsListResID,
				formatStrIndex );
}

	void
CPGPtoolsTaskProgressDialog::SetNewProgressItem(ConstStr255Param itemName)
{
	Str255	msg;

	PrintPString( msg, mItemNameFormatStr, itemName );
	SetCaption( msg );
	UpdatePort();
	
	mTotalCompletedBytes 	+= mTaskCompletedBytes;
	mTaskCompletedBytes		= 0;
}

	void
CPGPtoolsTaskProgressDialog::SetCompletedOperations(UInt32 completedOps)
{
	CGAProgressDialog::SetCompletedOperations( completedOps );
	
	if( ! IsVisible() )
	{
	#if 0
		if( mShowDelayStartTicks == 0 )
		{
			mShowDelayStartTicks = TickCount();
		}
		else
		{
			// Don't show the progress window for the second.
			if( LMGetTicks() - mShowDelayStartTicks > 60L )
			{
				Select();
				Show();
				UpdatePort();
			}
		}
	#else
		Select();
		Show();
		UpdatePort();
	#endif
	}
}
