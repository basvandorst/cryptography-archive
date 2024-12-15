/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CPGPPreferenceDialog.cp,v 1.37 1997/10/15 22:31:20 lloyd Exp $
____________________________________________________________________________*/
#include "CPGPPreferenceDialog.h"
#include "ErrorDialogBox.h"
#include "CPGPKeys.h"
#include "MacMemory.h"
#include "CWhiteList.h"
#include "CWarningAlert.h"
#include "CKeyTable.h"
#include "CKeyserverTable.h"
#include "CPGPStDialogHandler.h"
#include "CGADurationEditField.h"
#include "CPopupList.h"
#include "ResourceConstants.h"
#include "MacFiles.h"
#include "MacStrings.h"
#include "MacDialogs.h"

#include "pgpUtilities.h"
#include "pgpErrors.h"
#include "pgpSDKPrefs.h"
#include "pgpClientPrefs.h"
#include "pgpAdminPrefs.h"
#include "pgpKeyServerPrefs.h"
#include "pflContext.h"

#include <Finder.h>
#include <LGALittleArrows.h>
#include <string.h>
#include "pgpStrings.h"

const ResIDT	kAdvancedPGPmenuDialogID	= 141;
const ResIDT	kNewServerDialogID			= 153;
const ResID		kGetFileShowAllPublicID 	= 1001;
const ResID		kGetFileShowAllPrivateID 	= 1002;
const ResID		kGetFileShowAllRandomID 	= 1003;
const PaneIDT	kPGPmenuAppListID			= 'pmAL';
const PaneIDT	kPGPmenuAddButton			= 'bAdd';
const PaneIDT	kPGPmenuRemoveButton		= 'bRmv';
const PaneIDT	kServerNameEditField		= 'eSrv';
const PaneIDT	kServerPortEditField		= 'ePrt';

const Int16		kMaxAlgorithms				= 3;

typedef struct SFInfo
{
	StandardFileReply			sfReply;
	Boolean						showAllFiles;
	enum PGPFileLocationTypes	type;
} SFInfo;

enum
{
	kShowAllFilesCheckboxItem	= 10
};

CPGPPreferenceDialog::CPGPPreferenceDialog()
{
}

CPGPPreferenceDialog::CPGPPreferenceDialog(LStream *inStream)
	: LGADialogBox(inStream)
{
	mCurrentPanel = nil;
	mPGPmenuApps = nil;
	mNumPGPmenuApps = 0;
	mChangedKeyFiles = FALSE;
}

CPGPPreferenceDialog::~CPGPPreferenceDialog()
{
	PGPSavePrefFile(gPrefRef);
	PGPsdkSavePrefs(gPGPContext);
	BroadcastMessage(kKeyTableRedrawMessage, NULL);
}

	void
CPGPPreferenceDialog::FinishCreateSelf()
{
	Int16 			panel;
	PGPError		err;
	PGPFileSpecRef	fileRef;
	
	LGADialogBox::FinishCreateSelf();
	
	err = PGPsdkPrefGetFileSpec(	gPGPContext,
									kPGPsdkPref_PublicKeyring,
									&fileRef);
	if(IsntPGPError(err) && fileRef)
	{
		err = PGPGetFSSpecFromFileSpec(fileRef, &mPublicSpec);
		pgpAssertNoErr(err);
		PGPFreeFileSpec(fileRef);
	}
	else
		GetDefaultSpec(mPublicSpec, kPGPLTPublic);
	err = PGPsdkPrefGetFileSpec(	gPGPContext,
									kPGPsdkPref_PrivateKeyring,
									&fileRef);
	if(IsntPGPError(err) && fileRef)
	{
		err = PGPGetFSSpecFromFileSpec(fileRef, &mPrivateSpec);
		pgpAssertNoErr(err);
		PGPFreeFileSpec(fileRef);
	}
	else
		GetDefaultSpec(mPrivateSpec, kPGPLTPrivate);
	err = PGPsdkPrefGetFileSpec(	gPGPContext,
									kPGPsdkPref_RandomSeedFile,
									&fileRef);
	if(IsntPGPError(err) && fileRef)
	{
		err = PGPGetFSSpecFromFileSpec(fileRef, &mRandomSpec);
		pgpAssertNoErr(err);
		PGPFreeFileSpec(fileRef);
	}
	else
		GetDefaultSpec(mRandomSpec, kPGPLTRandom);
	
	((LGAPushButton *) FindPaneByID(kOKButton))->AddListener(this);
	((LGAPushButton *) FindPaneByID(kCancelButton))->AddListener(this);
	mTabPanel = (LGATabPanel *) FindPaneByID(kPrefPanel);
	pgpAssertAddrValid(mTabPanel, VoidAlign);
	if(mTabPanel)
	{
		mTabPanel->StartBroadcasting();
		mTabPanel->AddListener(this);
	}
	InsertPanel(kEncryptionPanelID);
	mSelectedPanel = kNoPanel;
	panel = kEncryptionPanel;
	ListenToMessage(kPanelMessage, &panel);
}

	Boolean
CPGPPreferenceDialog::ObeyCommand(
	CommandT	inCommand,
	void		*ioParam)
{
	return true;
}

	void
CPGPPreferenceDialog::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	Char16		&outMark,
	Str255		outName)
{
	outEnabled = false;
	return;
}

	void
