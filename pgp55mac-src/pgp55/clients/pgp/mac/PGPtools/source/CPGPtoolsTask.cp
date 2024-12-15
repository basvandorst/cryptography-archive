/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CPGPtoolsTask.cp,v 1.21 1997/10/22 01:07:10 heller Exp $
____________________________________________________________________________*/

#include <UModalDialogs.h>

#include "CPGPStDialogHandler.h"
#include "CWarningAlert.h"
#include "MacEvents.h"
#include "MacStrings.h"
#include "MacFiles.h"
#include "MacErrors.h"

#include "PGPtoolsResources.h"
#include "PGPtoolsUtils.h"

#include "CPGPtoolsTask.h"
#include "CPGPtoolsTaskProgressDialog.h"

CPGPtoolsTask::CPGPtoolsTask(PGPtoolsOperation operation)
{
	mOperation 			= operation;
	mTaskModifiers		= GetModifiers();
}

CPGPtoolsTask::~CPGPtoolsTask(void)
{
}

// Default error reporting behavior
// errorStrIndex contains the formatting string for
// the operation

	void
CPGPtoolsTask::BuildErrorMessage(
	CToolsError 	err,
	short 		errorStrIndex,
	StringPtr	msg)
{
	Str255	errorStr;

	GetIndString( msg, kErrorStringListResID, errorStrIndex );
	GetErrorString( err, errorStr );
	PrintPString( msg, msg, errorStr );
}

	WarningAlertType
CPGPtoolsTask::GetAlertTypeForError(CToolsError err)
{
	WarningAlertType	alertType;
	
	(void)err;
	alertType = kWAStopAlertType;
	
	return( alertType );
}

	CToolsError
CPGPtoolsTask::ReportError(
	CToolsError 	err,
	short 			errorStrIndex)
{
	Str255				msg;
	WarningAlertType	alertType;
	
	BuildErrorMessage( err, errorStrIndex, msg );
	
	alertType = GetAlertTypeForError( err );
	
	SysBeep( 1 );
	CWarningAlert::Display( alertType, kWAOKStyle, msg );
	
	err.err 	= noErr;
	err.pgpErr 	= kPGPError_UserAbort;
	
	return( err );
}

	OSStatus
CPGPtoolsTask::DoProgress(
	CPGPtoolsTaskProgressDialog *	progress,
	UInt32							bytesProcessed,
	UInt32							totalBytesToProcess )
{
	OSStatus					status = noErr;
	static UInt32				sLastEventTicks;

	pgpAssertAddrValid( progress, VoidAlign );
	(void)totalBytesToProcess;
	
	progress->SetTaskCompletedBytes( bytesProcessed );
	progress->SetCompletedOperations( bytesProcessed +
				progress->GetTotalCompletedBytes() );
	
	#define kYieldTicks		6
	if( LMGetTicks() - sLastEventTicks > kYieldTicks )
	{
		CPGPStDialogHandler 		*progressDialogHandler;
		
		progressDialogHandler = progress->GetDialogHandler();

		if( IsntNull( progressDialogHandler ) )
		{
			if( progressDialogHandler->DoDialog() == msg_Cancel )
			{
				status = userCanceledErr;
			}
		}

		sLastEventTicks = LMGetTicks();
	}
	
	return( status );
}

