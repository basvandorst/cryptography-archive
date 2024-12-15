/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#include <UDesktop.h>

#include "CWarningAlert.h"
#include "MacMemory.h"
#include "MacStrings.h"

#include "CPGPtools.h"
#include "CPGPtoolsTextTask.h"
#include "CPGPtoolsTaskProgressDialog.h"
#include "CPGPtoolsTextWindow.h"
#include "CResultsWindow.h"
#include "PGPtoolsEncryptDecrypt.h"
#include "PGPtoolsResources.h"
#include "PGPtoolsUtils.h"
#include "pgpUserInterface.h"
#include "pgpUtilities.h"
#include "PGPSharedEncryptDecrypt.h"

const ResIDT	kSaveOrClipboardWAResID		= 10000;

CPGPtoolsTextTask::CPGPtoolsTextTask(PGPtoolsOperation operation) :
		CPGPtoolsBufferTask( operation )
{
	mSourceFileName[0] = 0;
}

CPGPtoolsTextTask::~CPGPtoolsTextTask(void)
{
}

	CToolsError
CPGPtoolsTextTask::EncryptSign(
	PGPContextRef	context,
	CEncodeParams		*state)
{
	CToolsError	err;
	
	err = inherited::EncryptSign( context, state );
	
	if( ShouldReportError( err ) )
	{
		err = ReportError( err,
			state->encrypting ?
				kTextCannotBeEncryptedStrIndex : kTextCannotBeSignedStrIndex );
	}

	return( err );
}

	CToolsError
CPGPtoolsTextTask::DecryptVerify(
	PGPContextRef	context,
	CDecodeParams		*state)
{
	CToolsError	err;
	
	err = inherited::DecryptVerify( context, state );

	if( ShouldReportError( err ) )
	{
		err = ReportError( err, kTextCannotBeDecryptedVerifiedStrIndex );
	}
		
	return( err );
}

	void
CPGPtoolsTextTask::GetDecryptPassphrasePromptString(StringPtr prompt)
{
	if( mSourceFileName[0] != 0 )
	{
		GetIndString( prompt, kDialogStringsListResID,
					kGetPassphraseForNamedTextPromptStrIndex );
		PrintPString( prompt, prompt, mSourceFileName );
	}
	else
	{
		GetIndString( prompt, kDialogStringsListResID,
				kGetPassphraseForSelectedTextPromptStrIndex );
	}
}

	CToolsError
CPGPtoolsTextTask::ProcessOutputData(void)
{
	CToolsError	err;
	Boolean		processData = TRUE;
	
	if( mOperation == kPGPtoolsDecryptVerifyOperation &&
		mDataType == kPGPAnalyze_Unknown )
	{
		processData = FALSE;
	}
	
	if( processData )
	{
		if( mTextLength > max_Int16 )
		{
			MessageT	result;
			
			SysBeep( 1 );
			
			result = CWarningAlert::Display( kWACautionAlertType,
						kWAOKCancelStyle,
						kDialogStringsListResID,
						kTextLargerThan32KOptionsStrIndex );
			if( result == msg_OK )
			{
				ZeroScrap();

				PutScrap( mTextLength, 'TEXT', mText );
			}
		}
		else if( mTextLength > 0 )
		{
			CPGPtoolsTextWindow	*textWindow;	
			
			textWindow = (CPGPtoolsTextWindow *) LWindow::
						CreateWindow( window_Text, gApplication );
			if( IsntNull( textWindow ) )
			{
				if( mSourceFileName[0] != 0 )
				{
					textWindow->SetDescriptor( mSourceFileName );
				}
				
				err.err = textWindow->SetText( mText, mTextLength );
				if( err.IsntError() )
				{
					textWindow->Show();
				}
				else
				{
					delete( textWindow );
				}
			}
			else
			{
				err.pgpErr	= kPGPError_OutOfMemory;
			}
		}
	}
	
	return( err );
}

	void
CPGPtoolsTextTask::BuildErrorMessage(
	CToolsError 	err,
	short 			errorStrIndex,
	StringPtr		msg)
{
	Str255	errorStr;
	Boolean	addFileName = FALSE;
	
	if( mSourceFileName[0] != 0 )
	{
		addFileName = TRUE;
		
		switch( errorStrIndex )
		{
			case kTextCannotBeEncryptedStrIndex: 
				errorStrIndex = kNamedTextCannotBeEncryptedStrIndex;
				break;

			case kTextCannotBeSignedStrIndex: 
				errorStrIndex = kNamedTextCannotBeSignedStrIndex;
				break;

			case kTextCannotBeDecryptedVerifiedStrIndex: 
				errorStrIndex = kNamedTextCannotBeDecryptedVerifiedStrIndex;
				break;

			default:
				pgpDebugMsg(
				"CPGPtoolsTextTask::BuildErrorMessage(): Unknown string");
				addFileName = FALSE;
				break;
		}
	}
	
	GetIndString( msg, kErrorStringListResID, errorStrIndex );
	GetErrorString( err, errorStr );
	
	if( addFileName )
	{
		PrintPString( msg, msg, mSourceFileName );
	}
		
	PrintPString( msg, msg, errorStr );
}

	void
CPGPtoolsTextTask::SetSourceFileName(ConstStringPtr name)
{
	CopyPString( name, mSourceFileName );
}

	void
CPGPtoolsTextTask::GetTaskItemName(StringPtr name)
{
	if( mSourceFileName[0] != 0 )
	{
		CopyPString( mSourceFileName, name );
	}
	else
	{
		GetIndString( name, kResultsTableStringListResID,
						kSelectedTextStrIndex );
	}
}