CPGPPreferenceDialog::ServerEntryDialog(TableIndexT row)
{
	TableIndexT			numRows, numCols;
	STableCell			cell;
	KeyServerTableEntry	kstEntry,
						searchEntry;
	Uint32				dataSize = sizeof(KeyServerTableEntry);
	Str255				defaultDomain;
	
	defaultDomain[0] = 0;
	if(row > 0)
	{
		cell.col = 1;
		cell.row = row;
		mServerTable->GetCellData(cell, &kstEntry, dataSize);
		if(kstEntry.isDomain)
			return;
		CToPString(kstEntry.ks.domain, defaultDomain);
	}
	else
	{
		cell = mServerTable->GetFirstSelectedCell();
		if(cell.row > 0)
		{
			mServerTable->GetCellData(cell, &kstEntry, dataSize);
			CToPString(kstEntry.ks.domain, defaultDomain);
		}
		pgpClearMemory(&kstEntry, sizeof(KeyServerTableEntry));
		kstEntry.ks.flags |= kKeyServerListed;
	}
	{
		CPGPStDialogHandler	newServerDialog(
								kNewServerDialogID, this);
		LWindow				*nsd;
		MessageT			dialogMessage;
		Str255				pstr;

		const PaneIDT	kDomainEditField		= 'eDom';
		const PaneIDT	kURLEditField			= 'eURL';
		const PaneIDT	kListedCheckbox			= 'xLis';
		
		nsd = newServerDialog.GetDialog();
		((LGAEditField *)nsd->FindPaneByID(kDomainEditField))->
			SetDescriptor(defaultDomain);
		if(row > 0)
		{
			((LGAEditField *)nsd->FindPaneByID(kDomainEditField))->Disable();
			CToPString(kstEntry.ks.serverURL, pstr);
			((LGAEditField *)nsd->FindPaneByID(kURLEditField))->
				SetDescriptor(pstr);
		}
		if(IsKeyServerListed(kstEntry.ks.flags))
			((LGACheckbox *)nsd->FindPaneByID(kListedCheckbox))->
				SetValue(1);
		nsd->Show();
badInput:
		do
		{
			dialogMessage = newServerDialog.DoDialog();
		} while(dialogMessage != msg_OK &&
				dialogMessage != msg_Cancel);
		if(dialogMessage == msg_OK)
		{
			kstEntry.isDomain = FALSE;
			((LGAEditField *)nsd->FindPaneByID(kDomainEditField))->
				GetDescriptor(pstr);			
			PToCString(pstr, kstEntry.ks.domain);
			((LGAEditField *)nsd->FindPaneByID(kURLEditField))->
				GetDescriptor(pstr);
			PToCString(pstr, kstEntry.ks.serverURL);
			if(strlen(kstEntry.ks.serverURL) == 0 ||
				strlen(kstEntry.ks.domain) == 0)
			{
				CWarningAlert::Display(kWAStopAlertType, kWAOKStyle,
							kPreferencesStringsID,
							kNoDomainOrURLErrorStringID);
				goto badInput;
			}

			if(row < 1)
				kstEntry.ks.flags = 0;
			else
				kstEntry.ks.flags &= ~kKeyServerListed;
			if(((LGACheckbox *)nsd->FindPaneByID(kListedCheckbox))->
				GetValue())
				kstEntry.ks.flags |= kKeyServerListed;
			if(row > 0)
			{
				mServerTable->SetCellData(cell, &kstEntry, dataSize);
			}
			else
			{
				Boolean		found = FALSE;
				TableIndexT	insertRow;
				
				mServerTable->GetTableSize(numRows, numCols);
				cell.col = 1;
				for(cell.row = numRows;cell.row >= 1;cell.row--)
				{
					mServerTable->GetCellData(cell, &searchEntry, dataSize);
					if(!pgpCompareStringsIgnoreCase(kstEntry.ks.domain,
						searchEntry.ks.domain))
					{
						found = TRUE;
						mServerTable->InsertSiblingRows(
												1, cell.row, &kstEntry,
												sizeof(KeyServerTableEntry),
												FALSE, TRUE);
						break;
					}
				}
				if(!found)
				{
					kstEntry.isDomain = TRUE;
					insertRow = mServerTable->InsertSiblingRows(
											1, 0, &kstEntry,
											sizeof(KeyServerTableEntry),
											TRUE, TRUE);
					kstEntry.isDomain = FALSE;
					mServerTable->InsertChildRows(1, insertRow, &kstEntry,
									sizeof(KeyServerTableEntry),
									FALSE, TRUE);
				}
			}
		}
	}
}

	void
