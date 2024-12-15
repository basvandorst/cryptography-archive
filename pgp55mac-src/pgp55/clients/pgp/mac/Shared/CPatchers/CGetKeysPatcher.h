/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CGetKeysPatcher.h,v 1.2.10.1 1997/12/05 22:14:48 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include "CPatcher.h"

class CGetKeysPatcher	:	public	CPatcher {
public:
#if GENERATINGCFM
	enum {kProcInfo =	kPascalStackBased
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(UInt32 *)))};
#else
	enum {kProcInfo = 0};
	typedef pascal void			(*GetKeysProc)(KeyMap theKeys);
#endif
		
								CGetKeysPatcher();
	virtual						~CGetKeysPatcher();

protected:
	static CGetKeysPatcher *	sGetKeysPatcher;
	
	virtual void				NewGetKeys(KeyMap theKeys);

	virtual void				OldGetKeys(KeyMap theKeys);

	static pascal void			GetKeysCallback(KeyMap theKeys);
};
