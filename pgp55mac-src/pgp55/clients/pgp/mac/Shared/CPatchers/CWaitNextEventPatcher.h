/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CWaitNextEventPatcher.h,v 1.2.10.1 1997/12/05 22:15:02 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include "CPatcher.h"

class CWaitNextEventPatcher	:	public	CPatcher {
public:
#if	GENERATINGCFM
	enum {kProcInfo = kPascalStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(Boolean)))
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(EventMask)))
		| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(EventRecord *)))
		| STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(UInt32)))
		| STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(RgnHandle)))};
#else
	enum {kProcInfo = 0};
	typedef pascal Boolean 			(*WaitNextEventProc)(EventMask eventMask,
										EventRecord * theEvent,
										UInt32 sleep, RgnHandle mouseRgn);
#endif
		
									CWaitNextEventPatcher();
	virtual							~CWaitNextEventPatcher();

protected:
	static CWaitNextEventPatcher *	sWaitNextEventPatcher;
	
	virtual Boolean					NewWaitNextEvent(EventMask eventMask,
										EventRecord * theEvent,
										UInt32 sleep, RgnHandle mouseRgn);

	virtual Boolean					OldWaitNextEvent(EventMask eventMask,
										EventRecord * theEvent,
										UInt32 sleep, RgnHandle mouseRgn);	

	static pascal Boolean			WaitNextEventCallback(EventMask eventMask,
										EventRecord *theEvent, UInt32 sleep,
										RgnHandle mouseRgn);
};
