/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CWaitNextEventDispatcher.h,v 1.2.10.1 1997/12/05 22:15:00 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include "CWaitNextEventPatcher.h"

typedef struct EventParamsS {
	Boolean			result;
	EventMask		eventMask;
	EventRecord *	theEvent;
	UInt32			sleep;
	RgnHandle		mouseRgn;
} SEventParams, * SEventParamsPtr;

class CWaitNextEventDispatcher	:	public	CWaitNextEventPatcher {
public:
							CWaitNextEventDispatcher();
	virtual					~CWaitNextEventDispatcher();
	
	static SEventParamsPtr	GetEventParamsPtr() { return &sEventParams; }
		
protected:
	static SEventParams		sEventParams;
	
	virtual Boolean			NewWaitNextEvent(EventMask eventMask,
								EventRecord * theEvent,
								UInt32 sleep, RgnHandle mouseRgn);
	
	virtual void			InitEventParms(EventMask eventMask,
								EventRecord * theEvent, UInt32 sleep,
								RgnHandle mouseRgn);
	virtual void			DispatchEvent();

	virtual void			PreWaitNextEvent() { }
	
	virtual void			HandleEveryEvent() { }
	virtual void			HandleIdleEvents() { }
	
	virtual void			HandleNullEvent();
	virtual void			HandleMouseDown() { }
	virtual void			HandleMouseUp() { }
	virtual void			HandleKeyDown() { }
	virtual void			HandleKeyUp() { }
	virtual void			HandleAutoKeyEvent() { }
	virtual void			HandleUpdateEvent() { }
	virtual void			HandleDiskEvent() { }
	virtual void			HandleActivateEvent() { }
	virtual void			HandleOSEvent() { }
	virtual void			HandleHighLevelEvent() { }
};
