/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CSetWTitlePatcher.h,v 1.2.10.1 1997/12/05 22:14:58 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include "CPatcher.h"

class CSetWTitlePatcher	:	public	CPatcher {
public:
#if	GENERATINGCFM
	enum {kProcInfo =	kPascalStackBased
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(WindowRef)))
		| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(ConstStr255Param)))};
#else
	enum {kProcInfo = 0};
	typedef pascal void			(*SetWTitleProc)(WindowRef theWindow,
									ConstStr255Param title);
#endif
		
									CSetWTitlePatcher();
	virtual							~CSetWTitlePatcher();

protected:
	static CSetWTitlePatcher *	sSetWTitlePatcher;
	
	virtual void				NewSetWTitle(WindowRef theWindow,
									ConstStr255Param title);

	virtual void				OldSetWTitle(WindowRef theWindow,
									ConstStr255Param title);

	static pascal  void			SetWTitleCallback(WindowRef theWindow,
									ConstStr255Param title);
};