CPGPPreferenceDialog::ListenToMessage(MessageT inMessage, void *ioParam)
{
	PGPUInt32				longPref;
	PGPError				err;
	Int32					newValue;
	CGADurationEditField	*durationObj;
	LGALittleArrows			*arrowsObj;
	
	switch(inMessage)
	{
		case kAllowCASTCheckbox:
		case kAllowIDEACheckbox:
		case kAllowTripleDESCheckbox:
			if(!(((LGACheckbox *)
				FindPaneByID(kAllowCASTCheckbox))->GetValue() +
				((LGACheckbox *)
				FindPaneByID(kAllowIDEACheckbox))->GetValue() +
				((LGACheckbox *)
				FindPaneByID(kAllowTripleDESCheckbox))->GetValue()))
			{
				CWarningAlert::Display(kWAStopAlertType, kWAOKStyle,
							kPreferencesStringsID,
							kNoAlgorithmsErrorStringID);
				((LGACheckbox *)
					FindPaneByID(kAllowCASTCheckbox))->SetValue(1);
			}
			break;
		case kKSTSelectionChangedMessageID:
		{
			LGAPushButton	*removeKSButton,
							*setDefaultButton;
			
			removeKSButton = (LGAPushButton *)FindPaneByID(
									kRemoveKeyServerButton);
			setDefaultButton = (LGAPushButton *)FindPaneByID(
									kSetDefaultServerButton);
			if(mServerTable->GetTableSelector()->GetFirstSelectedRow() > 0)
			{
				removeKSButton->Enable();
				setDefaultButton->Enable();
			}
			else
			{
				removeKSButton->Disable();
				setDefaultButton->Disable();
			}
			break;
		}
		case kKSTCellDoubleClickedMessageID:
			TableIndexT			row = (TableIndexT)ioParam;
			
			ServerEntryDialog(row);
			break;
		case kWLSelectionChangedMessageID:
		{
			LGAPushButton	*removePMButton;
			
			removePMButton = (LGAPushButton *)FindPaneByID(
									kPGPmenuRemoveButton);
			if(mPGPmenuAppsList->GetTableSelector()->
				GetFirstSelectedRow() > 0)
				removePMButton->Enable();
			else
				removePMButton->Disable();
			break;
		}
		case kWLCellDoubleClickedMessageID:
			{
				TableIndexT			row = ((TableIndexT)ioParam) - 1;
				CPGPStDialogHandler	advMenuDialog(
										kAdvancedPGPmenuDialogID, this);
				LWindow				*amd;
				MessageT			dialogMessage;
				const PaneIDT	kPrivateScrapCheckbox	= 'xScr';
				const PaneIDT	kOutputDialogCheckbox	= 'xOut';
				const PaneIDT	kAppNameCaption			= 'cNam';
				
				amd = advMenuDialog.GetDialog();
				((LCaption *)amd->FindPaneByID(kAppNameCaption))->
					SetDescriptor(mPGPmenuApps[row].appName);
				((LGACheckbox *)amd->FindPaneByID(kPrivateScrapCheckbox))->
					SetValue(mPGPmenuApps[row].privateScrap);
				((LGACheckbox *)amd->FindPaneByID(kOutputDialogCheckbox))->
					SetValue(mPGPmenuApps[row].useOutputDialog);
				do
				{
					dialogMessage = advMenuDialog.DoDialog();
				} while(dialogMessage != msg_OK &&
						dialogMessage != msg_Cancel);
				if(dialogMessage == msg_OK)
				{
					mPGPmenuApps[row].privateScrap =
						((LGACheckbox *)amd->
						FindPaneByID(kPrivateScrapCheckbox))->
						GetValue();
					mPGPmenuApps[row].useOutputDialog =
						((LGACheckbox *)amd->
						FindPaneByID(kOutputDialogCheckbox))->
						GetValue();
				}
			}
			break;
		case kCacheKeyArrows:
			durationObj = (CGADurationEditField *)
							FindPaneByID( kCacheKeyDuration );
			pgpAssertAddrValid( durationObj, CGADurationEditField );
			newValue = *(Int32 *)ioParam;
			if(newValue < mLastArrowDecrypt)
				durationObj->AdjustValue(kDownAdjust);
			else
				durationObj->AdjustValue(kUpAdjust);
			mLastArrowDecrypt = newValue;
			break;
		case kSignCacheArrows:
			durationObj = (CGADurationEditField *)
							FindPaneByID( kSignCacheDuration );
			pgpAssertAddrValid( durationObj, CGADurationEditField );
			newValue = *(Int32 *)ioParam;
			if(newValue < mLastArrowSign)
				durationObj->AdjustValue(kDownAdjust);
			else
				durationObj->AdjustValue(kUpAdjust);
			mLastArrowSign = newValue;
			break;
		case kPanelMessage:
			Int16	panelIndex = *(Int16 *) ioParam;
			
			SavePanel();
			switch(panelIndex)
			{
				case kEncryptionPanel:
					{
						char			commentStr[256];
						Str255			commentPStr;
						PGPSize			commentLen;
						LGAEditField	*commentField;
						
						InsertPanel(kEncryptionPanelID);
						mSelectedPanel = kEncryptionPanel;
						
						PGPSetPrefCheckbox(	kPGPPrefEncryptToSelf,
											kEncryptSelfCheckbox);
						PGPSetPrefCheckbox(	kPGPPrefFastKeyGen,
											kFasterKeyGenCheckbox);
						PGPSetPrefCheckbox(	kPGPPrefWarnOnWipe,
											kWarnOnWipeCheckbox);
						
						err = PGPGetPrefNumber(gPrefRef,
								kPGPPrefMacBinaryDefault, &longPref);
						pgpAssertNoErr(err);
						((LGACheckbox *)
							FindPaneByID(kUseMacBinaryCheckbox))->
								SetValue((longPref == kPGPPrefMacBinaryOn));
						commentLen = 256;
						err = PGPGetPrefStringBuffer(gPrefRef,
							kPGPPrefComment, commentLen, commentStr);
						pgpAssertNoErr(err);
						CToPString(commentStr, commentPStr);
						commentField =
							(LGAEditField *)FindPaneByID(kCommentEditField);
						commentField->SetDescriptor(commentPStr);
#if PGP_BUSINESS_SECURITY						
						err = PGPGetPrefStringBuffer(gAdminPrefRef,
							kPGPPrefComments, commentLen, commentStr);
						if(IsntPGPError(err) && commentStr[0] != '\0')
						{
							CToPString(commentStr, commentPStr);
							commentField->SetDescriptor(commentPStr);
							commentField->Disable();
						}
#endif
					}
					break;
				case kKeyFilesPanel:
					InsertPanel(kKeyFilesPanelID);
					mSelectedPanel = kKeyFilesPanel;
					((LGATextButton *) FindPaneByID(kSetPublicButton))->
									AddListener(this);
					((LGATextButton *) FindPaneByID(kSetPrivateButton))->
									AddListener(this);
					((LGATextButton *) FindPaneByID(kSetRandomButton))->
									AddListener(this);
					mPublicCaption =
						(LCaption *)FindPaneByID(kPublicCaption);
					pgpAssertAddrValid(mPublicCaption, VoidAlign);
					mPrivateCaption =
						(LCaption *)FindPaneByID(kPrivateCaption);
					pgpAssertAddrValid(mPrivateCaption, VoidAlign);
					mRandomCaption =
						(LCaption *)FindPaneByID(kRandomCaption);
					pgpAssertAddrValid(mPrivateCaption, VoidAlign);
					SetCaptionFromSpec(mPublicCaption, mPublicSpec);
					SetCaptionFromSpec(mPrivateCaption, mPrivateSpec);
					SetCaptionFromSpec(mRandomCaption, mRandomSpec);
					break;
				case kEmailPanel:
					InsertPanel(kEmailPanelID);
					mSelectedPanel = kEmailPanel;
					
					PGPSetPrefCheckbox(	kPGPPrefMailEncryptPGPMIME,
										kPGPMIMEEncryptCheckBox);
					PGPSetPrefCheckbox(	kPGPPrefMailSignPGPMIME,
										kPGPMIMESignCheckBox);
					PGPSetPrefCheckbox(	kPGPPrefDecryptCacheEnable,
										kCacheKeyCheckBox);
					PGPSetPrefCheckbox(	kPGPPrefSignCacheEnable,
										kSignCacheCheckBox);
					PGPSetPrefCheckbox(	kPGPPrefWordWrapEnable,
										kWordWrapCheckBox);

					err = PGPGetPrefNumber(gPrefRef,
						kPGPPrefWordWrapWidth, &longPref);
					pgpAssertNoErr(err);
					((LGACheckbox *)FindPaneByID(kWordWrapEditField))->
						SetValue(longPref);
					
					err = PGPGetPrefNumber(gPrefRef,
						kPGPPrefDecryptCacheSeconds, &longPref);
					pgpAssertNoErr(err);
					durationObj = (CGADurationEditField *)
						FindPaneByID(kCacheKeyDuration);
					durationObj->SetDurationType(kLengthDurationType);
					durationObj->SetDurationValue(longPref);
					durationObj->Refresh();
					
					err = PGPGetPrefNumber(gPrefRef,
						kPGPPrefSignCacheSeconds, &longPref);
					pgpAssertNoErr(err);
					durationObj = (CGADurationEditField *)
						FindPaneByID(kSignCacheDuration);
					durationObj->SetDurationType(kLengthDurationType);
					durationObj->SetDurationValue(longPref);
					durationObj->Refresh();
					
					arrowsObj = (LGALittleArrows *)
						FindPaneByID(kCacheKeyArrows);
					mLastArrowDecrypt =  arrowsObj->GetValue();
					arrowsObj->AddListener(this);
					arrowsObj = (LGALittleArrows *)
						FindPaneByID(kSignCacheArrows);
					mLastArrowSign =  arrowsObj->GetValue();
					arrowsObj->AddListener(this);
					break;
				case kPGPmenuPanel:{
					size_t sigsSize;
					
					InsertPanel(kPGPmenuPanelID);
					mSelectedPanel = kPGPmenuPanel;
					
					sigsSize = 0;
					err = PGPGetPrefData(
									gPrefRef,
									kPGPPrefPGPmenuMacAppSignatures,
									&sigsSize, &mPGPmenuApps);
					if(IsNull(mPGPmenuApps))
					{
						mNumPGPmenuApps = 0;
						mPGPmenuApps = (PGPmenuApps *)
							PFLContextMemAlloc(gPFLContext, 0, 0);
					}
					else
						mNumPGPmenuApps = sigsSize / sizeof(PGPmenuApps);
					pgpAssert((sigsSize % sizeof(PGPmenuApps)) == 0);
					if(IsntNull(mPGPmenuApps))
					{
						((LGAPushButton *)FindPaneByID(kPGPmenuAddButton))->
							AddListener(this);
						((LGAPushButton *)FindPaneByID(
							kPGPmenuRemoveButton))->AddListener(this);
						mPGPmenuAppsList = (CWhiteList *)FindPaneByID(
							kPGPmenuAppListID);
						mPGPmenuAppsList->InsertCols(1, 1, NULL, 0, TRUE);
						for(short inx=0;inx<mNumPGPmenuApps;inx++)
						{
							mPGPmenuAppsList->InsertRows(1, 32767,
								&mPGPmenuApps[inx].appName,
								sizeof(Str31),
								true);
						}
						mPGPmenuAppsList->AddListener(this);
					}
					break;}
				case kKeyServerPanel:
					InsertPanel(kKeyServerPanelID);
					mSelectedPanel = kKeyServerPanel;
	
					SetupKeyServerPanel();
					break;
				case kAdvancedPanel:
					LoadAdvancedPanel();
					break;
			}
			break;
		case kSetPublicButton:
			if(IsntErr(SelectKeyFile(&mPublicSpec, kPGPLTPublic)))
			{
				SetCaptionFromSpec(mPublicCaption, mPublicSpec);
				mChangedKeyFiles = TRUE;
			}
			break;
		case kSetPrivateButton:
			if(IsntErr(SelectKeyFile(&mPrivateSpec, kPGPLTPrivate)))
			{
				SetCaptionFromSpec(mPrivateCaption, mPrivateSpec);
				mChangedKeyFiles = TRUE;
			}
			break;
		case kSetRandomButton:
			if(IsntErr(SelectKeyFile(&mRandomSpec, kPGPLTRandom)))
			{
				SetCaptionFromSpec(mRandomCaption, mRandomSpec);
				mChangedKeyFiles = TRUE;
			}
			break;
		case kPGPmenuAddButton:
			{
				SFTypeList sfTypes;
				StandardFileReply reply;
				FInfo fndrInfo;
				
				sfTypes[0] = 'APPL';
				sfTypes[1] = 'FNDR';
				sfTypes[2] = 'adrp';
				StandardGetFile(NULL, 3, sfTypes, &reply);
				if((reply.sfGood) && (!FSpGetFInfo(&reply.sfFile, &fndrInfo)))
				{
					err = PFLContextMemRealloc(gPFLContext, &mPGPmenuApps,
								sizeof(PGPmenuApps) * (mNumPGPmenuApps + 1),
								0);
					if(IsntPGPError(err))
					{
						mPGPmenuApps[mNumPGPmenuApps].privateScrap = TRUE;
						mPGPmenuApps[mNumPGPmenuApps].useOutputDialog = TRUE;
						mPGPmenuApps[mNumPGPmenuApps].creator =
							fndrInfo.fdCreator;
						CopyPString(reply.sfFile.name,
							mPGPmenuApps[mNumPGPmenuApps].appName);
						mPGPmenuAppsList->InsertRows(1, 32767,
							&mPGPmenuApps[mNumPGPmenuApps].appName,
							sizeof(Str31), true);
						mNumPGPmenuApps++;
					}
				}
			}
			break;
		case kPGPmenuRemoveButton:
			{
				STableCell	cell;
				
				cell = mPGPmenuAppsList->GetFirstSelectedCell();
				if((cell.row > 0) && (cell.col == 1))
				{
					mPGPmenuAppsList->RemoveRows(1, cell.row, TRUE);
					for(short inx = cell.row;inx < mNumPGPmenuApps;inx++)
						mPGPmenuApps[inx - 1] = mPGPmenuApps[inx];
					mNumPGPmenuApps--;
				}
			}
			break;
		case kNewKeyServerButton:
			ServerEntryDialog(0);
			break;
		case kRemoveKeyServerButton:
			{
				TableIndexT			numRows, numCols;
				STableCell			cell;
				KeyServerTableEntry	kstEntry;
				Uint32				dataSize = sizeof(KeyServerTableEntry);
				Boolean				delNextServers = FALSE,
									delThis = FALSE;
				
				mServerTable->GetTableSize(numRows, numCols);
				for(cell.row = cell.col = 1;cell.row <= numRows;cell.row++)
				{
					mServerTable->GetCellData(cell, &kstEntry, dataSize);
					if(mServerTable->CellIsSelected(cell)) 
					{
						if(kstEntry.isDomain)
							delNextServers = TRUE;
						else
							delThis = TRUE;
					}
					else if(delNextServers)
					{
						if(kstEntry.isDomain)
							delNextServers = FALSE;
						else
							delThis = TRUE;
					}
					if(delThis)
					{
						mServerTable->RemoveRows(1, cell.row, false);
						numRows--;
						cell.row--;
						delThis = FALSE;
					}
				}
				SaveKeyServerPanel();
				mServerTable->RemoveAllRows(false);
				SetupKeyServerPanel();
				mServerTable->Refresh();
			}
			break;
		case kSetDefaultServerButton:
			{
				TableIndexT			numRows, numCols;
				STableCell			cell;
				KeyServerTableEntry	kstEntry;
				Uint32				dataSize = sizeof(KeyServerTableEntry);
				Boolean				nextDefault = FALSE;
				
				mServerTable->GetTableSize(numRows, numCols);
				for(cell.row = cell.col = 1;cell.row <= numRows;cell.row++)
				{
					mServerTable->GetCellData(cell, &kstEntry, dataSize);
					if(mServerTable->CellIsSelected(cell) || nextDefault) 
					{
						if(kstEntry.isDomain)
							nextDefault = TRUE;
						else
						{
							kstEntry.ks.flags |= kKeyServerDefault;
							mServerTable->SetCellData(cell, &kstEntry,
														dataSize);
							nextDefault = FALSE;
						}
					}
					else if(IsKeyServerDefault(kstEntry.ks.flags))
					{
						kstEntry.ks.flags ^= kKeyServerDefault;
						mServerTable->SetCellData(cell, &kstEntry, dataSize);
					}
				}
				mServerTable->Refresh();
			}
			break;
		case 'bOK ':
			SavePanel();
		case 'bCan':
			if(mChangedKeyFiles)
				CPGPKeys::TheApp()->ResetKeyDB();
			delete this;
			break;
	}
}

	void
