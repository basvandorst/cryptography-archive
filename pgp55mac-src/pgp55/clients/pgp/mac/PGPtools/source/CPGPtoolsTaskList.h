/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	$ld$
_____________________________________________________________________________*/

#pragma once

#include <LArray.h>
#include <LLink.h>

#include "PGPtools.h"
#include "pgpUserInterface.h"
#include "pgpEncode.h"

class CPGPtoolsTask;
class CPGPtoolsTaskProgressDialog;
class CEncodeParams;

#pragma warn_hidevirtual off
template class TArray<CPGPtoolsTask *>;
#pragma warn_hidevirtual reset

class CPGPtoolsTaskList : public LLink
{
public:
				CPGPtoolsTaskList(
					PGPtoolsOperation operation );
	virtual		~CPGPtoolsTaskList(void);
	
	void		AppendTask(CPGPtoolsTask *theTask);
	void		ProcessTasks(PGPContextRef context);
	
	void		SetTaskListHasFileTasks(Boolean hasFiles)
							{ mTaskListHasFileTasks = hasFiles; };
	
private:

	struct
	{
		PGPBoolean			warnOnWipe;
		PGPCipherAlgorithm	convAlgorithm;		
		char				comment[ 256 ];
		PGPCipherAlgorithm	*allowedAlgorithms;
		PGPUInt32			numAllowedAlgorithms;
		
	} mPrefs;
	
	void		LoadCurrentPrefs( void );
	
	PGPtoolsOperation		mOperation;
	TArray<CPGPtoolsTask *>	*mTaskList;
	Boolean					mTaskListHasFileTasks;
	
	CToolsError	ConfirmationDialogs( CEncodeParams const *params );
	PGPError	ConfirmWipe( Boolean encrypting, Boolean signing );
	PGPError	ConfirmMacBinary( void );
	PGPError	ConfirmNotEncryptingToSelf( void );
	
	Boolean		RecipientsIncludeSelf( PGPKeySetRef recipients );
	
	PGPError	BeginTasks(PGPContextRef context);
	virtual	OSStatus	CalcProgressBytes(PGPContextRef context,
							ByteCount *bytes);
	void		DeleteTasks(void);
	
	void		ProcessDecryptVerifyTasks(PGPContextRef context,
						PGPKeySetRef allKeys,
						CPGPtoolsTaskProgressDialog *progressDialogObj);
						
	void		ProcessEncryptSignTasks(PGPContextRef context,
						PGPKeySetRef *allKeys,	/* I/O Parameter */
						CPGPtoolsTaskProgressDialog *progressDialogObj);
						
	void		ProcessWipeTasks( 
					CPGPtoolsTaskProgressDialog *progressDialogObj);
					
	CToolsError	SetupCommonOptions(PGPContextRef context,
						PGPOptionListRef encodeOptions);
						
	CToolsError	SetupEncryptionOptions(PGPContextRef context,
						PGPKeySetRef allKeys,
						PGPRecipientSettings recipientSettings,
						PGPRecipientOptions recipientOptions,
						PGPOptionListRef encodeOptions);
						
	CToolsError	SetupSigningOptions(PGPContextRef context,
						PGPGetPassphraseSettings settings,
						PGPGetPassphraseOptions options,
						PGPOptionListRef encodeOptions);
	
};



