/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CDrawMenuBarPatcher.h,v 1.2.10.1 1997/12/05 22:14:45 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include "CPatcher.h"

class CDrawMenuBarPatcher	:	public	CPatcher {
public:
#if GENERATINGCFM
	enum {kProcInfo =	kPascalStackBased};
#else
	enum {kProcInfo = 0};
	typedef pascal void				(*DrawMenuBarProc)();
#endif
		
									CDrawMenuBarPatcher();
	virtual							~CDrawMenuBarPatcher();

protected:
	static CDrawMenuBarPatcher *	sDrawMenuBarPatcher;
	
	virtual void					NewDrawMenuBar();

	virtual void					OldDrawMenuBar();	

	static pascal void				DrawMenuBarCallback();
};