CPGPPreferenceDialog::LoadAdvancedPanel()
{
	CPopupList			*algPopup;
	PGPInt32			algIndex;
	Str255				algString;
	PGPUInt32			prefValue;
	PGPSize				dataSize;
	PGPCipherAlgorithm	*allowedCiphers;
	PGPError			err;
	
	InsertPanel(kAdvancedPanelID);
	mSelectedPanel = kAdvancedPanel;
	
	((LGACheckbox *)FindPaneByID(kAllowIDEACheckbox))->AddListener(this);
	((LGACheckbox *)FindPaneByID(kAllowTripleDESCheckbox))->AddListener(this);
	((LGACheckbox *)FindPaneByID(kAllowCASTCheckbox))->AddListener(this);

	algPopup = (CPopupList *)FindPaneByID(kPreferredAlgPopup);
	for(algIndex = 1 ; algIndex <= kMaxAlgorithms ; algIndex++)
	{
		GetIndString(algString, kAlgorithmStringsID, algIndex);
		algPopup->AddItem(algString);
	}
	
	// kPGPPrefPreferredAlgorithm
	err = PGPGetPrefNumber(gPrefRef, kPGPPrefPreferredAlgorithm, &prefValue);
	pgpAssertNoErr(err);
	algPopup->SetValue(prefValue - 1);
	
	// kPGPPrefAllowedAlgorithmsList
	err = PGPGetPrefData(gPrefRef,
				kPGPPrefAllowedAlgorithmsList,
				&dataSize,
				&allowedCiphers);
	if(IsntPGPError(err))
	{
		PGPInt32	numAlgs = dataSize / sizeof(PGPCipherAlgorithm);
		
		for(algIndex = 0; algIndex < numAlgs; algIndex ++)
		{
			switch(allowedCiphers[algIndex])
			{
				case kPGPCipherAlgorithm_IDEA:
					((LGACheckbox *)FindPaneByID(kAllowIDEACheckbox))->
						SetValue(1);
					break;
				case kPGPCipherAlgorithm_3DES:
					((LGACheckbox *)FindPaneByID(kAllowTripleDESCheckbox))->
						SetValue(1);
					break;
				case kPGPCipherAlgorithm_CAST5:
					((LGACheckbox *)FindPaneByID(kAllowCASTCheckbox))->
						SetValue(1);
					break;
			}
		}
		PGPDisposePrefData(gPrefRef, allowedCiphers);
	}
	
	PGPSetPrefCheckbox(	kPGPPrefMarginalIsInvalid, kMarginalInvalidCheckbox);
	PGPSetPrefCheckbox(	kPGPPrefDisplayMarginalValidity,
						kShowMarginalCheckbox);
	PGPSetPrefCheckbox(	kPGPPrefWarnOnADK, kWarnOnMRK);
}

	void
