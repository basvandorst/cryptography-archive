/*____________________________________________________________________________	Copyright (C) 1997 Network Associates Inc. and affiliated companies.	All rights reserved.			$Id: CPGPKeys.h,v 1.50 1999/05/16 21:42:31 wprice Exp $____________________________________________________________________________*/#pragma once#include <LBroadcaster.h>#include <LDocApplication.h>#include <LPeriodical.h>#include <LGADialog.h>#include <LModelDirector.h>#include <LMenuBar.h>//#include <Navigation.h>#include "pgpKeys.h"#include "pgpTLS.h"#include "pgpMemoryMgr.h"#include "pflPrefTypes.h"#include "pgpKeyServer.h"class CKeyWindow;class CKeyView;class CSearchWindow;class CGroupsWindow;class LGADialogBox;extern ResIDT				gApplicationResFile;extern FSSpec				gApplicationFSSpec;extern PGPContextRef		gPGPContext;extern PGPMemoryMgrRef		gPGPMemoryMgr;extern PGPtlsContextRef		gTLSContext;extern PGPPrefRef			gPrefRef;extern PGPPrefRef			gAdminPrefRef;extern Boolean				gServerCallsPresent;extern PGPBoolean			gIsAdminConfigured;extern FSSpec				gAppFSSpec;const ResIDT				kSubstituteHelpMenuID	=	8322;enum	{			kAlgorithmStringsID			= 1012,			kIDEAString					= 1,			kTripleDESString			= 2,			kCASTString					= 3		};enum	{			kMiscStringsID				= 1006,			kPGPKeyringsFolderStringID	= 1		};		#define CKERR			if( IsPGPError( err ) ) goto done	#define kPGPMacFileCreator_SplitKey				'pgSK'#define kPGPMacFileCreator_X509Cert				'pgX5'class CPGPKeys	:	public LDocApplication,					public LPeriodical,					public LListener,					public LBroadcaster{public:						CPGPKeys( void );			virtual 			~CPGPKeys( void );					// this overriding function performs application functions			virtual Boolean		ObeyCommand(CommandT inCommand, void* ioParam);		virtual void		MakeMenuBar();	virtual void		MakeModelDirector();	virtual void		ShowAboutBox();	virtual void		SpendTime(const EventRecord &/*inMacEvent*/);			// this overriding function returns the status of menu items			virtual void		FindCommandStatus(CommandT inCommand,							Boolean &outEnabled, Boolean &outUsesMark,							Char16 &outMark, Str255 outName);	virtual void		HandleAppleEvent( const AppleEvent &inAppleEvent,							AppleEvent &outAEReply, AEDesc &outResult,							Int32 inAENumber );	virtual void		OpenDocument(FSSpec *inMacFSSpec);	virtual void		ListenToMessage(	MessageT	inMessage,											void		*ioParam);	PGPKeyListRef		GetKeyList();	void				SetKeyList(PGPKeyListRef keyList);	PGPKeySetRef		GetKeySet();	CKeyView			*GetDefaultKeyView()	{	return mDefaultKeysView;}	Boolean				IsKeyringWritable()	{	return mWritableKeyring;	}	void				InvalidateCaches();		PGPError			GetMinimumRandomData();			inline 	static	CPGPKeys	*TheApp( void );		void				CreatedKey( void );	void				ResetKeyDB();	void				CommitDefaultKeyrings();		protected:	void					ShowSplashScreen();	void					EndSplashScreen();	short					AskMakeNewKeyFiles();	void					ServerGroupUpdate(	CommandT	inCommand	);	void					PostPNEvent(										PGPUInt32		receiver,										UInt32			eventID,										void *			data = NULL,										PGPUInt32		dataLen = 0 );	static	CPGPKeys		*sTheApp;		PGPKeySetRef			mKeySet;	PGPKeyListRef			mKeyList;	CKeyWindow				*mKeyWindow;	CKeyView				*mDefaultKeysView;	CSearchWindow			*mSearchWindow;	CGroupsWindow			*mGroupsWindow;	LGADialog				*mSplashScreen;	ulong					mSplashDisplayTime;	LModelDirector			*mModelDirector;	LMenuBar				*mMenuBar;	Int16					mHelpMenuRegisterItemNumber;	Boolean					mCreatedKey,							mCheckedLicense,							mWritableKeyring,							mSearching,							mAppStarted;		Point					mLastMouseLoc;	PGPKeyServerThreadStorageRef	mPreviousStorage;		virtual void		StartUp();		// overriding startup functions			void		CleanUpPPLeaks( void );			Boolean		TryingToQuit( void );			void		ResetDefaultKeyringsPath(void);};const ResIDT	kMaxMenuID				 	= 500;const MessageT	cmd_CollapseAll				= 1101;	const MessageT	cmd_ExpandAll				= 1102;	const MessageT	cmd_Sign1					= 1201;	const MessageT	cmd_AddName					= 1202;	const MessageT	cmd_SetDefault				= 1203;	const MessageT	cmd_NewKey					= 1204;	const MessageT	cmd_Info					= 1205;	const MessageT	cmd_Revoke					= 1206;	const MessageT	cmd_ImportKeys				= 1207;const MessageT	cmd_ExportKeys				= 1208;const MessageT	cmd_ChangePassphrase		= 1209;const MessageT	cmd_OptionRevoke			= 1210;const MessageT	cmd_KSGetKeys				= 1212;const MessageT	cmd_KSSendKeys				= 1213;const MessageT	cmd_Search					= 1214;const MessageT	cmd_ViewValidity			= 1215;const MessageT	cmd_ViewTrust				= 1216;const MessageT	cmd_ViewSize				= 1217;const MessageT	cmd_ViewKeyID				= 1218;const MessageT	cmd_ViewCreationDate		= 1219;const MessageT	cmd_ViewExpirationDate		= 1220;const MessageT	cmd_ViewMRK					= 1221;const MessageT	cmd_ViewDescription			= 1222;const MessageT	cmd_ShowGroups				= 1223;const MessageT	cmd_NewGroup				= 1224;const MessageT	cmd_KSSendKeyToDomain		= 1225;const MessageT	cmd_SplitKey				= 1226;const MessageT	cmd_SendShareFile			= 1227;const MessageT	cmd_ReverifySignatures		= 1228;const MessageT	cmd_EnableKeys				= 1229;const MessageT	cmd_DisableKeys				= 1230;const MessageT	cmd_UpdateIntroducers		= 1231;const MessageT	cmd_AddToKey				= 1232;	const MessageT	cmd_AddPhotoUserID			= 1233;const MessageT	cmd_ServerDisable			= 1234;const MessageT	cmd_ServerDelete			= 1235;const MessageT	cmd_UpdateGroupLists		= 1236;const MessageT	cmd_SendGroupLists			= 1237;const MessageT	cmd_AddRevoker				= 1238;const MessageT	cmd_Toolbar					= 1239;const MessageT	cmd_ImportGroups			= 1240;const MessageT	cmd_GroupProperties			= 1241;const MessageT	cmd_AddCertificate			= 1242;const MessageT	cmd_RetrieveCertificate		= 1243;const MessageT	cmd_UpdateRevocations		= 1244;const MessageT	cmd_OptionClear				= 50;	CPGPKeys *CPGPKeys::TheApp(){	pgpAssertAddrValid(sTheApp, CPGPKeys);	return sTheApp;}void ReportPGPError(PGPError err);/*pascal void PKNavEventProc(	NavEventCallbackMessage	callbackSelector,	NavCBRecPtr				callbackParms,	NavCallBackUserData		callbackUD );*/Boolean CustomGetFileWithShowAll(	short				numTypes,	SFTypeList			typeList,	FSSpec				*fsSpec );#define min(x,y) (((x)<(y)) ? (x) : (y))#define max(x,y) (((x)>(y)) ? (x) : (y))