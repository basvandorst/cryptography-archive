/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CSearchWindow.cp,v 1.24 1997/09/30 07:36:19 wprice Exp $
____________________________________________________________________________*/
#include "CSearchWindow.h"

#include "CSearchPanel.h"
#include "CKeyView.h"
#include "CPGPKeys.h"
#include "CPopupList.h"
#include "CInfinityBar.h"
#include "CWarningAlert.h"

#include "pgpMem.h"
#include "MacStrings.h"
#include "pgpClientPrefs.h"
#include "pgpKeyServer.h"
#include "pgpErrors.h"

const Int16		kKeyViewTopSlop			= 4;
const Int16		kKeyViewSideSlop		= 7;
const Int16		kSearchBoxBorderSlop	= 4;
const ResIDT	kSearchPanelID			= 134;
const Int16		kMaxSearches			= 7;

enum	{
			kSWStringListID 	= 1013,
			kDefaultKeyserverPathStringID		= 1,
			kLocalKeyringStringID,
			kCurrentResultsStringID,
			kCancelSearchStringID,
			kSearchStringID,
			kKeysFoundStringID,
			kConnectingStringID,
			kWaitingStringID,
			kReceivingStringID,
			kSendingStringID,
			kDisconnectingStringID,
			kUnsupportedHKPFilterErrorStringID,
			kVerifyingSignaturesStringID
		};

static PGPError KSSearchCallback(	PGPContextRef	context,
									PGPEvent		*event,
									PGPUserValue	userValue);
static PGPError SigCheckCallback(	PGPContextRef		context,
									PGPEvent			*event,
									PGPUserValue		userValue);
static void SearchServerThreadProc(LThread& thread, void *arg);

CSearchWindow::CSearchWindow()
{
}

CSearchWindow::CSearchWindow(LStream *inStream)
	:	LWindow(inStream)
{
}

CSearchWindow::~CSearchWindow()
{
	SearchPanelList		*panelWalk,
						*panelNext;
		
	if(IsntNull(mServers))
		PGPDisposePrefData(gPrefRef, mServers);

	panelWalk = mSearchPanelList;
	while(IsntNull(panelWalk))
	{
		panelNext = panelWalk->next;
		pgpFree(panelWalk);
		panelWalk = panelNext;
	}
}

	void
CSearchWindow::FinishCreateSelf()
{
	LWindow::FinishCreateSelf();
	mServers = NULL;
	mNumSearches = 0;
	mSearchInfo.done = TRUE;
	mSearching = FALSE;
	mSearchPanelList = NULL;
	mPanelHeight = 0;
	mSearchButton = (LGAPushButton *)FindPaneByID(kSearchButton);
	mClearSearchButton = (LGAPushButton *)FindPaneByID(kClearSearchButton);
	mMoreButton = (LGAPushButton *)FindPaneByID(kMoreButton);
	mFewerButton = (LGAPushButton *)FindPaneByID(kFewerButton);
	mStatusCaption = (LGACaption *)FindPaneByID(kStatusCaption);
	mProgressBar = (CInfinityBar *)FindPaneByID(kSearchProgressBar);
	mServerPopup = (CPopupList *)FindPaneByID(kServerPopup);
	mSearchesBox = (LGAPrimaryBox *)FindPaneByID(kSearchesBox);
	mBottomBar = (LGASeparator *)FindPaneByID(kBottomBar);

	FillServerPopup();
	
	mSearchButton->SetDefaultButton(true, false);
	mSearchButton->AddListener(this);
	mClearSearchButton->AddListener(this);
	mMoreButton->AddListener(this);
	mFewerButton->AddListener(this);
	mFewerButton->Disable();
	
	LCommander::SetDefaultCommander( this );
	LPane::SetDefaultView( this );

	mKeyView = (CKeyView *)
		UReanimator::ReadObjects( 'PPob', kKeyViewResID );
	pgpAssertAddrValid(mKeyView, CKeyView *);
	mKeyView->FinishCreate();
	
	AddEmptySearch(TRUE);
	Show();
}

	void
