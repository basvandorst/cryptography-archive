/*  pgpSrvStd.h

	Standard defines, structures, and prototypes for all server products.

	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.

	$Id: pgpSrvStd.h,v 1.1.1.1 1997/10/04 19:17:14 marcd Exp $

*/

#ifndef Included_CertServ_pgpSrvStd_h
#define Included_CertServ_pgpSrvStd_h

/* For now, just remap mem allocs for key server stuff. */
#ifdef PGPKEYSERVER

#include "pgpMem.h"

#define malloc(size)					pgpMemAlloc(size)
#define realloc(userPtr, newSize)		pgpMemRealloc(userPtr, newSize)
#define free(userPtr)					pgpSrvFree(userPtr)
#define strdup(str)						pgpSrvStrDup(str, __FILE__, __LINE__)
#define calloc(num, size)				pgpSrvCAlloc(num, size, __FILE__, __LINE__)


PGP_BEGIN_C_DECLARATIONS

void
pgpSrvFree(void *pMem);

char *
pgpSrvStrDup(char *pszStr, char const *pszFile, long lLine);

void *
pgpSrvCAlloc(size_t ulNum, size_t ulSize, char const *pszFile, long lLine);

PGP_END_C_DECLARATIONS

#endif	/* PGPKEYSERVER */
#endif

