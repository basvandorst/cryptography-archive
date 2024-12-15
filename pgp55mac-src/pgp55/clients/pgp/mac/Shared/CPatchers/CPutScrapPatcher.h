/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CPutScrapPatcher.h,v 1.2.10.1 1997/12/05 22:14:55 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include "CPatcher.h"

class CPutScrapPatcher	:	public	CPatcher {
public:
#if GENERATINGCFM
	enum {kProcInfo = 	kPascalStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(long)))
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(long)))
		| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(ResType)))
		| STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(void *)))};
#else
	enum {kProcInfo = 0};
	typedef pascal long			(*PutScrapProc)(long length, ResType theType,
									void * source);
#endif
		
								CPutScrapPatcher();
	virtual						~CPutScrapPatcher();

protected:
	static CPutScrapPatcher *	sPutScrapPatcher;
	
	virtual long				NewPutScrap(long length, ResType theType,
									void * source);

	virtual long				OldPutScrap(long length, ResType theType,
									void * source);

	static pascal long			PutScrapCallback(long length, ResType theType,
									void * source);
};