CPGPPreferenceDialog::SaveAdvancedPanel()
{
	PGPInt32			numAlgs = 0;
	PGPCipherAlgorithm	prefCipher;
	PGPSize				dataSize;
	PGPCipherAlgorithm	allowedCiphers[4];
	PGPError			err;
	
	prefCipher = (PGPCipherAlgorithm)(((CPopupList *)
		FindPaneByID(kPreferredAlgPopup))->GetValue() + 1);
	err = PGPSetPrefNumber(gPrefRef,
				kPGPPrefPreferredAlgorithm, prefCipher);
	pgpAssertNoErr(err);
	
	numAlgs				= 1;
	allowedCiphers[0]	= prefCipher;
	
	if(prefCipher != kPGPCipherAlgorithm_CAST5)
	{
		if(((LGACheckbox *)FindPaneByID(kAllowCASTCheckbox))->GetValue())
			allowedCiphers[numAlgs++] = kPGPCipherAlgorithm_CAST5;
	}
	if(prefCipher != kPGPCipherAlgorithm_3DES)
	{
		if(((LGACheckbox *)FindPaneByID(kAllowTripleDESCheckbox))->GetValue())
			allowedCiphers[numAlgs++] = kPGPCipherAlgorithm_3DES;
	}
	if(prefCipher != kPGPCipherAlgorithm_IDEA)
	{
		if(((LGACheckbox *)FindPaneByID(kAllowIDEACheckbox))->GetValue())
			allowedCiphers[numAlgs++] = kPGPCipherAlgorithm_IDEA;
	}
	dataSize = numAlgs * sizeof(PGPCipherAlgorithm);
	err = PGPSetPrefData(gPrefRef, kPGPPrefAllowedAlgorithmsList,
				dataSize, &allowedCiphers);
	pgpAssertNoErr(err);
	
	PGPGetPrefCheckbox(	kMarginalInvalidCheckbox,
						kPGPPrefMarginalIsInvalid);
	PGPGetPrefCheckbox(	kShowMarginalCheckbox,
						kPGPPrefDisplayMarginalValidity);
	PGPGetPrefCheckbox(	kWarnOnMRK,
						kPGPPrefWarnOnADK);

}

	void
CPGPPreferenceDialog::PGPSetPrefCheckbox(
	PGPPrefIndex	prefIndex,
	PaneIDT			paneID)
{
	PGPError	err;
	PGPBoolean	boolPref;

	err = PGPGetPrefBoolean(gPrefRef, prefIndex, &boolPref);
	pgpAssertNoErr(err);					
	((LGACheckbox *)FindPaneByID(paneID))->SetValue(boolPref);	
}

	void
