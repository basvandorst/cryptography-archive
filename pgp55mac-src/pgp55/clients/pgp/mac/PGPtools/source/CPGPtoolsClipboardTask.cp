/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#include <UDesktop.h>

#include "MacMemory.h"
#include "MacStrings.h"

#include "CPGPtoolsClipboardTask.h"
#include "CPGPtoolsTaskProgressDialog.h"
#include "CResultsWindow.h"
#include "PGPtoolsEncryptDecrypt.h"
#include "PGPtoolsResources.h"
#include "PGPtoolsUtils.h"
#include "pgpUserInterface.h"
#include "pgpUtilities.h"
#include "PGPSharedEncryptDecrypt.h"

CPGPtoolsClipboardTask::CPGPtoolsClipboardTask(PGPtoolsOperation operation) :
		CPGPtoolsBufferTask( operation )
{
}

CPGPtoolsClipboardTask::~CPGPtoolsClipboardTask(void)
{
}

	OSErr
CPGPtoolsClipboardTask::GetClipboardData(PGPContextRef context)
{
	PGPError	err = kPGPError_NoErr;
	long		dataSize;
	long		scrapOffset;
	
	if( IsntNull( mText ) )
	{
		PGPFreeData( mText );
		
		mText = NULL;
		mTextLength	= 0;
	}
	
	// WARNING: It appears that this may return a bogus result with CW11 when
	// running with the symbolic debugger. It appears that I always get -108
	// (memFullErr?) as dataSize.
	
	dataSize = GetScrap( NULL, 'TEXT', &scrapOffset );
	if( dataSize > 0 )
	{
		Handle data;

		data = NewHandle( dataSize );
		if( IsntNull( data ) )
		{
			dataSize = GetScrap( data, 'TEXT', (long *) &scrapOffset );
			if( dataSize > 0 )
			{
				mText = PGPNewData( context, dataSize );
				if( IsntNull( mText ) )
				{
					pgpCopyMemory( *data, mText, dataSize );
					mTextLength = dataSize;
				}
				else
				{
					err = kPGPError_OutOfMemory;
				}
			}
			
			DisposeHandle( data );
		}
		else
		{
			err = kPGPError_OutOfMemory;
		}	
	}
	else
	{
		err	= dataSize;
	}
	
	return( err );
}

	CToolsError
CPGPtoolsClipboardTask::EncryptSign(
	PGPContextRef	context,
	CEncodeParams		*state)
{
	CToolsError	err;
	
	err.err = GetClipboardData( context );
	if( err.IsntError() )
	{
		err = inherited::EncryptSign( context, state );
	}
	
	if( ShouldReportError( err ) )
	{
		err = ReportError( err,
			state->encrypting ? kClipboardCannotBeEncryptedStrIndex :
			kClipboardCannotBeSignedStrIndex );
	}

	return( err );
}

	CToolsError
CPGPtoolsClipboardTask::DecryptVerify(
	PGPContextRef	context,
	CDecodeParams		*state)
{
	CToolsError	err;
	
	err.err = GetClipboardData( context );
	if( err.IsntError() )
	{
		err = inherited::DecryptVerify( context, state );
	}
	
	if( ShouldReportError( err ) )
	{
		err = ReportError( err, kClipboardCannotBeDecryptedVerifiedStrIndex );
	}
		
	return( err );
}

	CToolsError
CPGPtoolsClipboardTask::ProcessOutputData(void)
{
	Boolean	processData = TRUE;
	
	if( mOperation == kPGPtoolsDecryptVerifyOperation &&
		mDataType == kPGPAnalyze_Unknown )
	{
		processData = FALSE;
	}
	
	if( processData )
	{
		ZeroScrap();
		
		if( mTextLength > 0 )
			PutScrap( mTextLength, 'TEXT', mText );
	}
		
	return( CToolsError() );
}

	OSStatus
CPGPtoolsClipboardTask::CalcProgressBytes(
	PGPContextRef		context,
	ByteCount *			progressBytes)
{
	long		scrapSize;	// Intentionally signed
	long		scrapOffset;
	OSStatus	err	= noErr;
	
	(void) context;
	
	// WARNING: It appears that this may return a bogus result with CW11 when
	// running with the symbolic debugger. It appears that I always get -108
	// (memFullErr?) as progressBytes.
	
	scrapSize = GetScrap( nil, 'TEXT', &scrapOffset );
	if( scrapSize < 0 )
		scrapSize = 0;

	*progressBytes	= scrapSize;
	
	return( err );
}

	void
CPGPtoolsClipboardTask::GetTaskItemName(StringPtr name)
{
	GetIndString( name, kPGPtoolsMiscStringsResID,
					kClipboardStrIndex );
}

	void
CPGPtoolsClipboardTask::GetDecryptPassphrasePromptString(StringPtr prompt)
{
	GetIndString( prompt, kDialogStringsListResID,
				kGetPassphraseForClipboardPromptStrIndex );
}

