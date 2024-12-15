/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CInsertMenuPatcher.h,v 1.2.10.1 1997/12/05 22:14:49 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include "CPatcher.h"

class CInsertMenuPatcher	:	public	CPatcher {
public:
#if GENERATINGCFM
	enum {kProcInfo =	kPascalStackBased
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(MenuRef)))
		| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short)))};
#else
	enum {kProcInfo = 0};
	typedef pascal void			(*InsertMenuProc)(MenuRef theMenu,
									short beforeID);
#endif
		
								CInsertMenuPatcher();
	virtual						~CInsertMenuPatcher();

protected:
	static CInsertMenuPatcher *	sInsertMenuPatcher;
	
	virtual void				NewInsertMenu(MenuRef theMenu,
										short beforeID);

	virtual void				OldInsertMenu(MenuRef theMenu,
										short beforeID);	

	static pascal void			InsertMenuCallback(MenuRef theMenu,
									short beforeID);
};