CPGPPreferenceDialog::PGPGetPrefCheckbox(
	PaneIDT			paneID,
	PGPPrefIndex	prefIndex)
{
	PGPError		err;
	Boolean			boolPref;

	boolPref = ((LGACheckbox *)FindPaneByID(paneID))->GetValue();
	err = PGPSetPrefBoolean(gPrefRef, prefIndex, boolPref);
	pgpAssertNoErr(err);
}

	void
CPGPPreferenceDialog::SetupKeyServerPanel()
{
	PGPError	err;
	
	((LGAPushButton *)FindPaneByID(
		kNewKeyServerButton))->AddListener(this);
	((LGAPushButton *)FindPaneByID(
		kRemoveKeyServerButton))->AddListener(this);
	((LGAPushButton *)FindPaneByID(
		kSetDefaultServerButton))->AddListener(this);

#if PGP_BUSINESS_SECURITY
	PGPSetPrefCheckbox(	kPGPPrefKeyServerSyncUnknownKeys,
						kUnknownEncryptCheckbox);
	PGPSetPrefCheckbox(	kPGPPrefKeyServerSyncOnAddName,
						kAddNamesCheckbox);
	PGPSetPrefCheckbox(	kPGPPrefKeyServerSyncOnKeySign,
						kSignKeysCheckbox);
	PGPSetPrefCheckbox(	kPGPPrefKeyServerSyncOnRevocation,
						kRevokeKeysCheckbox);
#else
	((LGAPrimaryBox *)FindPaneByID(kKeyServerSyncGroup))->Hide();
#endif
		
	{
		PGPUInt32			ksCount;
		PGPKeyServerEntry	*ksEntries;
		char				lastDomain[256];
		KeyServerTableEntry	kstEntry;
		TableIndexT			lastDomainRow,
							lastServerRow;
		
		err = PGPGetKeyServerPrefs(gPrefRef,
			 &ksEntries, &ksCount);
		pgpAssertNoErr(err);					
		mServerTable = (CKeyserverTable *)
			FindPaneByID(kKeyServerTable);
		mServerTable->AddListener(this);
		mServerTable->InsertCols(1, 1, NULL, 0, TRUE);
		lastDomain[0] = 0;
		lastDomainRow = 0;
		for(short inx=0;inx<ksCount;inx++)
		{
			::BlockMoveData(&ksEntries[inx], &kstEntry.ks,
						sizeof(PGPKeyServerEntry));
			if(pgpCompareStringsIgnoreCase(lastDomain,
				ksEntries[inx].domain))
			{
				kstEntry.isDomain	= TRUE;
				lastDomainRow = mServerTable->InsertSiblingRows(
										1, lastDomainRow, &kstEntry,
										sizeof(KeyServerTableEntry),
										TRUE, TRUE);
				kstEntry.isDomain	= FALSE;
				mServerTable->InsertChildRows(1, lastDomainRow, &kstEntry,
								sizeof(KeyServerTableEntry),
								FALSE, TRUE);
				lastServerRow = lastDomainRow + 1;
			}
			else
			{
				kstEntry.isDomain	= FALSE;
				lastServerRow = mServerTable->InsertSiblingRows(
										1, lastServerRow, &kstEntry,
										sizeof(KeyServerTableEntry),
										FALSE, TRUE);
			}
			CopyCString(kstEntry.ks.domain, lastDomain);
		}
		PGPDisposePrefData(gPrefRef, ksEntries);
	}
}

	void
CPGPPreferenceDialog::SaveKeyServerPanel()
{
	PGPError			err;
	TableIndexT			numRows, numCols;
	STableCell			cell;
	PGPKeyServerEntry	*ksEntries;
	PGPUInt32			ksCount;
	KeyServerTableEntry	kstEntry;
	Uint32				dataSize = sizeof(KeyServerTableEntry);
	
	PGPGetPrefCheckbox(	kUnknownEncryptCheckbox,
						kPGPPrefKeyServerSyncUnknownKeys);
	PGPGetPrefCheckbox(	kAddNamesCheckbox,
						kPGPPrefKeyServerSyncOnAddName);
	PGPGetPrefCheckbox(	kSignKeysCheckbox,
						kPGPPrefKeyServerSyncOnKeySign);
	PGPGetPrefCheckbox(	kRevokeKeysCheckbox,
						kPGPPrefKeyServerSyncOnRevocation);
	
	mServerTable->GetTableSize(numRows, numCols);
	ksCount = 0;
	ksEntries = NULL;
	for(cell.row = cell.col = 1;cell.row <= numRows;cell.row++)
	{
		mServerTable->GetCellData(cell, &kstEntry, dataSize);
		if(!kstEntry.isDomain)
		{
			if(!ksCount)
				ksEntries = (PGPKeyServerEntry *)
					pgpAlloc(sizeof(PGPKeyServerEntry));
			else
			{
				err = pgpRealloc(&ksEntries,
					sizeof(PGPKeyServerEntry) * (ksCount + 1));
			}
			::BlockMoveData(&kstEntry.ks, &ksEntries[ksCount],
				sizeof(PGPKeyServerEntry));
			ksCount++;
		}
	}
	mServerTable->RemoveCols(1, 1, false);
	if(IsntNull(ksEntries) && ksCount > 0)
	{
		err = PGPSetKeyServerPrefs(gPrefRef, ksEntries, ksCount);
		pgpAssertNoErr(err);
		pgpFree(ksEntries);
	}
}
	
	void
CPGPPreferenceDialog::SetLibSpecs(FSSpec *fsPublic, FSSpec *fsPrivate,
									FSSpec *fsRandom)
{
	PGPFileSpecRef	fileRef;
	PGPError		err;
	
	err = PGPNewFileSpecFromFSSpec(	gPGPContext,
									fsPublic,
									&fileRef);
	pgpAssertNoErr(err);
	err = PGPsdkPrefSetFileSpec(	gPGPContext,
									kPGPsdkPref_PublicKeyring,
									fileRef);
	pgpAssertNoErr(err);
	PGPFreeFileSpec(fileRef);
	
	err = PGPNewFileSpecFromFSSpec(	gPGPContext,
									fsPrivate,
									&fileRef);
	pgpAssertNoErr(err);
	err = PGPsdkPrefSetFileSpec(	gPGPContext,
									kPGPsdkPref_PrivateKeyring,
									fileRef);
	pgpAssertNoErr(err);
	PGPFreeFileSpec(fileRef);

	if(IsntNull(fsRandom))
	{
		err = PGPNewFileSpecFromFSSpec(	gPGPContext,
										fsRandom,
										&fileRef);
		pgpAssertNoErr(err);
		err = PGPsdkPrefSetFileSpec(	gPGPContext,
										kPGPsdkPref_RandomSeedFile,
										fileRef);
		pgpAssertNoErr(err);
		PGPFreeFileSpec(fileRef);
	}
	if ( IsntPGPError( err ) )
	{
		err	= PGPsdkSavePrefs( gPGPContext );
	}
}

	void
