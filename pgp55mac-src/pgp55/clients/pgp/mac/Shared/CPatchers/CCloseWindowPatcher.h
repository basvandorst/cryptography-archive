/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CCloseWindowPatcher.h,v 1.2.10.1 1997/12/05 22:14:44 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include "CPatcher.h"

class CCloseWindowPatcher	:	public	CPatcher {
public:
#if GENERATINGCFM
	enum {kProcInfo =	kPascalStackBased
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(WindowRef)))};
#else
	enum {kProcInfo = 0};
	typedef pascal void				(*CloseWindowProc)(WindowRef theWindow);
#endif
		
									CCloseWindowPatcher();
	virtual							~CCloseWindowPatcher();

protected:
	static CCloseWindowPatcher *	sCloseWindowPatcher;
	
	virtual void					NewCloseWindow(WindowRef theWindow);

	virtual void					OldCloseWindow(WindowRef theWindow);

	static pascal  void				CloseWindowCallback(WindowRef theWindow);
};
