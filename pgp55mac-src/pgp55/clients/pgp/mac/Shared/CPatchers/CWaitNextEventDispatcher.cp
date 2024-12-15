/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CWaitNextEventDispatcher.cp,v 1.2.10.1 1997/12/05 22:15:00 mhw Exp $
____________________________________________________________________________*/

#include "CWaitNextEventDispatcher.h"


// Init statics
SEventParams	CWaitNextEventDispatcher::sEventParams;


CWaitNextEventDispatcher::CWaitNextEventDispatcher()
{
}



CWaitNextEventDispatcher::~CWaitNextEventDispatcher()
{
}
	


	Boolean
CWaitNextEventDispatcher::NewWaitNextEvent(
	EventMask		eventMask,
	EventRecord *	theEvent,
	UInt32			sleep,
	RgnHandle		mouseRgn)
{	
	try {
		InitEventParms(eventMask, theEvent, sleep, mouseRgn);
	}
	
	catch (...) {
	}
	
	// Get the event
	sEventParams.result = OldWaitNextEvent(	sEventParams.eventMask,
											sEventParams.theEvent,
											sEventParams.sleep,
											sEventParams.mouseRgn);
	try {
		DispatchEvent();
	}
				
	catch (...) {
	}
	
	return sEventParams.result;
}



	void
CWaitNextEventDispatcher::HandleNullEvent()
{
	HandleIdleEvents();
}



	void
CWaitNextEventDispatcher::DispatchEvent()
{
	// Call repeating function
	HandleEveryEvent();

	// Dispatch
	switch (sEventParams.theEvent->what) {
		case nullEvent:
		{
			HandleNullEvent();
		}
		break;
		
		case mouseDown:
		{
			HandleMouseDown();
		}
		break;
		
		case mouseUp:
		{
			HandleMouseUp();
		}
		break;
		
		case keyDown:
		{
			HandleKeyDown();
		}
		break;
		
		case keyUp:
		{
			HandleKeyUp();
		}
		break;

		case autoKey:
		{
			HandleAutoKeyEvent();
		}
		break;

		case updateEvt:
		{
			HandleUpdateEvent();
		}
		break;
		
		case diskEvt:
		{
			HandleDiskEvent();
		}
		break;
		
		case activateEvt:
		{
			HandleActivateEvent();
		}
		break;
		
		case osEvt:
		{
			HandleOSEvent();
		}
		break;
		
		case kHighLevelEvent:
		{
			HandleHighLevelEvent();
		}
		break;
	}
}



	void
CWaitNextEventDispatcher::InitEventParms(
	EventMask		eventMask,
	EventRecord *	theEvent,
	UInt32			sleep,
	RgnHandle		mouseRgn)
{
	// Set up eventparams
	sEventParams.eventMask = eventMask;
	sEventParams.theEvent = theEvent;
	sEventParams.sleep = sleep;
	sEventParams.mouseRgn = mouseRgn;
	
	// Call PreFunction to allow filtering
	PreWaitNextEvent();
}