CSearchWindow::AttemptClose()
{
	if(!mSearching)
	{
		BroadcastMessage(kSearchWindowClosed, NULL);
		LWindow::AttemptClose();
	}
	else
		::SysBeep(1);
}

	void
CSearchWindow::AddEmptySearch(Boolean	first)
{
	SearchPanelList *panelList,
					*panelWalk;
	Rect			frame;
	
	if(IsNull(mSearchPanelList))
		panelList = mSearchPanelList =
			(SearchPanelList *)pgpAlloc(sizeof(SearchPanelList));
	else
	{
		panelList = (SearchPanelList *)pgpAlloc(sizeof(SearchPanelList));
		for(panelWalk = mSearchPanelList; IsntNull(panelWalk->next) ;
			panelWalk = panelWalk->next)
			;
		panelWalk->next = panelList;
	}
	panelList->next = nil;
	mNumSearches++;
	LCommander::SetDefaultCommander( this );
	LPane::SetDefaultView( mSearchesBox );
	panelList->panel = (CSearchPanel *)
		UReanimator::ReadObjects( 'PPob', kSearchPanelID );
	pgpAssertAddrValid(panelList->panel, LView *);
	panelList->panel->FinishCreate();
	panelList->panel->AddListener(this);
	panelList->panel->AdjustCategory(first);
	panelList->panel->CalcLocalFrameRect(frame);
	mPanelHeight = frame.bottom - frame.top;
	mSearchesBox->ResizeFrameBy(0, mPanelHeight, true);
	panelList->panel->PlaceInSuperFrameAt(frame.left,
		mPanelHeight * (mNumSearches - 1), true);
	AdjustSizeToSearches();
	ResizeWindowBy(0, mPanelHeight);
	panelList->panel->Show();
	GetMinMaxSize(frame);
	frame.top += mPanelHeight;
	SetMinMaxSize(frame);
}

	void
CSearchWindow::RemoveLastSearch()
{
	SearchPanelList *panelLast,
					*panelWalk;
	Rect			frame;
					
	if(IsNull(mSearchPanelList))
		return;
	for(panelWalk = panelLast = mSearchPanelList;
		IsntNull(panelWalk->next) ;)
	{
		panelLast = panelWalk;
		panelWalk = panelWalk->next;
	}
	delete panelWalk->panel;
	if(panelWalk == mSearchPanelList)
		mSearchPanelList = NULL;
	panelLast->next = NULL;
	pgpFree(panelWalk);
	mSearchesBox->ResizeFrameBy(0, -mPanelHeight, true);
	AdjustSizeToSearches();
	mNumSearches--;
	ResizeWindowBy(0, -mPanelHeight);
	GetMinMaxSize(frame);
	frame.top -= mPanelHeight;
	SetMinMaxSize(frame);
}

	void
CSearchWindow::FillServerPopup()
{
	Str255		popupString;
	PGPSize		len;
	PGPError	err;
	Str255		itemString;
	
	mServerPopup->ClearItems();
	GetIndString(	popupString, kSWStringListID,
					kDefaultKeyserverPathStringID);
	mServerPopup->AddItem(popupString);
	GetIndString(	popupString, kSWStringListID,
					kLocalKeyringStringID);
	mServerPopup->AddItem(popupString);
	GetIndString(	popupString, kSWStringListID,
					kCurrentResultsStringID);
	mServerPopup->AddItem(popupString);
	mServerPopup->AddItem("\p-");
	
	mNumServers = 0;
	err = PGPGetPrefData(	gPrefRef, 
							kPGPPrefKeyServerList, 
							&len, 
							&mServers);
	pgpAssertNoErr(err);
	if(IsntPGPError(err))
	{
		mNumServers = len / sizeof(PGPKeyServerEntry);
		for(Int16 serverIndex = 0; serverIndex < mNumServers; serverIndex++)
		{
			if(IsKeyServerListed(mServers[serverIndex].flags))
			{
				CToPString(mServers[serverIndex].serverURL, itemString);
				mServerPopup->AddItem(itemString);
			}
		}
	}
}

	void
