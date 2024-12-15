/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CSetMenuBarPatcher.h,v 1.2.10.1 1997/12/05 22:14:56 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include "CPatcher.h"

class CSetMenuBarPatcher	:	public	CPatcher {
public:
#if GENERATINGCFM
	enum {kProcInfo = 	kPascalStackBased
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle)))};
#else
	enum {kProcInfo = 0};
	typedef pascal void			(*SetMenuBarProc)(Handle menuList);
#endif
		
								CSetMenuBarPatcher();
	virtual						~CSetMenuBarPatcher();

protected:
	static CSetMenuBarPatcher *	sSetMenuBarPatcher;
	
	virtual void				NewSetMenuBar(Handle menuList);

	virtual void				OldSetMenuBar(Handle menuList);

	static pascal void			SetMenuBarCallback(Handle menuList);
};