CPGPPreferenceDialog::ResetDefaultKeyringsPath()
{
	DirInfo			pb;
	Str255			name;
	FSSpec			fsSpec;
	PGPFileSpecRef	fileRef;
	PGPError		err;
	
	::GetIndString(name, kPreferencesStringsID, kPGPKeyringsFolderStringID);
	pb.ioVRefNum = gAppFSSpec.vRefNum;
	pb.ioDrDirID = gAppFSSpec.parID;
	pb.ioNamePtr = name;
	pb.ioFDirIndex = 0;
	if (PBGetCatInfoSync((CInfoPBPtr)&pb) == noErr
				&& (pb.ioFlAttrib & ioDirMask))
	{
		fsSpec.vRefNum = pb.ioVRefNum;
		fsSpec.parID = pb.ioDrDirID;
	}
	else if (FindPGPPreferencesFolder(kOnSystemDisk, &fsSpec.vRefNum,
								&fsSpec.parID) != noErr)
		return;
	
	::GetIndString(fsSpec.name, kPreferencesStringsID,
								kDefaultPrivateKeyFileName);	
	{
		OSErr	oserr;
		Int16	refNum;
		
		oserr = FSpOpenDF(&fsSpec, fsRdPerm, &refNum);
		if(!oserr)
			FSClose(refNum);
	}
	
	err = PGPNewFileSpecFromFSSpec(	gPGPContext,
									&fsSpec,
									&fileRef);
	pgpAssertNoErr(err);
	err = PGPsdkPrefSetFileSpec(	gPGPContext,
									kPGPsdkPref_PrivateKeyring,
									fileRef);
	pgpAssertNoErr(err);
	PGPFreeFileSpec(fileRef);
	
	::GetIndString(fsSpec.name, kPreferencesStringsID,
								kDefaultPublicKeyFileName);
	err = PGPNewFileSpecFromFSSpec(	gPGPContext,
									&fsSpec,
									&fileRef);
	pgpAssertNoErr(err);
	err = PGPsdkPrefSetFileSpec(	gPGPContext,
									kPGPsdkPref_PublicKeyring,
									fileRef);
	pgpAssertNoErr(err);
	PGPFreeFileSpec(fileRef);
	
	PGPsdkSavePrefs(gPGPContext);
}

	void
CPGPPreferenceDialog::SavePanel()
{
	Boolean		boolPref;
	PGPUInt32	longPref;
	PGPError	err;
	
	switch(mSelectedPanel)
	{
		case kEncryptionPanel:
			{
				Str255	commentPStr;
				char	commentCStr[ sizeof( commentPStr ) ];
				
				PGPGetPrefCheckbox(	kEncryptSelfCheckbox,
									kPGPPrefEncryptToSelf);
				PGPGetPrefCheckbox(	kFasterKeyGenCheckbox,
									kPGPPrefFastKeyGen);
				PGPGetPrefCheckbox(	kWarnOnWipeCheckbox,
									kPGPPrefWarnOnWipe);

				boolPref = ((LGACheckbox *)FindPaneByID(
					kUseMacBinaryCheckbox))->GetValue();
				if(boolPref)
				{
					err = PGPSetPrefNumber(gPrefRef,
						kPGPPrefMacBinaryDefault, kPGPPrefMacBinaryOn);
					pgpAssertNoErr(err);
				}
				else
				{
					err = PGPSetPrefNumber(gPrefRef,
						kPGPPrefMacBinaryDefault, kPGPPrefMacBinaryOff);
					pgpAssertNoErr(err);
				}
				((LGAEditField *)FindPaneByID(kCommentEditField))->
					GetDescriptor(commentPStr);
				PToCString( commentPStr, commentCStr );
				err = PGPSetPrefString(gPrefRef, kPGPPrefComment,
									commentCStr);
				pgpAssertNoErr(err);
			}
			break;
		case kKeyFilesPanel:
			SetLibSpecs(&mPublicSpec, &mPrivateSpec, &mRandomSpec);
			break;
		case kEmailPanel:
			PGPGetPrefCheckbox(	kPGPMIMEEncryptCheckBox,
								kPGPPrefMailEncryptPGPMIME);
			PGPGetPrefCheckbox(	kPGPMIMESignCheckBox,
								kPGPPrefMailSignPGPMIME);
			PGPGetPrefCheckbox(	kCacheKeyCheckBox,
								kPGPPrefDecryptCacheEnable);
			PGPGetPrefCheckbox(	kSignCacheCheckBox,
								kPGPPrefSignCacheEnable);
			PGPGetPrefCheckbox(	kWordWrapCheckBox,
								kPGPPrefWordWrapEnable);
			
			longPref = ((CGADurationEditField *)
				FindPaneByID(kCacheKeyDuration))->GetDurationValue();
			if(longPref < 0)
				longPref = 0;
			err = PGPSetPrefNumber(gPrefRef,
				kPGPPrefDecryptCacheSeconds, longPref);
			pgpAssertNoErr(err);
			
			longPref = ((CGADurationEditField *)
				FindPaneByID(kSignCacheDuration))->GetDurationValue();
			if(longPref < 0)
				longPref = 0;
			err = PGPSetPrefNumber(gPrefRef,
				kPGPPrefSignCacheSeconds, longPref);
			pgpAssertNoErr(err);
			
			longPref = ((LGACheckbox *)FindPaneByID(
				kWordWrapEditField))->GetValue();
			if(longPref < 30)
				longPref = 78;
			else if(longPref > 240)
				longPref = 240;
			err = PGPSetPrefNumber(gPrefRef,
				kPGPPrefWordWrapWidth, longPref);
			pgpAssertNoErr(err);
			break;
		case kPGPmenuPanel:
			err = PGPSetPrefData(	gPrefRef,
									kPGPPrefPGPmenuMacAppSignatures,
									mNumPGPmenuApps * sizeof(PGPmenuApps),
									(void *)mPGPmenuApps);
			pgpAssertNoErr(err);
			PGPDisposePrefData(gPrefRef, mPGPmenuApps);
			break;
		case kKeyServerPanel:
			SaveKeyServerPanel();
			BroadcastMessage(kRebuildServersMessage, NULL);
			break;
		case kAdvancedPanel:
			SaveAdvancedPanel();
			break;
	}
}

	void			