CSearchWindow::AdjustSizeToSearches()
{
	Rect	sbFrame,
			mbFrame,
			fbFrame,
			bbFrame,
			kvFrame;
	
	mSearchesBox->CalcPortFrameRect(sbFrame);
	mMoreButton->CalcPortFrameRect(mbFrame);
	mFewerButton->CalcPortFrameRect(fbFrame);
	mMoreButton->PlaceInSuperFrameAt(mbFrame.left,
									sbFrame.bottom + kSearchBoxBorderSlop,
									true);
	mFewerButton->PlaceInSuperFrameAt(fbFrame.left,
									sbFrame.bottom + kSearchBoxBorderSlop,
									true);
	mMoreButton->CalcPortFrameRect(mbFrame);
	mKeyView->PlaceInSuperFrameAt(kKeyViewSideSlop,
									mbFrame.bottom + kKeyViewTopSlop,
									true);
	mBottomBar->CalcPortFrameRect(bbFrame);
	mKeyView->CalcPortFrameRect(kvFrame);
	mKeyView->ResizeFrameTo(kvFrame.right - kvFrame.left,
							bbFrame.top - kKeyViewSideSlop - kvFrame.top,
							true);
}

	void
CSearchWindow::DrawSelf()
{
	Rect	frame;
	
	FocusDraw();
	CalcLocalFrameRect(frame);
	LWindow::DrawSelf();
	::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Gray7));
	::MoveTo(frame.right - 1, frame.top + 1);
	::LineTo(frame.right - 1, frame.bottom - 16);
	::LineTo(frame.right - 16, frame.bottom - 16);
	::LineTo(frame.right - 16, frame.bottom - 1);
	::LineTo(frame.left + 1, frame.bottom - 1);
	::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_White));
	::MoveTo(frame.left, frame.bottom);
	::LineTo(frame.left, frame.top);
	::LineTo(frame.right, frame.top);
}

	void
CSearchWindow::ResizeFrameBy(
	Int16		inWidthDelta,
	Int16		inHeightDelta,
	Boolean		inRefresh)
{
	Rect	frame;

	LWindow::ResizeFrameBy(inWidthDelta, inHeightDelta, inRefresh);
	
	// following corrects a drawing problem with resizing that
	// was caused by our 3D border effect
	CalcPortFrameRect(frame);
	InvalPortRect(&frame);
}

	PGPError
CSearchWindow::MakeFilterFromPanels(PGPFilterRef *outFilter)
{
	PGPError		err;
	PGPFilterRef	filter,
					filter2;
	SearchPanelList	*walk;
	
	*outFilter = filter = kPGPInvalidRef;
	err = kPGPError_UnknownError;
	walk = mSearchPanelList;
	while(IsntNull(walk))
	{
		walk->panel->MakeFilter(&filter2);
		if(!PGPRefIsValid(filter2))
		{
			if(PGPRefIsValid(filter))
				PGPFreeFilter(filter);
			err = kPGPError_UnknownError;
			goto fatalErr;
		}
		if(PGPRefIsValid(filter))
		{
			if(PGPRefIsValid(filter2))
			{
				err = PGPIntersectFilters(filter, filter2, &filter);
				pgpAssertNoErr(err);
				if(IsPGPError(err))
					goto fatalErr;
			}
		}
		else
			filter = filter2;
		walk = walk->next;
	}
	*outFilter = filter;
	err = kPGPError_NoErr;
fatalErr:
	return err;
}
	
	void
CSearchWindow::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	Char16		&outMark,
	Str255		outName)
{
	switch(inCommand) 
	{
		case cmd_Close:
			if(mSearching)
			{
				outEnabled = false;
				break;
			}
		default:
			 LWindow::FindCommandStatus(inCommand, outEnabled,
										outUsesMark, outMark, outName);
			break;
	}
}

	void
