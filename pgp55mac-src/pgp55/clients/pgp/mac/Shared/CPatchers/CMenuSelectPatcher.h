/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CMenuSelectPatcher.h,v 1.2.10.1 1997/12/05 22:14:51 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include "CPatcher.h"

class CMenuSelectPatcher	:	public	CPatcher {
public:
#if GENERATINGCFM
	enum {kProcInfo = 	kPascalStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(long)))
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Point)))};
#else
	enum {kProcInfo = 0};
	typedef pascal long			(*MenuSelectProc)(Point startPt);
#endif
		
								CMenuSelectPatcher();
	virtual						~CMenuSelectPatcher();

protected:
	static CMenuSelectPatcher *	sMenuSelectPatcher;
	
	virtual long				NewMenuSelect(Point startPt);

	virtual long				OldMenuSelect(Point startPt);

	static pascal long			MenuSelectCallback(Point startPt);
};
