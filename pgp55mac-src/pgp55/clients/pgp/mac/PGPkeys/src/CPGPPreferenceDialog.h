/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CPGPPreferenceDialog.h,v 1.17 1997/10/11 12:39:46 wprice Exp $
____________________________________________________________________________*/
#pragma once

#include "pflPrefTypes.h"

typedef struct PGPmenuApps
{
	PGPBoolean	privateScrap;
	PGPBoolean	useOutputDialog;
	OSType		creator;
	Str31		appName;
} PGPmenuApps;

enum PGPFileLocationTypes
{
	kPGPLTPublic = 0,
	kPGPLTPrivate,
	kPGPLTRandom
};

class CWhiteList;
class CKeyserverTable;

class CPGPPreferenceDialog	:	public LGADialogBox,
								public LListener,
								public LBroadcaster,
								public LCommander
{
public:
	enum { class_ID = 'pDlg' };
						CPGPPreferenceDialog();
						CPGPPreferenceDialog(LStream *inStream);
						~CPGPPreferenceDialog();
	void				ListenToMessage(MessageT inMessage, void *ioParam);
	void				FinishCreateSelf();
	virtual Boolean		ObeyCommand(CommandT inCommand, void* ioParam);	
	virtual void		FindCommandStatus(CommandT inCommand,
							Boolean &outEnabled, Boolean &outUsesMark,
							Char16 &outMark, Str255 outName);

	static OSStatus		SelectKeyFile(	FSSpec *ioSpec,
										enum PGPFileLocationTypes	type);
	static void			SetLibSpecs(	FSSpec *fsPublic,
										FSSpec *fsPrivate,
										FSSpec *fsRandom);
	static void			ResetDefaultKeyringsPath();
private:
	void				PGPSetPrefCheckbox(
										PGPPrefIndex	prefIndex,
										PaneIDT			paneID);
	void				PGPGetPrefCheckbox(
										PaneIDT			paneID,
										PGPPrefIndex	prefIndex);
	void				InsertPanel(Int16 inPanelID);
	void				SavePanel();
	
	void				SetupKeyServerPanel();
	void				SaveKeyServerPanel();
	void				ServerEntryDialog(TableIndexT row);
	
	void				LoadAdvancedPanel();
	void				SaveAdvancedPanel();
	
	static void			GetDefaultSpec(FSSpec& outSpec,
										enum PGPFileLocationTypes type);
	static void			SetCaptionFromSpec(LCaption* inCaptionP,
											const FSSpec& inSpec);
	
	LGATabPanel			*mTabPanel;
	LView				*mCurrentPanel;
	short				mSelectedPanel;
	
	LCaption			*mPublicCaption,
						*mPrivateCaption,
						*mRandomCaption;
	FSSpec				mPrivateSpec,
						mPublicSpec,
						mRandomSpec;
	CWhiteList			*mPGPmenuAppsList;
	PGPmenuApps			*mPGPmenuApps;
	long				mNumPGPmenuApps;
	CKeyserverTable		*mServerTable;
	Int32				mLastArrowDecrypt,
						mLastArrowSign;
	
	Boolean				mChangedKeyFiles;
	
	enum	{
				kNoPanel				= 0,
				kEncryptionPanel,
				kKeyFilesPanel,
				kEmailPanel,
				kPGPmenuPanel,
				kKeyServerPanel,
				kAdvancedPanel
			};
	enum	{	
				kPrefPanel		 		= 'pPnl',
				kPanelMessage	 		= 'pref',
				kOKButton				= 'bOK ',
				kCancelButton			= 'bCan',
				
				// Encryption Prefs
				kEncryptSelfCheckbox	= 'xEnc',
				kFasterKeyGenCheckbox	= 'xFas',
				kWarnOnWipeCheckbox		= 'xWip',
				kCommentEditField		= 'eCom',
				kUseMacBinaryCheckbox	= 'xMBi',

				// Key Files Prefs
				kSetPublicButton		= 'bPub',
				kSetPrivateButton		= 'bPri',
				kSetRandomButton		= 'bRnd',
				kPublicCaption			= 'cPub',
				kPrivateCaption			= 'cPri',
				kRandomCaption			= 'cRnd',
				
				// Email Prefs
				kPGPMIMEEncryptCheckBox	= 'xEnc',
				kPGPMIMESignCheckBox	= 'xSig',
				kCacheKeyCheckBox		= 'xDCa',
				kCacheKeyDuration		= 'eDDu',
				kCacheKeyArrows			= 'lArD',
				kSignCacheCheckBox		= 'xSCa',
				kSignCacheDuration		= 'eSDu',
				kSignCacheArrows		= 'lArS',
				kWordWrapCheckBox		= 'xWrp',
				kWordWrapEditField		= 'eWrp',
				
				// Server Prefs
				kKeyServerTable			= 'tKsv',
				kNewKeyServerButton		= 'bNSv',
				kRemoveKeyServerButton	= 'bRSv',
				kSetDefaultServerButton = 'bDSv',
				kKeyServerSyncGroup		= 'gSyn',
				kUnknownEncryptCheckbox = 'xUnk',
				kAddNamesCheckbox		= 'xAdd',
				kSignKeysCheckbox		= 'xSig',
				kRevokeKeysCheckbox		= 'xRev',
				
				//Advanced Prefs
				kPreferredAlgPopup		= 'pAlg',
				kAllowCASTCheckbox		= 'xCAS',
				kAllowIDEACheckbox		= 'xIDE',
				kAllowTripleDESCheckbox	= 'x3DE',
				kMarginalInvalidCheckbox= 'xInv',
				kShowMarginalCheckbox	= 'xMrg',
				kWarnOnMRK				= 'xMRK'
			};
	enum	{
				kEncryptionPanelID		= 143,
				kKeyFilesPanelID		= 144,
				kEmailPanelID			= 145,
				kPGPmenuPanelID			= 148,
				kKeyServerPanelID		= 149,
				kAdvancedPanelID		= 151
			};
	enum	{
				kPreferencesKeyFilePutDlgID	= 1006,
				kPreferencesStringsID		= 1006,
				kDefaultPrivateKeyFileName	= 1,
				kDefaultPublicKeyFileName,
				kDefaultRandomSeedFileName,
				kNoDomainOrURLErrorStringID,
				kNoAlgorithmsErrorStringID,
				kPGPKeyringsFolderStringID
			};
};