CSearchWindow::ListenToMessage(MessageT inMessage, void *ioParam)
{
	switch(inMessage)
	{
		case kMoreButton:
			if(mNumSearches < kMaxSearches)
			{
				AddEmptySearch(false);
				mFewerButton->Enable();
				if(mNumSearches == kMaxSearches)
					mMoreButton->Disable();
			}
			break;
		case kFewerButton:
			if(mNumSearches > 1)
			{
				RemoveLastSearch();
				if(mNumSearches == kMaxSearches - 1)
					mMoreButton->Enable();
				if(mNumSearches == 1)
					mFewerButton->Disable();
			}
			break;
		case kSearchButton:
			if(mSearchInfo.done)
				Search();
			else
				mSearchInfo.abort = TRUE;
			break;
		case kClearSearchButton:
			while(mNumSearches > 0)
				RemoveLastSearch();
			AddEmptySearch(TRUE);
			mStatusCaption->SetDescriptor("\p");
			mFewerButton->Disable();
			mMoreButton->Enable();
			break;
	}
}

	PGPError
KSSearchCallback(
	PGPContextRef	context,
	PGPEvent		*event,
	PGPUserValue	userValue)
{
	if(event->type == kPGPEvent_KeyServerEvent)
	{
		KSCallbackInfo *callbackInfo = (KSCallbackInfo *)userValue;
	
		if(LMGetTicks() - callbackInfo->lastIncrement > 5)
		{
			callbackInfo->progressBar->IncrementState();
			callbackInfo->lastIncrement = LMGetTicks();
			if(callbackInfo->lastState != event->data.keyServerData.state)
			{
				PGPInt16	stringID;
				Str255		caption,
							moreCaption;
				
				moreCaption[0] = 0;
				CToPString(callbackInfo->serverName, moreCaption);
				switch(event->data.keyServerData.state)
				{
					case kPGPKeyServerStateConnect:
						stringID = kConnectingStringID;
						break;
					case kPGPKeyServerStateWait:
						stringID = kWaitingStringID;
						break;
					case kPGPKeyServerStateReceive:
						stringID = kReceivingStringID;
						break;
					case kPGPKeyServerStateSend:
						stringID = kSendingStringID;
						break;
					case kPGPKeyServerStateDisconnect:
						stringID = kDisconnectingStringID;
						break;
					default:
						pgpAssert(0);
				}
				::GetIndString(caption, kSWStringListID, stringID);
				AppendPString(moreCaption, caption);
				callbackInfo->statusCaption->SetDescriptor(caption);
				callbackInfo->lastState = event->data.keyServerData.state;
			}
		}
		LThread::Yield();
		if(callbackInfo->abort)
			return kPGPError_UserAbort;
	}
	return kPGPError_NoErr;
}

	void
CSearchWindow::SpendTime(const EventRecord	&inMacEvent)
{
	if(mSearchInfo.done)
	{
		PGPUInt32	numKeys = 0;
		Str255		statusStr,
					str;
		PGPError	err;
		
		StopRepeating();
		if(IsntPGPError(mSearchInfo.result))
		{
			if(PGPRefIsValid(mSearchInfo.resultKeySet))
			{
				err = PGPCountKeys(mSearchInfo.resultKeySet, &numKeys);
				pgpAssertNoErr(err);
				mKeyView->CloseKeys();
				mKeyView->SetKeyDBInfo(mSearchInfo.resultKeySet,
										false, false);
			}
		}
		else if(mSearchInfo.result != kPGPError_UserAbort)
		{
			if(mSearchInfo.result == kPGPError_UnsupportedHKPFilter)
			{
				CWarningAlert::Display(kWACautionAlertType, kWAOKStyle,
								kSWStringListID,
								kUnsupportedHKPFilterErrorStringID);
			}
			else
				ReportPGPError(mSearchInfo.result);
		}
		::NumToString(numKeys, statusStr);
		::GetIndString(str, kSWStringListID,
						kKeysFoundStringID);
		AppendPString(str, statusStr);
		mStatusCaption->SetDescriptor(statusStr);
		if(PGPRefIsValid(mSearchInfo.filter))
		{
			err = PGPFreeFilter(mSearchInfo.filter);
			pgpAssertNoErr(err);
		}
		mProgressBar->Hide();
		::GetIndString(str, kSWStringListID, kSearchStringID);
		mSearchButton->SetDescriptor(str);
		BroadcastMessage(kSearchComplete, NULL);
		mSearching = FALSE;
	}
	LThread::Yield();
}

	PGPError
