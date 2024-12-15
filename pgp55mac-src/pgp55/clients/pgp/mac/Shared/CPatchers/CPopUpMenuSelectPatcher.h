/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CPopUpMenuSelectPatcher.h,v 1.2.10.1 1997/12/05 22:14:53 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include "CPatcher.h"

class CPopUpMenuSelectPatcher	:	public	CPatcher {
public:
#if GENERATINGCFM
	enum {kProcInfo = 	kPascalStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(long)))
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(MenuRef)))
		| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short)))
		| STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(short)))
		| STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(short)))};
#else
	enum {kProcInfo = 0};
	typedef pascal long			(*PopUpMenuSelectProc)(MenuRef menu,
									short top, short left, short popUpItem);
#endif
		
								CPopUpMenuSelectPatcher();
	virtual						~CPopUpMenuSelectPatcher();

protected:
	static CPopUpMenuSelectPatcher *	sPopUpMenuSelectPatcher;
	
	virtual long				NewPopUpMenuSelect(MenuRef menu, short top,
									short left, short popUpItem);

	virtual long				OldPopUpMenuSelect(MenuRef menu, short top,
									short left, short popUpItem);

	static pascal long			PopUpMenuSelectCallback(MenuRef menu,
									short top, short left, short popUpItem);
};
