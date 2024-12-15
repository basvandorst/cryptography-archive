/*____________________________________________________________________________	Copyright (C) 1997 Network Associates Inc. and affiliated companies.	All rights reserved.	$Id: BackupHandler.cp,v 1.15 1999/03/10 02:31:33 heller Exp $____________________________________________________________________________*/#include "BackupHandler.h"#include "CWarningAlert.h"#include "pgpKeys.h"#include "CPGPKeys.h"#include "MacFiles.h"#include "CPGPStDialogHandler.h"#include "pgpUtilities.h"#include "pgpSDKPrefs.h"#include <PP_Messages.h>#include <PP_KeyCodes.h>#include <UKeyFilters.h>	BooleanBackupHandler::Run(void){	CPGPStDialogHandler	theHandler(kBackupWindowID, this);	MessageT			hitMessage;		LWindow			*theDialog 	= theHandler.GetDialog();		pgpAssertAddrValid(theDialog, LWindow);		mOKButton = (LPushButton *)theDialog->FindPaneByID(kOKButtonID);		mDontSaveButton = (LPushButton *)		theDialog->FindPaneByID(kNoBackupButtonID);		mCancelButton = (LPushButton *)theDialog->FindPaneByID(kCancelButtonID);			mCancelButton->AddListener(&theHandler);	theDialog->Show();	SwitchTarget(theDialog);		do	{		hitMessage = theHandler.DoDialog();	}	while(hitMessage == msg_Nothing);		if(hitMessage == msg_OK)	{		Point		where	= { 0, 0 };		SFReply		reply;		Str255		prompt, fileName;				GetIndString(prompt, kBackupStrings, kBackupMessageID);		pgpAssert(prompt[ 0 ] > 0);				GetIndString(fileName, kBackupStrings, kBackupFileNameID);		pgpAssert(fileName[ 0 ] > 0);				SFPutFile(where, prompt, fileName, nil, &reply);		if(reply.good)		{			OSStatus		myErr;			PGPError		err;			Int16			vRefNum;			Int32			dirID, procID;			FSSpec			spec, srcSpec;						myErr = GetWDInfo(reply.vRefNum, &vRefNum, &dirID, &procID);			if(myErr == noErr)			{				myErr = FSMakeFSSpec(vRefNum, dirID, reply.fName, &spec);				if(myErr == fnfErr)					myErr = noErr;				FSpDelete( &spec );			}										if(myErr == noErr)			{				PGPFileSpecRef	fileRef;							err = PGPsdkPrefGetFileSpec(	gPGPContext,												kPGPsdkPref_PrivateKeyring,												&fileRef);				if(IsntErr(err) && fileRef)				{					err = PGPGetFSSpecFromFileSpec(fileRef, &srcSpec);					pgpAssertNoErr(err);					CPGPKeys::TheApp()->CommitDefaultKeyrings();					myErr = FSpCopyFiles( gPGPMemoryMgr, &srcSpec, &spec);					PGPFreeFileSpec( fileRef );					fileRef	= NULL;				}				else					myErr = -43;			}			if(myErr != noErr)				CWarningAlert::Display(kWACautionAlertType, kWAOKStyle,								kBackupStrings, kBackupErrorStringID);		}		GetIndString(prompt, kBackupStrings, kPublicBackupMessageID);		pgpAssert(prompt[ 0 ] > 0);				GetIndString(fileName, kBackupStrings, kPublicFileNameID);		pgpAssert(fileName[ 0 ] > 0);				SFPutFile(where, prompt, fileName, nil, &reply);		if(reply.good)		{			OSStatus		myErr;			PGPError		err;			PGPFileSpecRef	fileRef;			Int16			vRefNum;			Int32			dirID, procID;			FSSpec			spec, srcSpec;						myErr = GetWDInfo(reply.vRefNum, &vRefNum, &dirID, &procID);			if(myErr == noErr)			{				myErr = FSMakeFSSpec(vRefNum, dirID, reply.fName, &spec);				if(myErr == fnfErr)					myErr = noErr;				FSpDelete( &spec );			}										if(myErr == noErr)			{				err = PGPsdkPrefGetFileSpec(	gPGPContext,												kPGPsdkPref_PublicKeyring,												&fileRef);				if(fileRef)				{					err = PGPGetFSSpecFromFileSpec(fileRef, &srcSpec);					pgpAssertNoErr(err);					CPGPKeys::TheApp()->CommitDefaultKeyrings();					myErr = FSpCopyFiles(gPGPMemoryMgr, &srcSpec, &spec);				}				else					myErr = -43;			}			if(myErr != noErr)				CWarningAlert::Display(kWACautionAlertType, kWAOKStyle,								kBackupStrings, kBackupErrorStringID);		}	}	else if(hitMessage == msg_Cancel)		return false;	return true;}	BooleanBackupHandler::HandleKeyPress(const EventRecord&	inKeyEvent){	switch (inKeyEvent.message & charCodeMask) 	{		case char_Enter:		case char_Return:			pgpAssertAddrValid(mOKButton, VoidAlign);			mOKButton->SimulateHotSpotClick(kControlButtonPart);			return true;			break;					case char_Escape:			pgpAssertAddrValid(mCancelButton, VoidAlign);			if ((inKeyEvent.message & keyCodeMask) == vkey_Escape) 				mCancelButton->SimulateHotSpotClick(kControlButtonPart);			return true;			break;					default:		{			char	charCode = inKeyEvent.message & charCodeMask;						if ((charCode == 's') || (charCode == 'S'))			{				pgpAssertAddrValid(mOKButton, VoidAlign);				mOKButton->SimulateHotSpotClick(kControlButtonPart);				return true;			}			else if ((charCode == 'd') || (charCode == 'D'))			{				pgpAssertAddrValid(mDontSaveButton, VoidAlign);				mDontSaveButton->SimulateHotSpotClick(kControlButtonPart);				return true;			}			if (UKeyFilters::IsCmdPeriod(inKeyEvent)) 			{				mCancelButton->SimulateHotSpotClick(kControlButtonPart);				return true;			}			else 				return LCommander::HandleKeyPress(inKeyEvent);		}	}}