SigCheckCallback(
	PGPContextRef		context,
	PGPEvent			*event,
	PGPUserValue		userValue)
{
	KSCallbackInfo *callbackInfo = (KSCallbackInfo *)userValue;
	
	if(LMGetTicks() - callbackInfo->lastIncrement > 5)
	{
		callbackInfo->progressBar->IncrementState();
		callbackInfo->lastIncrement = LMGetTicks();
		LThread::Yield();
	}
	return kPGPError_NoErr;
}

	void
CSearchWindow::SearchServer(void *arg)
{
	PGPKeyServerRef			server;
	PGPError				err;

	mSearchInfo.resultKeySet = kPGPInvalidRef;
	err = PGPNewKeyServerFromURL(
				gPGPContext, mSearchInfo.serverName, 
				kPGPKSAccess_Normal, kPGPKSKeySpaceNormal,
				&server);
	pgpAssertNoErr(err);
	if(IsntPGPError(err))
	{
		err = PGPQueryKeyServer(server, mSearchInfo.filter,
								KSSearchCallback, &mSearchInfo,
								&mSearchInfo.resultKeySet,
								NULL);
		mSearchInfo.result = err;
		if(IsntPGPError(err) && PGPRefIsValid(mSearchInfo.resultKeySet))
		{
			PGPKeySetRef	combinedSet;
			Str255			caption;
			
			::GetIndString(caption, kSWStringListID,
							kVerifyingSignaturesStringID);
			mStatusCaption->SetDescriptor(caption);
			err = PGPNewKeySet(gPGPContext, &combinedSet);
			pgpAssertNoErr(err);
			err = PGPAddKeys(mSearchInfo.resultKeySet, combinedSet);
			pgpAssertNoErr(err);
			err = PGPAddKeys(CPGPKeys::TheApp()->GetKeySet(), combinedSet);
			pgpAssertNoErr(err);
			err = PGPCheckKeyRingSigs(	mSearchInfo.resultKeySet,
										combinedSet,
										FALSE, SigCheckCallback,
										&mSearchInfo);
			pgpAssertNoErr(err);
			err = PGPPropagateTrust(combinedSet);
			pgpAssertNoErr(err);
			PGPFreeKeySet(combinedSet);
		}
		err = PGPFreeKeyServer(server);
		pgpAssertNoErr(err);
	}
	else
		mSearchInfo.result = err;
	mSearchInfo.done = TRUE;
}

	void
SearchServerThreadProc(LThread& thread, void *arg)
{
	CSearchWindow *searchWindow = (CSearchWindow *)arg;
	
	searchWindow->SearchServer(NULL);
}

	void
CSearchWindow::StartServerThread(PGPFilterRef filter, char *serverName)
{
	mSearching = TRUE;
	BroadcastMessage(kSearchInProgress, NULL);
	mSearchInfo.progressBar = mProgressBar;
	mSearchInfo.statusCaption = mStatusCaption;
	mSearchInfo.lastIncrement = 0;
	mSearchInfo.lastState = 0;
	mSearchInfo.filter = filter;
	mSearchInfo.resultKeySet = kPGPInvalidRef;
	CopyCString(serverName, mSearchInfo.serverName);
	mSearchInfo.result = kPGPError_NoErr;
	mSearchInfo.abort = mSearchInfo.done = FALSE;
	mServerThread = new LSimpleThread(SearchServerThreadProc, this);
	mServerThread->Resume();
	StartRepeating();
}

	void
