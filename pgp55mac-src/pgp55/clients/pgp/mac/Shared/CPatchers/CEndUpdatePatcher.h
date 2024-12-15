/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CEndUpdatePatcher.h,v 1.2.10.1 1997/12/05 22:14:47 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include "CPatcher.h"

class CEndUpdatePatcher	:	public	CPatcher {
public:
#if GENERATINGCFM
	enum {kProcInfo =	kPascalStackBased
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(WindowRef)))};
#else
	enum {kProcInfo = 0};
	typedef pascal void			(*EndUpdateProc)(WindowRef theWindow);
#endif
		
									CEndUpdatePatcher();
	virtual							~CEndUpdatePatcher();

protected:
	static CEndUpdatePatcher *	sEndUpdatePatcher;
	
	virtual void				NewEndUpdate(WindowRef theWindow);

	virtual void				OldEndUpdate(WindowRef theWindow);

	static pascal  void			EndUpdateCallback(WindowRef theWindow);
};
