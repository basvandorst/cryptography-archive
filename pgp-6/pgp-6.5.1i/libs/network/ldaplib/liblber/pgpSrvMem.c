/*  pgpSrvMem.c

	Standard memory routines for server apps.

	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: pgpSrvMem.c,v 1.5 1999/03/25 17:58:30 melkins Exp $

*/

/*#ifdef PGPKEYSERVER*/
#include <string.h>
#include <stdio.h>

#include "pgpPFLErrors.h"
#include "pgpMemoryMgr.h"
#include "pgpSrvStd.h"
/*#include "pgpSrvGlobals.h"*/

/*extern PGPMemoryMgrRef	g_GlobalMemMgr;*/

/*#define ServerGlobalMemMgr	g_GlobalMemMgr*/

static PGPMemoryMgrRef vGlobalMemMgr = NULL;

void *
pgpSrvMalloc(size_t ulSize)
{
	void	*pszNew;

	if (vGlobalMemMgr == NULL)
		vGlobalMemMgr = PGPGetDefaultMemoryMgr();

	pszNew = PGPNewData(vGlobalMemMgr, ulSize, 0);

	return pszNew;
}

void *
pgpSrvCalloc(size_t ulNum, size_t ulSize)
{
	void	*pszNew;

	if (vGlobalMemMgr == NULL)
		vGlobalMemMgr = PGPGetDefaultMemoryMgr();

	pszNew = PGPNewData(vGlobalMemMgr, ulNum * ulSize,
						kPGPMemoryMgrFlags_Clear);

	return pszNew;
}

char *
pgpSrvStrDup(const char *pszStr)
{
	void	*pszNew;

	if (vGlobalMemMgr == NULL)
		vGlobalMemMgr = PGPGetDefaultMemoryMgr();

	pszNew = PGPNewData(vGlobalMemMgr, strlen(pszStr) + 1, 0);
	if (pszNew != NULL) {
		strcpy(pszNew, pszStr);
	}

	return pszNew;
}

void *
pgpSrvRealloc(void *pszPtr, size_t ulSize)
{	
	PGPError	pgpErr;
	void		*pszNew;

	if (vGlobalMemMgr == NULL)
		vGlobalMemMgr = PGPGetDefaultMemoryMgr();

	pszNew = pszPtr;
	pgpErr = PGPReallocData(vGlobalMemMgr, (void **) &pszNew, ulSize, 0);
	if (IsPGPError(pgpErr)) {
		pszNew = NULL;
	}

	return pszNew;
}

void
pgpSrvFree(void *pMem)
{
	if (pMem != NULL)
		PGPFreeData(pMem);
}

/*#endif*/

