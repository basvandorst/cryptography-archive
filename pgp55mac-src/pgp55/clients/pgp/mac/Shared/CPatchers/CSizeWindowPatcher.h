/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CSizeWindowPatcher.h,v 1.2.10.1 1997/12/05 22:14:59 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include "CPatcher.h"

class CSizeWindowPatcher	:	public	CPatcher {
public:
#if	GENERATINGCFM
	enum {kProcInfo =	kPascalStackBased
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(WindowRef)))
		| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short)))
		| STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(short)))
		| STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(Boolean)))};
#else
	enum {kProcInfo = 0};
	typedef pascal void				(*SizeWindowProc)(WindowRef theWindow,
										short w, short h,
										Boolean fUpdate);
#endif
		
									CSizeWindowPatcher();
	virtual							~CSizeWindowPatcher();

protected:
	static CSizeWindowPatcher *		sSizeWindowPatcher;
	
	virtual void					NewSizeWindow(WindowRef theWindow,
										short w, short h,
										Boolean fUpdate);

	virtual void					OldSizeWindow(WindowRef theWindow, short w,
										short h,
										Boolean fUpdate);

	static pascal  void				SizeWindowCallback(WindowRef theWindow,
										short w, short h,
										Boolean fUpdate);
};
