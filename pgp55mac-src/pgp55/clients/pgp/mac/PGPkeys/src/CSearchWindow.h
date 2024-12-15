/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CSearchWindow.h,v 1.8 1997/09/20 23:50:27 wprice Exp $
____________________________________________________________________________*/
#pragma once

#include "pgpPubTypes.h"
#include "pgpKeyServerPrefs.h"

#include <LGACaption.h>
#include <LSimpleThread.h>

class CKeyView;
class CPopupList;
class CSearchPanel;
class CInfinityBar;

const ResIDT	kSearchWindowID		= 133;
const MessageT	kSearchWindowClosed	= 'SWcl';
const MessageT	kSearchInProgress	= 'SIPr';
const MessageT	kSearchComplete		= 'SCom';

typedef struct SearchPanelList
{
	CSearchPanel *panel;
	SearchPanelList *next;
} SearchPanelList;

typedef struct KSCallbackInfo
{
	CInfinityBar		*progressBar;
	LGACaption			*statusCaption;
	ulong				lastIncrement;
	PGPUInt32			lastState;
	PGPFilterRef		filter;
	PGPKeySetRef		resultKeySet;
	char				serverName[kMaxServerNameLength + 1];
	PGPError			result;
	Boolean				done;
	Boolean				abort;
} KSCallbackInfo;


class	CSearchWindow	:	public LWindow,
							public LListener,
							public LBroadcaster,
							public LPeriodical
{
public:
	enum { class_ID = 'Swin' };
								CSearchWindow();
								CSearchWindow(LStream *inStream);
								~CSearchWindow();
	void						FinishCreateSelf();
	virtual void				AttemptClose();
	void						DrawSelf();
	Boolean						HandleKeyPress(const EventRecord& inKeyEvent);
	virtual void				FindCommandStatus(CommandT inCommand,
									Boolean &outEnabled, Boolean &outUsesMark,
									Char16 &outMark, Str255 outName);
	void						ListenToMessage(MessageT	inMessage,
												void *		ioParam);
	CKeyView					*GetKeyView()
									{	return mKeyView;	}
	
	virtual void				ResizeFrameBy(
									Int16		inWidthDelta,
									Int16		inHeightDelta,
									Boolean		inRefresh);
	
	void						SpendTime(
									const EventRecord		&inMacEvent);
	
	void						SearchServer(void *arg);
private:
	virtual void				AdjustSizeToSearches();
	virtual void				AddEmptySearch(Boolean	first);
	virtual void				RemoveLastSearch();
	virtual void				FillServerPopup();
	void						Search();
	void						StartServerThread(
									PGPFilterRef filter, char *serverName);
	PGPError					MakeFilterFromPanels(
									PGPFilterRef *outFilter);
	
	CKeyView					*mKeyView;
	LGAPushButton				*mSearchButton,
								*mClearSearchButton,
								*mMoreButton,
								*mFewerButton;
	LGACaption					*mStatusCaption;
	CInfinityBar				*mProgressBar;
	CPopupList					*mServerPopup;
	LGAPrimaryBox				*mSearchesBox;
	LGASeparator				*mBottomBar;
	Int16						mNumSearches;
	Int16						mPanelHeight;
	SearchPanelList				*mSearchPanelList;
	PGPKeyServerEntry			*mServers;
	Int32						mNumServers;
	KSCallbackInfo				mSearchInfo;
	Boolean						mSearching;
	LSimpleThread				*mServerThread;
	
	enum	{	
				kSearchesBox	 		= 'gPrm',
				kMoreButton		 		= 'bMor',
				kFewerButton			= 'bFew',
				kSearchButton			= 'bSea',
				kClearSearchButton		= 'bCle',
				kStatusCaption			= 'cSta',
				kSearchProgressBar		= 'pBar',
				kServerPopup			= 'pKSS',
				kBottomBar				= 'bBot'
			};
};

