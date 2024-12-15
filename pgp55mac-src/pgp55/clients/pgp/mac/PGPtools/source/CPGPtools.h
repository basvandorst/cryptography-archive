/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#pragma once

#include <LApplication.h>
#include <LPeriodical.h>
#include <LQueue.h>

#include "pgpPubTypes.h"

#include "PGPtools.h"

class CPGPtoolsMainWindow;
class CPGPtoolsTaskList;
class LWindow;
class CResultsWindow;

class CPGPtools : public LApplication, public LPeriodical
{
public:
						CPGPtools( void );
	virtual 			~CPGPtools();
	
	void				ShowAboutBox();
	
	virtual void		FindCommandStatus(CommandT inCommand,
									Boolean &outEnabled, Boolean &outUsesMark,
									Char16 &outMark, Str255 outName);
	virtual void		HandleAppleEvent( const AppleEvent &inAppleEvent,
									AppleEvent &outAEReply, AEDesc &outResult,
									Int32 inAENumber );
	virtual Boolean		ObeyCommand(CommandT inCommand, void* ioParam);	
	virtual void		Run(void);
	virtual	void		SpendTime(const EventRecord &inMacEvent);
	void				EnqueueTaskList(CPGPtoolsTaskList *taskList);

	static Boolean		AppCanDoOperation(PGPtoolsOperation operation);
	
protected:

	CResultsWindow 		*mResultsWindow;
	CPGPtoolsMainWindow	*mMainWindow;
	
	virtual	void		Initialize(void);
	Boolean				SecondaryVerifyEnvironment(void);
	virtual void		StartUp(void);

#if USE_MAC_DEBUG_LEAKS
	// we need to suspend leaks checking for the duration of these things
	// because PowerPlant will never release certain items
	virtual void		MakeMenuBar( void )
							{ MacLeaks_Suspend(); LApplication::MakeMenuBar();
							  MacLeaks_Resume(); }
	virtual void		MakeModelDirector( void )
							{ MacLeaks_Suspend(); LApplication::
									MakeModelDirector(); MacLeaks_Resume(); }
#endif // USE_MAC_DEBUG_LEAKS

private:

	Boolean				mAutoQuit;
	Boolean				mUserLaunchedAppOnly;
	LQueue				mTaskListQueue;
	Point				mLastMouseLocation;
	short				mApplicationResFile;
	PGPContextRef		mContext;
	
	static Boolean		mAppCanEncrypt;
	static Boolean		mAppCanSign;
	static Boolean		mAppCanDecryptVerify;
	
	PGPError			CreateContext( Boolean verifyContext );
	PGPError			DisposeContext( void );
	void				ProcessNextTask( void );
	
	virtual Boolean		AttemptQuitSelf(Int32 inSaveOption);
	void				GetFinderSelection(TArray<FSSpec> * outSpecsArray);
	void				ProcessFinderSelection(long inEventCode,
							TArray<FSSpec> &inSpecsArray);
	
	void				ReportTaskError( CToolsError err );
	CToolsError			CreateAndEnqueueFileTaskList(
							PGPtoolsOperation operation,
							ushort taskModifiers,
							const FSSpec *inSpecs, PGPUInt16 numSpecs );
	CToolsError			CreateAndEnqueueClipboardTask(
							PGPtoolsOperation operation, ushort taskModifiers );
	void				CreateAndEnqueueTaskPrompt(PGPtoolsOperation operation);
	OSErr				DoOpenDocAppleEvent(const AppleEvent &inAppleEvent );
	void				LaunchPGPkeys(void);
	OSErr				ProcessDragAndDrop(const FSSpec *specList,
									UInt32 numSpecs);
	
#if PGP_DEBUG
	virtual	void		InstallDebugMenu(void);
	virtual	void		DebugTest(void);
#endif	// PGP_DEBUG
};

extern CPGPtools	*gApplication;