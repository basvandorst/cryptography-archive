/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CPGPtoolsTask.h,v 1.17 1997/08/12 04:36:01 lloyd Exp $
____________________________________________________________________________*/
#pragma once

#include "WarningAlertTypes.h"

#include "PGPtools.h"
#include "CResultsTable.h"

class CPGPtoolsTaskProgressDialog;


class CPGPtoolsTask
{		
private:
	ushort					mTaskModifiers;
	
								
protected:
	PGPtoolsOperation		mOperation;
	
					CPGPtoolsTask(PGPtoolsOperation operation);

	virtual	void	BuildErrorMessage(CToolsError status, short errorStrIndex,
								StringPtr msg);
	virtual	WarningAlertType	GetAlertTypeForError(CToolsError err);
	virtual	CToolsError		ReportError(CToolsError err, short errorStrIndex);
	
	virtual	void		GetTaskItemName(StringPtr name) = 0;
	
	virtual OSStatus	DoProgress(  CPGPtoolsTaskProgressDialog *progress,
							UInt32  bytesProcessed,
							UInt32 totalToProcess);
public:

	virtual			~CPGPtoolsTask(void);
	
	virtual	OSStatus	CalcProgressBytes(PGPContextRef context,
							ByteCount *bytes) = 0;
	
	ushort				GetTaskModifiers(void)				
							{ return( mTaskModifiers ); };
	void				SetTaskModifiers(ushort modifiers)
							{ mTaskModifiers = modifiers; };
};



