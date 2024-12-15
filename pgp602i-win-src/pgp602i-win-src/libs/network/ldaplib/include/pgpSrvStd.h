/*  pgpSrvStd.h

	Standard defines, structures, and prototypes for all server products.

	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.

	$Id: pgpSrvStd.h,v 1.2.10.1 1998/11/12 03:17:31 heller Exp $

*/

#ifndef Included_CertServ_pgpSrvStd_h
#define Included_CertServ_pgpSrvStd_h

/* For now, just remap mem allocs for key server stuff. */
#ifdef PGPKEYSERVER

#include <stdlib.h>
#include "pgpBase.h"

/*
#define malloc(size)					pgpMemAlloc(size)
#define realloc(userPtr, newSize)		pgpMemRealloc(userPtr, newSize)
#define free(userPtr)					pgpSrvFree(userPtr)
#define strdup(str)						pgpSrvStrDup(str, __FILE__, __LINE__)
#define calloc(num, size)				pgpSrvCAlloc(num, size, __FILE__, __LINE__)
*/

#define malloc(size)					pgpSrvMalloc(size)
#define realloc(userPtr, newSize)		pgpSrvRealloc(userPtr, newSize)
#define free(userPtr)					pgpSrvFree(userPtr)
#define strdup(str)						pgpSrvStrDup(str)
#define calloc(num, size)				pgpSrvCalloc(num, size)

PGP_BEGIN_C_DECLARATIONS

char *
pgpSrvMalloc(size_t ulSize);

char *
pgpSrvCalloc(size_t ulNum, size_t ulSize);

void
pgpSrvFree(void *pMem);

char *
pgpSrvStrDup(const char *pszStr);

char *
pgpSrvRealloc(char *pszPtr, size_t ulSize);

PGP_END_C_DECLARATIONS

#endif	/* PGPKEYSERVER */
#endif