CSearchWindow::Search()
{
	PGPFilterRef			filter;
	PGPKeySetRef			resultKeys = kPGPInvalidRef,
							indKeySet;
	Int16					targetValue;
	PGPKeyServerEntry		*ksEntries;
	PGPUInt32				numKSEntries;
	char					serverName[kMaxServerNameLength];
	Str255					str;
	PGPError				err;
	
	mStatusCaption->SetDescriptor("\p");
	::GetIndString(str, kSWStringListID, kCancelSearchStringID);
	mSearchButton->SetDescriptor(str);
	mSearchButton->Draw(NULL);
	mProgressBar->Show();
	err = MakeFilterFromPanels(&filter);
	if(IsntPGPError(err) && PGPRefIsValid(filter))
	{
		targetValue = mServerPopup->GetValue();
		switch(targetValue)
		{
			case 0:		// Default KS Path
				serverName[0] = '\0';
				err = PGPCreateKeyServerPath(gPrefRef, "",
											&ksEntries, &numKSEntries);
				pgpAssertNoErr(err);
				if(IsntPGPError(err))
				{
					if(IsntNull(ksEntries) && numKSEntries > 0)
						CopyCString(ksEntries[0].serverURL, serverName);
					err = PGPDisposeKeyServerPath(ksEntries);
					pgpAssertNoErr(err);
				}
				if(IsntPGPError(err) && serverName[0] != '\0')
					StartServerThread(filter, serverName);
				break;
			case 1:		// Local Keyring
				{
					err = PGPFilterKeySet(CPGPKeys::TheApp()->GetKeySet(),
											filter, &indKeySet);
					pgpAssertNoErr(err);
					if(IsntPGPError(err) && PGPRefIsValid(indKeySet))
					{
						err = PGPNewKeySet(gPGPContext, &resultKeys);
						pgpAssertNoErr(err);
						if(IsntPGPError(err) && PGPRefIsValid(resultKeys))
						{
							err = PGPAddKeys(indKeySet, resultKeys);
							pgpAssertNoErr(err);
							err = PGPCommitKeyRingChanges(resultKeys);
							pgpAssertNoErr(err);
						}
						err = PGPFreeKeySet(indKeySet);
						pgpAssertNoErr(err);
					}
					mSearchInfo.filter = filter;
					mSearchInfo.resultKeySet = resultKeys;
					mSearchInfo.result = err;
					mSearchInfo.done = TRUE;
					StartRepeating();
					mSearching = TRUE;
					BroadcastMessage(kSearchInProgress, NULL);
				}
				break;
			case 2:		// Current Results
				{
					if(PGPRefIsValid(mKeyView->GetKeySet()))
					{
						err = PGPFilterKeySet(
								mKeyView->GetKeySet(),
								filter, &indKeySet);
						pgpAssertNoErr(err);
						if(IsntPGPError(err) && PGPRefIsValid(indKeySet))
						{
							err = PGPNewKeySet(gPGPContext, &resultKeys);
							pgpAssertNoErr(err);
							if(IsntPGPError(err) &&
								PGPRefIsValid(resultKeys))
							{
								err = PGPAddKeys(indKeySet, resultKeys);
								pgpAssertNoErr(err);
							}
							err = PGPFreeKeySet(indKeySet);
							pgpAssertNoErr(err);
						}
					}
					mSearchInfo.filter = filter;
					mSearchInfo.resultKeySet = resultKeys;
					mSearchInfo.result = err;
					mSearchInfo.done = TRUE;
					StartRepeating();
					mSearching = TRUE;
					BroadcastMessage(kSearchInProgress, NULL);
				}
				break;
			default:
				targetValue -= 4;
				CopyCString(mServers[targetValue].serverURL, serverName);
				if(IsntPGPError(err) && serverName[0] != '\0')
					StartServerThread(filter, serverName);
				break;
		}
	}
	else
	{
		StartRepeating();
		mSearchInfo.result = kPGPError_UserAbort;
		mSearchInfo.done = TRUE;
		mSearchInfo.filter = kPGPInvalidRef;
	}
	mKeyView->Empty();
}

	Boolean
CSearchWindow::HandleKeyPress(const EventRecord& inKeyEvent)
{
	Boolean		keyHandled = false;

	if(inKeyEvent.modifiers & cmdKey)
		keyHandled = LCommander::HandleKeyPress(inKeyEvent);
	else if(UKeyFilters::IsActionKey(inKeyEvent.message))
	{
		switch (inKeyEvent.message & charCodeMask)
		{
			case char_Return:
			case char_Enter: 
				mSearchButton->SimulateHotSpotClick(kControlButtonPart);
				keyHandled = true;
				break;
		}
	}

	return keyHandled;
}

