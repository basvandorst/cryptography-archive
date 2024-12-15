/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: pflContextPriv.h,v 1.4 1997/09/03 22:01:04 lloyd Exp $
____________________________________________________________________________*/
#ifndef Included_pflContextPriv_h	/* [ */
#define Included_pflContextPriv_h


#include "pflContext.h"


#define kPFLContextMagic	0x436e7478	/* 'Cntx' */

struct PFLContext
{
	PGPUInt32		magic;				/* Always kPFLContextMagic */
	PGPUserValue	userValue;			/* Client storage */
	
	PFLMemoryAllocationProc		allocProc;		/* Always valid */
	PFLMemoryReallocationProc	reallocProc;	/* Always valid */
	PFLMemoryDeallocationProc	deallocProc;	/* Always valid */
	PGPUserValue				allocUserValue;
};



#endif /* ] Included_pflContextPriv_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

