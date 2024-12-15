/*____________________________________________________________________________
	Copyright (C) 1999 Network Associates, Inc.
	All rights reserved.
	
	$Id: testNDIS.h,v 1.4 1999/01/12 19:00:47 dgal Exp $
____________________________________________________________________________*/
#ifndef Included_testNDIS_h	/* [ */
#define Included_testNDIS_h

#ifndef _NDIS_

#include "pgpConfig.h"
#include <string.h>

typedef PGPUInt32	NDIS_PHYSICAL_ADDRESS;
typedef PGPUInt32	NDIS_STATUS;

#define NDIS_PHYSICAL_ADDRESS_CONST(a,b)		a

#ifndef IN
#define IN
#endif

#ifndef PVOID
typedef void *	PVOID;
#endif

#define NdisAllocateMemory(addr, length, flags, max)  *(addr) = malloc(length)

#define NdisFreeMemory(addr, length, flags)		free(addr)

#define NdisZeroMemory(dest, length)			memset(dest, 0, length)

#define NdisMoveMemory(dest, source, length)	memmove(dest, source, length)

#endif /* _NDIS_ */

#endif /* ] Included_testNDIS_h */



/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
