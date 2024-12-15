/*____________________________________________________________________________	Copyright (C) 1997-1998 Network Associates, Inc. and its affiliates.	All rights reserved.	$Id: CPGPtools.h,v 1.25.8.1 1998/11/12 03:09:02 heller Exp $____________________________________________________________________________*/#pragma once#include <LApplication.h>#include <LListener.h>#include <LPeriodical.h>#include <LQueue.h>#include "pgpPubTypes.h"#include "PGPtools.h"class CPGPtoolsMainWindow;class CPGPtoolsTaskList;class CWipeFreeSpaceWizardDialog;class LWindow;class CResultsWindow;class CPGPtools : 	public LApplication,					public LPeriodical,					public LListener{public:						CPGPtools( void );	virtual 			~CPGPtools();		void				ShowAboutBox();		void				EnqueueTaskList(CPGPtoolsTaskList *taskList);	virtual void		FindCommandStatus(CommandT inCommand,									Boolean &outEnabled, Boolean &outUsesMark,									Char16 &outMark, Str255 outName);	virtual void		ListenToMessage(MessageT inMessage, void *ioParam);	virtual void		HandleAppleEvent( const AppleEvent &inAppleEvent,									AppleEvent &outAEReply, AEDesc &outResult,									Int32 inAENumber );	virtual Boolean		ObeyCommand(CommandT inCommand, void* ioParam);		virtual void		Run(void);	virtual	void		SpendTime(const EventRecord &inMacEvent);	static Boolean		AppCanDoOperation(PGPtoolsOperation operation);	static void			WipeFreeSpace(short vRefNum = -1)							{ mWipeFreeSpaceVRefNum = vRefNum; };	protected:	CResultsWindow 				*mResultsWindow;	CPGPtoolsMainWindow			*mMainWindow;	CWipeFreeSpaceWizardDialog	*mWipeFreeDialog;		virtual	void		Initialize(void);	Boolean				SecondaryVerifyEnvironment(void);	#if USE_MAC_DEBUG_LEAKS	// we need to suspend leaks checking for the duration of these things	// because PowerPlant will never release certain items	virtual void		MakeMenuBar( void )							{ MacLeaks_Suspend(); LApplication::MakeMenuBar();							  MacLeaks_Resume(); }	virtual void		MakeModelDirector( void )							{ MacLeaks_Suspend(); LApplication::									MakeModelDirector(); MacLeaks_Resume(); }#endif // USE_MAC_DEBUG_LEAKSprivate:	Boolean				mAutoQuit;	Boolean				mUserLaunchedAppOnly;	LQueue				mTaskListQueue;	Point				mLastMouseLocation;	short				mApplicationResFile;	PGPContextRef		mContext;	PGPtlsContextRef	mTLSContext;		static Boolean		mAppCanEncrypt;	static Boolean		mAppCanSign;	static Boolean		mAppCanDecryptVerify;	static short		mWipeFreeSpaceVRefNum;		void				ProcessNextTask( void );		virtual Boolean		AttemptQuitSelf(Int32 inSaveOption);	void				GetFinderSelection(TArray<FSSpec> * outSpecsArray);	void				ProcessFinderSelection(long inEventCode,							TArray<FSSpec> &inSpecsArray);		void				ReportTaskError( CToolsError err );	CToolsError			CreateAndEnqueueFileTaskList(							PGPtoolsOperation operation,							ushort taskModifiers,							const FSSpec *inSpecs, PGPUInt16 numSpecs );	CToolsError			CreateAndEnqueueClipboardTask(							PGPtoolsOperation operation, ushort taskModifiers );	void				CreateAndEnqueueTaskPrompt(PGPtoolsOperation operation);	OSErr				DoOpenDocAppleEvent(const AppleEvent &inAppleEvent );	void				LaunchPGPkeys(void);	OSErr				ProcessDragAndDrop(const FSSpec *specList,									UInt32 numSpecs);	PGPError			UpdateContext( Boolean verifyKeyRings );	void				WipeFreeSpaceWizard(short defaultVRefNum);	#if PGP_DEBUG	virtual	void		InstallDebugMenu(void);	virtual	void		DebugTest(void);#endif	// PGP_DEBUG};extern CPGPtools	*gApplication;