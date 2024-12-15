/*  pgpSrvMem.c

	Standard memory routines for server apps.

	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.

	$Id: pgpSrvMem.c,v 1.2 1997/10/21 01:07:34 marcd Exp $

*/

#ifdef PGPKEYSERVER
#include "pgpSrvStd.h"

char *
pgpSrvStrDup(char *pszStr, char const *pszFile, long lLine)
{
	char	*pszNew;

#if PGP_DEBUG_FIND_LEAKS
	pszNew = (char *) PGP_INTERNAL_MEMALLOC(strlen(pszStr) + 1, pszFile, 
			 lLine);
#else
	pszNew = (char *) PGP_INTERNAL_MEMALLOC(strlen(pszStr) + 1);
#endif
	if (pszNew != NULL) {
		strcpy(pszNew, pszStr);
	}

	return(pszNew);
}

void *
pgpSrvCAlloc(size_t ulNum, size_t ulSize, char const *pszFile, long lLine)
{
	void	*pMem;
	
#if PGP_DEBUG_FIND_LEAKS
	pMem = PGP_INTERNAL_MEMALLOC(ulNum * ulSize, pszFile, lLine);
#else
	pMem = PGP_INTERNAL_MEMALLOC(ulNum * ulSize);
#endif
	if (pMem != NULL) {
		pgpClearMemory(pMem, ulNum * ulSize);
	}
	
	return(pMem);
}

void
pgpSrvFree(void *pMem)
{
	if (pMem != NULL)
		pgpFree(pMem);
}

#endif

