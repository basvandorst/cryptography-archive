/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CPGPKeys.h,v 1.20.4.1 1997/11/06 12:01:45 wprice Exp $
____________________________________________________________________________*/
#pragma once

#include "pgpKeys.h"
#include "pflPrefTypes.h"

class CKeyWindow;
class CKeyView;
class CSearchWindow;
class CGroupsWindow;

extern ResIDT				gApplicationResFile;
extern FSSpec				gApplicationFSSpec;
extern PGPContextRef		gPGPContext;
extern PFLContextRef		gPFLContext;
extern PGPPrefRef			gPrefRef;
extern PGPPrefRef			gAdminPrefRef;
extern Boolean				gServerCallsPresent;
extern FSSpec				gAppFSSpec;

const ResIDT				kSubstituteHelpMenuID	=	8322;

enum	{
			kAlgorithmStringsID			= 1012,
			kIDEAString					= 1,
			kTripleDESString			= 2,
			kCASTString					= 3
		};

class CPGPKeys	:	public LDocApplication,
					public LPeriodical,
					public LListener
{
public:
						CPGPKeys( void );		
	virtual 			~CPGPKeys( void );		
	
		// this overriding function performs application functions
		
	virtual Boolean		ObeyCommand(CommandT inCommand, void* ioParam);	
	virtual void		MakeMenuBar();
	virtual void		MakeModelDirector();
	virtual void		ShowAboutBox();
	virtual void		SpendTime(const EventRecord &/*inMacEvent*/);
	
		// this overriding function returns the status of menu items
		
	virtual void		FindCommandStatus(CommandT inCommand,
							Boolean &outEnabled, Boolean &outUsesMark,
							Char16 &outMark, Str255 outName);
	virtual void		HandleAppleEvent( const AppleEvent &inAppleEvent,
							AppleEvent &outAEReply, AEDesc &outResult,
							Int32 inAENumber );
	virtual void		OpenDocument(FSSpec *inMacFSSpec);
	virtual void		ListenToMessage(	MessageT	inMessage,
											void		*ioParam);


	PGPKeyListRef		GetKeyList();
	void				SetKeyList(PGPKeyListRef keyList);
	PGPKeySetRef		GetKeySet();
	CKeyView			*GetDefaultKeyView()	{	return mDefaultKeysView;}
	Boolean				IsKeyringWritable()	{	return mWritableKeyring;	}
	void				InvalidateCaches();
	
	void				GetMinimumRandomData();
		
	inline 	static	CPGPKeys	*TheApp( void );
	
	void				CreatedKey( void );
	void				ResetKeyDB();
	void				CommitDefaultKeyrings();
	
#if PGP_FREEWARE
	void				ShowFreewareUpgradeDialog(Boolean skipDateCheck);
#endif
	
protected:
	void					ShowSplashScreen();
	void					EndSplashScreen();
	void					MakeLicenseStr(Str255 outLicenseStr);
	short					AskMakeNewKeyFiles();

	static	CPGPKeys		*sTheApp;
	
	PGPKeySetRef			mKeySet;
	PGPKeyListRef			mKeyList;
	CKeyWindow				*mKeyWindow;
	CKeyView				*mDefaultKeysView;
	CSearchWindow			*mSearchWindow;
	CGroupsWindow			*mGroupsWindow;
	LGADialogBox			*mSplashScreen;
	ulong					mSplashDisplayTime;
	LModelDirector			*mModelDirector;
	LMenuBar				*mMenuBar;
	Int16					mHelpMenuRegisterItemNumber;
	Boolean					mCreatedKey,
							mCheckedLicense,
							mWritableKeyring,
							mSearching,
							mAppStarted;
	
	Point					mLastMouseLoc;

	
	virtual void		StartUp();		// overriding startup functions
			void		CleanUpPPLeaks( void );
			Boolean		TryingToQuit( void );
};

const ResIDT	kMaxMenuID				 	= 500;

const MessageT	cmd_CollapseAll				= 1101;	
const MessageT	cmd_ExpandAll				= 1102;	
const MessageT	cmd_Preferences				= 1103;	
const MessageT	cmd_Sign1					= 1201;	
const MessageT	cmd_AddName					= 1202;	
const MessageT	cmd_SetDefault				= 1203;	
const MessageT	cmd_NewKey					= 1204;	
const MessageT	cmd_Info					= 1205;	
const MessageT	cmd_Revoke					= 1206;	
const MessageT	cmd_ImportKeys				= 1207;
const MessageT	cmd_ExportKeys				= 1208;
const MessageT	cmd_ChangePassphrase		= 1209;
const MessageT	cmd_OptionRevoke			= 1210;
const MessageT	cmd_KSGetKeys				= 1212;
const MessageT	cmd_KSSendKeys				= 1213;
const MessageT	cmd_KSFindKey				= 1214;
const MessageT	cmd_ViewValidity			= 1215;
const MessageT	cmd_ViewTrust				= 1216;
const MessageT	cmd_ViewSize				= 1217;
const MessageT	cmd_ViewKeyID				= 1218;
const MessageT	cmd_ViewCreationDate		= 1219;
const MessageT	cmd_ViewExpirationDate		= 1220;
const MessageT	cmd_ViewMRK					= 1221;
const MessageT	cmd_ViewDescription			= 1222;
const MessageT	cmd_ShowGroups				= 1223;
const MessageT	cmd_NewGroup				= 1224;
const MessageT	cmd_KSSendKeyToDomain		= 1225;
const MessageT	cmd_OptionClear				= 50;

	CPGPKeys *
CPGPKeys::TheApp()
{
	pgpAssertAddrValid(sTheApp, CPGPKeys);
	return sTheApp;
}

void ReportPGPError(PGPError err);

#define min(x,y) (((x)<(y)) ? (x) : (y))

#define max(x,y) (((x)>(y)) ? (x) : (y))