CPGPPreferenceDialog::InsertPanel(Int16 inPanelID)
{
	LView* newPanel = UReanimator::CreateView(	inPanelID,
											mTabPanel->GetPanelHostView(),
											this );
	pgpAssert(IsntNull(newPanel));
	mTabPanel->InstallPanel(newPanel);
	if(mCurrentPanel)
	{
		mCurrentPanel->Hide ();
		delete mCurrentPanel;
		mCurrentPanel = nil;
	}
	mCurrentPanel = newPanel;
}

	void
CPGPPreferenceDialog::SetCaptionFromSpec(
	LCaption		*inCaptionP,
	const FSSpec	&inSpec)
{
	if (inCaptionP)
	{
		Str255	thePathStr;
		
		thePathStr[0] = 0;
		FSpGetFullPath(&inSpec, (uchar *)&thePathStr);
		
		inCaptionP->SetDescriptor(thePathStr);
	}
}

	void
CPGPPreferenceDialog::GetDefaultSpec(
	FSSpec						&outSpec,
	enum PGPFileLocationTypes	type)
{
	OSStatus	err;
	Int16		theVRef;
	Int32		theDirID;
	Str255		theFileName;
	Int16		theStringIndex;
		
	switch(type)
	{
		case kPGPLTPublic:
			theStringIndex = kDefaultPublicKeyFileName;
			break;
		case kPGPLTPrivate:
			theStringIndex = kDefaultPrivateKeyFileName;
			break;
		case kPGPLTRandom:
			theStringIndex = kDefaultRandomSeedFileName;
			break;
	}
	err = FindPGPPreferencesFolder(-1, &theVRef, &theDirID);
	pgpAssertNoErr(err);
	::GetIndString(	theFileName,
					kPreferencesStringsID,
					theStringIndex);
	err = ::FSMakeFSSpec(theVRef, theDirID, theFileName, &outSpec);
	pgpAssert((err == noErr) || (err == fnfErr));
}

	static pascal Boolean
VisibleItemFileFilterProc(CInfoPBPtr cpb)
{
	Boolean	shouldFilter = FALSE;
	
	pgpAssertAddrValid( cpb, CInfoPBRec);

	if( cpbIsFile( cpb ) )
	{
		if( ( cpb->hFileInfo.ioFlFndrInfo.fdFlags & kIsInvisible ) != 0 )
		{
			shouldFilter = TRUE;
		}
	}
	else
	{
		if( ( cpb->dirInfo.ioDrUsrWds.frFlags & kIsInvisible ) != 0 )
		{
			shouldFilter = TRUE;
		}
	}
	
	return( shouldFilter );
}

	static pascal Boolean
KeyFileFilterHook(CInfoPBPtr cpb, const SFInfo *sfInfo)
{
	Boolean	shouldFilter = FALSE;
	
	shouldFilter = VisibleItemFileFilterProc(cpb);
	if(!shouldFilter)
	{
		if(cpbIsFile(cpb) && !sfInfo->showAllFiles)
		{
			// Show only the files possibly created by PGP.
			switch(cpbFileType(cpb))
			{
				case 'PKey':	// ancient MacPGP 2.6.2 filetype
				case kPGPPublicKeyringType:
					if(sfInfo->type != kPGPLTPublic)
						shouldFilter = TRUE;
					break;
				case 'SKey':	// ancient MacPGP 2.6.2 filetype
				case kPGPPrivateKeyringType:
					if(sfInfo->type != kPGPLTPrivate)
						shouldFilter = TRUE;
					break;
				case kPGPRandomSeedType:
					if(sfInfo->type != kPGPLTRandom)
						shouldFilter = TRUE;
					break;
				default:
					shouldFilter = TRUE;
					break;
			}
		}
	}
		
	return( shouldFilter );
}

	static pascal short
KeyFileDialogHook(short item, DialogPtr dialog, SFInfo *sfInfo)
{
	pgpAssertAddrValid( dialog, DialogRecord );
	pgpAssertAddrValid( sfInfo, SFInfo );

	switch( item )
	{
		case kShowAllFilesCheckboxItem:
		{
			sfInfo->showAllFiles = ToggleDialogCheckbox( dialog,
										kShowAllFilesCheckboxItem );
			item = sfHookRebuildList;
			break;
		}
	}
	return item;
}

	OSStatus
CPGPPreferenceDialog::SelectKeyFile(
	FSSpec 						*fsSpec,
	enum PGPFileLocationTypes	type)
{
	SFInfo				sfInfo;
	SFTypeList			typeList;
	DlgHookYDUPP		cbHook;
	FileFilterYDUPP		filterUPP;
	static Point		where={0,0};
	OSStatus			status = noErr;
	short				dialogResID;
	
	pgpAssertAddrValid( fsSpec, FSSpec );
	
	pgpClearMemory( &sfInfo, sizeof( sfInfo ) );
	
	cbHook 		= NewDlgHookYDProc( KeyFileDialogHook );
	filterUPP	= NewFileFilterYDProc( KeyFileFilterHook );
	
	sfInfo.showAllFiles = FALSE;
	sfInfo.type	= type;
	switch(type)
	{
		case kPGPLTPublic:
			dialogResID = kGetFileShowAllPublicID;
			break;
		case kPGPLTPrivate:
			dialogResID = kGetFileShowAllPrivateID;
			break;
		case kPGPLTRandom:
			dialogResID = kGetFileShowAllRandomID;
			break;
	}
	UDesktop::Deactivate();
	
	::CustomGetFile(	filterUPP,
						-1,
						typeList,
						&sfInfo.sfReply,
						dialogResID,
						where,
						cbHook,
						NULL,
						NULL,
						NULL,
						&sfInfo);
						
	UDesktop::Activate();
	
	DisposeRoutineDescriptor( cbHook );
	DisposeRoutineDescriptor( filterUPP );
	
	if( sfInfo.sfReply.sfGood )
		*fsSpec = sfInfo.sfReply.sfFile;
	else
		status = userCanceledErr;

	return( status );
}

